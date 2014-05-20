/***************************************************************************
 * 
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 * netreactor.cpp 2009/05/25 12:34:04 xiaowei Exp 
 * 
 **************************************************************************/
 
 
 
/**
 * @file netreactor.cpp
 * @author xiaowei(com@baidu.com)
 * @date 2009/05/25 12:34:04
 * @brief 
 *  
 **/

#ifdef __i386
#include "../ub_server/epoll.h"
#else
#include <sys/epoll.h>
#endif
#include "netreactor.h"
#include "ievent.h"
#include "../common/utils.h"

namespace ub
{
EPollEx::EPollEx(IReactor *r) : _fd(-1), _size(0)
	, _waito(10)
	, _evs(0)
	, _selfid(0), _task(r), _cancel(0)
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
		EVENT_WARNING(0, err, "yes", "create pipe failed, through pipe()");
		goto fail;
	}
	flags = fcntl(_pipe[0], F_GETFL, 0);
	fcntl(_pipe[0], F_SETFL, flags|O_NONBLOCK);
	flags = fcntl(_pipe[1], F_GETFL, 0);
	fcntl(_pipe[1], F_SETFL, flags|O_NONBLOCK);

	_evs = static_cast<epoll_event *>
		(::malloc(sizeof(epoll_event) * size));
	if (_evs == NULL) {
		EVENT_WARNING(0, 0, "yes", "malloc(size=%d) Failed!", (int)(sizeof(epoll_event) * size));
		goto fail;
	}

	_fd = epoll_create(size);
	if (_fd <= 0) {
		int err = errno;
		EVENT_WARNING(0, err, "yes", "create epoll instance Failed, through epoll_create()");
		goto fail;
	}
	_size = size;

	ev.data.ptr = (void *)(-1);
	ev.events = EPOLLHUP | EPOLLERR | EPOLLIN;
	ret = epoll_ctl(_fd, EPOLL_CTL_ADD, _pipe[0], &ev);
	if (ret != 0) {
		int err = errno;
		EVENT_WARNING(0, err, "yes",
                        "register pipe_fd on epoll instance Failed,through epoll_ctl(EPOLL_CTL_ADD)");
		goto fail;
	}

	return 0;
fail:
	destroy();
	return -1;

}

void EPollEx::destroy()
{
	IEvent *ev = _elq.pop();
	while (NULL != ev) {
		//ev->setResult(IEvent::REACTOR_DESTROY);
		//ev->callback();
		ev->release();
		ev = _elq.pop();
	}
	ev = _a_q.pop();
	while (NULL != ev) {
		ev->release();
		ev = _a_q.pop();
	}

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
		U_DEBUG("not checking  %d", _waito);
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
			U_DEBUG("checking ev[%ld] status[%x]", (long)ev, ev->status());
			bool bcb = false;
			{
				AutoMLock ___lock(_lock1);
				next = ev->next();
			}
			if (ev->status() == IEvent::CANCELED) {
				bcb = true;
				ev->setResult(IEvent::CANCELED);
				if (this->del(ev) != 0) {
					int err = errno;
					EVENT_WARNING(ev, err, "no", "when event's status is IEvent::CANCELED, "
						"deregister its sockfd  from the epoll instance Failed, through epoll_ctl(EPOLL_CTL_DEL)");
				}
			} else if (ev->timeout()) {
				if (tvComp(tv, *(ev->timeout())) >= 0) {
					bcb = true;
					ev->setResult(IEvent::TIMEOUT);
					if (this->del(ev) != 0) {
					    int err = errno;
					    EVENT_WARNING(ev, err, "no", "when event's status is IEvent::TIMEOUT, "
						"deregister its sockfd  from the epoll instance Failed, through epoll_ctl(EPOLL_CTL_DEL)");
					}
				} 
#if 0
				else if (tvComp(_nexto, *(ev->timeout())) > 0) {
					_nexto = *(ev->timeout());
				}
#endif
			}

			if (bcb) {
				_elq.erase(ev);
#if 0
				if (_task) {
					if (static_cast<Net2Reactor *>(_task)->post2Task(ev) != 0) {
						ev->callback();
					}
				} else {
					ev->callback();
				}
#else
				ev->callback();
#endif
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
	iev->setDev(this);

	if (itype == IReactor::CPU) {
		_a_q.push(iev);
		this->signal();
		return 0;
	}
	if (itype != IReactor::NET) {
		EVENT_WARNING(iev, 0, "no", "Calling netreactor::post(event, type) failed, beacause the param type != IReactor::NET");
		return -1;
	}
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
		    EVENT_WARNING(iev, err, "no", "register event's sockfd on epoll instance Failed,through epoll_ctl(EPOLL_CTL_ADD)");
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
	IEvent *ev = _a_q.pop();
	while (NULL != ev) {
		ev->callback();
		ev->release();
		ev = _a_q.pop();
	}
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
			U_DEBUG("events %d, hd:%d", _evs[i].events, iev->handle());

			//根据实验看，HUP和ERROR实际是同时发生的, chenyifei
			if ((_evs[i].events & EPOLLHUP) ||
				(_evs[i].events & EPOLLERR)) {
				//events = events | IEvent::CLOSESOCK;
				events = events | IEvent::SOCKERROR;
				
				EVENT_WARNING(iev, 0, "no", "EPOLLHUP|EPOLLERR event happened in sockfd, then set event's result SOCKERROR");
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
			iev->setResult(events);

			int ret = epoll_ctl(_fd, EPOLL_CTL_DEL, iev->handle(), NULL);
			if (ret != 0) {
				int err = errno;
				EVENT_WARNING(iev, err, "no",
					"deregister event()'s sockfd from the epoll instance Failed, through epoll_ctl(EPOLL_CTL_DEL)");
			}
#ifdef UB_EPOLL_USESELF_TIMEOUT
			_elq.erase(iev);
#endif
			if ((NULL !=_task->getExternReactor()) && (iev->isDevided())) {
				_task->getExternReactor()->post(iev);
			} else {
				iev->callback();
			}
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

NetReactor::NetReactor() : _check_timer(100), _ext_task(0)
{
	_maxevs = 10000;
	_threadsnum = 1;
}
NetReactor::~NetReactor() 
{
	if (NULL != _ext_task) {
		ub::IReactorIoc.destroy(_ext_task);
	}
	for (size_t i=0; i<_vecs.size(); ++i) {
		if (_vecs[i]) {
			_vecs[i]->destroy();
			delete _vecs[i];
			_vecs[i] = 0;
		}
	}
}

IReactor * NetReactor::getExternReactor() {
	if (0 !=_use_ext_task && NULL != _ext_task) {
		return _ext_task;
	}
	return NULL;
}

int NetReactor::load(const comcfg::ConfigUnit &cfg)
{
	cfg["QueueType"].get_bsl_string(&_qstr, "LockEQueue");
	cfg["ThreadNum"].get_int32(&_threadsnum, 1);
	U_DEBUG("set ThreadNum %d", _threadsnum);
	cfg["Priority"].get_int32(&_pri, 1);
	cfg["MaxEvents"].get_int32(&_maxevs, 10000);
	cfg["CheckTime"].get_int32(&_check_timer, 100);
	cfg["Usetask"].get_int32(&_use_ext_task, 0);
	if (_maxevs < 1) {
		EVENT_FATAL(0, 0, "yes", "MaxEvents(< 1) is too small, Netreactor failed to initialize");
	}
	if (0 != _use_ext_task) {
		_ext_task = ub::IReactorIoc.create(cfg["task"]["Type"].to_bsl_string());
		if (NULL == _ext_task) {
			int errcode;
			EVENT_FATAL(0, 0, "yes", "when Usetask in configure file set 1, create reactor(%s) instance Failed!", 
					cfg["task"]["Type"].to_cstr(&errcode, ""));
			return -1;
		}
		if (0!= _ext_task->load(cfg["task"])) {
			_ext_task = NULL;
			return -1;
		}
	}
	return 0;
}

int NetReactor::run_tv(const timeval *tv)
{
	_pos.set(0);
	if (0 !=_use_ext_task && NULL != _ext_task) {
		_ext_task->run_tv(tv);
	}
	return UBTask::run_tv(tv);
}

void NetReactor::callback()
{
	initVecs();
	int pos = _pos.getAndIncrement();
	EPollEx *poll = getPoll(pos);
	if (poll == NULL) {
		EVENT_WARNING(0, 0, "no", "in NetReactor::callback(), getPoll(pos=%d) err!(ret == NULL)", pos);
		return;
	}
	poll->initPthreadId();

	U_DEBUG("NetReactor Debug");
	{
		static const int fetch = 100;
		IEvent *iev[fetch];
		while (_queue->size() > 0) {
			timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			int ret = _queue->pop_tv(iev, fetch, &tv);
			for (int i=0; i<ret; ++i) {
				long type = (long)(iev[i]->dev());
				this->post(iev[i], type);
				iev[i]->release();
			}
		}
	}

	while (_run) {
		int ms = _check_timer;
		//U_DEBUG("start poll %d", ms);
		poll->poll(ms);
		//U_DEBUG("stop poll");
		if (isStop(time(0))) {
			break;
		}
	}
}

int NetReactor::initVecs()
{
	AutoMLock __lock(_lock);
	int ts = _threadsnum;
	if (ts <= 0) { ts = 1; }
	if ((int)_vecs.size() != ts) {
		for (int i=ts; i<(int)_vecs.size(); ++i) {
			if (_vecs[i]) {
				_vecs[i]->destroy();
				delete _vecs[i];
				_vecs[i] = 0;
			}
		}
		_vecs.resize(ts);
	}
	return 0;
}

EPollEx *NetReactor::getPoll(int pos)
{
	if ((int)_vecs.size() <= pos) {
		initVecs();
		if ((int)_vecs.size() <= pos) {
			return NULL;
		}
	}
	if (_vecs[pos] == NULL) {
		AutoMLock __lock(_lock);
		if (_vecs[pos] == NULL) {
			EPollEx * tmp = new EPollEx(this);
			if (tmp->create(_maxevs) != 0) {
				delete tmp;
				return NULL;
			}
			_vecs[pos] = tmp;
		}
	}
	return _vecs[pos];
}

int NetReactor::smartPost(IEvent *ev)
{
	return this->smartPost(ev, IReactor::NET);
}
int NetReactor::smartPost(IEvent *ev, int type)
{
	if (NULL == ev) {
		EVENT_WARNING(0, 0, "no", "Calling NetReactor::smartPost(event, type) failed, beacause the param event == NULL");
		return -1;
	}
	if (this->status() != IReactor::RUNNING) {
		return this->post(ev);
	}	
	ev->setResult(0);
	ev->setStatus(IEvent::READY);
	if (_queue && (int)_queue->size() >= _maxevs) {
		EVENT_WARNING(ev, 0, "no",
		"Calling NetReactor::smartPost(event, type) failed, beacause the number of events is greater than MaxEvents in configure file");
		return -1;
	}
	ev->setReactor(this);

	if ((0 !=_use_ext_task && NULL != _ext_task) && (ev->isDevided())) {
		return _ext_task->post(ev);
	}

	pthread_t pid = pthread_self();
	for (int i=0; i<_pos.get(); ++i) {
		if (_vecs[i] && _vecs[i]->pid() == pid) {
			ev->setResult(0);
			ev->addRef();
			if (_vecs[i]->add(ev, type) != 0) {
				ev->release();
				return this->post(ev);
			}
			return 0;
		}
	}
	return this->post(ev);
}

int NetReactor::post(IEvent *ev)
{
	return this->post(ev, IReactor::NET);
}
int NetReactor::post(IEvent *ev, int type)
{
	if (ev == NULL) {
		EVENT_WARNING(0, 0, "no", "Calling NetReactor::post(event, type) failed, beacause the param event == NULL");
		return -1;
	}
	ev->setResult(0);
	ev->setStatus(IEvent::READY);

	if (_queue && (int)_queue->size() >= _maxevs) {
		EVENT_WARNING(ev, 0, "no",
		    "Calling NetReactor::post(event, type) failed, beacause the number of events is greater than MaxEvents in configure file");
		return -1;
	}

	ev->setReactor(this);

	if (this->status() != IReactor::RUNNING) {
		U_DEBUG("push ev[%lx] into queue", long(ev));
		ev->setDev((void *)((long)type));
		return UBTask::post(ev);
	}
	ev->addRef();
	int rd = 0;
	if (_threadsnum > 0) { rd = _rand.rand() % _threadsnum; }

	if ((0 !=_use_ext_task && NULL != _ext_task) && (ev->isDevided())) {
		return _ext_task->post(ev);
	}

	for (int i=rd; i>=0; --i) {
		EPollEx *poll = getPoll(i);
		if (poll && poll->add(ev, type) == 0) {
			/* by wuliping
			 * 请务必考虑多线程问题；
			 * 此ev已被add到reactor中，reactor线程有可能已完成读写和release，
			 * 如果用户在callback也调用release，此ev已被释放 
			*/
			return 0;
		}
	}
	for (int i= rd+1; i<_threadsnum; ++i) {
		EPollEx *poll = getPoll(i);
		if (poll && poll->add(ev, type) == 0) {
			return 0;
		}
	}
	ev->release();
	U_DEBUG("push ev[%lx] error", long(ev));
	return -1;
}

int NetReactor::cancel(IEvent *ev)
{
	if (NULL == ev) {
		U_DEBUG("invalid param ev == NULL");
		return -1;
	}
	if (this->status() != IReactor::RUNNING) {
		return UBTask::cancel(ev);
	}
	/*
	if (ev->timeout()) {//超时事件暂时不支持取消
		UB_IN_WARNING("ev->timeout()");
		return -1;
	}*/
	EPollEx *poll = static_cast<EPollEx *>(ev->dev());
	if (poll == NULL) {
		return -1;
	}
	poll->_cancel = 1;
	ev->setStatus(IEvent::CANCELED);
	return 0;
}

void NetReactor::setCheckTime(int msec)
{
	if (msec >= 0) {
		_check_timer = msec;
	}
}

#if 0
Net2Reactor::Net2Reactor() : _check_timer(100)
{
	_maxevs = 10000;
	_threadsnum = 1;
	_poll = NULL;
	_add_ev_done = false;
}

Net2Reactor::~Net2Reactor()
{
	if (_poll) {
		_poll->destroy();
		delete _poll;
		_poll = 0;
	}
}

int Net2Reactor::load(const comcfg::ConfigUnit &cfg)
{
	cfg["QueueType"].get_bsl_string(&_qstr, "LockEQueue");
	cfg["ThreadNum"].get_int32(&_threadsnum, 1);
	UB_IN_DEBUG("set ThreadNum %d", _threadsnum);
	cfg["Priority"].get_int32(&_pri, 1);
	cfg["MaxEvents"].get_int32(&_maxevs, 10000);
	return 0;
}

void Net2Reactor::epollCallback(IEvent *ev, void *p)
{
	Net2Reactor *r = static_cast<Net2Reactor *>(p);
	r->_poll->poll(r->_check_timer);

	while (r->post2Task(ev) != 0) {
		r->_poll->poll(r->_check_timer);
	}
}

int Net2Reactor::run_tv(const timeval *tv)
{
	if (!_add_ev_done) {
		ub::EventPtr ev;
		ev->setCallback(Net2Reactor::epollCallback, this);
		UBTask::post(&ev);
		_add_ev_done = true;
	}
	return UBTask::run_tv(tv);
}

EPollEx * Net2Reactor::getPoll() 
{
	if (_poll == NULL) {
		AutoMLock __lock(_lock);
		if (_poll == NULL) {
			_poll = new EPollEx(this);
			if (_poll->create(_maxevs) != 0) {
				delete _poll;
				_poll = 0;
				UB_IN_WARNING("create EPollEx fail in callback");
				return NULL;
			}
		}
	}
	return _poll;
}

int Net2Reactor::post(IEvent *ev)
{
	if ((ev->type() & IEvent::IOWRITEABLE)
			|| (ev->type() & IEvent::IOREADABLE)) {
		ev->addRef();
		EPollEx *p = getPoll();
		if (p == NULL) {
			UB_IN_WARNING("get poll error %m");
			ev->release();
			return -1;
		}
		if (p->add(ev) != 0) {
			UB_IN_WARNING("add to poll error %m");
			ev->release();
			return -1;
		}
		return 0;
	}
	return UBTask::post(ev);
}

int Net2Reactor::cancel(IEvent *ev)
{
	if ((ev->type() & IEvent::IOWRITEABLE) 
			|| (ev->type() & IEvent::IOREADABLE)) {
		if (ev->timeout()) {//超时事件暂时不支持取消
			return -1;
		}
		EPollEx *poll = static_cast<EPollEx *>(ev->dev());
		if (poll == NULL) {
			return -1;
		}
		ev->setStatus(IEvent::CANCELED);
		return poll->del(ev);
	}
	return UBTask::cancel(ev);
}
#endif
};

/* vim: set ts=4 sw=4 sts=4 tw=100 */
