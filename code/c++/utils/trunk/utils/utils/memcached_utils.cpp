#include "memcached_utils.h"
int MemCachedClient::Connect()
{
	memc_ = memcached_create(NULL);
  if (memc_ ==NULL) return -1;

  memcached_return errorCode = memcached_server_add(memc_, host_.c_str(), port_);

  if (MEMCACHED_SUCCESS!=errorCode)
  {
    //    WEBSEARCH_ERROR((LM_ERROR,"[HandleTask]::connectMemc memcached_server_add failed\n"));
    _err("memcached_server_add error\n");
    return -1;
  }
  errorCode = memcached_behavior_set(memc_, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL,1ULL);
  errorCode = memcached_set(memc_, "__test_server__", sizeof("__test_server__")-1, "test", sizeof("test")-1, 100, 0);
  if (MEMCACHED_SUCCESS!=errorCode)
  {
   // WEBSEARCH_ERROR((LM_ERROR,"[HandleTask]::connectMemc test failed\n"));
    _err("connectMemc test failed\n");
    return -1;
  }
  char *tmp;
  size_t tmp_len;
  uint32_t flags;
  tmp = memcached_get(memc_,"__test_server__", sizeof("__test_server__")-1,&tmp_len,&flags,&errorCode);
  if (MEMCACHED_SUCCESS==errorCode){
    //WEBSEARCH_DEBUG((LM_DEBUG,"[HandleTask]::connectMemc OK!\n"));
    _debug("connectMemc OK!\n");
    free(tmp);
  }else {
    //WEBSEARCH_ERROR((LM_ERROR,"[HandleTask]::connectMemc failed.\n"));
    _err("connectMemc failed.!\n");
    return -1;
  }
  return 0;
}
int MemCachedClient::GetValue(void* key,int length,std::string& content)
{
  uint32_t flags;
  int ret=-1;
  memcached_return memc_err;
  char *memc_res=NULL;
  size_t memc_res_len;

  memc_res = memcached_get(memc_,(char *)key,length,&memc_res_len,&flags,&memc_err);
  if( memc_err != MEMCACHED_SUCCESS )
    _err("get memcached failed,%s\n",memcached_strerror(memc_,memc_err));
  if(memc_err == MEMCACHED_SERVER_ERROR){
    if(ReConnect(1)==0){
      memc_res = memcached_get(memc_,(char *)key,length,&memc_res_len,&flags,&memc_err);
    if(memc_err == MEMCACHED_SERVER_ERROR)
      ReConnect(1);
    }else 
      _err("MEMCACHED_SERVER_ERROR\n");
  }

  if(memc_res!=NULL){
    ret = 0;
    _debug("memc_get %s\n",memc_res==NULL?"failed":"successed");
    content.assign(memc_res,memc_res_len);
    free(memc_res);
  }
  return ret;
}
int MemCachedClient::SetValue(void* key,int length,const std::string& content,int expiretime)
{
  memcached_return rc = memcached_set(memc_,(char*)key,length,content.c_str(),content.length(),expiretime,0);
  if( rc != MEMCACHED_SUCCESS ){
    _err("set memcached failed\n");
    return -1;
  }else 
    return 0;

}
int MemCachedClient::DelValue(void* key,int length)
{
	memcached_return errorCode = memcached_delete(memc_,(char *)key, length,0);

	if (MEMCACHED_SUCCESS!=errorCode){
		_err("deleted failed:%s\n",memcached_strerror(memc_,errorCode));
		return -1;
	}else 
    	return 0;
}

int MemCachedClient::CloseMemc()
{
	if (memc_){
    memcached_free(memc_);
    memc_=NULL;
	}
	return 0;
}

int MemCachedClient::ReConnect(int retries)
{
	int ret;
	for(;retries;retries--){
		CloseMemc();
		if(Connect()){			
			_err("%s:%d failed, reconnectiong...\n",host_.c_str(),port_);
		}
		else{
			return 0;
		}
		sleep(1);
	}
	return -1;
}

