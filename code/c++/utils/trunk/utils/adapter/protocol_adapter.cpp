#include "protocol_adapter.h"
#include <string>
std::string ProtocolAdapter::GetParameter(const std::string& key)
{
	std::map<std::string, std::string>::const_iterator entry = param_list.find(key);
	
	if (entry != param_list.end()) {
		return entry->second;
	} else {
		return "";
	}
}
int ProtocolAdapter::SetParameter(const std::string& key,const std::string& value)
{
	std::map<std::string, std::string>::iterator entry = param_list.find(key);	
	if (entry != param_list.end()) {
		entry->second = value;
	} else {
		param_list.insert(std::map<std::string,std::string>::value_type(key,value));		
	}
	return 0;
}

