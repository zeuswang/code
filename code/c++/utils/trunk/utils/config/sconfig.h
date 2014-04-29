#ifndef _SCONFIG_HPP
#define _SCONFIG_HPP
#include <ace/SString.h>
#include <ace/Configuration.h>
#include <ace/Configuration_Import_Export.h>
#include <vector>
#include <string>

class SConfig
{
	public:
		SConfig(){
			conf_impexp=NULL;	
		};
		~SConfig(){
			if (conf_impexp)
				delete conf_impexp;
		}
		int Open(const std::string& filename);
		int SetSection(const std::string& section_name);
		std::string GetParameter(const std::string& param_name);
		std::string GetParameterSafe(const std::string& param_name,const std::string& default_str);
		bool GetExistParameter(const std::string& param_name,std::string& result);
		int GetParameterList(std::vector<std::string>& list,std::string pre_fix,int num,int start_num);
		
	protected:
		ACE_Configuration_Heap conf_heap;
		ACE_Registry_ImpExp* conf_impexp;
		ACE_Configuration_Section_Key conf_key;
		
};
#endif
