#include "sstring.h"
namespace sogou_string {
int RemoveNonPrintingCharacters(std::string& str)
{
	for (int i=1;i<32;i++){
		StrReplace(str,(char)i,""); 
	}
	return 0;
}
int StrReplace(std::string& src,const std::string& word1,const std::string& word2)
{
	int pos =0;
	pos=src.find(word1,pos);
	while (pos!=std::string::npos){
		src.replace(pos,word1.length(),word2);
		pos = src.find(word1,pos);
	}
	return 0;
}
int  StrReplace(std::string& src,const char char1,const std::string& word2)
{
	int pos =0;
	pos=src.find(char1,pos);
	while (pos!=std::string::npos){
		src.replace(pos,1,word2);
		pos = src.find(char1,pos);
	}
	return 0;
}
int SplitToMap( const std::string& src,
                const std::string& word_split,
                const std::string& value_split,
                std::map<std::string,std::string>& map)
{
	std::string name;
	std::string value;
	size_t pos=0;
	size_t loc=0;
	size_t pos2=0;
	map.clear();
	if (src.find(word_split)==0)
		pos = pos +word_split.length();
	while (1){

		loc = src.find(value_split, pos);
		if ( loc == std::string::npos )
			break;
		pos2= src.find(word_split, loc);
		if (pos2 == std::string::npos)
			pos2 = src.length() -1;
		else 
			pos2  = pos2 -1;
		name=src.substr(pos,loc-pos);
		value = src.substr(loc+1,pos2-loc);
	 	pos=pos2+2;

	 	if (name.find(word_split)!=std::string::npos)
	 	{
	 		int loc1 = name.find(word_split);
	 		name = name.substr(loc1+1);
	 	}

	 	ToLowerCase(name);
		map.insert(std::pair<std::string,std::string>(name,value));
		//WEBSEARCH_DEBUG((LM_DEBUG, "name: %s, value: %s\n",name.c_str(), value.c_str()));
		//fprintf(stderr, "name:%s, value:%s\n",name.c_str(), value.c_str());
	}
	return 0;
}

void ToLowerCase(std::string& str)
{
	transform(str.begin(),str.end(),str.begin(),::tolower);
}

void ToUpperCase(std::string& str)
{
	transform(str.begin(),str.end(),str.begin(),::toupper);
}


int Split2List( const std::string& src,
                const std::string& word_split,
                std::vector<std::string>& list)
{
	std::string value;
	size_t pos=0;
	size_t pos2=0;
	list.clear();
	if (src.find(word_split)==0)
		pos = pos +word_split.length();
	while (1){
    if (pos >= src.length())
      break;
		pos2= src.find(word_split, pos);
		if (pos2 == std::string::npos)
			pos2 = src.length() -1;
		else 
			pos2  = pos2 -1;
		value = src.substr(pos,pos2- pos +1);
  		list.push_back(value);
	 	pos=pos2 + word_split.length() +1;

		//WEBSEARCH_DEBUG((LM_DEBUG, "name: %s, value: %s\n",name.c_str(), value.c_str()));
		//printf("name:%s, value:%s\n",name.c_str(), value.c_str());
	}
	return 0;


}

int StringSplit(char chr,char * src,uint32_t dstlimit,char ** dstlist,uint32_t * dstnum)
{
	char * ptab;
	ptab = src;
	*dstnum = 0;
	if(*ptab != 0)
		dstlist[(*dstnum)++] = ptab;
	ptab = strchr(ptab,chr);
	while(ptab != NULL && *dstnum < dstlimit){
		*ptab++ = '\0';
		dstlist[(*dstnum)++] = ptab;
		ptab=strchr(ptab,chr);
	}
	return 0;
}
}
