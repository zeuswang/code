#ifndef _SOGOU_CONNECTION_SERVER_H_
#define _SOGOU_CONNECTION_SERVER_H_


#include <utils/stask_base.h>
#include "sconnection.h"
#include "connection_manager.h"
//#include "socket_handle.hpp"
#include <sys/time.h>
#include <sys/epoll.h>
#include <map>
#include <string>
#include <utils/implement_base.h>

#define LISTEN_PORT_1				0x00000001U

#define MAX_EPOLL_EVENT_NUM         256
#define MAX_FD_NUM                  1024
#define HTTP_RECEIVE_TIMEOUT				50
#define HTTP_REPLY_TIMEOUT				50
#define SOCKET_SND_BUF_SIZE         (1024*1024)
#define SOCKET_RCV_BUF_SIZE         (1024*1024)
#define READ_BUF_SIZE       512
#define HTTP_HEADER_MAX_LENGTH  1024

class ConnectionServer:public STaskBase<Connection*>,ConnectionManager
{
	public:
		ConnectionServer(ImplementBase* impl);
		virtual ~ConnectionServer();

		int Open(size_t thread_num,int port);
		int Close();
		
		//virtual int stop();
		virtual int Svc();
		virtual void ErrorHandle(class Connection* conn);

	protected:
		int create_pipe();
		int create_listen(int &socket_fd, unsigned short port);
		int set_socket(int fd, int flag);
		int add_input_fd(int fd);
		int del_input_fd(int fd);

		int CreateConnection(int fd,int flag);
		int ReleaseConnection(Connection* conn);
		Connection* GetConnection(int fd);

	protected:
		ImplementBase* impl_;
		int _epoll_fd;	
		int _pipe_read;
		int _pipe_write;
		int _socket_server_listen;
		int _epoll_ready_event_num;
		pthread_mutex_t _epoll_mutex;
		pthread_mutex_t _map_mutex;
		int _stop_task;
		char current_ip[1024];
		int _now_connections;
		pthread_mutex_t _connections_mutex;
		epoll_event _epoll_ready_event[MAX_EPOLL_EVENT_NUM];

		std::map<int, Connection *> m_fd_set;	
};



#endif 
