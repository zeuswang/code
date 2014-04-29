#ifndef _SOGOU_CONNECT_CLIENT_H_
#define _SOGOU_CONNECT_CLIENT_H_

#include <sys/epoll.h>
#include <iconv.h>
#include <map>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <utils/stask_base.h>
#include "sconnection.h"
#include <utils/implement_base.h>
#include "topology.h"

#define MAX_FD_NUM                  1024
#ifndef INVALID_FD
#define INVALID_FD                  (-1)
#endif
#define MAX_EPOLL_EVENT_NUM         256
class ConnectionClient;
class tcp_check:public STaskBase<int>
{
public:
	tcp_check(ConnectionClient* handler){_tcp_handler=handler;};
	virtual ~tcp_check(){};
     	int Svc();
	int Close(){_stop_task = 1;STaskBase<int>::Close();};
private:
	ConnectionClient* _tcp_handler;
	int _stop_task;
};


class ConnectionClient:public STaskBase<Connection*>,public ConnectionManager
{
public:
	ConnectionClient(ImplementBase* base);
	virtual ~ConnectionClient();
	
	virtual int Open(const std::string& conf,int receive_thread);
	virtual int Close();
	virtual int Svc();
	
	int LoadConf(const std::string& conf);
	Connection * SelectConnection(const std::string& key);
	void ErrorHandle(Connection* conn);
	int CheckRoutine();

protected:
	void check_connect_inter(int time){m_CheckInter = time;};
	void CheckConnection(Topology* tpy);
	Connection* GetConnection(int fd);
	
	int create_pipe();
	//int set_socket(int fd, int flag = 0);
	int add_input_fd(int fd);
	int del_input_fd(int fd);
	int add_input_fd_record(int fd);
	int update_fd();

protected:
	int _epoll_fd;
	int _epoll_ready_event_num;
	epoll_event _epoll_ready_event[MAX_EPOLL_EVENT_NUM];
	pthread_mutex_t _epoll_mutex;
	pthread_mutex_t _fds_update_mutex;
	pthread_mutex_t _err_connect_mutex;
	//pthread_mutex_t _segmet_mutex;

	int _pipe_read;
	int _pipe_write;
	int _update_record_num;
	struct
	{
		int op;
		int fd;
		epoll_event event;
	} _update_record[MAX_FD_NUM * 2];

	int _stop_task;
	tcp_check* _check;
	ImplementBase* impl_base_;
	std::map< int, Connection*> connect_set;
	Topology* topology_;
	int max_fd_num_;
	int m_CheckInter;
};

#endif
