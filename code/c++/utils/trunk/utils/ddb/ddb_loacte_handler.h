#ifndef _DDB_INDEX_HANDLER_
#define _DDB_INDEX_HANDLER_

class DDBLocateHandler:public SyncClient
{
public:
  int Open(const std::string& conf);
  int GetSockAddr(void * key,int key_len,struct sockaddr *& serv_addr,int& addr_len,int timeout);

protected:
  
};


#endif