/**
 * @file netreactor.cpp
 * @author wangwei115120@sogou-inc.com
 * @date 2014/05/25 
 * @brief 
 *  
 **/
#include "common.h"
#include "epoll.h"
#include "reactor.h"
#include "ievent.h"
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
namespace ub
{

EPollEx::EPollEx() : _fd(-1), _size(0)
	, _waito(10)
	, _evs(0)
	, _selfid(0), _cancel(0)
{
	gettimeofday(&_nexto, NULL);
}
EPollEx::~EPollEx()
{
	destroy();
}

int EPollEx::create(size_t size)
{	
	struct epoll_event ev;
	int ret = 0;
	int flags = 0;
	if (size < 1) {
		return -1;
	}
	if (::pipe(_pipe) < 0) {
		int err = errno;
		_err("create pipe failed, through pipe()\n");
		goto fail;
	}
	flags = fcntl(_pipe[0], F_GETFL, 0);
	fcntl(_pipe[0], F_SETFL, flags|O_NONBLOCK);
	flags = fcntl(_pipe[1], F_GETFL, 0);
	fcntl(_pipe[1], F_SETFL, flags|O_NONBLOCK);

	_evs = static_cast<epoll_event *>(::malloc(sizeof(epoll_event) * size));
	if (_evs == NULL) {
		_err("malloc(size=%d) Failed!", (int)(sizeof(epoll_event) * size));
		goto fail;
	}

	_fd = epoll_create(size);
	if (_fd <= 0) {
		int err = errno;
		_err("create epoll instance Failed, through epoll_create()\n");
		goto fail;
	}
	_size = size;

	ev.data.ptr = (void *)(-1);
	ev.events = EPOLLHUP | EPOLLERR | EPOLLIN;
	ret = epoll_ctl(_fd, EPOLL_CTL_ADD, _pipe[0], &ev);
	if (ret != 0) {
		int err = errno;
		_err("register pipe_fd on epoll instance Failed,through epoll_ctl(EPOLL_CTL_ADD)\n");
		goto fail;
	}

	return 0;
fail:
	destroy();
	return -1;

}

void EPollEx::destroy()
{
#ifdef UB_EPOLL_USESELF_TIMEOUT
	IEvent *ev = _elq.pop();
	while (NULL != ev) {
		//ev->setResult(IEvent::REACTOR_DESTROY);
		//ev->callback();
		ev->release();
		ev = _elq.pop();
	}
#endif
	/*ev = _a_q.pop();
	while (NULL != ev) {
		ev->release();
		ev = _a_q.pop();
	}*/

	if (_fd > 0) {
		::close(_fd);
		_fd = -1;
	}
	if (_evs) {
		::free(_evs);
		_evs = 0;
	}
	if (_pipe[0] >=0) {
		::close(_pipe[0]);
		::close(_pipe[1]);
		_pipe[0] = _pipe[1] = -1;
	}

}

int EPollEx::checker(int to, timeval &tv)
{
#if 1
	if (_cancel == 0) {
        _waito = tvTv2Ms(tvSub(_nexto, tv));

        if (_waito > 0) {
            if (_waito > to) {
            	_waito = to;
            }
            //U_DEBUG("not checking  %d", _waito);
            return 0;
        }

        _nexto = tv;
        _nexto.tv_usec += to * 1000;
        if (_nexto.tv_usec > 1000000) {
            _nexto.tv_sec ++;
            _nexto.tv_usec -= 1000000;
        }
	}
#endif
#ifdef UB_EPOLL_USESELF_TIMEOUT
	{
        IEvent *ev = NULL;
        {
        AutoMLock ___lock(_lock1);
        ev = _elq.begin();
        }
        while (ev != NULL) {
            IEvent * next = NULL;
            //U_DEBUG("checking ev[%ld] status[%x]", (long)ev, ev->status());
            bool bcb = false;
            {
                AutoMLock ___lock(_lock1);
                next = ev->next();
            }
            if (ev->status() == IEvent::CANCELED) {
                bcb = true;
                ev->setEvents(IEvent::CANCELED);
                if (this->del(ev) != 0) {
                int err = errno;
                //EVENT_WARNING(ev, err, "no", "when event's status is IEvent::CANCELED, "
                //	"deregister its sockfd  from the epoll instance Failed, through epoll_ctl(EPOLL_CTL_DEL)");
            }
            } else if (ev->isTimeout()) {
                bcb = true;
                ev->setResult(IEvent::TIMEOUT);
                ev->setStatus(IEvent::ERROR);
                if (this->del(ev) != 0) {
                    int err = errno;
                //EVENT_WARNING(ev, err, "no", "when event's status is IEvent::TIMEOUT, "
                //"deregister its sockfd  from the epoll instance Failed, through epoll_ctl(EPOLL_CTL_DEL)");
                }
#if 0
				else if (tvComp(_nexto, *(ev->timeout())) > 0) {
					_nexto = *(ev->timeout());
				}
#endif
			}

			if (bcb) {
				_elq.erase(ev);

				ev->callback();

				ev->release();
			}
			ev = next;
		}
	}
#endif

	_cancel = 0;
	_waito = to;
	//_waito = tvTv2Ms(tvSub(_nexto, tv));
	//if (_waito <= 0 || _waito > to) _waito = to;

	//U_DEBUG("%ld:%ld %ld:%ld to[%d]", _nexto.tv_sec, _nexto.tv_usec,
	//		tv.tv_sec, tv.tv_usec, _waito);

	return 0;
}

int EPollEx::add(IEvent *iev, int itype)
{
	//iev->setDev(this);

	/*if (itype == IReactor::CPU) {
		_a_q.push(iev);
		this->signal();
		return 0;
	}*/
	if (itype != NetReactor::NET) {
		//EVENT_WARNING(iev, 0, "no", "Calling netreactor::post(event, type) failed, beacause the param type != IReactor::NET");
		return -1;
	}
	_debug("add event to EPoll\n");
	struct epoll_event ev;
	ev.data.ptr = iev;
	ev.events = EPOLLHUP | EPOLLERR;
	int type = iev->type();
	if (type & IEvent::IOREADABLE) {
		ev.events = ev.events | EPOLLIN;
	}
	if (type & IEvent::IOWRITEABLE) {
		ev.events = ev.events | EPOLLOUT;
	}
	int ret = 0;
	{
		AutoMLock ____lock(_lock1);
#ifdef UB_EPOLL_USESELF_TIMEOUT
	    _elq.push(iev);
#endif
	    ret = epoll_ctl(_fd, EPOLL_CTL_ADD, iev->handle(), &ev);
#ifdef UB_EPOLL_USESELF_TIMEOUT
	    if (ret != 0) {
		    int err = errno;
		    _err("register event's sockfd on epoll instance Failed,through epoll_ctl(EPOLL_CTL_ADD)\n");
		    _elq.erase(iev);
	    }
#endif
	}
	return ret;
}

int EPollEx::del(IEvent *ev)
{
	int ret = epoll_ctl(_fd, EPOLL_CTL_DEL, ev->handle(), NULL);
	if (0 != ret) {
	    ret = epoll_ctl(_fd, EPOLL_CTL_DEL, ev->handle(), NULL);    
	}

	return ret;
}
void EPollEx::signal()
{
	::write(_pipe[1], "a", 1);
}

void EPollEx::signalDeal()
{
	do {
		char buf[64];
		int ret = ::read(_pipe[0], buf, sizeof(buf));
		if (ret < (int)sizeof(buf)) {
			break;
		}
	} while (true);
/*	IEvent *ev = _a_q.pop();
	while (NULL != ev) {
		ev->callback();
		ev->release();
		ev = _a_q.pop();
	}*/
}


int EPollEx::poll(int timeout)
{

	if (_waito > timeout) { _waito = timeout; }
	if (_waito <= 0) { _waito = 1; }
#if 0
	timeval now;
	gettimeofday(&now, NULL);
	U_DEBUG("start wait %d	-	%ld:%ld", _waito,
			now.tv_sec, now.tv_usec);
#endif
	int num = epoll_wait(_fd, _evs, _size, _waito);
	//U_DEBUG("wait done %d", num);
	{
		for (int i=0; i<num; ++i) {
			//_lock.lock();
			IEvent *iev = static_cast<IEvent *>(_evs[i].data.ptr);
			if (iev == (IEvent *)(-1)) {
				this->signalDeal();
				continue;
			}

			int events = 0;
			//U_DEBUG("events %d, hd:%d", _evs[i].events, iev->handle());

			//根据实验看，HUP和ERROR实际是同时发生的, chenyifei
			if ((_evs[i].events & EPOLLHUP) ||
				(_evs[i].events & EPOLLERR)) {
				//events = events | IEvent::CLOSESOCK;
				events = events | IEvent::SOCKERROR;
                  iev->setStatus(IEvent::ERROR);
				//EVENT_WARNING(iev, 0, "no", "EPOLLHUP|EPOLLERR event happened in sockfd, then set event's result SOCKERROR");
				/*
				int err;
				socklen_t len = sizeof(err);
				int val = getsockopt(iev->handle(), SOL_SOCKET, SO_ERROR, &err, &len);
				U_WARNING("EPOLLHUP fd[%d] error[%d] ret[%d]", iev->handle(), err, val);
				*/
			}
			
			/*
			if (_evs[i].events & EPOLLERR) {
				events = events | IEvent::CLOSESOCK; //IEvent::ERROR;
				int err;
				socklen_t len = sizeof(err);
				int val = getsockopt(iev->handle(), SOL_SOCKET, SO_ERROR, &err, &len);
				U_WARNING("EPOLLERR fd error[%d] ret[%d]", err, val);
			}
			*/
			
			if (_evs[i].events & EPOLLIN) {
				events = events | IEvent::IOREADABLE;
			}
			if (_evs[i].events & EPOLLOUT) {
				events = events | IEvent::IOWRITEABLE;
			}

			if (iev->status() == IEvent::CANCELED) {
				events = events | IEvent::CANCELED;
			}
			iev->setEvents(events);

			int ret = epoll_ctl(_fd, EPOLL_CTL_DEL, iev->handle(), NULL);
			if (ret != 0) {
				int err = errno;
				_err("deregister event()'s sockfd from the epoll instance Failed, through epoll_ctl(EPOLL_CTL_DEL)\n");
			}
			
#ifdef UB_EPOLL_USESELF_TIMEOUT
			_elq.erase(iev);
#endif

        		iev->callback();

			iev->release();
		}

		
#ifdef UB_EPOLL_USESELF_TIMEOUT
		//U_DEBUG("---------------start to check");
		timeval tv;
		gettimeofday(&tv, NULL);
		this->checker(timeout, tv);
#endif

	}
	return num;
}

NetReactor::NetReactor() : check_timer_(10)
{
	maxevs_ = 10000;
	pos_ =-1;
	epoll_index_ = -1;
    pthread_mutex_init(&mutex_, NULL);


}
NetReactor::~NetReactor() 
{
    pthread_mutex_destroy(&mutex_);


}


int NetReactor::Svc()
{

    int index=0;
    pthread_mutex_lock(&mutex_);
    pos_++;
    index = pos_;
    pthread_mutex_unlock(&mutex_);


    EPollEx *poll = getPoll(index);
    if (poll == NULL) {
        _err("in NetReactor::callback(), getPoll(pos=%d) err!(ret == NULL)\n", index);
        return 0;
    }
    poll->initPthreadId();

    while (1) {
        int ms = check_timer_;
        poll->poll(ms);
        if (IsStop()) {
            break;
        }
    }
	return 0;
}

int NetReactor::Open(int pthread_num)
{
    STaskBase<int>::Open(pthread_num);
    initVecs();
    return 0;
}

int NetReactor::Close()
{
    for (int i =0;i<m_thread_num;i++)
    {
        vecs_[i].destroy();
        
    }
    delete [] vecs_;

    
    STaskBase<int>::Close();

    return 0;
}

int NetReactor::initVecs()
{
	int ts = m_thread_num;
	vecs_ = new EPollEx[m_thread_num];

	int index=-1;
    if (vecs_){
        for (int i =0;i<m_thread_num;i++)
        {
            if (vecs_[i].create(maxevs_) != 0)
            {
                index = i;
                break;
            }
        }

        if (index <0)
            return 0;

        for (int i =0;i<index;i++)
        {
            vecs_[i].destroy();
        }
        delete [] vecs_;

    }
    return -1;
}

EPollEx *NetReactor::getPoll(int pos)
{
	if (  pos >= m_thread_num) {
			return NULL;		
	}
	return &vecs_[pos];
}


int NetReactor::post(IEvent *ev)
{
	if (ev == NULL) {
		_err("Calling NetReactor::post(event, type) failed, beacause the param event == NULL\n");
		return -1;
	}
    ev->setEvents(0);
    ev->setStatus(IEvent::READY);

    ev->setReactor(this);

    /*if (this->status() != IReactor::RUNNING) {
    U_DEBUG("push ev[%lx] into queue", long(ev));
    ev->setDev((void *)((long)type));
    return UBTask::post(ev);
    }*/
    ev->addRef();

    int index=0;
    pthread_mutex_lock(&mutex_);
    epoll_index_++ % m_thread_num;
    index=epoll_index_;
    pthread_mutex_unlock(&mutex_);
    

    for (int i= index; i<m_thread_num; ++i) {
        EPollEx *poll = getPoll(i);
        if (poll && poll->add(ev, NetReactor::NET) == 0) {
            return 0;
        }
    }
	for (int i=0; i<index; ++i) {
        EPollEx *poll = getPoll(i);
        if (poll && poll->add(ev, NetReactor::NET) == 0) {
        /* 
         * 请务必考虑多线程问题；
         * 此ev已被add到reactor中，reactor线程有可能已完成读写和release，
         * 如果用户在callback也调用release，此ev已被释放 
        */
        return 0;
        }
	}
	ev->release();
	//U_DEBUG("push ev[%lx] error", long(ev));
	return -1;
}

int NetReactor::cancel(IEvent *ev)
{
	if (NULL == ev) {
		//U_DEBUG("invalid param ev == NULL");
		return -1;
	}

	/*
	if (ev->timeout()) {//超时事件暂时不支持取消
		UB_IN_WARNING("ev->timeout()");
		return -1;
	}*/
	/*EPollEx *poll = static_cast<EPollEx *>(ev->dev());
	if (poll == NULL) {
		return -1;
	}
	poll->_cancel = 1;
	ev->setStatus(IEvent::CANCELED);
	return 0;*/
}

void NetReactor::setCheckTime(int msec)
{
	if (msec >= 0) {
		check_timer_ = msec;
	}
}

};

/* vim: set ts=4 sw=4 sts=4 tw=100 */

