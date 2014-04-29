#include "ddb_locate_handler.h"

std::string DDBLocateHandler::GetHost(void * key,int key_len,struct sockaddr *& sock_addr,int& addr_len,int timeout)
{
  int ret=0;
  Connection * conn = GetConnection(sogou_utils::MD5_128(key,key_len),4);
  if (!conn)
    return -1;

  conn->LockSelf();
  DDBProtocolAdapter adapter(conn);
  struct timeval tv;
  tv.tv_sec = 0;
	tv.tv_usec = timeout*1000;
  //int GetSockAddr(int sockfd,void * key,int key_len,struct sockaddr *& serv_addr,int& addr_len,int flags,struct timeval* tv );
  ret =adapter.GetHost(conn->GetFd(),key,key_len,sock_addr,addr_len,0,tv);
  if (ret<0)
    	conn->SetFlag(conn->GetFlag()| ERROR_SOCKET);
  conn->UnLockSelf();
  ReleaseConnection(conn);
  return ret;  
}
