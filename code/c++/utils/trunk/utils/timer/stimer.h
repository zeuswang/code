#ifndef _STIMER_H_
#define _STIMER_H_
/*#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <event.h>
#include <evutil.h>
#include <utils/stask_base.h>
#include <map>*/


#if 1

#include <semaphore.h>
#include <sys/time.h>
#include <utils/stask_base.h>
#include <utils/utils.h>

class STimer: public STaskBase<int>
{
	public:
		STimer();
		virtual ~STimer();

    int Start();
    virtual int Svc();
    int Stop();
		//virtual int open(size_t stack_size);
		//virtual int stop();
		//virtual int svc();

		//virtual int put(server_worker &worker);
		void SetTimeout(unsigned int timeout_ms);
		//int maxtime(unsigned int timeout_ms);
		//void register_manager(class SummaryTask* sum){
			//mSummary=sum;
		//}
		//int register_timer(unsigned int id, const timeval &time);
    int Add(void (*func)(void *),void*);
		//int unregister_timer(int timer_id);
    int Del(int timer_id);

	//private:
		//virtual int open(size_t, size_t) { return -1; }

	protected:
		struct queue_node
		{
			sogou_utils::linked_list_node_t list_node;
			int timer_id;
			timeval time;
			int flag;
			unsigned int id;
       void (*func)(void * arg);
       void * arg;
		};

		enum
		{
			TIMER_FREE,
			TIMER_BUSY,
			TIMER_TIMEOUT
		};

	protected:
		queue_node *queue_node_pool;
		sem_t free_list_sem;
		pthread_cond_t busy_list_cond;
		int busy_list_cnt;
		sogou_utils::linked_list_t<queue_node, &queue_node::list_node> free_list;
		sogou_utils::linked_list_t<queue_node, &queue_node::list_node> busy_list;
		sogou_utils::linked_list_t<queue_node, &queue_node::list_node> timeout_list;
		pthread_mutex_t list_mutex;

		timeval timeout;
		bool svc_active;

		//class SummaryTask* mSummary;
};




#endif
#if 0
class TimerEvent
{
public:
	void (*func)(void* arg);
	void *data;
	struct event event_handler;
	unsigned int index;
	class STimer * s_timer;
};

static void timeout_cb(int fd, short event, void *arg);
class STimer: STaskBase<int>
{
//friend static void timeout_cb(int fd,short event,void * arg);
public:
	STimer(){
		index_=0;
		base_=NULL;
		pthread_mutex_init(&mutex_, NULL);
		//pthread_mutex_init(&loop_mutex_, NULL);
	}
	~STimer(){
		pthread_mutex_destroy(&mutex_);
		//pthread_mutex_destroy(&loop_mutex_);
    if (base_)
      event_base_loopexit(base_,NULL);
		//if (base_)
			//event_base_free(base_);
	}
	int Start();
	unsigned int Add(void (*timeout_cb)(void *),void* arg,int timeout);
	void Del(unsigned int index);
  /*int Stop(){
    event_base_loopexit(base_,NULL);
    base_=NULL;
  }
  */
protected:
	int Svc();
  unsigned int index_;
	struct event_base * base_;
	std::map<unsigned int,TimerEvent*> event_map_;
	pthread_mutex_t mutex_;
	//pthread_mutex_t loop_mutex_;
  int pipe_read_;
  int pipe_write_;

};
static void timeout_cb(int fd, short event, void *arg)
{
	TimerEvent* ev =(TimerEvent*)arg;
	ev->func(ev->data);
	ev->s_timer->Del(ev->index);
}
/*
static void timer_stop(int fd, short event, void *arg)
{
  STimer* timer=(Stimer* )arg;
  timer->Stop();
}*/
#endif
#endif 

