 
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
		 * @brief 创建epoll
		 *
		 * @param [in/out] size   : size_t
		 * @return  int 成功返回0
		 * @retval   
		 * @see 
		 * @author xiaowei 
		 * @date 2009/06/10 16:39:20
		**/
		int create(size_t size);
		/**
		 * @brief 销毁epoll
		 *
		 * @return  void 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:39:30
		**/
		void destroy();
		/**
		 * @brief 添加事件句柄
		 *
		 * @return  int 	成功返回0
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:39:35
		**/
		int add(IEvent *, int type);
		/**
		 * @brief 删除事件句柄
		 *
		 * @return  int 	成功返回0
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:39:43
		**/
		int del(IEvent *);
		/**
		 * @brief 开始轮询
		 *
		 * @param [in/out] timeout   : int	轮询的超时时间
		 * @return  int 毫秒超时
		 * @retval   返回处理的事件数
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:40:04
		**/
		int poll(int timeout);

		/**
		 * @brief 初始化自己的线程id
		 *
		 * @return  void 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:40:43
		**/
		inline void initPthreadId() { _selfid = pthread_self(); }
		/**
		 * @brief 获取线程id
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
    * @brief 从配置中初始化reactor
    *
    * @return  int 成功返回0，其他失败
    * @retval   
    * @see 
    * @author xiaowei
    * @date 2009/06/10 14:07:25
    **/
    int Svc();
    int load(const comcfg::ConfigUnit &);


    /**
    * @brief 抛起事件，进入事件循环
    * 	如果发起的事件，fd==-1，该事件会被马上执行
    *
    * @return  int 0 表示事件抛起成功 
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
