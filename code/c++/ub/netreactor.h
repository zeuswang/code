 
/**
 * @file treactor.h
 * @author wangwei115120@sogou-inc.com  
 * @date 2014-05-01
 * @brief 
 *  
 **/


#ifndef  __UB_NETREACTOR_H_
#define  __UB_NETREACTOR_H_
#include "common.h"
#include "ievent.h"
namespace ub
{
class NetReactor;
class EPollEx
{
	public:
		EPollEx();
		~EPollEx();
		/**
		 * @brief ����epoll
		 *
		 * @param [in/out] size   : size_t
		 * @return  int �ɹ�����0
		 * @retval   
		 * @see 
		 * @author xiaowei 
		 * @date 2009/06/10 16:39:20
		**/
		int create(size_t size);
		/**
		 * @brief ����epoll
		 *
		 * @return  void 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:39:30
		**/
		void destroy();
		/**
		 * @brief ����¼����
		 *
		 * @return  int 	�ɹ�����0
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:39:35
		**/
		int add(IEvent *, int type);
		/**
		 * @brief ɾ���¼����
		 *
		 * @return  int 	�ɹ�����0
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:39:43
		**/
		int del(IEvent *);
		/**
		 * @brief ��ʼ��ѯ
		 *
		 * @param [in/out] timeout   : int	��ѯ�ĳ�ʱʱ��
		 * @return  int ���볬ʱ
		 * @retval   ���ش�����¼���
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:40:04
		**/
		int poll(int timeout);

		/**
		 * @brief ��ʼ���Լ����߳�id
		 *
		 * @return  void 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:40:43
		**/
		inline void initPthreadId() { _selfid = pthread_self(); }
		/**
		 * @brief ��ȡ�߳�id
		 *
		 * @return  pthread_t 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:40:53
		**/
		inline pthread_t pid() { return _selfid; }
	private:
		int _fd;
		int _size;
		timeval _nexto;
		int _waito;
		struct epoll_event *_evs;
		pthread_t _selfid;
        MLock  _lock1;
#ifdef UB_EPOLL_USESELF_TIMEOUT
		ELQueue _elq;
#endif
		ELQueue _a_q;
		IReactor *_task;

		int _pipe[2];
	public:
		int _cancel;

	protected:
		int checker(int mint, timeval &now);
		void signalDeal();
		void signal();

		friend class NetReactor;
};
    
class NetReactor : public STaskBase<int>
{
    public:
    NetReactor();
    ~NetReactor();
    /**
    * @brief �������г�ʼ��reactor
    *
    * @return  int �ɹ�����0������ʧ��
    * @retval   
    * @see 
    * @author xiaowei
    * @date 2009/06/10 14:07:25
    **/
    int Svc();
    int load(const comcfg::ConfigUnit &);


    /**
    * @brief �����¼��������¼�ѭ��
    * 	���������¼���fd==-1�����¼��ᱻ����ִ��
    *
    * @return  int 0 ��ʾ�¼�����ɹ� 
    * @retval   
    * @see 
    * @author xiaowei
    * @date 2009/06/10 14:15:22
    **/
    int post(IEvent *);

    int cancel(IEvent *);
    void setCheckTime(int msec);

protected:

    int initVecs();
    EPollEx *getPoll(int pos);

    EPollEx * vecs_;
    int maxevs_;
    int pos_;
    int epoll_index_;
    int check_timer_;
    friend class EPollEx;
    pthread_mutex_t mutex_;
};

};

#endif  //__NETREACTOR_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
