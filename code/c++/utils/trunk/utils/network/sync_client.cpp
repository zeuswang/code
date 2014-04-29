#include "sync_client.h"
#include <utils/debug.h>

static void timeout_func(void * arg)
{
	Connection *conn=(Connection* )arg;
	if (!conn)
		return ;
	if (conn->GetVNode()->Busy()){
		conn->GetVNode()->SetUnBusy();
		_info("set the vnode(host:%s,port:%s) unbusy\n",
			conn->GetVNode()->GetAddress().c_str(),
			conn->GetVNode()->GetPort().c_str());
	}
}
int SyncClient::Open(const std::string& conf,int t)
{
	int ret=0;
	// load topology configure
	topology_ = NULL;
	TopologyFactory factory;
	topology_ = factory.GetNewTopology(conf);

	if (!topology_)
		return -1;
	//start the timer
	timer_.SetTimeout(10000);
	timer_.Start();

	timeout_=t;
	
	//start check routine
	/*
	check_.Init(this);
	check_.Open(1);
	check_.Activate();
	*/
	return 0;
}

void SyncClient::CheckTopology(Topology* tpy)
{
	TopologyType type = tpy->GetType();
	
	if (type == T_Node){
		std::vector<Connection*>& clist = tpy->GetVNode()->GetList();
		for (size_t i=0;i<clist.size();i++){
			CheckConnection(clist[i]);	
		}
	}else{
		std::vector<Topology*>& list = tpy->GetList();
		for (size_t i=0;i<list.size();i++){
			CheckTopology(list[i]);
		}
	}
}

int SyncClient::CheckRoutine()
{
	stop_= 0;
	timeval timeout;
	while (!stop_)
	{
		timeout.tv_sec = check_interval_;
		timeout.tv_usec = 0;
		if (select(1, NULL, NULL, NULL, &timeout)) continue;
		CheckTopology(topology_);
	}
	return 0;
}

void SyncClient::TimeoutHandle(Connection* conn)
{
  //if ((conn->cost_info_.freq>10)&&(conn->cost_info_.timeout_freq>8)){
  if (conn->cost_info_.timeout_freq>8){
    conn->cost_info_.timeout_freq=0;
    if (!conn->GetVNode()->Busy()){
      _info("set the vnode(host:%s,port:%s) busy\n",conn->GetVNode()->GetAddress().c_str(),
                                                    conn->GetVNode()->GetPort().c_str());

      conn->GetVNode()->SetBusy();
      conn->GetVNode()->LockSelf();
        std::vector<Connection*>& conn_list =conn->GetVNode()->GetList();
        for (int i=0;i<conn_list.size();i++){
          conn_list[i]->ClearCostInfo();
        }
      conn->GetVNode()->UnLockSelf();
      timer_.Add(timeout_func,(void *)conn);
    }
  }	
}
Connection* SyncClient::GetConnection(const std::string& key)
{
//	pthread_mutex_lock(&mutex_);

	if (!topology_)
		return NULL;

	NodeTopology* t = topology_->Select(key);
	if (!t) return NULL;

  if (t->GetVNode()->Busy()){
    return NULL;
  }

	t->GetVNode()->LockSelf();
	
		Connection* conn =t->GetVNode()->SelectConnection(key);
		if (conn){
			conn->acquire();
			gettimeofday(&conn->cost_info_.start_time,NULL);
		}
		
	t->GetVNode()->UnLockSelf();
	//fprintf(stderr,"ConnectionServer::GetConnection <btlswkxt> do not get connection !!\n");
	return conn;
}
Connection* SyncClient::GetConnection(const std::string&key,int len)
{
//	pthread_mutex_lock(&mutex_);

	if (!topology_)
		return NULL;

	NodeTopology* t = topology_->Select(key,len);
	if (!t) return NULL;

  if (t->GetVNode()->Busy()){
    return NULL;
  }

	t->GetVNode()->LockSelf();
	
		Connection* conn =t->GetVNode()->SelectConnection(key);
		if (conn){
			conn->acquire();
			gettimeofday(&conn->cost_info_.start_time,NULL);
		}
		
	t->GetVNode()->UnLockSelf();
	//fprintf(stderr,"ConnectionServer::GetConnection <btlswkxt> do not get connection !!\n");
	return conn;
}

void SyncClient::ReleaseConnection(Connection* conn)
{
	if (conn){
		//conn->LockSelf();
		gettimeofday(&conn->cost_info_.end_time,NULL);	
		int cost = (conn->cost_info_.end_time.tv_sec - conn->cost_info_.start_time.tv_sec)*1000+
						(conn->cost_info_.end_time.tv_usec - conn->cost_info_.start_time.tv_usec)/1000;
		conn->cost_info_.cost_sum +=cost;

		conn->cost_info_.freq++;	
		if ((cost > timeout_)||(conn->GetFlag()&ERROR_SOCKET)){
			conn->cost_info_.timeout_freq++;
			ErrorHandle(conn);
		}else {
      if (conn->cost_info_.timeout_freq >0)
        			conn->cost_info_.timeout_freq--;
		}
		
		TimeoutHandle(conn);	
		conn->release();
		//conn->UnLockSelf();
	}
}

