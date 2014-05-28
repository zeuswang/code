/**
 * @file ievent.h
 * @author wangwei115120@sogou-inc.com
 * @date 2014/04/27 
 * @brief
 *
 **/
#include "reactor.h"

#ifndef  __IEVENT_H_
#define  __IEVENT_H_
namespace ub
{
	class NetReactor;
	class IEvent
	{
		public:
			enum {
				INIT,//��ʼ��״̬
				READY,//�ȴ�״̬
				CONTINUE,//���ڱ�����
				DONE,//�������
				/*CANCELED,*///��ȡ��
			};

			enum {
				TIMEOUT = 1UL,	//��ʱ�¼�
				IOREADABLE = 1UL<<1UL,	//io�ɶ��¼�
				IOWRITEABLE = 1UL<<2UL,	//io��д�¼�
				SIGNAL = 1UL<<3UL,	//ϵͳ�ź��¼�
				CLOSESOCK = 1UL<<4UL,	//�Զ˹رվ��
				ERROR = 1UL<<5UL,	//δ֪����
				CPUEVENT = 1UL<<6UL,	//cpu�¼�
				CANCELED = 1UL<<7UL,	//�¼���ȡ��
				SOCKERROR = 1UL<<8UL,   //socket�Ϸ��������¼�

				//REACTOR_DESTROY = 1UL<<8UL,	//reactor���ټ�����¼�,	�´��������ڼ�
			};

			//�¼��ص�����������
			//typedef void (*cb_t)(IEvent *, void *);
		public:


			/**
			 * @brief �����¼��ľ��
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:18:45
			**/
			virtual int handle() = 0;
			/**
			 * @brief �����¼��ľ��
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:18:52
			**/
			virtual void setHandle(int) = 0;

			/**
			 * @brief �����¼���reactor
			 *
			 * @return  IReactor*
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:19:03
			**/
			virtual NetReactor *reactor() = 0;
			/**
			 * @brief ���ü������¼���reactor
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:19:15
			**/
			virtual void setReactor(NetReactor *) = 0;

			/**
			 * @brief reactor�����¼���Ļص�
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:19:29
			**/
			virtual void callback() = 0;
			/**
			 * @brief �����¼��߼���������еĻص�
			 *
			 * @param [in/out] cb   : cb_t
			 * @param [in/out] p   : void*
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:19:40
			**/
			//virtual void setCallback(cb_t cb, void *p) = 0;

			//TODO
			//virtual int getCallback(cb_t *pcb, void ** pprm)=0;

			virtual bool isTimeout() = 0;
			virtual void setTimeout_ms(int tv) = 0;

			virtual int clear() = 0;

			/**
			 * @brief ��ȡ�����ԭ����bitλ��ѯ
			 * TIMEOUT = 1UL,	//��ʱ�¼�
			 * IOREADABLE = 1UL<<1UL,	//io�ɶ��¼�
			 * IOWRITEABLE = 1UL<<2UL,	//io��д�¼�
			 * SIGNAL = 1UL<<3UL,	//ϵͳ�ź��¼�
			 * CLOSESOCK = 1UL<<4UL,	//�Զ˹رվ��
			 * ERROR = 1UL<<5UL,	//δ֪����
			 * CPUEVENT = 1UL<<6UL,	//cpu�¼�
			 * CANCELED = 1UL<<7UL,	//�¼���ȡ��
			 *
			 * @return  int  �����¼�����ԭ��
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:21:38
			**/
			virtual int events() = 0;

			/**
			 * @brief �����¼�����ԭ��
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:22:35
			**/
			virtual void setEvents(int) = 0;

			/**
			 * @brief �����¼�������
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:22:45
			**/
			virtual int type() = 0;
			/**
			 * @brief �����¼�������
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:22:59
			**/
			virtual void setType(int) = 0;

			//�¼�״̬
			/**
			 * @brief �����¼�Ŀǰ��״̬
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:23:10
			**/
			virtual int status() = 0;
			/**
			 * @brief ��ѯ�¼�Ŀǰ��״̬
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:23:19
			**/
			virtual void setStatus(int) = 0;

			/**
			 * @brief ���ü���+1�������ؽ��
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:23:28
			**/
			virtual int addRef() = 0;
			/**
			 * @brief ���ü���-1�������ؽ��
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:23:43
			**/
			virtual int delRef() = 0;
			/**
			 * @brief �������ü���
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:23:57
			**/
			virtual int getRefCnt() = 0;
			/**
			 * @brief �Զ��ͷ���Դ
			 *
			 * @return  bool
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:24:05
			**/
			virtual bool release() = 0;

			/**
			 * @brief �����û��¼�������ڴ��
			 *
			 * @return  bsl::mempool*
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:24:40
			**/
			//virtual bsl::mempool * pool() { return NULL; }

			/**
			 * @brief �����¼�����һ��ָ��
			 *
			 * @return  IEvent*
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:24:59
			**/
			virtual IEvent * next() = 0;	//���ָ��
			/**
			 * @brief �����¼�����ָ�����һ��ָ��
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:25:07
			**/
			virtual void setNext(IEvent *) = 0;
			/**
			 * @brief �����¼���ǰһ��ָ��
			 *
			 * @return  IEvent*
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:25:18
			**/
			virtual IEvent * previous() = 0;	//��ǰָ��
			/**
			 * @brief �����¼����е�ǰһ��ָ��
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:25:29
			**/
			virtual void setPrevious(IEvent *) = 0;
         


			IEvent() {}

			virtual ~IEvent() {}

			virtual bool isError(){return false;};

	};

//�����¼�ָ����֯���¼����й�����
class EQueue
{
	public:
		EQueue() : _begin(0), _end(0) {}
		~EQueue() {}
		/**
		 * @brief ����¼�����
		 *
		 * @return  void 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:25:57
		**/
		inline void clear() { _begin = _end = 0; }
		/**
		 * @brief �ж϶����Ƿ�Ϊ��
		 *
		 * @return  bool 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:26:06
		**/
		inline bool empty() { return _begin == NULL; }
		//��֤һ���߳�push��һ���̱߳����̰߳�ȫ
		/**
		 * @brief ���¼��ŵ��¼�������ȥ
		 *
		 * @param [in/out] ev   : IEvent*
		 * @return  void 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:26:15
		**/
		void push(IEvent *ev);
		/**
		 * @brief ������ͷ���¼�����
		 *
		 * @return  IEvent* 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:26:27
		**/
		IEvent *pop();
		/**
		 * @brief ���ض���ͷ
		 *
		 * @return  IEvent* 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:26:41
		**/
		inline IEvent *begin() { return _begin; }
		/**
		 * @brief ɾ���¼�
		 *
		 * @param [in/out] ev   : IEvent*
		 * @return  void 
		 * @retval   
		 * @see 
		 * @author xiaowei
		 * @date 2009/06/10 16:26:54
		**/
		void erase(IEvent *ev);
	public:
		IEvent *_begin;
		IEvent *_end;
};


//�̰߳�ȫ�ķ�װ
class ELQueue
{
    public:
        /**
         * @brief ����¼�����
         *
         * @return  void 
         * @retval   
         * @see 
         * @author xiaowei
         * @date 2009/06/10 16:25:57
        **/
        inline void clear() {
            AutoSLock __l(_lock);
            _queue.clear();
        }
        /**
         * @brief �ж϶����Ƿ�Ϊ��
         *
         * @return  bool 
         * @retval   
         * @see 
         * @author xiaowei
         * @date 2009/06/10 16:26:06
        **/
        inline bool empty() {
            return _queue.empty();
        }
        //��֤һ���߳�push��һ���̱߳����̰߳�ȫ
        /**
         * @brief ���¼��ŵ��¼�������ȥ
         *
         * @param [in/out] ev   : IEvent*
         * @return  void 
         * @retval   
         * @see 
         * @author xiaowei
         * @date 2009/06/10 16:26:15
        **/
        inline void push(IEvent *ev) {
            AutoSLock __l(_lock);
            _queue.push(ev);
        }

        /**
         * @brief ���¼�����
         *
         * @param [in/out] ev   : IEvent*
         * @return  void 
         * @retval   
         * @see 
         * @author xiaowei
         * @date 2009/06/10 16:26:15
        **/
        inline IEvent * pop() {
            AutoSLock __l(_lock);
            return _queue.pop();
        }
        /**
         * @brief ���ض���ͷ
         *
         * @return  IEvent* 
         * @retval   
         * @see 
         * @author xiaowei
         * @date 2009/06/10 16:26:41
        **/
        inline IEvent *begin() {
            AutoSLock __l(_lock);
            return _queue.begin();
        }
        /**
         * @brief ɾ���¼�
         *
         * @param [in/out] ev   : IEvent*
         * @return  void 
         * @retval   
         * @see 
         * @author xiaowei
         * @date 2009/06/10 16:26:54
        **/
        inline void erase(IEvent *ev) {
            AutoSLock __l(_lock);
            _queue.erase(ev);
        }

    private:
        SpinLock _lock;
        EQueue _queue;
};



};
#endif  //__IEVENT_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
