#include "httpevent.h"
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class clienthttpevent:public ub::HttpEvent
{
public:
    int read_head_done(char* buf,int len)
    {
        std::string head(buf,len);
        printf("headeris=%s,len=%d",head.c_str(),len);
        return 0;
    };
    int read_done(char * buf,int hlen,int blen)
    {
        std::string head(buf,hlen);
        std::string body(buf+hlen,blen - hlen);
        printf("headeris=%s\n",head.c_str());
        printf("body=%s\n",body.c_str());
        finish();

    };

    int write_done(){return 0;};

};


int main()


{
    ub::NetReactor* reactor = new ub::NetReactor();
    clienthttpevent * httpEvent = new clienthttpevent();
    char *reqbuf = (char *)httpEvent->get_write_buffer(1000);
    int ret = snprintf(reqbuf, 1000, "%s", "GET / \n");
    httpEvent->get_write_buffer(ret);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);

    if (sockfd >=0)
    {

            httpEvent->setReactor(reactor);
            struct sockaddr_in server_addr ;
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr =  inet_addr("61.55.167.120");
            server_addr.sin_port = htons(80);
            if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) >= 0)
            {
                httpEvent->setHandle(sockfd);
                httpEvent->post();
            }


    }




while(1)
{
};
}
