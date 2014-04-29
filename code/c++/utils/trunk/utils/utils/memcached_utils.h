#ifndef _MEMCACHED_UTILS_H_
#define _MEMCACHED_UTILS_H_
#include <string>
#include <libmemcached/memcached.h>
#include <utils/debug.h>
class MemCachedClient
{
public:
  MemCachedClient(){
    memc_=NULL;
  };
  virtual ~MemCachedClient(){
    if (memc_)
      free(memc_);
  }
  void Init(std::string & host){
    size_t pos = host.find(":");
    host_= host.substr(0,pos);
    port_ = atoi(host.substr(pos+1,host.length() -pos).c_str());
    _info("host=%s,port=%d\n",host_.c_str(),port_);
  }
  int Connect();
	int GetValue(void* key,int length,std::string& content);
	int SetValue(void* key,int length,const std::string& content,int );
  int DelValue(void* key,int length);
  int ReConnect(int retries=3);
  int CloseMemc();
  
protected:
  
  std::string host_;
  unsigned int port_;
  memcached_st *memc_;
};
#endif 
