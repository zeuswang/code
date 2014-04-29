#include "ddb_data_handler.cpp"
connpool_conn* DDBDataHandler::GetConn(struct sockaddr* sock_addr,int addr_len,int timeout)
{
  if (conn_pool_)
    return connpool_getconn(sock_addr,addr_len,timeout,conn_pool_);
  else 
    return NULL;

}
void DDBDataHandler::FreeConn(connpool_conn* conn,int ret)
{
  unsigned int keepalive;
  if (ret<0)
    keepalive =0;
  else 
    keepalive =60;
  if (conn_pool_)
    return connpool_freeconn(conn,keepalive,conn_pool_);
  else 
    return ;
}

int DDBDataHandler::Del(void* key,int length,struct sockaddr* sock_addr,int addr_len,int timeout=50)
{
  int ret=0;
  connpool_conn* conn = GetConn(sock_addr,addr_len,1);
  if (!conn)
    return -1;
  DDBProtocolAdapter adapter;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = timeout*1000;
  ret =adapter.Del(conn->sockfd,key,length,0,&tv);
  FreeConn(conn,ret);
	return ret;  
}
int DDBDataHandler::Get(void* key,int length,void*& value,int& value_len,struct sockaddr*& sock_addr,int& addr_len)
{
  int ret=0;
  connpool_conn* conn = GetConn(sock_addr,addr_len,1);
  if (!conn)
    return -1;
  DDBProtocolAdapter adapter;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 50*1000;
  ret =adapter.Get(conn->sockfd,key,length,sock_addr,addr_len,0,&tv);
  FreeConn(conn,ret);
	return ret;  
}
int DDBDataHandler::Set(void* key,int length,void* value,int value_len,struct sockaddr* sock_addr,int addr_len)
{
  int ret=0;
  connpool_conn* conn = GetConn(sock_addr,addr_len,1);
  if (!conn)
    return -1;

  DDBProtocolAdapter adapter;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 50*1000;
  ret =adapter.Set(conn->sockfd,key,length,sock_addr,addr_len,0,0,&tv);
  FreeConn(conn,ret);
	return ret;  
}

