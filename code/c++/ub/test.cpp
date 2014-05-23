#include "httpevent.h"
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

class clienthttpevent:public ub::HttpEvent
{
public:
    int read_head_done(char* buf,int len)
    {
        std::string head(buf,len);
        printf("zzzzzzzzzzzzzzzzzzzheaderis=%s,len=%d",head.c_str(),len);
        return 0;
    };
    int read_done(char * buf,int hlen,int blen)
    {
		printf("\nlen=%d\n",blen);
		printf("buf=%s\n",buf);
        //std::string body(buf,blen);
        //printf("body=%s\n",body.c_str());
        finish();

    };

    int write_done(){return 0;};
	void error_handle(){};

};

static int __set_fd_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL);

	if (flags >= 0)
		flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	return flags;
}
int main()


{
    ub::NetReactor* reactor = new ub::NetReactor();
	reactor->Open(1);
	reactor->Activate();
    clienthttpevent * httpEvent = new clienthttpevent();
    char *reqbuf = (char *)httpEvent->get_write_buffer(1000);
    int ret = snprintf(reqbuf, 1000, "%s", "GET / HTTP/1.1\n\n");
    httpEvent->get_write_buffer(ret);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);

    if (sockfd >=0)
    {
	printf("ok\n");
            httpEvent->setReactor(reactor);
            struct sockaddr_in server_addr ;
            server_addr.sin_family = AF_INET;
            //server_addr.sin_addr.s_addr =  inet_addr("61.55.167.120");
            server_addr.sin_addr.s_addr =  inet_addr("115.239.211.110");
            server_addr.sin_port = htons(80);
            if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) >= 0)
            {
				printf(" connect ok\n");
				__set_fd_nonblock(sockfd);				
                httpEvent->setHandle(sockfd);
                httpEvent->post();
            }


    }




while(1)
{
};
}
