#ifndef _CONNPOOL_H_
#define _CONNPOOL_H_

#include <sys/types.h>
#include <sys/socket.h>

typedef struct __connpool connpool_t;

struct connpool_conn
{
	int sockfd;
};

#ifdef __cplusplus
extern "C"
{
#endif

connpool_t *connpool_create(size_t conn_max);
struct connpool_conn *connpool_getconn(const struct sockaddr *sockaddr,
									   size_t addrlen, unsigned int timeout,
									   connpool_t *connpool);
void connpool_freeconn(struct connpool_conn *conn, unsigned int keepalive,
					   connpool_t *connpool);
void connpool_destroy(connpool_t *connpool);

#ifdef __cplusplus
}
#endif

#endif

