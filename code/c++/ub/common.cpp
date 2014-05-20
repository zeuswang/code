#include "common.h"
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

}
