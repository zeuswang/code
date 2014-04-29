#ifndef _SSTRING_HPP_
#define _SSTRING_HPP_
#include <string>
#include <map>
#include <vector>
#include <algorithm>
namespace sogou_string {
int StrReplace( std::string& src,const std::string& str1,const std::string& str2);
int StrReplace( std::string& src,const char char1,const std::string& str);
int RemoveNonPrintingCharacters(std::string& str);
int SplitToMap( const std::string& src,
                const std::string& word_split,
                const std::string& value_split,
                std::map<std::string,std::string>& map);

int Split2List( const std::string& src,
                const std::string& word_split,
                std::vector<std::string>& list);
void ToLowerCase(std::string& str);
void ToUpperCase(std::string& str);

}
#endif 

