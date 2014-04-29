#include "connection_server.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <utils/debug.h>
ConnectionServer::ConnectionServer(ImplementBase * impl)
{
  _epoll_fd = INVALID_FD;
  _pipe_read = INVALID_FD;
  _pipe_write = INVALID_FD;
  _socket_server_listen = INVALID_FD;
  _stop_task = 0;
  _epoll_ready_event_num = 0;
  _now_connections = 0;

  pthread_mutex_init(&_epoll_mutex, NULL);
  //pthread_mutex_init(&_map_mutex,NULL);
  //pthread_mutex_init(&_connections_mutex,NULL);
  impl_=impl;
}

ConnectionServer::~ConnectionServer()
{
  ::close(_epoll_fd);
  ::close(_pipe_read);
  ::close(_pipe_write);
  ::close(_socket_server_listen);
  pthread_mutex_destroy(&_epoll_mutex);
  //pthread_mutex_destroy(&_map_mutex);
  //pthread_mutex_destroy(&_connections_mutex);

  /*for (std::map<int, Connection*>::const_iterator it = m_fd_set.begin(); it != m_fd_set.end(); ++it)
  {
    ::close(it->second->GetFd());
    delete it->second;
  }*/
}



int ConnectionServer::Open(size_t thread_num, int port)
{
	struct sockaddr_in addr; 
	struct hostent *host;     
	struct ifreq req; 
	int sock; 
	char *dn_or_ip; 

	sock = socket(AF_INET, SOCK_DGRAM, 0); 
	strncpy(req.ifr_name, "eth0", IFNAMSIZ); 

	if ( ioctl(sock, SIOCGIFADDR, &req) < 0 ) { 
		_err("ConnectionServer::Open ioctl error: %s Cann't get current IP\n", strerror (errno)); 
		sprintf(current_ip,"Get currentip error %d",errno); 
	} 
	else 
	{ 
		dn_or_ip = (char *)inet_ntoa(*(struct in_addr *) &((struct sockaddr_in *) &req.ifr_addr)->sin_addr); 
		strcpy(current_ip,dn_or_ip); 
	} 
	_info("ConnectionServer::Open Current IP:%s\n", current_ip); 
	shutdown(sock, 2); 
	::close(sock);

	if ((_epoll_fd = epoll_create(MAX_FD_NUM)) == -1)
	{
		_err("error: epoll create fail!!\n");
		return -1;
	}
	if (create_pipe())
	{
		_err("error: create pipe fail!! \n");
		return -1;
	}
	if (create_listen(_socket_server_listen, port))
	{
		_err("error: create listen fail !! %d\n",port);
		return -1;
	}

	add_input_fd(_pipe_read);
	add_input_fd(_socket_server_listen);

	return STaskBase<Connection*>::Open(thread_num);
}

int ConnectionServer::Close()
{
	_stop_task = 1;
	write(_pipe_write, &_stop_task, 1);
	write(_pipe_write, &_stop_task, 1);
	STaskBase<Connection*>::Close();
	return 0;
}

int ConnectionServer::CreateConnection(int fd,int flag)
{

	/*
	SocketHandle * newhdle = new SocketHandle;
	newhdle->set_flag(flag);
	newhdle->set_fd(fd);
	insert_handle(fd,newhdle);
	*/
	Connection* conn = new Connection(fd);
	conn->SetConnectionManager(this);
	conn->SetFlag(flag);
	pthread_mutex_lock(&_map_mutex);
	std::pair< std::map<int, Connection*>::iterator, bool> ret = 
	m_fd_set.insert(std::map<int, Connection*>::value_type(fd, conn));
	pthread_mutex_unlock(&_map_mutex);
	if(!ret.second)
	{
		_err("ConnectionServer::CreateConnection <btlswkxt> insert map error!!\n");
		return -1;
	}
	
	return 0;

}
int ConnectionServer::ReleaseConnection(Connection* conn)
{
	pthread_mutex_lock(&_map_mutex);
	_info("ConnectionServer::ReleaseConnection coming\n");
	if(conn->release() == 0 && (conn->GetFlag() & ERROR_SOCKET))
	//if(conn->release() == 0)
	{
		int fd = conn->GetFd();
        	del_input_fd(fd);
		int ret=::close(fd);
		delete conn;
		conn = NULL;
		m_fd_set.erase(fd);
		pthread_mutex_lock(&_connections_mutex);
		_now_connections--;
		pthread_mutex_unlock(&_connections_mutex);
		_info("ConnectionServer::ReleaseConnection close connection (%d) ret=%d\n",fd,ret);
	}	
	pthread_mutex_unlock(&_map_mutex);
	return 0;
}

void ConnectionServer::ErrorHandle(class Connection* conn)
{
	if (conn)
		ReleaseConnection(conn);
};
Connection* ConnectionServer::GetConnection(int fd)
{	
	pthread_mutex_lock(&_map_mutex);
	std::map<int, Connection*>::iterator it = m_fd_set.find(fd);
	if (it != m_fd_set.end()) 
	{
		it->second->acquire();
		pthread_mutex_unlock(&_map_mutex);
		return it->second;
	}
	pthread_mutex_unlock(&_map_mutex);
	_info("ConnectionServer::GetConnection <btlswkxt> do not get connection !!\n");
	return NULL;
}


int ConnectionServer::Svc()
{
	int fd,new_fd;

	_info("ConnectionServer::Svc() begin !\n");
	while (!_stop_task)
	{
		pthread_mutex_lock(&_epoll_mutex);
		if (_stop_task) 
		{
			pthread_mutex_unlock(&_epoll_mutex);
			
			_debug("ConnectionServer::Svc() get exit pipe signal !\n");
			break;
		}

		if (_epoll_ready_event_num <= 0){
			//fprintf(stderr,"ConnectionServer::Svc() #### wait !\n");
			_epoll_ready_event_num = epoll_wait(_epoll_fd, _epoll_ready_event, MAX_EPOLL_EVENT_NUM, -1);
		}
		if (_epoll_ready_event_num-- < 0)
		{
			pthread_mutex_unlock(&_epoll_mutex);
			if (errno == EINTR){
				
				_debug("ConnectionServer::Svc() errno == EINTR !\n");
				continue;
			}else{
				//_debug("ConnectionServer::Svc() errno == EINTR else !\n");
				break;
			}
		}

		//fprintf(stderr,"ConnectionServer::Svc() #### 1 !\n");
		fd = _epoll_ready_event[_epoll_ready_event_num].data.fd;
		if (fd == _socket_server_listen)
		{
			while ((new_fd = accept(fd, NULL, NULL)) >= 0)
			{
                                pthread_mutex_lock(&_connections_mutex);
				if(_now_connections>=MAX_FD_NUM)
				{
					_debug("new connection error,because the max connection(%d) is achieved\n",MAX_FD_NUM);
					::close(new_fd);
					//pthread_mutex_unlock(&_connections_mutex);
				}
				else if(new_fd >= MAX_FD_NUM) {
					_debug("new connection error,because the max fd(%d) is achieved\n",new_fd);
					::close(new_fd);
					pthread_mutex_unlock(&_connections_mutex);
				}
				else
				{                                                       
                                	_now_connections++;
					pthread_mutex_unlock(&_connections_mutex);
					set_socket(new_fd, O_NONBLOCK);
					//create_handle(new_fd,LISTEN_PORT_1);
					CreateConnection(new_fd,READABLE_SOCKET);
					add_input_fd(new_fd);
					//WEBSEARCH_DEBUG((LM_DEBUG,"[new connection on socket(%d)]\n", new_fd));
                  		}
			}
			pthread_mutex_unlock(&_epoll_mutex);
			//fprintf(stderr,"ConnectionServer::Svc() fd == _socket_server_listen continue !\n");
			continue;
		}
		if (fd == _pipe_read)
		{
			pthread_mutex_unlock(&_epoll_mutex);
			continue;
		}

		del_input_fd(fd);
		pthread_mutex_unlock(&_epoll_mutex);
		
		Connection* conn = GetConnection(fd);
		

		if (conn && impl_ && !(impl_->Run((void*)conn)<0)){
			add_input_fd(fd);
		}else {
			if (conn){
          _err("Connection error!!\n");
				ReleaseConnection(conn);
			}
		}
			
	}
	_info("ConnectionServer::Svc() exit !\n");
	return 0;
}

int ConnectionServer::create_pipe()
{
	int options;
	int pipe_fd[2];

	if (pipe(pipe_fd))
		return -1;

	_pipe_read = pipe_fd[0];
	_pipe_write = pipe_fd[1];

	for (int i=0; i<2; i++)
	{
		if ((options = fcntl(pipe_fd[i], F_GETFL)) == -1)
			return -1;
		if (fcntl(pipe_fd[i], F_SETFL, options | O_NONBLOCK) == -1)
			return -1;
	}

	return 0;
}

int ConnectionServer::create_listen(int &socket_fd, unsigned short port)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		return -1;
	if (set_socket(socket_fd, O_NONBLOCK))
		return -1;
	if (bind(socket_fd, (const sockaddr*)&addr, sizeof addr))
		return -1;
	if (listen(socket_fd, MAX_FD_NUM))
		return -1;

	return 0;
}

int ConnectionServer::set_socket(int fd, int flag)
{
	int options;
	options = SOCKET_SND_BUF_SIZE;
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &options, sizeof(int));
	options = SOCKET_RCV_BUF_SIZE;
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &options, sizeof(int));
	options = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(int));
	options = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, options | flag);
	int on = 1;
	int ret = -1;
	ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
	return ret;
}

int ConnectionServer::add_input_fd(int fd)
{
	epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &event);

	return 0;
}

int ConnectionServer::del_input_fd(int fd)
{
	epoll_event event;
	event.events = EPOLLIN | EPOLLET;
	event.data.fd = fd;
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, &event);

	return 0;
}










