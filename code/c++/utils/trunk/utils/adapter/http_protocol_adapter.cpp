#include "http_protocol_adapter.h"
#include <utils/sstring.h> 

std::string HttpProtocolAdapter::GetContentParameter(const std::string& key)
{
	std::map<std::string,std::string>::const_iterator entry = content_map_.find(key);
	
	if (entry != content_map_.end()) {
		return entry->second;
	} else {
		return "";
	}

}

int HttpProtocolAdapter::SetHeaderParameter(const std::string& key,const std::string& value)
{
	return 0;
}

int HttpProtocolAdapter::SetContentParameter(const std::string& key,const std::string& value)
{
	
	content_map_.insert(std::map<std::string,std::string>::value_type(key, value));
	return 0;
}

int HttpProtocolAdapter::PutRequest()
{
	if (conn_==NULL)
		return -1;
	if (conn_->GetFlag()& ERROR_SOCKET)
		return -1;
	int ret=0;
	std::string msg="";
	for (std::map<std::string,std::string>::const_iterator it = content_map_.begin(); it != content_map_.end(); ++it)
	{
		msg += "&"+it->first+"="+sogou_utils::UrlEncode(it->second);
	}
	char buf[32];
	snprintf(buf,32,"%d",msg.length());
	httpheader_ = "";
	httpheader_+= "POST / HTTP/1.0\r\n";
	httpheader_+= "Content-Type: application/qdb\r\n";
	httpheader_+="Content-Length: "+std::string(buf)+"\r\n\r\n";

	msg=httpheader_ +msg;
	timeval t1={0,50000};
	conn_->LockSelf();
	if (conn_->WriteTimeout((void*)msg.c_str(),msg.length(),&t1)){
		conn_->SetFlag(ERROR_SOCKET);
		ret=-1;
	}
	conn_->SetFlag(conn_->GetFlag()| READABLE_SOCKET);
	conn_->UnLockSelf();
	return ret;
}

int HttpProtocolAdapter::GetRequest()
{
	timeval timeout = { 0, HTTP_RECEIVE_TIMEOUT * 1000 };
	int length,source;
	//worker->state=START;
	int ret=0;
	conn_->LockSelf();
	if (!(conn_->GetFlag()&READABLE_SOCKET)){
		ret =-1;
		fprintf(stderr,"HttpProtocolAdapter::GetRequest [GetFlag() is not READABLE_SOCKET]\n");
		goto OUT;
	}
		
	if (read_http_header_timeout(httpheader_, &timeout) == 0 )
	{
		if(httpheader_.length() >0 && GetInfo(httpheader_,length,source) == 0)
		{		
			if( read_http_content_timeout(content_,length,&timeout) == length )
			{
				fprintf(stderr,"head:%s\ncontent:%s\n",httpheader_.c_str(),content_.c_str());
				sogou_string::SplitToMap(content_,"&","=",content_map_);
				sogou_string::SplitToMap(httpheader_,"\n",":",header_map_);
			}
		}
		else{
			fprintf(stderr,"HttpProtocolAdapter::GetRequest [Package Invalid (%s)]\n",httpheader_.c_str());
			//return 0;
			ret = -1;
		}		
	}else {
		fprintf(stderr,"HttpProtocolAdapter::GetRequest [read_http_header_timeout() error]\n");
		ret=-1;
	}
OUT:
	if (ret ==0){
		conn_->SetFlag((conn_->GetFlag()&~READABLE_SOCKET) | WRITABLE_SOCKET);
	}else{ 
		fprintf(stderr,"HttpProtocolAdapter::PutResult() set socket error(fd=%d)\n",conn_->GetFd());
		conn_->SetFlag(conn_->GetFlag() | ERROR_SOCKET);
	}
	conn_->UnLockSelf();
	
	//fprintf(stderr,"HttpProtocolAdapter::GetRequest [disconnect error socket(%d)]\n", conn_->GetFd());
	return ret;
}

int HttpProtocolAdapter::GetResult()
{
	timeval timeout = { 0, HTTP_RECEIVE_TIMEOUT * 1000 };
	int length,source;
	//worker->state=START;
	int ret=0;
	unsigned int http_ret=0;
	conn_->LockSelf();
	if (!(conn_->GetFlag()&READABLE_SOCKET)){
		ret =-1;
		fprintf(stderr,"HttpProtocolAdapter::GetRequest [GetFlag() is not READABLE_SOCKET]\n");
		goto OUT;
	}
	if (read_http_header_timeout(httpheader_, &timeout) == 0 )
	{
		if(httpheader_.length() >0 && GetResultInfo(httpheader_,length,ret,source) == 0)
		{		
			content_.clear();
			if( read_http_content_timeout(content_,length,&timeout) == length )
			{
				fprintf(stderr,"head:%s\ncontent:%s\n",httpheader_.c_str(),content_.c_str());
				sogou_string::SplitToMap(content_,"&","=",content_map_);
				sogou_string::SplitToMap(httpheader_,"\n",":",header_map_);
			}
		}
		else{
			fprintf(stderr,"HttpProtocolAdapter::GetRequest [Package Invalid (%s)]\n",httpheader_.c_str());
			//return 0;
			ret = -1;
		}		
	}else {
		fprintf(stderr,"HttpProtocolAdapter::GetRequest [read_http_header_timeout() error]\n");
		ret=-1;
	}
OUT:
	if (ret ==0){
		conn_->SetFlag((conn_->GetFlag()&~READABLE_SOCKET) | WRITABLE_SOCKET);
	}else{ 
		fprintf(stderr,"HttpProtocolAdapter::PutResult() set socket error(fd=%d)\n",conn_->GetFd());
		conn_->SetFlag(conn_->GetFlag() | ERROR_SOCKET);
	}
	conn_->UnLockSelf();
	
	//fprintf(stderr,"HttpProtocolAdapter::GetRequest [disconnect error socket(%d)]\n", conn_->GetFd());
	return ret;

}
int HttpProtocolAdapter::PutResult()
{
	//int buf_len)
	int ret = 0;
	timeval timeout = { 0, HTTP_REPLY_TIMEOUT * 1000 };
	conn_->LockSelf();
	if (!(conn_->GetFlag()&WRITABLE_SOCKET)){
		ret =-1;
		goto OUT;
	}

	//std::string httpheader;
	httpheader_.clear();
	if(generate_http_header(httpheader_,result_.length()) <=0 )
	{
		goto OUT;
	}

	if(conn_->WriteTimeout((void *)httpheader_.c_str(),httpheader_.length(),&timeout) != (int )httpheader_.length())
		ret = -2;
	else if(conn_->WriteTimeout((void*)result_.c_str(), result_.length(), &timeout) != (int) result_.length())
		ret = -3;	
OUT:
	if (ret ==0){
		conn_->SetFlag((conn_->GetFlag()&~WRITABLE_SOCKET) | READABLE_SOCKET);
	}else{
		fprintf(stderr,"HttpProtocolAdapter::PutResult() set socket error(fd=%d)\n",conn_->GetFd());
 		conn_->SetFlag(conn_->GetFlag() | ERROR_SOCKET);
	}
	conn_->UnLockSelf();
	return ret;
}

int HttpProtocolAdapter::GetInfo(std::string header, int& length,int& source)
{
	char uri[2048];
	unsigned int http_version[2];
	const char * buf = header.find("POST") + header.c_str();
	if (sscanf(buf, "POST / HTTP/%u.%u", &(http_version[0]),&(http_version[1])) != 2)
//	if (sscanf(buf, "POST /%2040s HTTP/%u.%u",uri, &(http_version[0]),&(http_version[1])) != 3)
	{
		fprintf(stderr,"HttpProtocolAdapter::GetInfo get post error!!\n");
		return -1;
	}
	buf = header.c_str() + header.find("Content-Length:");
	if(sscanf(buf,"Content-Length: %d",&length) != 1)
	{
		fprintf(stderr,"HttpProtocolAdapter::GetInfo get content length error!!\n");
		return -1;
	}
	source = 0;
	return 0;
}
int HttpProtocolAdapter::GetResultInfo(std::string header, int& length,int& ret,int& source)
{
	char uri[2048];
	unsigned int http_version[2];
	unsigned int http_ret;
	const char * buf = header.find("HTTP") + header.c_str();
	if (sscanf(buf, "HTTP/%u.%u %u", &(http_version[0]),&(http_version[1]),&http_ret) != 3)
//	if (sscanf(buf, "POST /%2040s HTTP/%u.%u",uri, &(http_version[0]),&(http_version[1])) != 3)
	{
		fprintf(stderr,"HttpProtocolAdapter::GetInfo get post error!!\n");
		return -1;
	}
	buf = header.c_str() + header.find("Content-Length:");
	if(sscanf(buf,"Content-Length: %d",&length) != 1)
	{
		fprintf(stderr,"HttpProtocolAdapter::GetInfo get content length error!!\n");
		return -1;
	}
	source = 0;
	return 0;
}

int HttpProtocolAdapter::read_http_content_timeout(std::string &content, int buf_len, timeval *timeout)
{
	char buf[READ_BUF_SIZE+1];
	int n,len;
	std::string & _buffer = read_buf_;
	int left = buf_len - _buffer.length();

	if(left<0)
	{
		content = _buffer.substr(0,buf_len);
		_buffer = _buffer.substr(buf_len);
		return buf_len;
	}
	content = _buffer;
	_buffer = "";

	while (left > 0)
	{
		len = left>READ_BUF_SIZE?READ_BUF_SIZE:left;
                if ((n = conn_->ReadTimeoutOnce( buf, len,timeout)) <= 0)
			return buf_len - left;

		buf[n] = '\0';
		content += buf;
		left -= n;
	}

	return buf_len;
}

int HttpProtocolAdapter::read_http_header_timeout(std::string &http, timeval *timeout)
{
	char buf[READ_BUF_SIZE + 1];
	int n,pos;	
	std::string&  _buffer = read_buf_;
	//int fd = conn_->GetFd();

	while(1)
	{
		if ((pos = _buffer.find("\r\n\r\n")) >= 0)
		{
			http = _buffer.substr(0, pos);
			_buffer = _buffer.substr(pos + 4);
			return 0;
		}
		if((n = conn_->ReadTimeoutOnce(buf,READ_BUF_SIZE,timeout))<0){
			fprintf(stderr,"HttpProtocolAdapter::read_http_header_timeout conn_->ReadTimeout error\n");
			return -1;
		}
		buf[n] = '\0';
		_buffer += buf;
	}
	fprintf(stderr,"HttpProtocolAdapter::read_http_header_timeout error\n");
	return -1;
}


int HttpProtocolAdapter::generate_http_header(std::string& res,int len)
{
	char buf[HTTP_HEADER_MAX_LENGTH];

	//generate header
	int ret = snprintf(buf, HTTP_HEADER_MAX_LENGTH, "HTTP/1.1 %d %s\r\n",200,"OK");
	res = buf;

	// 生成响应报文时间戳
	time_t t;
	struct tm tmp_time;
	time(&t);
	char timestr[64];
	strftime(timestr, sizeof(timestr), "%a, %e %b %Y %H:%M:%S %Z", gmtime_r(&t, &tmp_time));
//	ret = snprintf(buf,HTTP_HEADER_MAX_LENGTH,"Date:%s\r\n",timestr);
//	res += buf;

	//add content length
	ret = snprintf(buf,HTTP_HEADER_MAX_LENGTH,"Content-Length:%d\r\n",len);
	res += buf;

	snprintf(buf,HTTP_HEADER_MAX_LENGTH,"\r\n");
	res += buf;

	return res.length();
}
/*
int Http_Server::send_result(Connection* conn,const void * buf, int buf_len)
{
	int ret = 0;
	timeval timeout = { 0, HTTP_REPLY_TIMEOUT * 1000 };

	std::string httpheader;
	if(generate_http_header(httpheader,buf_len) <=0 )
	{
		return -1;
	}
	conn->LockSelf();
	if(conn->WriteTimeout(httpheader.c_str(),httpheader.length(),&timeout) != httpheader.length())
		ret = -2;
	else if(conn->WriteTimeout(buf, buf_len, &timeout) != (int)buf_len)
		ret = -3;	
	conn->UnLockSelf();

	return ret;
}

*/

