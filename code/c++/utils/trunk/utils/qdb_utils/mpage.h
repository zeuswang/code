#ifndef _MPAGE_H_
#define _MPAGE_H_

#include "offdb/OffDB.h"
#include "offdb/OffPage.h"
#include <ace/Malloc_Base.h>
#include "Platform/log.h"
#include <sys/time.h>
#include <stdlib.h>
#include <map>

class MPage
{
public:
	MPage(){};
	virtual ~MPage(){};
	int SetAttribute(const std::string& key,const std::string& value);
	std::string  GetAttribute(const std::string& key);
  int RemoveAttribute(const std::string& key);
	int GetValue(const std::string &key,std::string &reuslt);
	int SetValue(const std::string &key,const std::string &result);
	bool HaveData(const std::string &se);
  int LoadOffPage(const OffPage & page);
  int Flush2OffPage(OffPage & page);
  std::string GetHeadInfo();
  std::string GetAttributeInfo();

  std::map<std::string,std::string> content_map_;
  std::map<std::string,std::string> attribute_map_;
};

#endif

