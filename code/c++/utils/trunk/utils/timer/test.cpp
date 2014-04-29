#include <stdio.h>
#include "stimer.h"
void timeout_cb(void * arg)
{
fprintf(stderr,"hahhaha\n");
//STimer* t=(STimer*)arg;
//t->Add(timeout_cb,arg,1000);
}
int main()
{

STimer timer;
timer.SetTimeout(1000);
int ret=timer.Start();
fprintf(stderr,"ret=%d\n",ret);
fprintf(stderr,"h121212121212\n");
int id=timer.Add(timeout_cb,(void *)&timer);
fprintf(stderr,"id=%d\n",id);
pause();
}
