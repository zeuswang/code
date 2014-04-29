#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <list.h>
#include <rbtree.h>
#include "connpool.h"

struct __srvinfo
{
	struct rb_node rb;
	struct list_head idle_list;
	size_t conn_count;
	pthread_cond_t cond;
	size_t addrlen;
	struct sockaddr sockaddr;
};

struct __conninfo
{
	int sockfd;
	time_t valid_time;
	struct list_head list;
	struct rb_node rb;
	struct __srvinfo *srv;
};

struct __connpool
{
	struct rb_root srv_root;
	struct rb_root idle_root;
	pthread_mutex_t mutex;
	size_t conn_max;
	pthread_t tid;
	int terminate;
};

static void *__connpool_cleanup(void *context)
{
	connpool_t *connpool = (connpool_t *)context;
	struct __conninfo *conn;
	struct rb_node *cur;
	time_t cur_time;

	while (!connpool->terminate)
	{
		pthread_mutex_lock(&connpool->mutex);
		cur_time = time(NULL);
		cur = rb_first(&connpool->idle_root);
		while (cur)
		{
			conn = rb_entry(cur, struct __conninfo, rb);
			if (conn->valid_time <= cur_time)
			{
				struct rb_node *next = rb_next(cur);
				rb_erase(cur, &connpool->idle_root);
				list_del(&conn->list);
				close(conn->sockfd);
				conn->srv->conn_count--;
				free(conn);
				cur = next;
			}
			else
				break;
		}

		pthread_mutex_unlock(&connpool->mutex);
		sleep(1);
	}

	return NULL;
}

static int __connpool_init(connpool_t *connpool)
{
	int ret;

	ret = pthread_mutex_init(&connpool->mutex, NULL);
	if (ret == 0)
	{
		connpool->srv_root.rb_node = NULL;
		connpool->idle_root.rb_node = NULL;
		return 0;
	}

	errno = ret;
	return -1;
}

static void __connpool_destroy(connpool_t *connpool)
{
	struct list_head *pos, *tmp;
	struct __conninfo *conn;
	struct __srvinfo *srv;

	pthread_mutex_destroy(&connpool->mutex);
	while (connpool->srv_root.rb_node)
	{
		srv = rb_entry(connpool->srv_root.rb_node, struct __srvinfo, rb);
		rb_erase(&srv->rb, &connpool->srv_root);
		list_for_each_safe(pos, tmp, &srv->idle_list)
		{
			conn = list_entry(pos, struct __conninfo, list);
			list_del(pos);
			close(conn->sockfd);
			free(conn);
		}

		pthread_cond_destroy(&srv->cond);
		free(srv);
	}
}

void connpool_destroy(connpool_t *connpool)
{
	void *ret;
	connpool->terminate = 1;
	pthread_join(connpool->tid, &ret);
	__connpool_destroy(connpool);
	free(connpool);
}

connpool_t *connpool_create(size_t conn_max)
{
	connpool_t *connpool = (connpool_t *)malloc(sizeof (connpool_t));
	pthread_t tid;
	int ret;

	if (connpool)
	{
		if (__connpool_init(connpool) >= 0)
		{
			connpool->conn_max = conn_max;
			connpool->terminate = 0;
			ret = pthread_create(&tid, NULL, __connpool_cleanup, connpool);
			if (ret == 0)
			{
				connpool->tid = tid;
				return connpool;
			}

			errno = ret;
			__connpool_destroy(connpool);
		}

		free(connpool);
	}

	return NULL;
}

static struct __srvinfo *__connpool_insert_srv(
							const struct sockaddr *sockaddr, size_t addrlen,
							struct rb_node **link, struct rb_node *parent,
							connpool_t *connpool)
{
	size_t size = offsetof(struct __srvinfo, sockaddr) + addrlen;
	struct __srvinfo *srv = (struct __srvinfo *)malloc(size);
	int ret;

	if (srv)
	{
		ret = pthread_cond_init(&srv->cond, NULL);
		if (ret == 0)
		{
			INIT_LIST_HEAD(&srv->idle_list);
			srv->conn_count = 0;
			memcpy(&srv->sockaddr, sockaddr, addrlen);
			srv->addrlen = addrlen;
			rb_link_node(&srv->rb, parent, link);
			rb_insert_color(&srv->rb, &connpool->srv_root);
			return srv;
		}

		errno = ret;
		free(srv);
	}

	return NULL;
}

static int __set_fd_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL);

	if (flags >= 0)
		flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	return flags;
}

static int __connect_timeout(int sockfd, const struct sockaddr *sockaddr,
							 size_t addrlen, int timeout/*milliseconds*/)
{
	struct pollfd pollfd = { sockfd, POLLIN | POLLOUT, };
	int ret;

	if (__set_fd_nonblock(sockfd) < 0)
		return -1;

	if (connect(sockfd, sockaddr, addrlen) >= 0)
		return 0;
	else if (errno != EINPROGRESS)
		return -1;

	ret = poll(&pollfd, 1, timeout);
	if (ret > 0)
	{
		socklen_t optlen = sizeof (int);
		int error;

		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &optlen) >= 0)
		{
			if (error == 0)
				return 0;

			errno = error;
		}
	}
	else if (ret == 0)
		errno = ETIMEDOUT;

	return -1;
}

static int __connpool_connect(struct __srvinfo *srv, unsigned int timeout,
							  connpool_t *connpool)
{
	int sockfd = socket(srv->sockaddr.sa_family, SOCK_STREAM, 0);

	if (sockfd >= 0)
	{
		int mtimeout = timeout > 1024 ? -1 : 1000 * timeout;

		if (__connect_timeout(sockfd, &srv->sockaddr, srv->addrlen,
							  mtimeout) >= 0)
			return sockfd;

		close(sockfd);
	}

	return -1;
}

static struct __conninfo *__connpool_new_conn(struct __srvinfo *srv,
											  unsigned int timeout,
											  connpool_t *connpool)
{
	struct __conninfo *conn;

	conn = (struct __conninfo *)malloc(sizeof (struct __conninfo));
	if (conn)
	{
		conn->sockfd = __connpool_connect(srv, timeout, connpool);
		if (conn->sockfd >= 0)
		{
			conn->srv = srv;
			return conn;
		}

		free(conn);
	}

	return NULL;
}

static struct __conninfo *__connpool_get_conn(struct __srvinfo *srv,
											  unsigned int timeout,
											  connpool_t *connpool)
{
	struct __conninfo *conn;
	struct timespec abstime;
	struct timeval now;
	int ret;

	gettimeofday(&now, NULL);
	abstime.tv_sec = now.tv_sec + timeout;
	abstime.tv_nsec = 1000 * now.tv_usec;
	while (1)
	{
		if (!list_empty(&srv->idle_list))
		{
			conn = list_entry(srv->idle_list.next, struct __conninfo, list);
			rb_erase(&conn->rb, &connpool->idle_root);
			list_del(&conn->list);
			return conn;
		}

		if (connpool->conn_max == 0 || srv->conn_count < connpool->conn_max)
			break;

		ret = pthread_cond_timedwait(&srv->cond, &connpool->mutex, &abstime);
		if (ret != 0)
		{
			errno = ret;
			return NULL;
		}
	}

	srv->conn_count++;
	pthread_mutex_unlock(&connpool->mutex);
	conn = __connpool_new_conn(srv, timeout, connpool);
	pthread_mutex_lock(&connpool->mutex);
	if (!conn)
	{
		srv->conn_count--;
		pthread_cond_signal(&srv->cond);
	}

	return conn;
}

struct connpool_conn *connpool_getconn(const struct sockaddr *sockaddr,
									   size_t addrlen, unsigned int timeout,
									   connpool_t *connpool)
{
	struct rb_node **p = &connpool->srv_root.rb_node;
	struct rb_node *parent = NULL;
	struct __conninfo *conn;
	struct __srvinfo *srv;
	int n;

	pthread_mutex_lock(&connpool->mutex);
	while (*p)
	{
		parent = *p;
		srv = rb_entry(*p, struct __srvinfo, rb);
		if (addrlen < srv->addrlen)
			p = &(*p)->rb_left;
		else if (addrlen > srv->addrlen)
			p = &(*p)->rb_right;
		else if ((n = memcmp(sockaddr, &srv->sockaddr, addrlen)) < 0)
			p = &(*p)->rb_left;
		else if (n > 0)
			p = &(*p)->rb_right;
		else
			break;
	}

	if (!*p)
		srv = __connpool_insert_srv(sockaddr, addrlen, p, parent, connpool);

	if (srv)
		conn = __connpool_get_conn(srv, timeout, connpool);
	else
		conn = NULL;

	pthread_mutex_unlock(&connpool->mutex);
	return (struct connpool_conn *)conn;
}

static void __connpool_add_idle(struct __conninfo *conn,
								unsigned int keepalive,
								connpool_t *connpool)
{
	struct rb_node **p = &connpool->idle_root.rb_node;
	struct rb_node *parent = NULL;
	struct __conninfo *entry;

	conn->valid_time = time(NULL) + keepalive;
	while (*p)
	{
		parent = *p;
		entry = rb_entry(*p, struct __conninfo, rb);
		if (conn->valid_time < entry->valid_time)
			p = &(*p)->rb_left;
		else
			p = &(*p)->rb_right;
	}

	rb_link_node(&conn->rb, parent, p);
	rb_insert_color(&conn->rb, &connpool->idle_root);
}

void connpool_freeconn(struct connpool_conn *conn, unsigned int keepalive,
					   connpool_t *connpool)
{
	struct __conninfo *conninfo = (struct __conninfo *)conn;
	struct __srvinfo *srv = conninfo->srv;

	pthread_mutex_lock(&connpool->mutex);
	if (keepalive == 0)
	{
		close(conninfo->sockfd);
		srv->conn_count--;
		free(conninfo);
	}
	else
	{
		list_add_tail(&conninfo->list, &srv->idle_list);
		__connpool_add_idle(conninfo, keepalive, connpool);
	}

	pthread_cond_signal(&srv->cond);
	pthread_mutex_unlock(&connpool->mutex);
}

