#ifndef _DDB_DATA_HANDLER_H_
#define _DDB_DATA_HANDLER_H_
#include "utils/sync_client.h"
#include <utils/debug.h>
#include <map>

class DDBDataHandler
{
public:
	DDBDataHandler(int conn_max){
		conn_pool_ = connpool_create(conn_max);
	}
	virtual ~DDBDataHandler(){
    if (conn_pool_)
      connpool_destroy(conn_pool_);
	};

  int Del(void* key,int length,struct sockaddr* sock_addr,int addr_len);
  int Get(void* key,int length,void*& value,int& value_len,struct sockaddr* sock_addr,int addr_len);
  int Set(void* key,int length,void* value,int value_len,struct sockaddr* sock_addr,int addr_len);

protected:
  connpool_conn* GetConn(struct sockaddr* sock_addr,int addr_len,unsigned int timeout);
  void  FreeConn(connpool_conn* conn,int ret);
  connpool_t*  conn_pool_;

};
#endif

