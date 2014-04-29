#include "mpage.h"
#include <utils/sstring.h>
#include <utils/debug.h>

int MPage::SetAttribute(const std::string& key,const std::string& value)
{
  std::map<std::string,std::string>::iterator it = attribute_map_.find(key);
  if (it != attribute_map_.end()) 
  {
    it->second = value;
  }else {
    attribute_map_.insert(std::map<std::string,std::string>::value_type(key,value));
  }
  return 0;
}
int MPage::RemoveAttribute(const std::string& key)
{
  attribute_map_.erase(key);
  return 0;
}

std::string  MPage::GetAttribute(const std::string& key)
{
  std::map<std::string,std::string>::iterator it = attribute_map_.find(key);
  if (it != attribute_map_.end()) 
  {
    return it->second;
  }else {
    return  "";
  }
}

int MPage::GetValue(const std::string & key,std::string &result)
{
  std::map<std::string,std::string>::iterator it = content_map_.find(key);
  if (it != content_map_.end()) 
  {
    result = it->second;
  }else {
    result = "";
  }
  return 0;
}
int MPage::SetValue(const std::string &key,const std::string &value)
{
  std::map<std::string,std::string>::iterator it = content_map_.find(key);
  if (it != content_map_.end()) 
  {
    it->second = value;
  }else {
    content_map_.insert(std::map<std::string,std::string>::value_type(key,value));
  }
  return 0;
}


bool MPage::HaveData(const std::string& key)
{
  	std::map<std::string,std::string>::iterator it = content_map_.find(key);
	if (it != content_map_.end()) 
	{
    return true;
	}else 
	  return false;
}
int MPage::LoadOffPage(const OffPage& page)
{
  std::string headinfo;
  std::string ainfo;

  //insert content
  if (page.attributes.get("headinfo",headinfo)){//get ok
    //_info("headinfo=%s\n",headinfo.c_str());
    std::vector<std::string> list;
    if (sogou_string::Split2List(headinfo,";",list)){
      return -1;
    }
    
    	for (int i=0;i<list.size();i++){
      void* valuePos=NULL;
      size_t valueLen;
      std::string value;
      if (page.getContent(list[i].c_str(),(void *&)valuePos,valueLen)){//get failed
        return -1;
      }else {
        value.assign((char*)valuePos,valueLen);
        if (valuePos){
          free(valuePos);
        }
      }
      content_map_.insert(std::map<std::string,std::string>::value_type(list[i],value));
    }
  }else {
    return -1;
  }

  //insert attribute
  if (page.attributes.get("attributeinfo",ainfo)){//get ok
    //_info("attributeinfo=%s\n",ainfo.c_str());
    std::vector<std::string> list;
    if (sogou_string::Split2List(ainfo,";",list)){
      return -1;
    }
    
   	for (int i=0;i<list.size();i++){
      std::string value;
      if (page.attributes.get(list[i],value)){//get ok
        //_info("attributeinfo=%s,value=%s\n",list[i].c_str(),value.c_str());
        attribute_map_.insert(std::map<std::string,std::string>::value_type(list[i],value));
      }
    }
  }else 
    return -1;
  return 0;
}

std::string MPage::GetHeadInfo()
{
  std::string headinfo;
  std::map<std::string,std::string>::iterator it;
  for(it = content_map_.begin(); it != content_map_.end(); ++it) {
    headinfo +=it->first+";";
  }
  return headinfo;
}
std::string MPage::GetAttributeInfo()
{
  std::string ainfo;
  std::map<std::string,std::string>::iterator it;
  for(it =attribute_map_.begin(); it != attribute_map_.end(); ++it) {
    ainfo +=it->first+"="+it->second+";";
  }
  return ainfo;
}

int MPage::Flush2OffPage(OffPage & page)
{
  std::string headinfo;
  std::string ainfo;
  int ret=0;
  std::map<std::string,std::string>::iterator it;
  for(it = content_map_.begin(); it != content_map_.end(); ++it) {
  //_info("content length=%d\n",it->second.length());
    ret=page.addContent(it->first.c_str(), const_cast<char *>(it->second.c_str()),it->second.length());
    if (ret!=0)
      return -1;
    headinfo +=it->first+";";
  }

  for(it =attribute_map_.begin(); it != attribute_map_.end(); ++it) {
    page.attributes.set(it->first,it->second);
    ainfo +=it->first+";";
  }
  //_info("headinfo=%s\n",headinfo.c_str());
  page.attributes.set("headinfo", headinfo);

  char buf[64];
  snprintf(buf,64,"%u",time(NULL));
  std::string wtime(buf);
  //_info("writetime=%s\n",wtime.c_str());
  page.attributes.set("writetime", wtime);

  //_info("attributeinf=%s",ainfo.c_str());
  page.attributes.set("attributeinfo", ainfo);
  return 0;

}

