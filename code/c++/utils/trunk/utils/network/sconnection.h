#ifndef _SOGOU_CONNECTION_H_
#define _SOGOU_CONNECTION_H_
#include <string>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "connection_manager.h"
//#include "topology.h"
#define INVALID_FD                  (-1)
#define SOCKET_SND_BUF_SIZE         (1024*1024)
#define SOCKET_RCV_BUF_SIZE         (1024*1024)

#define SERVER_SOCKET               0x00000001U
#define MANAGER_SOCKET              0x00000002U
#define SYSTEM_SOCKET               0x00000004U
#define SYSTEM_PIPE                 0x00000008U
#define READABLE_SOCKET             0x00000010U
#define WRITABLE_SOCKET             0x00000020U

#define LISTEN_PORT_1				0x00000001U
#define ERROR_SOCKET				0x00000100U
class Topology;
class VNode;

class CostInfo
{
public:
	CostInfo():cost_sum(0),freq(0),timeout_freq(0){};
	timeval start_time;
	timeval end_time;
	unsigned long cost_sum;
	unsigned long freq;
	unsigned long timeout_freq;
};

class Connection{
friend class HttpClient;
friend class SyncClient;
public:
	Connection(int fd = INVALID_FD, int ref = 0,int flag = 0):_fd(fd),_ref(ref),_flag(flag)
	{
		pthread_mutex_init(&_ref_mutex, NULL);
		pthread_mutex_init(&rw_mutex_, NULL);
	}
	~Connection()
	{
		pthread_mutex_destroy(&_ref_mutex);
		pthread_mutex_destroy(&rw_mutex_);
	}
	int Connect(const std::string& ip,const std::string& port);
  int ConnectTimeout(const std::string& ip,const std::string& port,int time_out);
	int Reconnect();
	int SetSocket(int fd,int flag);
	int Close();
	
	void SetVNode(VNode* node){vnode_ = node;};
	VNode* GetVNode(){return vnode_;};
	//void SetProtocolAdapter(ProtocolAdapter* adapter){adapter_ =adapter ;};
	//ProtocolAdapter* GetProtocolAdapter(){return adapter_;};
	void SetConnectionManager(ConnectionManager* conn_mgr){conn_mgr_ = conn_mgr;};
	ConnectionManager* GetConnectionManager(){return conn_mgr_;};
	
	int WriteTimeout(void *buf, size_t buf_len, timeval *timeout);
	int ReadTimeout(void *buf, size_t buf_len, timeval *timeout);
	int ReadTimeoutOnce( void * buf, int buf_len, timeval * timeout);
	void ClearCostInfo(){
		cost_info_.cost_sum=0;
		cost_info_.freq=0;		
		cost_info_.timeout_freq=0;
	}

	bool StateOk()
	{
		bool ret=true;
		pthread_mutex_lock(&_ref_mutex);
		if (_fd<0)
			ret= false;
		if (_flag& ERROR_SOCKET)
			ret = false;
		pthread_mutex_unlock(&_ref_mutex);	
		return ret;
	}
	
	int GetFd()
	{
		return _fd;
	}
	void SetFd(int fd)
	{
		_fd = fd;
	}
	void LockSelf()
	{
		pthread_mutex_lock(&rw_mutex_);
	}
	void UnLockSelf()
	{
		pthread_mutex_unlock(&rw_mutex_);
	}

	int acquire()
	{		
		pthread_mutex_lock(&_ref_mutex);
		int ret = ++_ref;
		pthread_mutex_unlock(&_ref_mutex);
		//fprintf(stderr,"acquire connection ref = %d,fd = %d\n",ret,_fd);
		return ret;
	}
	int release()
	{
		pthread_mutex_lock(&_ref_mutex);
		int ret = --_ref;
		pthread_mutex_unlock(&_ref_mutex);
		//fprintf(stderr,"release connection ref = %d,fd = %d\n",ret,_fd);
		return ret;
	}
	int ref()
	{
		pthread_mutex_lock(&_ref_mutex);
		int ret = _ref;
		pthread_mutex_unlock(&_ref_mutex);
		return ret;
	}
	int GetFlag()
	{
		pthread_mutex_lock(&_ref_mutex);
		int ret=_flag;
		pthread_mutex_unlock(&_ref_mutex);
		return ret;
	}
	int SetFlag(int flag)
	{
		pthread_mutex_lock(&_ref_mutex);
		_flag = flag;
		pthread_mutex_unlock(&_ref_mutex);	
		return 0;
	}
	
  void* private_data;
protected:
	CostInfo cost_info_;
	int _fd;
	int _ref;
	int _flag;
	pthread_mutex_t _ref_mutex;
	pthread_mutex_t rw_mutex_;
	VNode* vnode_;
	ConnectionManager* conn_mgr_;

	//ProtocolAdapter* adapter_;
};



#endif
