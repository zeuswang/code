#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>
#include <stdint.h>
struct ForInfo
{
	int start;
	int end;
	int * list;
};
void * init_function(void * arg)
{
	ForInfo * info = (ForInfo*)arg;
	printf("%d,%d\n",info->start,info->end);
	time_t t1=time(NULL);
	uint64_t sum=0;
	for (int i=info->start;i<info->end;i++)
	{
		for (int j=0;j<100;j++){
			info->list[i]= 10000+2^16+2^15+2^13;	
			//sum+= info->list[i];	
		};

		for (int j=0;j<100;j++){
			sum+= info->list[i];	
		};;
	}
	time_t t2=time(NULL);
	printf("xxxx:%d\n",t2 -t1);

};
int seed = 1242341;
long unsigned int rand31_next()
{
    long unsigned int hi,lo;
    lo = 16807 * (seed & 0xFFFF);
    hi = 16807 * (seed >>16);
    lo += (hi & 0x7FFF) << 16;
    lo += hi >>15;
    if (lo >0x7fffffff )lo -= 0x7fffffff;
    return (seed = (long)lo);
}
int main()
{
printf("%d\n",sizeof(long unsigned int)); 
srandom(time(0));
int* list= (int*)malloc(1024*1024*1024*sizeof(int) +100);
int* list2= (int*)malloc(1024*1024*1024*sizeof(int) +100);
        
time_t t1=time(NULL);
uint64_t sum =0;
#if 1
#pragma omp parallel for 
for (int i=0;i<1024*1024*1024;i++)
{
	//list[i]=rand31_next() % 10000000;	
    //int a= list[i];
    //list[i]=list[i+1]; 
    //list[i+1]=a;
	//for (int j=0;j<100;j++){};
	//printf("%d\n",list[i]);
    //list2[i]=list[i];
	for (int j=0;j<100;j++){
			list[i]= 10000+2^16+2^15+2^13;	
			//sum+= info->list[i];	
	};

	for (int j=0;j<100;j++){
		sum+= list[i];	
	};
}
#else 

	int thread_num=24;
	pthread_t* pid = (pthread_t*)malloc(thread_num*sizeof(pthread_t)) ;
	int distant = 1024*1024*1024 / thread_num;
	for (int i=0;i<thread_num ;i++)
	{
		ForInfo* info = new ForInfo;
		info->list= list;
		info->start = distant*(i) ;
		info->end = info->start + distant +1;
    	if (0!=pthread_create(&pid[i], NULL, init_function, info))
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
}
