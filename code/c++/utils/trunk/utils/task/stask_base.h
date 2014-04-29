#ifndef _STASK_BASE_HPP
#define _STASK_BASE_HPP

#include <pthread.h>
#include <list>
#include <signal.h>
#include <stdlib.h>

template<typename T>
class swait_list_t
{
public:
	swait_list_t(): _alive(1),_num(0)
	{
		pthread_mutex_init(&_mutex, NULL);
		pthread_cond_init(&_cond, NULL);
	}

	~swait_list_t()
	{
		pthread_cond_destroy(&_cond);
		pthread_mutex_destroy(&_mutex);
	}

	int length()
	{
		int size=0;
		pthread_mutex_lock(&_mutex);
		size = _list.size();
		pthread_mutex_unlock(&_mutex);
		return size;
	}

	void put(T ptr)
	{
		pthread_mutex_lock(&_mutex);
		if (_alive)
		{
			_list.push_back(ptr);
			++_num;
		}
		pthread_cond_signal(&_cond);
		pthread_mutex_unlock(&_mutex);
	}

	int get(T& ptr)
	{
		int ret=0;
		pthread_mutex_lock(&_mutex);
		while (_alive&&_list.empty()){
			//fprintf(stderr,"list = empty\n");
			pthread_cond_wait(&_cond, &_mutex);
			if (!_alive)break;
		}
			//fprintf(stderr,"list = empty done\n");
		if (_alive)
		{
			ptr = _list.front();
			_list.pop_front();
			--_num;
		}
		else
		{
			//fprintf(stderr,"get ret = -1\n");
			ret = -1;
		}
		pthread_mutex_unlock(&_mutex);
		return ret;
	}

	void flush()
	{
		pthread_mutex_lock(&_mutex);
		_alive = 0;
		pthread_cond_broadcast(&_cond);
		pthread_mutex_unlock(&_mutex);
	}

protected:
	pthread_mutex_t _mutex;
	pthread_cond_t _cond;
	int _alive;
	int _num;
	std::list<T> _list;
};

template<typename T>
class STaskBase {
	public:
		STaskBase():m_thread(NULL), m_thread_num(0),m_stack_size(256*1024){};
		virtual ~STaskBase(){};

		virtual int Open(size_t thread_num);
		virtual int Activate();
		virtual int Close();
		virtual int Stop();
		virtual int Svc() = 0;
		virtual int Put(T);
		virtual int Get(T&);
		virtual int Join();		
		virtual int GetCount();		
		void SetStackSize(size_t stack_size);

	private:
		static void* RunSvc(void *arg);

	protected:
		pthread_t *m_thread;
		size_t m_thread_num;
		size_t m_stack_size;
		pthread_barrier_t m_barrier;	
		swait_list_t<T>  m_list;
};
template<typename T>
int STaskBase<T>::Open(size_t thread_num)
{
	int ret = -1;
	size_t i;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	do {
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		if (pthread_attr_setstacksize(&attr, m_stack_size))
			break;

		if (thread_num == 0 || (m_thread = (pthread_t*)malloc(thread_num * sizeof(pthread_t))) == NULL)
			break;

		pthread_barrier_init(&m_barrier, NULL, thread_num + 1);
		for (i=0; i<thread_num; i++)
			if (pthread_create(m_thread+i, &attr, RunSvc, this))
				break;

		if ((m_thread_num = i) != thread_num)
			break;

		ret = 0;
	} while (false);

	pthread_attr_destroy(&attr);
	return ret;
}
template<typename T>
void STaskBase<T>::SetStackSize(size_t stack_size)
{
	m_stack_size=stack_size;
}
template<typename T>
int STaskBase<T>::Put(T msg)
{
	m_list.put(msg);
	return 0;
}
template<typename T>
int STaskBase<T>::Get(T &msg)
{
	int ret = m_list.get(msg);
	if (ret==0)
		return 0;
	else{ 
		return -1;
		//fprintf(stderr,"STaskBase<T>::Get error!\n");
	}
}

template<typename T>
int STaskBase<T>::Activate()
{
	pthread_barrier_wait(&m_barrier);
	return 0;
}
template<typename T>
int STaskBase<T>::Stop()
{
	//fprintf(stderr,"Stop ret = -1\n");
	m_list.flush();
	return 0;
}

template<typename T>
int STaskBase<T>::Join()
{
	if (m_thread) {
		for (size_t i=0; i<m_thread_num; i++) {
			pthread_kill(m_thread[i], SIGTERM);
			pthread_join(m_thread[i], NULL);
		}
		free(m_thread);
		m_thread = NULL;
		pthread_barrier_destroy(&m_barrier);
	}
	return 0;
}
template<typename T>
int STaskBase<T>::Close()
{
	STaskBase<T>::Stop();
	STaskBase<T>::Join();
	return 0;
}
template<typename T>
void* STaskBase<T>::RunSvc(void *arg)
{
	STaskBase *task = (STaskBase *)arg;
	pthread_barrier_wait(&task->m_barrier);
	task->Svc();
	return NULL;
}
template<typename T>
int STaskBase<T>::GetCount()
{
	return m_list.length();
}


#endif //QO_TASK_BASE_HPP

