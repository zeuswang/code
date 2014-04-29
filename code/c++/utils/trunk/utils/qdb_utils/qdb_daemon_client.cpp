#include "qdb_daemon_client.h"

void QdbDaemonClient::ErrorHandle(Connection * conn)
{
	_info("QdbDaemonClient met a error,refresh the qdb connection\n");
  /*conn->LockSelf();
  if (!ConnectionOk(conn)){
    conn->UnLockSelf();
    return -1;
  }
	OffDB* db= (OffDB* )conn->private_data;
	struct timeval db_timeout = {0,50000};
	db->reconnect(&db_timeout,1,false);
	conn->SetFlag(conn->GetFlag()&~ERROR_SOCKET);
  conn->UnLockSelf();
  */
  conn->LockSelf();
  pthread_mutex_lock(&mutex_);
  int ret= conn_list_.erase(conn);
  if (ret>0){
    OffDB* db= (OffDB* )conn->private_data;
    if (db)
      db->close();
  }
  pthread_mutex_unlock(&mutex_);
  conn->SetFlag(conn->GetFlag()&~ERROR_SOCKET);
  conn->UnLockSelf();

}
/*
int QdbDaemonClient::Set(const std::string& key,const std::string& value)
{

	Connection * conn = GetConnection(key);
	OffDB* db= (OffDB* )conn->private_data;
	//std::string addr = node_topology->GetVNode()->GetAddress()+node_topology->GetVNode()->Getport();
	int ret= WriteQdb(db,key,value);
	if (ret<0)
		conn->SetFlag(ERROR_SOCKET);
	ReleaseConnection(conn);
	return ret;

}
int QdbDaemonClient::Get(const std::string& key,std::string& value)
{
	Connection * conn = GetConnection(key);
	OffDB* db= (OffDB* )conn->private_data;
	//std::string addr = node_topology->GetVNode()->GetAddress()+node_topology->GetVNode()->Getport();
	int ret= ReadQdb(db,key,value);
	if (ret<0)
		conn->SetFlag(ERROR_SOCKET);
	ReleaseConnection(conn);
	return ret;
}
*/

int QdbDaemonClient::SetContent(void* key,int length,const std::string& str,const std::string& circle)
{
  int ret=0;
	Connection * conn = GetConnection(circle);
	if (!conn)
		return -1;
  
  conn->LockSelf();
  if (!ConnectionOk(conn)){
    ret=-1;
    goto OUT;
  }
  {
	OffDB* db= (OffDB* )conn->private_data;
	timeval t1,t2;
  OffPage page(ACE_Allocator::instance());
  page.addContent("data",(void*)str.c_str(),str.length());
  page.addAttributes("writeTime",sogou_utils::GetUnixTimeStr());
	gettimeofday(&t1,NULL);
	//std::string addr = node_topology->GetVNode()->GetAddress()+node_topology->GetVNode()->Getport();
	ret= WriteQdb(db,key,length,page);  
	gettimeofday(&t2,NULL);
	int cost = (t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);

	_info("SetPage(%s,%s) cost time:%d\n",conn->GetVNode()->GetAddress().c_str(),
										conn->GetVNode()->GetPort().c_str(),
										cost);
	if (ret<0)
		conn->SetFlag(ERROR_SOCKET);
  }
OUT:
  conn->UnLockSelf();
	ReleaseConnection(conn);
	return ret;
}


int QdbDaemonClient::GetContent(void* key,int length,std::string& str,const std::string& circle)
{
  int ret=0;
	Connection * conn = GetConnection(circle);
	if (!conn)
		return -1;
  
  conn->LockSelf();
  if (!ConnectionOk(conn)){
    ret=-1;
    goto OUT;
  }
  { 
	OffDB* db= (OffDB* )conn->private_data;
	timeval t1,t2;
  OffPage page(ACE_Allocator::instance());
	gettimeofday(&t1,NULL);
	//std::string addr = node_topology->GetVNode()->GetAddress()+node_topology->GetVNode()->Getport();
	ret= ReadQdb(db,key,length,page);
	gettimeofday(&t2,NULL);
	int cost = (t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);

  {//get content from OffPage
    void* valuePos;
    size_t valueLen;
    if (!(page.getContent("data",(void *&)valuePos,valueLen))){
      str.assign((char*)valuePos,valueLen);
      if (valuePos){
        ACE_Allocator::instance()->free(valuePos);
      }
    }
  }

	_info("GetPage(%s,%s) cost time:%d\n",
        conn->GetVNode()->GetAddress().c_str(),
        conn->GetVNode()->GetPort().c_str(),
        cost);
	//fprintf(stderr,"2222222222222222222222222222=%s,db=%p,ret=%d\n",circle.c_str(),db,ret);
	if (ret<0)
		conn->SetFlag(ERROR_SOCKET);
  }
OUT:
  conn->UnLockSelf();
	ReleaseConnection(conn);
	return ret;
}


int QdbDaemonClient::GetPage(void* key,int length,OffPage& page,const std::string& circle)
{
  int ret=0;
	Connection * conn = GetConnection(circle);
	if (!conn)
		return -1;
  
  conn->LockSelf();
  if (!ConnectionOk(conn)){
    ret=-1;
    goto OUT;
  }
  { 
	OffDB* db= (OffDB* )conn->private_data;
	timeval t1,t2;
	gettimeofday(&t1,NULL);
	//std::string addr = node_topology->GetVNode()->GetAddress()+node_topology->GetVNode()->Getport();
	ret= ReadQdb(db,key,length,page);
		gettimeofday(&t2,NULL);
	int cost = (t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);

	_info("GetPage(%s,%s) cost time:%d\n",conn->GetVNode()->GetAddress().c_str(),
										conn->GetVNode()->GetPort().c_str(),
										cost);
	//fprintf(stderr,"2222222222222222222222222222=%s,db=%p,ret=%d\n",circle.c_str(),db,ret);
	if (ret<0)
		conn->SetFlag(ERROR_SOCKET);
  }
OUT:
  conn->UnLockSelf();
	ReleaseConnection(conn);
	return ret;
}

int QdbDaemonClient::SetPage(void* key,int length,OffPage& page,const std::string& circle)
{
  int ret=0;
	Connection * conn = GetConnection(circle);
	if (!conn)
		return -1;
  
  conn->LockSelf();
  if (!ConnectionOk(conn)){
    ret=-1;
    goto OUT;
  }
  {
	OffDB* db= (OffDB* )conn->private_data;
	timeval t1,t2;
	gettimeofday(&t1,NULL);
	//std::string addr = node_topology->GetVNode()->GetAddress()+node_topology->GetVNode()->Getport();
	ret= WriteQdb(db,key,length,page);
	gettimeofday(&t2,NULL);
	int cost = (t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);

	_info("SetPage(%s,%s) cost time:%d\n",conn->GetVNode()->GetAddress().c_str(),
										conn->GetVNode()->GetPort().c_str(),
										cost);
	if (ret<0)
		conn->SetFlag(ERROR_SOCKET);
  }
OUT:
  conn->UnLockSelf();
	ReleaseConnection(conn);
	return ret;
}
int QdbDaemonClient::Del(void* key,int length,const std::string& circle)
{
  int ret=0;
  Connection * conn = GetConnection(circle);
  if (!conn)
    return -1;
  
  conn->LockSelf();
  if (!ConnectionOk(conn)){
    ret=-1;
    goto OUT;
  }
  {
  OffDB* db= (OffDB* )conn->private_data;
  if (db==NULL)
    ret=-1;
  else {
    struct timeval t1={0,500000};
    ret=db->del(key,length,1, &t1);
  }
  /*timeval t1,t2;
  gettimeofday(&t1,NULL);
  //std::string addr = node_topology->GetVNode()->GetAddress()+node_topology->GetVNode()->Getport();
  ret= WriteQdb(db,key,length,page);
  gettimeofday(&t2,NULL);
  int cost = (t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);

  _info("SetPage(%s,%s) cost time:%d\n",conn->GetVNode()->GetAddress().c_str(),
                    conn->GetVNode()->GetPort().c_str(),
                    cost);*/
  if (ret<0)
    conn->SetFlag(ERROR_SOCKET);
  }
OUT:
  conn->UnLockSelf();
  ReleaseConnection(conn);
  return ret;
}

int QdbDaemonClient::ReadQdb(OffDB *m_qdb,void* key,int length,OffPage& page)
{
	int ret(-1);
	if (!m_qdb)
		return -1;
	struct timeval t1 = {0,50000}, t2 = {0,50000};
	struct timeval db_timeout = {0,50000};
	int expireTime = 0, writetime;
	ret = m_qdb->get(key,length,page,&t1);
	//_err("QdbDaemonClient::ReadQdb get qdb failed,ret:%d,errno:%d\n",ret,errno);
	if (ret < 0 && errno != EINVAL ){
		if(m_qdb->reconnect(&db_timeout,1,false)==0){
			ret = m_qdb->get(key,length,page,&t2);
			if (ret < 0 && errno != EINVAL )
				m_qdb->reconnect(&db_timeout,1,false);
		}
		else {
			_err("QdbDaemonClient::ReadQdb get qdb failed,ret:%d,errno:%d\n",ret,errno);
			//worker->debuginfo += " [get qdb failed,ret:%d,]";
		}
	}
	return ret;
}

int QdbDaemonClient::WriteQdb(OffDB *m_qdb,void* key,int length,OffPage& page)
{
	//WEBSEARCH_ERROR((LM_ERROR,"[writeQDB1] begin to write qdb  \n"));
	if (!m_qdb)
		return -1;
	struct timeval t1 = {0,100000}, t2 = {0,100000};
	struct timeval db_timeout = {0,50000};
	int ret = m_qdb->put(key,length,page,0,&t1);
	if (ret < 0 && errno != EINVAL){
		if (m_qdb->reconnect(&db_timeout,1,false)==0){
			ret = m_qdb->put(key,length,page,0,&t2);
			if (ret < 0 && errno != EINVAL)
				m_qdb->reconnect(&db_timeout,1,false);
		}
		else 
			 _err("QdbDaemonClient::WriteQdb: ret:%d,errno:%d, reconnect\n",ret,errno);
	}
	return ret;
}

bool QdbDaemonClient::ConnectionOk(Connection* conn)
{

  std::map<Connection*,int >::iterator it;
  //std::map<Connection*,int >::iterator index=NULL;
  int max_visit_time;
  pthread_mutex_lock(&mutex_);
  size_t num = conn_list_.size();
  it = conn_list_.find(conn);

  std::string url=conn->GetVNode()->GetAddress()+":"+conn->GetVNode()->GetPort();
  if (it != conn_list_.end()){
    it->second=time(NULL);;
    goto  TRUE_OUT;
  }else {
    if (num <max_conn_num_){
      //just insert it ;
      OffDB* db= (OffDB* )conn->private_data;
      db->open(url.c_str());
      conn_list_.insert(std::map<Connection*,int>::value_type(conn,time(NULL)));	
      _debug("pool(not full),insert the conn to conn pool,url=%s\n",url.c_str());
      goto TRUE_OUT;
    }else if (num >= max_conn_num_){    
      //find the oldest visit connection
      time_t long_time=9990000000; 
      Connection* old_conn= NULL;
      for(it = conn_list_.begin(); it != conn_list_.end(); ++it) {
        if ((it->second <long_time)&&(it->second!=0)){
          long_time=it->second;
          old_conn=it->first;
          //it->second = 0;
          //index=it;
        }
      }
      if (!old_conn)
        goto ERROR_OUT;
      conn_list_[old_conn]=0;
      pthread_mutex_unlock(&mutex_);

      //remove the connection form map,and close the real connection
      old_conn->LockSelf();
      pthread_mutex_lock(&mutex_);
      //_debug("before erase,pool_num=%d\n",conn_list_.size());
      int ret=0;
      ret=conn_list_.erase(old_conn);
      if (ret>0){
        OffDB* old_db= (OffDB* )old_conn->private_data;
        old_db->close();
        std::string old_url=old_conn->GetVNode()->GetAddress()+":"+old_conn->GetVNode()->GetPort();
        _debug("poll(full),remove the conn from conn poll,url=%s\n",old_url.c_str());
        //_debug("after erase,pool_num=%d\n",conn_list_.size());

        //insert new connection and open it 
        OffDB* db= (OffDB* )conn->private_data;
        db->open(url.c_str());
        conn_list_.insert(std::map<Connection*,int>::value_type(conn,time(NULL)));	
        _debug("pool(full),insert the conn to conn pool,url=%s\n",url.c_str());
      }else{
        _debug("remove the conn from conn poll error,pool_num=%d\n",conn_list_.size());
        std::map<Connection*,int >::iterator entry= conn_list_.find(old_conn);
        if (entry != conn_list_.end()) {
          entry->second=1;
        } else {
          
        }
      }
      //_debug("after insert,pool_num=%d\n",conn_list_.size());
      pthread_mutex_unlock(&mutex_);
      old_conn->UnLockSelf();
      return ret>0?true:false;
    }
  }
TRUE_OUT:
	pthread_mutex_unlock(&mutex_);
  return true;
ERROR_OUT:
  	pthread_mutex_unlock(&mutex_);
  return false;

}


