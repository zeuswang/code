#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
struct ReadLineInfo
{

    std::string filename;
    void* args;
    uint32_t (*func)(void*, char *,uint32_t,uint32_t);
};


static void *init_function(void* arg)
{
    ReadLineInfo * info= (ReadLineInfo*)arg;
    char buffer[4096];
    char handle_buffer[4096*20];
    uint32_t sum=0;
    ssize_t ret=1;
    int fd = open(info->filename.c_str(),O_RDONLY);
    
    if (fd<0)
    {
         return NULL;
    }

    while(ret>0)
    {
        ret =read(fd,buffer,4096);
        
        if (sum >= 4096*8||ret==0)
        {
            uint32_t start = info->func(info->args,handle_buffer,0,sum);
            if (start>0)
            {
				start ++;
                strncpy(handle_buffer,&handle_buffer[start],sum - start);
                sum=sum - start;
                strncpy(&handle_buffer[sum],buffer,ret);
                sum=sum+ret;
            }else 
            {
                ::close(fd);
                return NULL;
            }; 
        }
        else 
        {
            strncpy(&handle_buffer[sum],buffer,ret);
            sum +=ret;
        }
		
    }
    ::close(fd);
}
class ReadFileBackgroud
{
public:
    ReadFileBackgroud(char * filename,void* args,uint32_t (*f)(void*, char *,uint32_t,uint32_t)){

        filename_= std::string(filename);
        func_ =f;     
        user_param_= args;
    };
    int Start();
    int Join();

    std::string filename_;
    void * user_param_;
    uint32_t (*func_)(void*, char *,uint32_t,uint32_t);
    pthread_t pthread_id_;
};

int ReadFileBackgroud::Start()
{
    ReadLineInfo* info= new ReadLineInfo;
    info->args = user_param_;
    info->func = func_;
    info->filename = filename_;
    if (0!=pthread_create(&pthread_id_, NULL, init_function, info))
    {
        fprintf(stderr,"[ReadFileBackgroud::Start()]pthread_create error]\n");
        return -1;
    }    
}

int ReadFileBackgroud::Join()
{
    pthread_join(pthread_id_,NULL);
    return 0;
}



static int findline(char * str,int start,int end)
{
	for (int i =end -1;i>=0; i--)
	{
		if (str[i] == '\n')
			return i;
	}
	return -1;
};


uint32_t process(void * args,char * handle_buffer,uint32_t start,uint32_t end)
{

    int index=findline(handle_buffer,start,end);
    if (index>0)
    {
        char * work_buf = (char * )malloc(10*4096);
        if (work_buf)
        {

        strncpy(work_buf,&handle_buffer[0],index+1);
                        work_buf[index+1]='\0';

        }

    }
};


int main()

{

ReadFileBackgroud rfb("test",NULL,process);
rfb.Start();
rfb.Join();

}

