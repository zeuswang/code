#ifndef _DDB_CLIENT_PROTOCOL_H_
#define _DDB_CLIENT_PROTOCOL_H_

#include <utils/sconnection.h>
/*
enum RoleCode
{
  DDBUser,
  DDBMaster,
  DDBClient,
  DDBData,
}*/
#define DDB_GET_LOCATION_CMD  1
#define DDB_CLIENT_GET_CMD    2
#define DDB_CLIENT_SET_CMD    3
#define DDB_CLIENT_DEL_CMD    4

class DDBProtocolAdapter
{
public:
  DDBProtocolAdapter(Connection* conn){conn_=conn};
  DDBProtocolAdapter(){};
  int GetSockAddr(int sockfd,void * key,int key_len,struct sockaddr *& serv_addr,int& addr_len,int flags,struct timeval* tv );
  int Get(int sockfd,void* key,int key_len,void*& value,int& value_len,int flags,struct timeval* tv);
  int Set(int sockfd,void* key,int key_len,void* value,int value_len,int flags,struct timeval* tv);
  int Del(int scokfd,void* key,int key_len,int flags,struct timeval* tv);
  
protected:
  Connection * conn_;
  //MakeRequest(uint8_t cmd,void* key,uint32_t len,void* value,uint32_t v_len,uint8_t flags);
  //MakeResult(uint8_t ret,int error,void* value,uint32_t v_len);
};
/*class DDBClientProtocolReceiver
{
public:
  DDBClientProtocolReceiver(Connection* conn){
    conn_=conn;
  }
  //DDBClientProtocolReceiver();
  virtual ~DDBClientProtocolReceiver();
  uint8_t Parse();
  DDBClientCMD GetCmd();
  void * GetKey(int& len);
  void * GetValue(int & len);  

protected:
  Connection* conn_;
  void * key_;
  int key_len_;
  void * value_;
  int value_len_;
};
class DDBClientProtocolSender
{
public:
  DDBClientProtocolSender();
  virtual ~DDBClientProtocolSender();

protected:

};
*/
#endif 