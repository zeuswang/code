#ifndef _SOGOU_CONNECTION_MANAGER_H_
#define _SOGOU_CONNECTION_MANAGER_H_
#include "sconnection.h"
class ConnectionManager
{
public: 
	ConnectionManager(){};
	virtual ~ConnectionManager(){};
	virtual void ErrorHandle(class Connection* conn){};

};

#endif
