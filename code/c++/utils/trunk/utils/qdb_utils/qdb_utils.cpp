#include "qdb_utils.h"
int QdbHandler::Get(const std::string&key,std::string& value)
{
	HttpProtocolAdapter adapter;
	adapter.SetContentParameter("cmd","get");
	adapter.SetContentParameter("key",key);
	int ret= http_client_->Get(&adapter,key);
	if (ret<0)
		return -2;
	std::string result =adapter.GetResultContent();	
	ret = atoi(result.substr(0,2).c_str());
	errno= atoi(result.substr(2,4).c_str());
	value = result.substr(6,result.length() -6);
	return ret;
}
int QdbHandler::Set(const std::string &key,const std::string& value)
{
	HttpProtocolAdapter adapter;
	adapter.SetContentParameter("cmd","set");
	adapter.SetContentParameter("key",key);
	adapter.SetContentParameter("value",value);
	int ret= http_client_->Get(&adapter,key);
	if (ret<0)
		return -2;
	std::string result =adapter.GetResultContent();	
	ret = atoi(result.substr(0,2).c_str());
	errno= atoi(result.substr(2,4).c_str());
	return ret;
}



