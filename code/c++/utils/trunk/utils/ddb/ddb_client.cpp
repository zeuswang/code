#include "ddb_client.h"
int DDBClient::Open(const std::string& conf)
{
  int ret = master_handler_.Open(conf);
  if (ret<0)
    return -1;
  std::string index_server_conf = master_handler_.GetLocateConf();
  if (index_server_conf.empty())
    return -1;
  ret =locate_handler_.Open(index_server_conf);
  if (ret<0)
    return -1;
  return 0;
}

int DDBClient::Del(void* key,int length)
{
  struct sockaddr* sock_addr;
  int addr_len;
  int ret= locate_handler_.GetSockAddr(key,length,sock_addr,addr_len,timeout_);
  if (ret<0)
    return -1;
  ret=data_handler_.Del(key,length,sock_addr,addr_len,timeout_);
  free(sock_addr);
  return ret;  
}
int DDBClient::Get(void* key,int length,void*& value,int& value_len)
{
  struct sockaddr* sock_addr;
  int addr_len;
  int ret= locate_handler_.GetSockAddr(key,length,sock_addr,addr_len,timeout_);
  if (ret<0)
    return -1;
  ret=data_handler_.Del(key,length,sock_addr,addr_len);
  free(sock_addr);
  return ret;  
}
int DDBClient::Set(void* key,int length,void* value,int value_len)
{
  std::string ip = locate_handler_.GetHost(key,length);
  return data_handler_.Set(key,length,value,value_len,ip);
}