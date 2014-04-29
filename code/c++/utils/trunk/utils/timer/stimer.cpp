#include "stimer.h"
#if 0
int  STimer::Start()
{
	/* Initalize the event library */
	base_ = event_base_new();

  	int options;
	int pipe_fd[2];

	if (pipe(pipe_fd))
		return -1;

	pipe_read_= pipe_fd[0];
	pipe_write_= pipe_fd[1];

	for (int i=0; i<2; i++)
	{
		if ((options = fcntl(pipe_fd[i], F_GETFL)) == -1)
			return -1;
		if (fcntl(pipe_fd[i], F_SETFL, options | O_NONBLOCK) == -1)
			return -1;
	}

  	struct event ev;
	event_set(&ev,pipe_read_, EV_READ, timer_stop, &ev);
  	event_base_set(base_, &t_ev->event_handler);
  	event_add(&ev, NULL);
    
	STaskBase<int>::Open(1);
	STaskBase<int>::Activate();
}

int STimer::Svc()
{
		event_base_dispatch(base_);
}

unsigned int STimer::Add(void (*timeout_func)(void *),void* arg,int timeout)
{

 	TimerEvent* t_ev= new TimerEvent();
	t_ev->index=index_;
	t_ev->s_timer=this;
	t_ev->data=arg;
	t_ev->func=timeout_func;

	struct timeval tv;
	evutil_timerclear(&tv);
	if (timeout >1000){
		tv.tv_usec = (timeout % 1000 ) *1000;
		tv.tv_sec = timeout /1000 ;
	}else if (timeout>0){
		tv.tv_usec = timeout*1000;
	}else 
		return -1;

	pthread_mutex_lock(&mutex_);
	index_++;
	/* Initalize one event */
	evtimer_set(&t_ev->event_handler, timeout_cb, t_ev);
	event_base_set(base_, &t_ev->event_handler);
	event_add(&t_ev->event_handler, &tv);

	event_map_.insert(std::map<unsigned int,TimerEvent*>::value_type(index_,t_ev));	

	pthread_mutex_unlock(&mutex_);
	pthread_mutex_unlock(&loop_mutex_);
}
	
void STimer::Del(unsigned int index)
{
	pthread_mutex_lock(&mutex_);
	
	std::map<unsigned int ,TimerEvent* >::iterator entry = 
		event_map_.find(index);	
	if (entry != event_map_.end()){
		event_del(&entry->second->event_handler);
		if (entry->second)
			delete entry->second;
		event_map_.erase(entry);
	}
	pthread_mutex_unlock(&mutex_);
}
#endif

#define TIMEOUT_ADJUST          0
#define MAX_TIME_CNT            10240
#define COND_WAIT_TIME          500

STimer::STimer()
{
	queue_node_pool = NULL;
	sem_init(&free_list_sem, 0, MAX_TIME_CNT);
	pthread_cond_init(&busy_list_cond, NULL);
	busy_list_cnt = 0;
	pthread_mutex_init(&list_mutex, NULL);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	svc_active = false;
}

STimer::~STimer()
{
	pthread_mutex_destroy(&list_mutex);
	pthread_cond_destroy(&busy_list_cond);
	sem_destroy(&free_list_sem);
	delete []queue_node_pool;
}

int STimer::Start()
{
	svc_active = true;
	queue_node_pool = new queue_node[MAX_TIME_CNT];
	for (int i=0; i<MAX_TIME_CNT; ++i)
	{
		queue_node_pool[i].timer_id = i;
		queue_node_pool[i].flag = TIMER_FREE;
		free_list.add(queue_node_pool[i]);
	}

//	return stask_base::open(1, stack_size);
  int ret =STaskBase<int>::Open(1);
  STaskBase<int>::Activate();
  return ret;
}

int STimer::Stop()
{
	pthread_mutex_lock(&list_mutex);
	svc_active = false;
	pthread_cond_signal(&busy_list_cond);
	pthread_mutex_unlock(&list_mutex);
	//stask_base::close();
	STaskBase<int>::Close();
	return 0;
}

int STimer::Svc()
{
	timeval tv;
	timeval now;
	queue_node *node;
	unsigned int id;
	int ret;

  //fprintf(stderr,"xxxxxxxxxxxxxxxx\n");
	for (;;)
	{
		//WEBSEARCH_DEBUG((LM_DEBUG,"timer svc 1 is \n"));
  //fprintf(stderr,"xxxxxxxxxxxxxxxx\n");
		pthread_mutex_lock(&list_mutex);
		while (svc_active && busy_list_cnt <= 0)
			pthread_cond_wait(&busy_list_cond, &list_mutex);
		if (!svc_active)
		{
			pthread_mutex_unlock(&list_mutex);
			break;
		}

		node = busy_list.prev(busy_list.head());
		gettimeofday(&now, NULL);
		tv.tv_sec = node->time.tv_sec + timeout.tv_sec - now.tv_sec;
		tv.tv_usec = node->time.tv_usec + timeout.tv_usec - now.tv_usec;
		if (tv.tv_usec < 0)
		{
			tv.tv_usec += 1000 * 1000;
			--tv.tv_sec;
		}
		else if (tv.tv_usec >= 1000 * 1000)
		{
			tv.tv_usec -= 1000 * 1000;
			++tv.tv_sec;
		}

		if (tv.tv_sec < 0 || (tv.tv_sec == 0 && tv.tv_usec == 0))
		{
			busy_list.del(*node);
			node->flag = TIMER_TIMEOUT;
			timeout_list.add(*node);
			--busy_list_cnt;
			id = node->id;
			pthread_mutex_unlock(&list_mutex);
			//WEBSEARCH_DEBUG((LM_DEBUG,"timer timeout is \n"));
		//	ret = mSummary->timeout(id);
		  node->func(node->arg);
			//if (ret)
				Del(node->timer_id);
		}
		else
		{
			pthread_mutex_unlock(&list_mutex);
			select(1, NULL, NULL, NULL, &tv);
		}
	}

	return 0;
}
/*
int STimer::put(server_worker &worker)
{
	return 0;
}*/

/*int STimer::maxtime(unsigned int timeout_ms)
{
	timeout.tv_sec = timeout_ms / 1000;
	timeout.tv_usec = timeout_ms % 1000 * 1000;
	return 0;
}*/
void STimer::SetTimeout(unsigned int timeout_ms)
{
	timeout.tv_sec = timeout_ms / 1000;
	timeout.tv_usec = timeout_ms % 1000 * 1000;
}

/*int STimer::register_timer(unsigned int id, const timeval &time)
{
	if (!svc_active || sem_trywait(&free_list_sem) == -1){ 
		fprintf(stderr, "[ERROR] [Server_Timer] [No queue_node]");
		return -1;
	}

	pthread_mutex_lock(&list_mutex);
	queue_node *node = free_list.next(free_list.head());
	free_list.del(*node);
	node->time = time;
	node->flag = TIMER_BUSY;
	node->id = id;
	busy_list.add(*node);
	++busy_list_cnt;
	pthread_cond_signal(&busy_list_cond);
	pthread_mutex_unlock(&list_mutex);
	return node->timer_id;
}*/

int STimer::Add(void (*func)(void *),void* arg)
{
	if (!svc_active || sem_trywait(&free_list_sem) == -1){ 
		fprintf(stderr, "[ERROR] [Server_Timer] [No queue_node]");
		return -1;
	}

	pthread_mutex_lock(&list_mutex);
	queue_node *node = free_list.next(free_list.head());
	free_list.del(*node);
  timeval time;
  gettimeofday(&time,NULL);
	node->time = time;
	node->flag = TIMER_BUSY;
	//node->id = id;
  node->func =func;
  node->arg =arg;
	busy_list.add(*node);
	++busy_list_cnt;
	pthread_cond_signal(&busy_list_cond);
	pthread_mutex_unlock(&list_mutex);
	return node->timer_id;
}

/*int STimer::unregister_timer(int timer_id)
{
	if (svc_active)
	{
		queue_node *node = &queue_node_pool[timer_id];
		pthread_mutex_lock(&list_mutex);

		if (node->flag == TIMER_FREE)
		{
			pthread_mutex_unlock(&list_mutex);
			return 1;
		}
		else if (node->flag == TIMER_TIMEOUT)
		{
			timeout_list.del(*node);
			node->flag = TIMER_FREE;
			free_list.add(*node);
			pthread_mutex_unlock(&list_mutex);
			sem_post(&free_list_sem);
			return 0;
		}
		else if (node->flag == TIMER_BUSY)
		{
			busy_list.del(*node);
			node->flag = TIMER_FREE;
			free_list.add(*node);
			--busy_list_cnt;
			pthread_mutex_unlock(&list_mutex);
			sem_post(&free_list_sem);
			return 0;
		}
	}
	return -1;
}
*/
int STimer::Del(int timer_id)
{
	if (svc_active)
	{
		queue_node *node = &queue_node_pool[timer_id];
		pthread_mutex_lock(&list_mutex);

		if (node->flag == TIMER_FREE)
		{
			pthread_mutex_unlock(&list_mutex);
			return 1;
		}
		else if (node->flag == TIMER_TIMEOUT)
		{
			timeout_list.del(*node);
			node->flag = TIMER_FREE;
			free_list.add(*node);
			pthread_mutex_unlock(&list_mutex);
			sem_post(&free_list_sem);
			return 0;
		}
		else if (node->flag == TIMER_BUSY)
		{
			busy_list.del(*node);
			node->flag = TIMER_FREE;
			free_list.add(*node);
			--busy_list_cnt;
			pthread_mutex_unlock(&list_mutex);
			sem_post(&free_list_sem);
			return 0;
		}
	}
	return -1;
}

