#include "sconfig.h"
#include <ace/Configuration.h>
#include <ace/Configuration_Import_Export.h>
//#include <Platform/log.h>
#include <string>
//SS_LOG_MODULE_DEF(ods_utils);

int SConfig::Open(const std::string& filename)
{
	if (conf_heap.open()){
		fprintf(stderr,"[SConfig::Open] [open heap error]\n");
		return -1;
	}
	conf_impexp = new ACE_Registry_ImpExp(conf_heap);
	if (conf_impexp->import_config(filename.c_str())){
		fprintf(stderr,"[SConfig::Open] [open file error: %s]\n",filename.c_str());
		return -1;
	}
	return 0;
}
int SConfig::SetSection(const std::string& section_name)
{
	if (conf_heap.expand_path(conf_heap.root_section(), section_name.c_str(), conf_key, 0)){
		fprintf(stderr,"[SConfig::SetSection] [error: %s]\n", section_name.c_str());
		return -1;
	}
	return 0;
}
std::string SConfig::GetParameter(const std::string& param_name)
{
	ACE_TString value;
	//read_string(conf_heap, conf_key, param_name.c_str(),value);	
	 if (conf_heap.get_string_value(conf_key, param_name.c_str(), value)){
		fprintf(stderr,"[SConfig::GetParameter] [error %s]\n", param_name.c_str());
		return "";
	 }
	fprintf(stdout,"SConfig::GetParameter() key=%s,value=%s\n",param_name.c_str(),value.c_str());
	return value.c_str();
}
std::string SConfig::GetParameterSafe(const std::string& param_name,const std::string& default_str)
{
	ACE_TString value;
	//read_string(conf_heap, conf_key, param_name.c_str(),value);	
	 if (conf_heap.get_string_value(conf_key, param_name.c_str(), value)){
        	//WEBSEARCH_DEBUG((LM_DEBUG,"[SConfig::GetParameter] [error %s] [return default value %s]\n", param_name.c_str(),default_str.c_str()));
        	fprintf(stderr,"[SConfig::GetParameter] [error %s] [return default value %s]\n", param_name.c_str(),default_str.c_str());
		return default_str ;
	 }
	//fprintf(stdout,"SConfig::GetParameter() key=%s,value=%s\n",param_name.c_str(),value.c_str());
	return value.c_str();
}

bool SConfig::GetExistParameter(const std::string& param_name,std::string& result)
{
	ACE_TString value;
	//read_string(conf_heap, conf_key, param_name.c_str(),value);	
	 if (conf_heap.get_string_value(conf_key, param_name.c_str(), value)){
	 	fprintf(stderr,"[SConfig::GetExistParameter] [error %s]\n", param_name.c_str());
		return false;
	 }
	result=value.c_str();
	return true;
}
int SConfig::GetParameterList(std::vector<std::string>& list,std::string pre_fix,int num,int start_num)
{
	int index=start_num;
	std::string value;
	int ret=0;
	for (index;index<num+start_num;index++){

		char buf[1024];
		snprintf(buf,1024,"%s%d",pre_fix.c_str(),index);
		ret=GetExistParameter(std::string(buf),value);
		if (!ret)
			return -1;
		list.push_back(value);		
	}
	return 0;
}

