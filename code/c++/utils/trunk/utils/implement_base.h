#ifndef _SOGOU_IMPLEMENT_BASE_H_
#define _SOGOU_IMPLEMENT_BASE_H_

class ImplementBase
{
public:
	ImplementBase(){};
	virtual ~ImplementBase(){};
	virtual int Run(void * arg)=0;
};
#endif

