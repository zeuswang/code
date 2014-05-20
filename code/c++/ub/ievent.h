/***************************************************************************
 *
 * Copyright (c) 2009 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/



/**
 * @file ievent.h
 * @author xiaowei(com@baidu.com)
 * @date 2009/04/27 13:02:26
 * @brief
 *
 **/


#ifndef  __UB_IEVENT_H_
#define  __UB_IEVENT_H_

#include <new>

#include "ubinlog.h"
#include "ubutils.h"
#include "bsl/pool.h"

namespace ub
{
	class IReactor;
	class IEvent;
	//typedef unsigned long long ev_id;

	//�¼���ͳһ�ӿ�
	class IEvent
	{
		public:
			enum {
				INIT,//��ʼ��״̬
				READY,//�ȴ�״̬
				INPOOL,//�ڵȴ���ѵ
				DOING,//���ڱ�����
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
			typedef void (*cb_t)(IEvent *, void *);
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
			virtual IReactor *reactor() = 0;
			/**
			 * @brief ���ü������¼���reactor
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:19:15
			**/
			virtual void setReactor(IReactor *) = 0;

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
			virtual void setCallback(cb_t cb, void *p) = 0;

			//TODO
			virtual int getCallback(cb_t *pcb, void ** pprm)=0;

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
			virtual int result() = 0;

			/**
			 * @brief �����¼�����ԭ��
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:22:35
			**/
			virtual void setResult(int) = 0;

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
			virtual bsl::mempool * pool() { return NULL; }

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

			virtual bool isError();

	};


};
#endif  //__IEVENT_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 */
