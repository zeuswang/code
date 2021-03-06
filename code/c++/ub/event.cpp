#include "event.h"
#include "ievent.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
namespace ub
{
void EQueue::push(IEvent *ev)
{
	if (ev) {
		if (_end) {
			ev->setNext(NULL);
			_end->setNext(ev);
			ev->setPrevious(_end);
			_end = ev;
		} else {
			ev->setPrevious(NULL);
			ev->setNext(NULL);
			_begin = ev;
			_end = ev;
		}
	}
}

IEvent *EQueue::pop() {
	IEvent *p = _begin;
	if (_begin) {
		_begin = _begin->next();
		if (_begin == NULL) {
			_end = NULL;
		} else {
			_begin->setPrevious(NULL);
		}
	}
	if (p) {
		p->setNext(NULL);
		p->setPrevious(NULL);
	}
	return p;
}

void EQueue::erase(IEvent *ev) {
	if (ev == NULL) { return; }
	if (ev == _begin) {
		_begin = _begin->next();
		if (_begin == NULL) {
			_end = NULL;
		} else {
			_begin->setPrevious(NULL);
		}
	} else if (ev == _end) {
		_end = _end->previous();
		if (_end == NULL) {
			_begin = NULL;
		} else {
			_end->setNext(NULL);
		}
	} else {
		if (ev->previous()) {
			ev->previous()->setNext(ev->next());
		}
		if (ev->next()) {
			ev->next()->setPrevious(ev->previous());
		}
	}
	ev->setNext(NULL);
	ev->setPrevious(NULL);
}


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
    int stat = status();
    int event = events();

    if (stat == IEvent::ERROR )
    {
        goto ERROR;
    }

    if (event & IEvent::IOREADABLE)
    {

    	//_debug("get read event\n");
        if ( read_func() <0)
		{
    		_err("read_func() error\n");
            goto ERROR;
		}

    }
    if (event & IEvent::IOWRITEABLE)
    {
    	//_debug("get write event\n");
        if (write_func() <0)
		{
    		_err("write_func() error\n");
            goto ERROR;
		}
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
		int t =type();
		setType(t| IEvent::IOWRITEABLE);
	} else {
		setEvents(IEvent::ERROR);
		//set_sock_status(UbEvent::STATUS_WRITE | UbEvent::STATUS_MEMERROR);
	}
	return retbuf;
}


int UbEvent :: read_buffer_process(int len)
{
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
		//_debug("FOR_HEAD\n");
        _fheader_length = get_head_length(sock_data.read_buf,sock_data.read_buf_used);
        if (_fheader_length >0)
        {
			read_head_done(sock_data.read_buf,_fheader_length);
			_fbody_length = get_body_length(sock_data.read_buf,_fheader_length);
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
			}	
        }
    }
	if (_io_status == FOR_BODY)
	{
    	_fbody_readdone = sock_data.read_buf_used - _fheader_length;
        if (_fbody_readdone >= _fbody_length) {
			//_debug("read_done body_read_done=%d\n",_fbody_readdone);
            read_done(sock_data.read_buf,_fheader_length+ _fbody_length);
			sock_data.read_buf_used=0;
			_fheader_length=0;
			_fbody_readdone=0;
			_fbody_length=0;
			_io_status=FOR_HEAD;
        }
	}

    return 0;
}
int UbEvent::read_func()
{

    //ret = ::read(this->handle(), ((char *)(_buf)) + _readcnt, _cnt - _readcnt);

	//ret = recv(this->handle(), sock_data.read_buf + sock_data.read_buf_used ,
	//		sock_data.read_buf_len - sock_data.read_buf_used, MSG_DONTWAIT);

    char* _buf = sock_data.read_buf + sock_data.read_buf_used;
    unsigned int len = sock_data.read_buf_len - sock_data.read_buf_used;
    int ret = ::recv(handle(), _buf, len, MSG_DONTWAIT);
//	_debug("read buf=%s\n,ret=%d",_buf,ret);

    if (ret>0)
    {
    	sock_data.read_buf_used += ret;
        return read_buffer_process(ret);
    }
	else if (ret ==0)
	{
		int ev = events();
		setEvents( ev | IEvent::CLOSESOCK);
	} 

    /*if (errno == EAGAIN) {
        return 0;
    }*/

    setStatus(IEvent::ERROR);
    return -1;
}

int UbEvent::write_func()
{
    char* _buf = sock_data.write_buf + sock_data.write_buf_done;
    unsigned int len = sock_data.write_buf_used - sock_data.write_buf_done;
    //ret = ::write(this->handle(), ((char *)(_buf)) + _readcnt, _cnt - _readcnt);
    int ret = ::send(this->handle(), _buf, len, MSG_DONTWAIT);

	//_debug("write_buf=%s\n,ret=%d",_buf,ret);

    if (ret > 0)
    {
        sock_data.write_buf_done += ret;
        if (sock_data.write_buf_done>=sock_data.write_buf_used){
			int t = type();	
			setType(t & ~IEvent::IOWRITEABLE);
            write_done();
		}

        return 0;
    }
    
    setStatus(IEvent::ERROR);
    return -1;
}

void UbEvent::release_read_buf() {
	if ((sock_data.small_readbuf != sock_data.read_buf) && (NULL != sock_data.read_buf)){
		ub_event_free(sock_data.read_buf, sock_data.read_buf_len);
	}
	sock_data.read_buf = sock_data.small_readbuf;
	sock_data.read_buf_len = sizeof(sock_data.small_readbuf);
	sock_data.read_buf_used = 0;
}
void UbEvent::release_write_buf() {
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
		_debug(" release\n");
		delete this;
		return true;
	} else {
		return false;
	}
}
void UbEvent::event_error_callback() 
{
    _err("error handle, close sockfd\n");
    error_handle();
    if (0 <= handle() ) {
        ::close(handle());
    }
    this->setHandle(-1);
	
}

UbEvent::~UbEvent() {
    release_read_buf();
    release_write_buf();
    if (0 <= handle()) {
        ::close(handle());
    }
    pthread_mutex_destroy(&_mutex);
}


};

