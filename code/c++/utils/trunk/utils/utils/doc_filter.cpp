#include "doc_filter.h"
#include "Platform/docId/UrlInfo.h"
#include "Platform/log.h"
#include "Platform/encoding/URLValidator.h"
#include <Platform/encoding.h>

SS_LOG_MODULE_DEF(ods_utils_util);

static int parse_interact(std::string _subtype,interact_info& _inteInfo){
	int _allow_type[]={1,2,3,4,5,6,7};	
	
	WEBSEARCH_DEBUG((LM_DEBUG,"[interact attrInfo]%s\n",_subtype.c_str()));
	uint32_t _typeid=(uint32_t)atol(_subtype.c_str());
	_inteInfo.typeid_high	=	_typeid>>16;
	_inteInfo.typeid_low	=	_typeid&0xffff;
	fprintf(stderr,"[interAct_init]high:%d,low:%d\n",_inteInfo.typeid_high,_inteInfo.typeid_low);

	int _size=sizeof(_allow_type)/sizeof(int);
	for(int i=0;i<_size;++i)
	{
		if( (_inteInfo.typeid_high==3) &&(_inteInfo.typeid_low==0))
		{
			WEBSEARCH_DEBUG((LM_DEBUG,"[interact attrInfo]ignor 3,0\n"));
			return -1;
		}
		if((_inteInfo.typeid_high==7) &&(_inteInfo.typeid_low==0) )
		{
			WEBSEARCH_DEBUG((LM_DEBUG,"[interact attrInfo]ignor 7,0\n"));
			return -1;
		}
		if(_inteInfo.typeid_high==_allow_type[i])
		{
			WEBSEARCH_DEBUG((LM_DEBUG,"[interact attrInfo]high:%d,low:%d\n",_inteInfo.typeid_high,_inteInfo.typeid_low));
    		return 0;
		}
	} 
	WEBSEARCH_DEBUG((LM_DEBUG,"[interact attrInfo]high:%d,low:%d Ignored\n",_inteInfo.typeid_high
																   ,_inteInfo.typeid_low));
	return -1;
}


bool InterAct_filter::parseSiteQuery(std::string &src_query,std::string &site_type,std::string &para_name)
{
	size_t pos=src_query.find(para_name);
	site_type="";

	if(pos==std::string::npos)
	{
		fprintf(stderr,"[InterAct_filter::parseSiteQuery]none SiteQuery\n");
		return -1;
	}
	pos=src_query.find('=', pos);
	if(pos==std::string::npos)
	{
		fprintf(stderr,"[InterAct_filter::parseSiteQuery]parse ERROR\n");
		return false; 
	}   
	
	std::string interActType=src_query.substr(pos+1);
	std::string::iterator it = interActType.begin();
	while(it != interActType.end() && *it != '&' && *it != '\r')
		++it;
	interActType = interActType.substr(0, it-interActType.begin());
	fprintf(stderr,"[InterAct_filter::parseSiteQuery]interAct_value:%s\n",interActType.c_str());
	site_type=interActType;
	if(interActType.empty())
	{
		fprintf(stderr,"[InterAct_filter::parseQuery]parse ERROR\n");
		return false;
		
	}
}

bool InterAct_filter::parseQuery(std::string &src_query,std::vector<interact_info> &interact_types,std::string &para_name)
{
	interact_types.clear();
	interact_info _inteInfo;
	
	size_t pos=src_query.find(para_name);
	if(pos==std::string::npos)
	{
		fprintf(stderr,"[InterAct_filter::parseQuery]none interAct\n");
		return -1;
	}
	pos=src_query.find('=', pos);
	if(pos==std::string::npos)
	{
			fprintf(stderr,"[InterAct_filter::parseQuery]parse ERROR\n");
			return -1;
	}
	std::string interActType=src_query.substr(pos+1);
	
	std::string::iterator it = interActType.begin();
	while(it != interActType.end() && *it != '&' && *it != '\r')
  		++it;
  	interActType = interActType.substr(0, it-interActType.begin());
	fprintf(stderr,"[InterAct_filter::parseQuery]interAct_value:%s\n",interActType.c_str());
	
	if(interActType.empty())
	{
			fprintf(stderr,"[InterAct_filter::parseQuery]parse ERROR\n");
			return -1;
	}
	
	//提取出了interactValue之后
	int  _begindx=0;
	int _pos=interActType.find(",",_begindx);
	int _len;
	fprintf(stderr,"_pos=%ld,%ld\n",_pos,std::string::npos);

	std::string _subtype;
	while(_pos != -1) {
		_len=_pos-_begindx;
		_subtype=interActType.substr(_begindx,_len);
		 fprintf(stderr,"_pos=%d\n",_pos);

		if(parse_interact(_subtype,_inteInfo)) {
			WEBSEARCH_DEBUG((LM_DEBUG,"[interacttype_parse_params]ERROR:%s\n",_subtype.c_str()));
		}else
			interact_types.push_back(_inteInfo);
		_begindx=_pos+1;
		_pos=interActType.find(",",_begindx);
	}
	
	_subtype=interActType.substr(_begindx);
	if(parse_interact(_subtype,_inteInfo)) {
		WEBSEARCH_DEBUG((LM_DEBUG,"[interacttype_parse_params]ERROR:%s\n",_subtype.c_str()));
	}else
		interact_types.push_back(_inteInfo);
	return 0;
	
}

//使用interAct_types过滤url
bool InterAct_filter::interact_filter(std::string &url,std::vector<interact_info> &interact_types,std::string &site_type)
{
	if( (interact_types.size()==0) && (site_type.empty())){
		return true;
	}

	char _url[1024];
	EncodingConvertor::getInstance()->sbc2dbc(url.c_str(), _url, 1024, true);
	WEBSEARCH_DEBUG((LM_DEBUG,"[interact_filter]gbk url:%s\n",_url));
	
	if(!site_type.empty())
	{
		if(!pass_site_filter(site_type,_url))
			return false;
	}

	if(interact_types.size()==0)
		return true;

	std::vector<interact_info>::iterator it;
	for(it=interact_types.begin();it!=interact_types.end();it++) {
			uint16_t _high_id=it->typeid_high;
			uint16_t _low_id=it->typeid_low;
			int _value;
			if((_high_id==4)||(_high_id==5)||(_high_id==6))
			{
				fprintf(stderr,"[interact_filter]high_id=(4,5,6) alway return false\n");
				continue;
			}
			if((_high_id==2)||(_high_id==3)) {
				_value=_low_id;
			}else{
				_value=_high_id;
			}
			if(pass_filter(_value,_url)){
				fprintf(stderr,"[interact_filter]pass filter\n"); 
				return true;
			}
	}
	return false;
}

bool InterAct_filter::pass_site_filter(std::string &site_filt,std::string _url)
{
	platform::UrlInfo _site_info(site_filt.c_str());
	platform::UrlInfo _url_info(_url.c_str());
	
	const char *_siteptr;   
	const char *_urlPtr;    
	size_t siteLen=-1;
	size_t urlLen=-1;
	
	std::string src_site;
	std::string url_site;
	_site_info.GetHost(_siteptr,siteLen);
	_url_info.GetHost(_urlPtr,urlLen);
	
	if( (siteLen<=0) || (urlLen<=0) ||(_siteptr==NULL) || (_urlPtr==NULL))
		return false;
	
	const char *tmp;
	const char *tmp1=strstr(_siteptr,"www.");
	const char *tmp2=strstr(_siteptr,"http://");
	const char *tmp3=strstr(_siteptr,"https://");
	
	if(tmp1!=NULL)
		tmp=tmp1+4;
	else
	{   
		if(tmp2!=NULL)																											
			tmp=tmp2+7;
		else if(tmp3!=NULL)
			tmp=tmp3+8;
		else
			tmp=_siteptr;
	}
	src_site.append(tmp,siteLen);
	url_site.append(_urlPtr,urlLen);
	fprintf(stderr,"[InterAct_filter::pass_site_filter]site=%s,url=%s\n",src_site.c_str(),url_site.c_str());
	size_t pos=url_site.find(src_site);
	if(pos!=std::string::npos)
		return true;
	return false;
}

bool InterAct_filter::pass_filter(int ivalue,std::string _url)
{
	if(ivalue==0)
		return true;

	char tempValue[32];	
	snprintf(tempValue,32,"%d",ivalue);
	std::string _value(tempValue);
	_url.append("/");		

	std::string _url_key(_url);

	WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]url:%s,value:%s\n",_url.c_str(),_value.c_str()));
	std::string _site_key;
	std::string _domain_key;

	const char *_ptr;
	size_t str_len;
	_url_key.append(_value);
	WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]urlKey:%s\n",_url_key.c_str()));
	if(filter_map.find(_url_key)!=filter_map.end()){
		WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]find url:%s\n",_url.c_str()));
		WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]url:%s ,pass\n",_url.c_str()));
		return true;
	}
	platform::UrlInfo _urlinfo(_url.c_str());
	_urlinfo.GetDomain(_ptr,str_len);
	_domain_key.assign("http://",7);
	_domain_key.append(_ptr,str_len);
	_domain_key.append("/");
	_domain_key.append(_value);
	WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]domainKey:%s\n",_domain_key.c_str()));
	if(filter_map.find(_domain_key)!=filter_map.end()) {
		WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]find domain:%s\n",_domain_key.c_str()));
		WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]url:%s ,pass domain\n",_url.c_str()));
		return true;
	}
	_urlinfo.GetHost(_ptr,str_len);
	_site_key.append(_ptr,str_len);
	_site_key.append("/");
	_site_key.append(_value);
	_ptr=_site_key.c_str();

	WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]siteKey:%s\n",_ptr));
	if(filter_map.find(_ptr)!=filter_map.end()) {
		WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]find site:%s\n",_ptr));
		WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]url:%s ,pass site\n",_url.c_str()));
		return true;
	}
	WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::pass_filter]url:%s ,not pass\n",_url.c_str()));
	return false;
}

int InterAct_filter::init(std::string &fileName){
	const int L_BUFFER=2048;
	char buff[L_BUFFER];

	int _typeid;
	char _data[1024];
	int _value;

		
	HashMap *curMap=NULL;
	WEBSEARCH_DEBUG((LM_DEBUG,"[InterAct_filter::init]Begin\n"));
	FILE* fp=fopen(fileName.c_str(),"rb");

	if(fp==NULL){
		WEBSEARCH_DEBUG((LM_ERROR,"[InterAct_filter::init]open %s error!\n",fileName.c_str()));
		return -1;
	}
	char *ret=NULL;
	while((ret=fgets(buff,L_BUFFER,fp))!=NULL){
		int iLen = strlen(buff);
        	if ( iLen<=0 )
        	{
           	 	break;
        	}
        	if ( buff[iLen-1]!='\n' )
        	{
            		if ( iLen==L_BUFFER-1 )
           		 {
                		WEBSEARCH_DEBUG((LM_ERROR,"[InterAct_filter::init]get too long >1024 string!\n"));
				fclose(fp);
				return -1;
            		}
        	}
		sscanf(buff,"%d\t%s\t%d",&_typeid,_data,&_value);
		char _tmpValue[32];
        	snprintf(_tmpValue,32,"%d",_value);
		fprintf(stderr,"[InterAct_filter::init]get new line:%d,%s,%s\n",_typeid,_data,_tmpValue);
		std::string _newKey(_data);
		_newKey.append(_tmpValue);
		fprintf(stderr,"[InterAct_filter::init]new key:%s,value=%s\n",_newKey.c_str(),_data);	
		if(filter_map.find(_newKey)!=filter_map.end()){
			fprintf(stderr,"[InterAct_filter::init]ERROR,two same url,%s\n",_data);
		}
		filter_map[_newKey]=_data;
	}
	fclose(fp);
	return 0;

}
