#ifndef _QDB_UTILS_
#define _QDB_UTILS_

class QdbHandler
{
public:
	QdbHandler(){
	}
	~QdbHandler(){
	}
	int Open(const std::string& conf){
		http_client_->LoadConf(conf);
	}
	//int Reconnect(int retry=0);
	int Get(const std::string&key,std::string value);
	int Set(const std::string &key,const std::string value);
protected:
	HttpClient http_client_;
	
};

#endif
