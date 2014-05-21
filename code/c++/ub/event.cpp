#include "event.h"
namespace ub
{


int UbEvent::addRef()
{
    int ret=0;
    pthread_mutex_lock(&_mutex);
    _ref ++;
    ret =_ref;
    pthread_mutex_unlock(&_mutex);
    return ret;
}
int UbEvent::delRef()
{
    int ret=0;
    pthread_mutex_lock(&_mutex);
    _ref --;
    ret =_ref;
    pthread_mutex_unlock(&_mutex);
    return ret;
}

int UbEvent::getRefCnt()
{

    int ret=0;
    pthread_mutex_lock(&_mutex);
    ret =_ref;
    pthread_mutex_unlock(&_mutex);
    return ret;

}

bool UbEvent::isTimeout() 
{
    struct timeval now;    
    gettimeofday(&now, NULL);

    now = tvSub(now,_start_time);
    if (tvTv2Ms(now)> _tv)
        return true;
    else 
        return false;
}
void UbEvent::setTimeout_ms(int tv) 
{
    _tv =tv; 
}

void UbEvent::finish()
{
    setStatus(IEvent::DONE);
}
int UbEvent :: post() 
{
    int ret=0;
    NetReactor * r = reactor();
    if (NULL != r) {
        ret = r->post(this);
    } else {
        ret = -1;
    }
    
	return ret;
}


void UbEvent::callback()
{
/*
     TIMEOUT = 1UL,  //超时事件
    IOREADABLE = 1UL<<1UL,  //io可读事件
    IOWRITEABLE = 1UL<<2UL, //io可写事件
    SIGNAL = 1UL<<3UL,  //系统信号事件
    CLOSESOCK = 1UL<<4UL,   //对端关闭句柄
    ERROR = 1UL<<5UL,   //未知错误
    CPUEVENT = 1UL<<6UL,    //cpu事件
    CANCELED = 1UL<<7UL,    //事件被取消
    SOCKERROR = 1UL<<8UL,   //socket上发生错误事件*/


    int status = status();
    int events = events();

    if (status == IEvent::ERROR )
    {
        goto ERROR;
    }

    if (events & IEvent::IOREADABLE)
    {

        if ( read() <0)
            goto ERROR;

    }
    if (events & IEvent::IOWRITEABLE)
    {
        if (write() <0)
            goto ERROR;
    }

    if (status() == IEvent::READY )
    {
        if ( 0 != post())
        {
            goto ERROR;
        }
    }
    
    return ;
    
ERROR:
    _err("UbEvent::callback() : post error\n");
    event_error_callback();


}

void * UbEvent::ub_event_malloc(size_t size)
{
	void * result;
	/*if (mgr_) {
		result = mgr_->malloc(size);
	} else {
		result = malloc(size);
	}*/

	result = malloc(size);
	if(NULL == result) {
		_err("allocate memory(size=%lu) Failed, in ub_event_malloc()\n", (unsigned long)size);
		return NULL;
	}
	return result;
}

void UbEvent::ub_event_free(void * mem, size_t msize) {
	/*if (mgr_) {
		mgr_->free(mem, msize);
	} else {
		free(mem);
	}*/
	free(mem);
}


const char * UbEvent :: get_read_buffer(unsigned int size, int copy) {
	char * retbuf = NULL;
	if (0 == size) {
		return sock_data.read_buf;
	}
	/*if (sock_data.ext_readbuf) {
		if (size > sock_data.read_buf_len) {
			UBEVENT_WARNING(this, "ext_readbuf_size(%d) is smaller than the needed readbuf(%d), in UbEvent::get_read_buffer()",
				sock_data.read_buf_len, size);
			retbuf =  NULL;
			goto END;
		}
		retbuf =   sock_data.read_buf;
		goto END;
	}*/

	if ((0 < sock_data.max_readbuf_size) && (size > sock_data.max_readbuf_size)) {
		_err("The needed readbuf(%d) is bigger than max_read_bufsize(%d) setting in configure file\n",
				size, sock_data.max_readbuf_size);
		retbuf =  NULL;
		goto END;
	}

	if (size > sock_data.read_buf_len) {
		retbuf = (char *)ub_event_malloc(size);
		if ((0 != copy) && (NULL != retbuf)) {
			memcpy(retbuf, sock_data.read_buf, sock_data.read_buf_used);
		}
		if (NULL != retbuf) {
			if (sock_data.small_readbuf != sock_data.read_buf) {
				ub_event_free(sock_data.read_buf, sock_data.read_buf_len);
			}
			sock_data.read_buf = retbuf;
			sock_data.read_buf_len = size;
		}
	} else {
		retbuf = sock_data.read_buf;
	}
END:
	if (NULL != retbuf) {
	} else {
		setEvents(IEvent::ERROR);
		//set_sock_status(UbEvent::STATUS_READ | UbEvent::STATUS_MEMERROR);
	}
	return retbuf;
}

char * UbEvent :: get_write_buffer(unsigned int size)
{
	char * retbuf = NULL;

	if ((0 < sock_data.max_writebuf_size) && (size > sock_data.max_writebuf_size)) {
		_err("The needed writebuf(%d) is bigger than max_write_bufsize(%d) setting in configure file\n",
				size, sock_data.max_writebuf_size);
		retbuf =  NULL;
		goto END;
	}

	if (size > sock_data.write_buf_len) {
		retbuf = (char *)ub_event_malloc(size);
		if (NULL != retbuf) {
			if (sock_data.small_writebuf != sock_data.write_buf) {
				ub_event_free(sock_data.write_buf, sock_data.write_buf_len);
			}
			sock_data.write_buf = retbuf;
			sock_data.write_buf_len = size;
		}
	} else {
		retbuf = sock_data.write_buf;
	}
END:
	if (NULL != retbuf) {
		sock_data.write_buf_used = size;
	} else {
		setEvents(IEvent::ERROR);
		//set_sock_status(UbEvent::STATUS_WRITE | UbEvent::STATUS_MEMERROR);
	}
	return retbuf;
}


int UbEvent :: read_buffer_process(int len)
{
    sock_data.read_buf_used += len;
    if (sock_data.read_buf_used == sock_data.read_buf_len) {
        if (NULL == get_read_buffer(sock_data.read_buf_len * 2, 1)) {
        //setResult(IEvent::ERROR);
        //set_sock_status(UbEvent::STATUS_READ | UbEvent::STATUS_MEMERROR);
        _err("HttpEvent(http_readheader_process) : get readbuf error\n");
        return -1;
        }
    }
     
    if (_io_status == FOR_HEAD)
    {
        _fbody_length = check_header(sock_data.read_buf,sock_data.read_buf_len);
        if (_fbody_length >0)
        {

            if (NULL == get_read_buffer(_fbody_length+_fheader_length + 1, 1)) {
                    //setResult(IEvent::ERROR);
                    //set_sock_status(UbEvent::STATUS_READ | UbEvent::STATUS_MEMERROR);
                    _err("HttpEvent(body_read) : get readbuf error\n");
                    //event_error_callback();
                return -1;
            }
            _io_status = FOR_BODY;
            _fbody_readdone = sock_data.read_buf_used - _fheader_length;            
        }
    }else{
    
        _fbody_readdone = sock_data.read_buf_used - _fheader_length;
    
        if (_fbody_readdone >= _fbody_length) {
            read_done(HttpEvent);
        }
    }

    return 0;
}

int UbEvent :: read()
{

    //ret = ::read(this->handle(), ((char *)(_buf)) + _readcnt, _cnt - _readcnt);

	//ret = recv(this->handle(), sock_data.read_buf + sock_data.read_buf_used ,
	//		sock_data.read_buf_len - sock_data.read_buf_used, MSG_DONTWAIT);

    char* _buf = sock_data.read_buf + sock_data.read_buf_used;
    unsigned int len = sock_data.read_buf_len - sock_data.read_buf_used;
    int ret = ::recv(handle(), _buf, len, MSG_DONTWAIT);

    if (ret>0)
    {
        return read_buffer_process(ret);
    }

    /*if (errno == EAGAIN) {
        return 0;
    }*/

    setEvents(IEvent::ERROR);
    return -1;
}

int UbEvent::write(char * buf,int len)
{
    char* _buf = sock_data.write_buf + sock_data.write_buf_used;
    unsigned int len = sock_data.write_buf_len - sock_data.write_buf_used;

    //ret = ::write(this->handle(), ((char *)(_buf)) + _readcnt, _cnt - _readcnt);
    int ret = ::send(this->handle(), _buf, len, MSG_DONTWAIT);


    if (ret > 0)
    {
        sock_data.read_buf_used += ret;
        if (sock_data.read_buf_used>=sock_data.read_buf_len)
            write_done();

        return 0;
    }
    
    setEvents(IEvent::ERROR);
    return -1;
}

void UbEvent :: release_read_buf() {
	if ((sock_data.small_readbuf != sock_data.read_buf) && (NULL != sock_data.read_buf)){
		ub_event_free(sock_data.read_buf, sock_data.read_buf_len);
	}
	sock_data.read_buf = sock_data.small_readbuf;
	sock_data.read_buf_len = sizeof(sock_data.small_readbuf);
	sock_data.read_buf_used = 0;
}
void UbEvent :: release_write_buf() {
	if ((sock_data.small_writebuf != sock_data.write_buf) && (NULL != sock_data.write_buf)) {
		ub_event_free(sock_data.write_buf, sock_data.write_buf_len);
	}
	sock_data.write_buf = sock_data.small_writebuf;
	sock_data.write_buf_len = sizeof(sock_data.small_writebuf);
	sock_data.write_buf_used = 0;
}

bool UbEvent::release()
{
	if (delRef()<= 0) {
		/*bsl::mempool *p = _pool;
		if (p) {
			this->~EventBase();
			p->free(this, sizeof(*this));
		} else {
			delete this;
		}*/
		delete this;
		return true;
	} else {
		return false;
	}
}
void UbEvent :: event_error_callback() 
{
    error_handle();
    _err("now in the UbEvent::event_error_callback(), close sockfd\n");
    if (0 <= handle() ) {
        ::close(handle());
    }
    this->setHandle(-1);
	
}

UbEvent :: ~UbEvent() {
    release_read_buf();
    release_write_buf();
    if (0 <= handle()) {
        ::close(handle());
    }
    pthread_mutex_destroy(&_mutex);
}


};

