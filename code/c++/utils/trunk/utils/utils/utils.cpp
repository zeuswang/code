#include "utils.h"
#include <map>
#include <Platform/md5/md5_deprecated.h>
#include <utils/debug.h>
namespace sogou_utils {
std::string UrlEncode(const std::string& szToEncode)
{
    std::string src = szToEncode;
    char hex[] = "0123456789ABCDEF";
    std::string dst;
 
    for (size_t i = 0; i < src.size(); ++i)
    {
        unsigned char cc = src[i];
        if (isascii(cc))
        {
            if (cc == ' ')
            {
                dst += "%20";
            }
            else
                dst += cc;
        }
        else
        {
            unsigned char c = static_cast<unsigned char>(src[i]);
            dst += '%';
            dst += hex[c / 16];
            dst += hex[c % 16];
        }
    }
    return dst;
}
static int char2num(char ch)
{ 
	if(ch >= '0' && ch <= '9')
		return (ch - '0'); 
	if(ch >= 'a' && ch <= 'f')
		return (ch- 'a' + 10); 
	if(ch >= 'A' && ch <= 'F')
		return (ch- 'A' + 10); 
	return -1; 
} 

void URLDecode(const std::string input, std::string& output)
{
	output.clear();
	size_t i = 0;
	size_t len = input.size();
	int num1, num2;
	while(i < len)
	{
		char ch = input[i];
		switch(ch)
		{
		case '+':
			output.append(1, ' ');
			i++;
			break;
		case '%':
			if(i+2 < len)
			{
				num1 = char2num(input[i+1]);
				num2 = char2num(input[i+2]);
				if(num1 != -1 && num2 != -1) 
				{
					char res = (char)((num1 << 4) | num2);
					output.append(1, res);
					i += 3;
					break;
				}
			}
			//go through
		default:
			output.append(1, ch);
			i++;
			break;
		}
	}
}
std::string UrlDecode(const std::string& src)
{
	std::string tmp;
	URLDecode(src,tmp);
	return tmp;
}

std::string GetUnixTimeStr()
{
  char buf[64];
  snprintf(buf,64,"%d",time(NULL));
  return buf;
}

std::string MD5_128(const std::string& str) 
{
	if (str.empty()){
		return "";
	}

	platform::md5_state_t state;
	unsigned char digest[16];
	platform::md5_init(&state);
	platform::md5_append(&state, (const platform::md5_byte_t *)str.c_str(),str.length());
	platform::md5_finish(&state, digest);	
	std::string md5_str;
	md5_str.reserve(32);
	for (int i=0;i<16;i++){
		char buf[10];
		snprintf(buf,10,"%02x",digest[i]);
		md5_str += std::string(buf);
	}

	/*fprintf(stderr,"dig is%s \n ",md5_str.c_str());
	fprintf(stderr,"digest is \n ");
	for (int i=0;i<16;i++){
		fprintf(stderr,"%x",digest[i]);

	}
	fprintf(stderr,"\ndigest done \n ");*/

	return md5_str;
#if 0

	if (str.empty()){
		return "";
	}
	char tmp[128];
	std::string md5_str;
	platform::md5_long_128 id_128=platform::getSign128(str.c_str(),str.size());
	//snprintf(tmp,128,"%llu",id_64);
	//WEBSEARCH_DEBUG((LM_DEBUG, "vs:%s\n",vrSign.c_str()));
	//return std::string(tmp);

	/*fprintf(stderr,"dig is%s \n ",md5_str.c_str());
	fprintf(stderr,"digest is \n ");
	for (int i=0;i<16;i++){
		fprintf(stderr,"%x",digest[i]);

	}
	fprintf(stderr,"\ndigest done \n ");*/

	std::string md5_str;
	md5_str.reserve(32);
	for (int i=0;i<16;i++){
		char buf[10];
		snprintf(buf,10,"%02x",id_128[i]);
		md5_str += std::string(buf);
	}
	return md5_str;
#endif
}
std::string MD5_128(void * key,int key_len) 
{
	if (key ==NULL||key_len ==0){
		return "";
	}

	platform::md5_state_t state;
	unsigned char digest[16];
	platform::md5_init(&state);
	platform::md5_append(&state, (const platform::md5_byte_t *)key,key_len);
	platform::md5_finish(&state, digest);	
	std::string md5_str;
	md5_str.reserve(32);
	for (int i=0;i<16;i++){
		char buf[10];
		snprintf(buf,10,"%02x",digest[i]);
		md5_str += std::string(buf);
	}
	return md5_str;
}

std::string GetLocalIp()
{
  struct ifreq req; 
  int sock; 
  char *dn_or_ip; 
  char current_ip[1024];

  sock = socket(AF_INET, SOCK_DGRAM, 0); 
  strncpy(req.ifr_name, "eth0", IFNAMSIZ); 

  if ( ioctl(sock, SIOCGIFADDR, &req) < 0 ) { 
    	_err("Open ioctl error: %s Cann't get current IP\n", strerror (errno)); 
    	sprintf(current_ip,"Get currentip error %d",errno); 
  } 
  else 
  { 
  	dn_or_ip = (char *)inet_ntoa(*(struct in_addr *) &((struct sockaddr_in *) &req.ifr_addr)->sin_addr); 
  	strcpy(current_ip,dn_or_ip); 
  } 
  //fprintf(stderr,"ConnectionServer::Open Current IP:%s\n", current_ip); 
  shutdown(sock, 2); 
  ::close(sock);
  
  return current_ip;
}


}
