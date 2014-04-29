#ifndef _QDB_DAEMON_CLIENT_H_
#define _QDB_DAEMON_CLIENT_H_
#include "utils/sync_client.h"
#include "offdb/OffDB.h"
#include "offdb/OffPage.h"
#include <utils/debug.h>
#include <map>
#include <utils/sxml.h>
class QdbDaemonClient:public SyncClient
{
public:
	QdbDaemonClient(){
		pthread_mutex_init(&mutex_,NULL);	
	}
	virtual ~QdbDaemonClient(){
		pthread_mutex_destroy(&mutex_);
	};
	int SetMaxConnNum(int n){max_conn_num_=n;};
	int GetPage(void* key,int length,OffPage& page,const std::string& circle);
	int SetPage(void* key,int length,OffPage& page,const std::string& circle);
  int Del(void* key,int length,const std::string& circle);
  	int GetContent(void* key,int length,std::string& str,const std::string& circle);
	int SetContent(void* key,int length,const std::string& str,const std::string& circle);

protected:
  void ErrorHandle(Connection * conn);
  bool ConnectionOk(Connection* conn);
	int ReadQdb(OffDB *m_qdb,void* key,int length,OffPage& page);
	int WriteQdb(OffDB *m_qdb,void* key,int length,OffPage& page);
	std::map<Connection*,int> conn_list_;
	pthread_mutex_t mutex_;
	int max_conn_num_;
	
};

#endif 
