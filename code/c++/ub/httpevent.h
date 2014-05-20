/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * 
 **************************************************************************/
 
 
 
/**
 * @file HttpEvent.h
 * @author wangwei115120@sogou-inc.com
 * @date 2014-05-01
 * @brief 
 *  
 **/


#ifndef HTTPEVENT_H_
#define HTTPEVENT_H_

#include <ub/ubevent.h>
#include "UbSockEvent.h"

namespace ub {



class HttpEvent : public UbEvent {
public:
    int post();


public:
    virtual int check_header(char * buf,int len);

    virtual int read_head_done(char * buf,int len)=0;
    virtual int read_done(char * buf,int head_len,int body_len)=0;

    int get_headlen();
    int get_bodylen();
    int is_finished();


protected :
    int _fheader_length;   /**<  header length      */
    int _fsection_size;		  /**<  section size      */
    int _fbody_length;		  /**<  http body length(Content-Length)      */
    int _fbody_readdone;		  /**<   http body read     */

    int http_find_crlfcrlf(int rdlen); 
    int get_content_length();
};

} // namespace ub



#endif /* HTTPEVENT_H_ */

