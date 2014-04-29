#ifndef _SOGOU_HTTP_PROTOCOL_ADAPTER_H_
#define _SOGOU_HTTP_PROTOCOL_ADAPTER_H_
#include "protocol_adapter.h"
#include "sconnection.h"
#include <utils/utils.h>
//#define LISTEN_PORT_1				0x00000001U
//#define ERROR_SOCKET				0x00000010U


#define HTTP_RECEIVE_TIMEOUT				50
#define HTTP_REPLY_TIMEOUT				50
#define SOCKET_SND_BUF_SIZE         (1024*1024)
#define SOCKET_RCV_BUF_SIZE         (1024*1024)
#define READ_BUF_SIZE       512
#define HTTP_HEADER_MAX_LENGTH  1024

class HttpProtocolAdapter:public ProtocolAdapter
{
public:
	HttpProtocolAdapter(Connection* conn):conn_(conn){};
	int GetRequest();
	int GetResult();
	int PutRequest();
	int PutResult();
	
	void SetConnection(Connection* conn){conn_ = conn;};
	void SetResponseHeader();
	void SetResponseContent(const std::string& str){result_ = str;};
	int SetContentParameter(const std::string&, const std::string&);
	int SetHeaderParameter(const std::string& key,const std::string& value);
	std::string GetContentParameter(const std::string& );
	std::string GetResultContent(){return sogou_utils::UrlDecode(content_);};

	int GetInfo(std::string header, int& length,int& source);
	int GetResultInfo(std::string header, int& length,int& ret,int& source);
	int read_http_header_timeout(std::string &http, timeval *timeout);
	int read_http_content_timeout(std::string &content, int buf_len, timeval *timeout);
	int generate_http_header(std::string& res,int len);
	//void SetWordSegmentorInfo(const std::string& query_string,UINT16 terms_count_,tTermRange* term_range);
	
protected:
	Connection * conn_;
	std::string httpheader_;
	std::string content_;
	std::string read_buf_;
	std::string result_;
	std::map<std::string,std::string> content_map_;
	std::map<std::string,std::string> header_map_;

};

#endif 

