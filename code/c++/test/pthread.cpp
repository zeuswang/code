#include <stdio.h>
 #include<unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
static void sighandler(int signum)
{
	if (signum == SIGUSR1)
	{
    	pthread_exit(NULL);
	}
}
void * function(void * arg)
{
	while(1)
	{

	}
}
int main()
{
    signal(SIGUSR1, sighandler);    
pthread_t pids[30];
for (int i=0;i<30;i++)
{
if (pthread_create(&pids[i], NULL, function, NULL))
	printf("xxxxxx pthread_create error\n");

}
sleep(5);
		for (size_t i=0; i<30; i++) {
			pthread_kill(pids[i], SIGUSR1);
			pthread_join(pids[i], NULL);
		}


pause();

}
