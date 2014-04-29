#ifndef ODS_DOC_FILTER_H__
#define ODS_DOC_FILTER_H__
#include <string>
#include <vector>

#include <ext/hash_map>
struct _str_hash{
    size_t operator()(const std::string &str)const{
        return __gnu_cxx::hash<const char*>()(str.c_str());
    }   
};

typedef __gnu_cxx::hash_map<std::string ,std::string,_str_hash> HashMap;
typedef __gnu_cxx::hash_map<std::string ,std::string,_str_hash>::iterator It_HashMap;

class interact_info{
public:
    uint16_t typeid_high;
    uint16_t typeid_low;
    std::string _name;
    std::string _value;
};


class InterAct_filter{
public:
    InterAct_filter(){};
    int init(std::string &fileName);//init必须单线程初始化，其他都是可重入的

    static bool parseQuery(std::string &src_query,std::vector<interact_info> &interact_types,std::string &para_name);
	static bool parseSiteQuery(std::string &src_query,std::string &site_type,std::string &para_name);

	bool interact_filter(std::string &url,std::vector<interact_info> &interact_types,std::string &site_type);
    
    ~InterAct_filter(){
    };  
private:
    bool pass_filter(int _value,std::string _url);
	bool pass_site_filter(std::string &site_filt,std::string _url); 
private:
    HashMap filter_map;
};



#endif
