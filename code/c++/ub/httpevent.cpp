
 
 
/**
 * @file HttpEvent.cpp
 * @author wangwei115120@sogou-inc.com
 * @date 2014-05-01
 * @brief 
 *  
 **/
#include "httpevent.h"
#include <string.h>
#include <errno.h>

namespace ub {

static const char G_HTTP_CRLF[] = "\r\n"; /**<   \r\n     */
static const char G_HTTP_CRLFCRLF[] = "\r\n\r\n"; /**<   http头结束标志     */
static const int G_HTTP_INT_CRLFCRLF = *(int *)G_HTTP_CRLFCRLF; /**<   int类型结束标志     */
static const char G_HTTP_CONTENTLEN[] = "Content-Length"; /**<   content-length字段名     */


/**
 * @brief 跳过空格
**/
static char * skip_http_blank(char * str) {
	if (NULL == str) {
		return NULL;
	}
	char * ret = str;
	while (*ret && (*ret == ' ')|| (*ret == '\t')) {
		ret++;
	}
	if (*ret) {
		return ret;
	}
	return NULL;
}

/**
 * @brief 查找字段
**/
static char * http_find_field_nocase(char *src, int reallen, const char *dest,int destlen) {
	if (NULL == src || NULL == dest) {
		return NULL;
	}
	char * srcstr = skip_http_blank(src);
	if (!srcstr) {
		return NULL;
	}
	int bound_check = 0;
	for (;*srcstr;) {
		if (0 == strncasecmp(srcstr, dest, destlen)) {
			char * mh = skip_http_blank(srcstr + destlen);
			if (NULL ==mh) {
				return NULL;
			}
			if (':' == *mh) {
				if (mh - src >= reallen) {
                    return NULL;
				}
				return mh;
			}
		}
		//_debug("http_find_field_nocase: srcstr:%s\n", srcstr);
		bound_check = srcstr + strlen(G_HTTP_CRLF) - src;
		if (bound_check >= reallen) {
			return NULL;
		}		

		char * ctl = strstr(srcstr, G_HTTP_CRLF);
		if (NULL == ctl) {
			_debug("http_find_field_nocase: %s do not have %s\n", src, dest);
			return NULL;
		}
		srcstr = ctl +2;
	}
	return NULL;
}
int HttpEvent::get_head_length(char * buf,int len)
{
    int head_len = http_find_crlfcrlf(buf,len);
    if (head_len >0){
		return head_len;
    }
    return -1;
}
int HttpEvent::get_body_length(char *buf,int len)
{

	int body_len = get_content_length(buf,len);
    if (body_len>0)
		return body_len;
	else
		return -1;
}
int HttpEvent::http_find_crlfcrlf(char * buf,int len) {
	int length = len -3;
	if (0 > len) {
		length = 0;
	}
	char * head = buf;
	//rdlen = rdlen -3;
	for (int i=0; i< length; ++i, ++head) {
		if ( G_HTTP_INT_CRLFCRLF == (*(int *)head)) {
			return i+4;
		}
	}
	return -1;
}
int  HttpEvent::get_content_length(char * buf,int len) {
//	char * ctl = strstr(sock_data.read_buf, G_HTTP_CONTENTLEN);http_find_field_nocase
//	if (NULL == ctl) {
//		UBEVENT_TRACE(this,"HttpEvent(http_get_contentlength): no Content-Length field.");
//		goto NOLENGTH;
//	}
//	mh = strchr(ctl, ':');
	char *endptr = NULL;
	long long num;
	char * line;
	char * mh = http_find_field_nocase(buf, len, G_HTTP_CONTENTLEN, strlen(G_HTTP_CONTENTLEN));
	if (NULL == mh) {
		//_debug("HttpEvent(http_get_contentlength): no Content-Length field.\n");//Content-Length, lack ':'
		goto NOLENGTH;
	}
	line = strstr(mh, G_HTTP_CRLF);//ctl
	if (NULL == mh || (line-mh <=0)) {
		//UBEVENT_TRACE(this,"HttpEvent(http_get_contentlength): Content-Length, lack number value");
		goto NOLENGTH;
	}
	*line = 0;
	errno = 0;
	num = strtoll(mh+1, &endptr, 10);
	if (errno == ERANGE) {
		//UBEVENT_TRACE(this,"HttpEvent(http_get_contentlength): Content-Length, out of range");
		*line = '\r';
		goto NOLENGTH;
	}
	if(*endptr != '\0'){
		//UBEVENT_TRACE(this,"HttpEvent(http_get_contentlength): Content-Length, invalid number(%s)", mh+1);
		*line = '\r';
		goto NOLENGTH;
	}
	*line = '\r';
	//_debug("HttpEvent(http_get_contentlength): set body length(%lld)\n", num);

	return num;
NOLENGTH:
	return 0;
}



int HttpEvent::get_headlen() {
	return _fheader_length;
}
int HttpEvent::get_bodylen() {
	return _fbody_length;
}

int HttpEvent::is_finished() {
	return _fbody_readdone >= _fbody_length;
}


}//namespace ub


