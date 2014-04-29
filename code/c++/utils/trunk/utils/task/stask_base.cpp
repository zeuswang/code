//#include "stask_base.h"
/*template<typename T>
STaskBase<T>::STaskBase(): m_thread(NULL), m_thread_num(0),m_stack_size(256*1024)
{
}
template<typename T>
STaskBase<T>::~STaskBase()
{
	Join();
}*/
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
int STaskBase<T>::Put(T* msg)
{
	m_list.put(msg);
}
template<typename T>
int STaskBase<T>::Get(T* &msg)
{
	msg = (T* )m_list.get();
	if (msg)
		return 0;
	else 
		return -1;
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
	Stop();
	Join();
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


