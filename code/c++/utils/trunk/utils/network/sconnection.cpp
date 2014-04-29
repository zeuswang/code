#include "sconnection.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netdb.h>
#include "topology.h"
#include <utils/debug.h>
int Connection::Connect(const std::string& ip,const std::string& port)
{
	addrinfo *res=NULL;
	//LockSelf();
	
	if (!(_fd<0)){
		::close(_fd);
	}
	
	int socket_fd;
	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		goto ERROR;

	//WEBSEARCH_DEBUG((LM_DEBUG,"ip:%s,port:%d\n",ip.c_str(),atoi(port.c_str())));


	static addrinfo hints = { 0, AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL };
	if (getaddrinfo(ip.c_str(),port.c_str(), &hints, &res))
		goto ERROR;
	
	if (connect(socket_fd, (struct sockaddr*)((sockaddr_in *)res->ai_addr), sizeof(sockaddr_in)) < 0) 
	{
		//fprintf(stderr,"connect summary error\n");
		//sSocketHandle * hdle = new sSocketHandle(INVALID_FD);
		//m_sock_fd[i] = hdle;
		::close(socket_fd);
		goto ERROR;
	}

	if (SetSocket(socket_fd, O_NONBLOCK))
	{
		::close(socket_fd);
		goto ERROR;
	}
	
	_debug("connect ok,ip:%s,port:%d\n",ip.c_str(),atoi(port.c_str()));
	_fd = socket_fd;
	
	//UnLockSelf();

	if (res)
		free(res);

	return 0;
ERROR:
	//UnLockSelf();
	_debug("connect faild,ip:%s,port:%d\n",ip.c_str(),atoi(port.c_str()));
	_fd =INVALID_FD;
	if (res)
		free(res);
	return -1;
}
int Connection::ConnectTimeout(const std::string& ip,const std::string& port,int time_out)
{
	addrinfo *res=NULL;
  int flags;
	//LockSelf();

	if (!(_fd<0)){
		::close(_fd);
	}
	
	int socket_fd;
	if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		goto ERROR;

	//WEBSEARCH_DEBUG((LM_DEBUG,"ip:%s,port:%d\n",ip.c_str(),atoi(port.c_str())));

  flags=fcntl(socket_fd,F_GETFL,0);
  fcntl(socket_fd,F_SETFL,flags|O_NONBLOCK);

	static addrinfo hints = { 0, AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL };
	if (getaddrinfo(ip.c_str(),port.c_str(), &hints, &res)){
    ::close(socket_fd);
		goto ERROR;
	}
	
	if (connect(socket_fd, (struct sockaddr*)((sockaddr_in *)res->ai_addr), sizeof(sockaddr_in)) < 0) 
	{
		//fprintf(stderr,"connect summary error\n");
		//sSocketHandle * hdle = new sSocketHandle(INVALID_FD);
		//m_sock_fd[i] = hdle;
		//::close(socket_fd);
		//goto ERROR;

    fd_set set;  
    FD_ZERO(&set);  
    FD_SET(socket_fd, &set);  

    struct timeval timeout;  
    timeout.tv_sec = 0;  
    timeout.tv_usec = time_out*1000;  

    if ( select(socket_fd+1, NULL, &set, NULL, &timeout) > 0 )  
    {  
      //conn_ok = true;  
    }else {
      ::close(socket_fd);
      goto ERROR;
    }
	}  

	if (SetSocket(socket_fd, O_NONBLOCK))
	{
		::close(socket_fd);
		goto ERROR;
	}
	
	_debug("connect ok,ip:%s,port:%d\n",ip.c_str(),atoi(port.c_str()));
	_fd = socket_fd;
	
	//UnLockSelf();

	if (res)
		free(res);

	return 0;
ERROR:
	//UnLockSelf();
	_debug("connect faild,ip:%s,port:%d\n",ip.c_str(),atoi(port.c_str()));
	_fd =INVALID_FD;
	if (res)
		free(res);
	return -1;
}
int Connection::Reconnect()
{
	Close();
	Connect(GetVNode()->GetAddress(),GetVNode()->GetPort());
	SetFlag(WRITABLE_SOCKET||READABLE_SOCKET);
}

int Connection::SetSocket(int fd,int flag)
{
   	int options;
	options = SOCKET_SND_BUF_SIZE;
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &options, sizeof(int));
	options = SOCKET_RCV_BUF_SIZE;
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &options, sizeof(int));
	options = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(int));
	options = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, options | flag);
	int on = 1;
	int ret = -1;
	ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
	return ret;
}

int Connection::Close()
{
	//LockSelf();
	::close(_fd);
	_fd = INVALID_FD;
	//UnLockSelf();
	return 0;
}
int Connection::ReadTimeout(void *buf, size_t buf_len, timeval *timeout)
{
	//buf_len =1024;
	//char * buff= new char[buf_len];	
	//char * old_buf =buff;
	int left = buf_len;
	int n;
	fd_set rset;
	if (_fd == INVALID_FD){
		_err("ReadTimeout fd is INVALID\n");
		goto ERROR;
	}
	while (left > 0)
	{
		//fprintf(stderr, "############3333 000000 readn_timout is left:%d,fd:%d ",left,_fd);	
		FD_ZERO(&rset);
		FD_SET(_fd, &rset);
		if (select(_fd + 1, &rset, NULL, NULL, timeout) <= 0){
			_err("Connection::ReadTimeout select error\n");
			goto ERROR;
		}
		if ((n = read(_fd, buf, left)) == 0){
			//fprintf(stderr, "############3333 readn_timout is left:%d,fd:%d ",left,_fd);	
			//return buf_len - left;
			buf_len = buf_len - left;
			goto OUT;
		}
		buf = (char *)buf + n;
		left -= n;
	}
OUT:
	/*fprintf(stderr,"========================\n");
	for (int i =0 ;i<buf_len;i++)
		fprintf(stderr," %x ",old_buf[i]);
	fprintf(stderr,"\n========================.done\n");
	*/
	//fprintf(stderr,"\n======================== buf_len =%d\n",buf_len);
	return buf_len;
ERROR:
	return -1;

}
int Connection::ReadTimeoutOnce( void * buf, int buf_len, timeval * timeout)
{
	if (_fd == INVALID_FD){
		_err("fd is INVALID\n");
		return  -1;
	}
	fd_set rset;
	int n;
	FD_ZERO(&rset);
	FD_SET(_fd, &rset);
	if (select(_fd + 1, &rset, NULL, NULL, timeout) <= 0)
		return -1;
	if ((n = read(_fd, buf, buf_len)) <= 0)
		return -1;
	return n;
}

int Connection::WriteTimeout(void *buf, size_t buf_len, timeval *timeout)
{
	int left = buf_len;
	int n;
	fd_set wset;

	if (_fd == INVALID_FD)
		goto ERROR;
	while (left > 0)
	{
		FD_ZERO(&wset);
		FD_SET(_fd, &wset);
		if (select(_fd + 1, NULL, &wset, NULL, timeout) <= 0)
			goto ERROR;
		if ((n = write(_fd, buf, left)) == 0){
			buf_len = buf_len - left;
			goto OUT;	
		}
		buf = (char *)buf + n;
		left -= n;
	}

OUT:
	return buf_len;
ERROR:
	return -1;
}



