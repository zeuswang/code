#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>
#include <stdint.h>

#include <sched.h>
pthread_mutex_t mutex[24];
struct ForInfo
{
	int start;
    int step;
	int end;
	int * list;
};
struct SQ
{
    int a;
    int b;
};
int* sq= (int*)malloc(1024*1024*1024*sizeof(int) +100);
unsigned int seed = 131244222;
inline long unsigned int rand31_next(unsigned int& seed)
{
    long unsigned int hi,lo;
    lo = 16807 * (seed & 0xFFFF);
    hi = 16807 * (seed >>16);
    lo += (hi & 0x7FFF) << 16;
    lo += hi >>15;
    if (lo >0x7fffffff )lo -= 0x7fffffff;
    return (seed = (long)lo);
}
void * init_function3(void *arg)
{

	ForInfo * info = (ForInfo*)arg;
    cpu_set_t mask;
    cpu_set_t get;
    char buf[256];
    int i;
    int j;
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    printf("system has %d processor(s)\n", cpu_num);

  
        CPU_ZERO(&mask);
        CPU_SET(info->start, &mask);
        if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
            fprintf(stderr, "set thread affinity failed\n");
        }
        CPU_ZERO(&get);
        if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0) {
            fprintf(stderr, "get thread affinity failed\n");
        }
        for (j = 0; j < cpu_num; j++) {
            if (CPU_ISSET(j, &get)) {
                printf("thread %d is running in processor %d\n", (int)pthread_self(), j);
            }
        }
        /*j = 0;
        while (j++ < 100000000) {
            memset(buf, 0, sizeof(buf));
        }*/
	printf("%d,%d\n",info->start,info->end);
    unsigned int seed = rand() % (100000000);
	time_t t1=time(NULL);
    int tmp;
    int index=0;
    int * list = info->list;
    int step = info->step;
    int num = 1024*1024*1024;
    int now =0;
	for (int i=0;i<num;i=i+step)
	{
	    index = (rand31_next(seed) % num);	
        index = index *step +info->start;
        now = i*step +info->start;
        tmp = list[now];
        list[now]= list[index];
        list[index]= tmp;
        //printf("zzzz:%d\n",index);
    /*       info->list[i] = info->list[i] +info->list[sq[i]];
        info->list[sq[i]] = info->list[i] - info->list[sq[i]];
        info->list[i] = info->list[i] - info->list[sq[i]];*/
//		for (int j=0;j<100;j++){};
	}
	time_t t2=time(NULL);

	printf("xxxx:%d\n",t2 -t1);
}
void * init_function2(void *arg)
{

	ForInfo * info = (ForInfo*)arg;
    cpu_set_t mask;
    cpu_set_t get;
    char buf[256];
    int i;
    int j;
    int cpu_num = sysconf(_SC_NPROCESSORS_CONF);
    printf("system has %d processor(s)\n", cpu_num);

  
        CPU_ZERO(&mask);
        CPU_SET(info->start, &mask);
        if (pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0) {
            fprintf(stderr, "set thread affinity failed\n");
        }
        CPU_ZERO(&get);
        if (pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0) {
            fprintf(stderr, "get thread affinity failed\n");
        }
        for (j = 0; j < cpu_num; j++) {
            if (CPU_ISSET(j, &get)) {
                printf("thread %d is running in processor %d\n", (int)pthread_self(), j);
            }
        }
        /*j = 0;
        while (j++ < 100000000) {
            memset(buf, 0, sizeof(buf));
        }*/
	printf("%d,%d\n",info->start,info->end);
    unsigned int seed = rand() % (100000000);
	time_t t1=time(NULL);
    int tmp;
    int index=0;
    int * list = info->list;
    int step = info->step;
    int num = 1024*1024*1024/step;
    int now =0;
	for (int i=0;i<num;i++)
	{
	    index = (rand31_next(seed) % num);	
        index = index *step +info->start;
        now = i*step +info->start;
        tmp = list[now];
        list[now]= list[index];
        list[index]= tmp;
        //printf("zzzz:%d\n",index);
    /*       info->list[i] = info->list[i] +info->list[sq[i]];
        info->list[sq[i]] = info->list[i] - info->list[sq[i]];
        info->list[i] = info->list[i] - info->list[sq[i]];*/
//		for (int j=0;j<100;j++){};
	}
	time_t t2=time(NULL);

	printf("xxxx:%d\n",t2 -t1);
}
void * init_function(void * arg)
{
	ForInfo * info = (ForInfo*)arg;
	printf("%d,%d\n",info->start,info->end);
    unsigned int seed = rand() % (100000000);
	time_t t1=time(NULL);
    int tmp;
    int index=0;
    int * list = info->list;
    int step = info->step;
    int num = 1024*1024*1024/step;
    int now =0;
	for (int i=0;i<num;i++)
	{
	    index = (rand31_next(seed) % num);	
        index = index *step +info->start;
        now = i*step +info->start;
        tmp = list[now];
        list[now]= list[index];
        list[index]= tmp;
        //printf("zzzz:%d\n",index);
    /*       info->list[i] = info->list[i] +info->list[sq[i]];
        info->list[sq[i]] = info->list[i] - info->list[sq[i]];
        info->list[i] = info->list[i] - info->list[sq[i]];*/
//		for (int j=0;j<100;j++){};
	}
	time_t t2=time(NULL);
	printf("xxxx:%d\n",t2 -t1);

}
int main()
{
srand(time(0));
int* list= (int*)malloc(1024*1024*1024*sizeof(int) +100);
#if 0
{
time_t t1 =time(NULL);

for (int i=0;i<1024*1024*1024;i++)
{
    //int n =rand31_next(seed) ;
    //printf("%d\n",n);
    //int n =rand31_next(seed) % (1024*1024*1024);
    int n =rand() % (1024*1024*1024);
	int tmp = list[i];
    list[i]= list[n];
    list[n]=tmp;	
}
time_t t2 =time(NULL);

printf("%d\n",t2- t1);
}
#endif
      /*  
time_t t3 =time(NULL);
int index=0;
int tmp=0;

for (int i=0;i<1024*1024*1024;i++)
{
    index = rand() % 1024*1024*1024;
    sq[i]= index;
}
time_t t4 =time(NULL);

printf("%d\n",t4- t3);

*/
time_t t1=time(NULL);
#if 0
#pragma omp parallel for 
for (int i=0;i<1024*1024*1024;i++)
{
	list[i]= 10000+2^16+2^15+2^13;	
	for (int j=0;j<100;j++){};
	//printf("%d\n",list[i]);
}
#else 

	int thread_num=16;
	pthread_t* pid = (pthread_t*)malloc(thread_num*sizeof(pthread_t)) ;
	int distant = 1024*1024*1024 / thread_num;
    int n = 16;
	for (int i=0;i<thread_num ;i++)
	{
		ForInfo* info = new ForInfo;
		info->list= list;
        info->start = i ;
        info->step = n;
		//info->start = distant*(i) ;
		//info->end = info->start + distant +1;
    	if (0!=pthread_create(&pid[i], NULL, init_function2, info))
    	{
        	printf("[load_valid_ad_hash][pthread_create init  Error]\n");
        	return -1;
    	}
	}    
	for (size_t i=0; i<thread_num; i++) {
		pthread_join(pid[i], NULL);
	}
#endif
time_t t2=time(NULL);
printf("%d\n",t2- t1);
/*
t1 =time(NULL);
int index=0;
int tmp=0;

for (int i=0;i<1024*1024*1024;i++)
{
    index = sq[i];
	tmp = list[i];
    list[i]= list[index];
    list[index]=tmp;	
}
t2 =time(NULL);

printf("%d\n",t2- t1);
*/
}
