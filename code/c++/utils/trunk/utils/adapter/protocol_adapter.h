#ifndef _SOGOU_PROTOCOL_ADAPTER_H_
#define _SOGOU_PROTOCOL_ADAPTER_H_
#include <string>
#include "sconnection.h"
#include <map>
class ProtocolAdapter
{
public:
	ProtocolAdapter(){};
	//ProtocolAdapter(Connection* conn){conn_=conn;};
	virtual ~ProtocolAdapter(){};
	/*virtual void SetConnection(Connection* conn){
		if (conn)
			conn_=conn;
	};*/
	virtual int GetRequest()=0;
	virtual int GetResult()=0;
	virtual int PutRequest()=0;
	virtual int PutResult()=0;
	
	//virtual std::string GetParameter(const std::string& key);
	//virtual int SetParameter(const std::string& key,const std::string& value);
protected:
	//Connection * conn_;
	//std::map<std::string,std::string> param_list;

};
#endif
