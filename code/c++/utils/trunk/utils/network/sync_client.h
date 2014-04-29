#ifndef _SYNC_CLIENT_H_
#define _SYNC_CLIENT_H_
#include <utils/stask_base.h>
#include "sconnection.h"
#include "topology.h"
#include <utils/stimer.h>
class SyncClient
{
public:
	SyncClient(){
		check_interval_=10;
		stop_=0;
		//pthread_mutex_init(&mutex_,NULL);	

	}
	virtual ~SyncClient(){
		//pthread_mutex_destroy(&mutex_);
	}

	class ClientCheck:public STaskBase<int>
	{
	public:
		void Init(SyncClient* client){
			client_=client;
		}
		int Svc(){client_->CheckRoutine();};
	protected:
		SyncClient* client_;
	};

	/*
	Open:init SyncClient 
	param:
		conf:config of topology
		t:timeout cost for reqeust
	return:
		0:success
		-1:faild
	*/	
	virtual int Open(const std::string& conf,int t);
	virtual int Close(){stop_=1;};

	virtual void CheckConnection(Connection*){};
	int SetCheckInterval(int interval){check_interval_=interval;};
	int CheckRoutine();
	/*
	TimeoutHandle:check if the connection is busy or unusable.if connection is busy,
	set the connection's vnode to busy,and active a timer.some time later,when the timer
	event trigger,set the vonde to unbusy and try this connection agagin.
	param:
		conn:
	return:
	*/	
	virtual void TimeoutHandle(Connection* conn);

	/*
	ErrorHandle:when the timeout happen or connection error,call this method
	param:
		conn:
	return:
	*/	
	virtual void ErrorHandle(Connection* conn){};

	/*
	GetConnection:get a connection
	param:
		key: the hash key
	return:
		if none of connection is usable,return NULL ,else return a (Connecion*). 
	*/	
	virtual Connection* GetConnection(const std::string& key);
  virtual Connection* GetConnection(void* key,int len);
	/*
	ReleaseConnection:if you get a Connecion*(not NULL) by call GetConnection ,you should
	call ReleaseConnection at last
	param:
		conn: Connection*
	return:
	*/	
	virtual void ReleaseConnection(Connection* conn);

protected:
	void CheckTopology(Topology*);
	Topology*  topology_;
	ClientCheck check_;
	int	check_interval_;
	int timeout_;
	int stop_;
	STimer timer_;
	//pthread_mutex_t mutex_;
};
#endif 
