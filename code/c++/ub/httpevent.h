/**
 * @file HttpEvent.h
 * @author wangwei115120@sogou-inc.com
 * @date 2014-05-01
 * @brief 
 *  
 **/


#ifndef HTTPEVENT_H_
#define HTTPEVENT_H_

#include "event.h"

namespace ub {



class HttpEvent : public UbEvent 
{

public:
    virtual int check_header(char * buf,int len);

    virtual int read_head_done(char * buf,int len)=0;
    virtual int read_done(char * buf,int head_len,int body_len)=0;
    virtual int write_done()=0;

    int get_headlen();
    int get_bodylen();
    int is_finished();


protected :

    int http_find_crlfcrlf(char *,int); 
    int get_content_length(char * ,int);
};

}; // namespace ub



#endif /* HTTPEVENT_H_ */

