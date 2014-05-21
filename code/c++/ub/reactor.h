#ifndef _REACTOR_H_
#define _REACTOR_H_

#include "common.h"
#include "stask_base.h"
namespace ub
{


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



    int _pipe[2];
public:
    int _cancel;

protected:
    int checker(int mint, timeval &now);
    void signalDeal();
    void signal();

    friend class NetReactor;
};

class NetReactor:public STaskBase<int>
{
public:
	NetReactor();
	~NetReactor();
    public:
        enum {
            INIT,//初始化状态
            STOP,//被停止
            RUNNING,//正在运行
            PAUSE,//
        };
        enum {
            CPU,
            NET
        };

    //int load(const comcfg::ConfigUnit &);
    int Svc();
    int Close();
    int Open(int );
    int post(IEvent *);
    void setCheckTime(int msec);


protected:
    int cancel(IEvent *);

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

#endif 
