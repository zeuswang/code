#ifndef _HTTP_CLIENT_
#define _HTTP_CLIENT_


class HttpClient:public SyncClient
{
public:
	HttpClient(){		
		check_interval_=10;
		//pthread_mutex_init(&mutex_,NULL);	
	}
	~HttpClient(){
		//pthread_mutex_destroy(&mutex_);
	}
	/*virtual int Open(const std::string& addr,int timeout)
	{
		//SetCheckInterval(check_interval_);
		//timeout_=timeout;
		SyncClient::Open(addr,timeout);
	}*/
		
	int Get(HttpProtocolAdapter* adapter);
	//virtual int Close(){};
	
protected:
	virtual int CheckConnection(Connection*);
	int check_interval_;
	//int timeout_;
	//pthread_mutex_t mutex_;
};
#endif
