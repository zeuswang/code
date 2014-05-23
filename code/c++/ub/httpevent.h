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
    virtual int get_head_length(char * buf,int len);
    virtual int get_body_length(char * buf,int len);

    virtual int read_head_done(char * buf,int len){};
    virtual int read_done(char * buf,int head_len,int body_len){};
    virtual int write_done(){};
    virtual void error_handle(){};

    int get_headlen();
    int get_bodylen();
    int is_finished();


protected :

    int http_find_crlfcrlf(char *,int); 
    int get_content_length(char * ,int);
};

}; // namespace ub



#endif /* HTTPEVENT_H_ */

