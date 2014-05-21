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
            INIT,//��ʼ��״̬
            STOP,//��ֹͣ
            RUNNING,//��������
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
