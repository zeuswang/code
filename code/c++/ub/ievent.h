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

	//事件的统一接口
	class IEvent
	{
		public:
			enum {
				INIT,//初始化状态
				READY,//等待状态
				INPOOL,//在等待轮训
				DOING,//正在被处理
				DONE,//处理完毕
				/*CANCELED,*///被取消
			};

			enum {
				TIMEOUT = 1UL,	//超时事件
				IOREADABLE = 1UL<<1UL,	//io可读事件
				IOWRITEABLE = 1UL<<2UL,	//io可写事件
				SIGNAL = 1UL<<3UL,	//系统信号事件
				CLOSESOCK = 1UL<<4UL,	//对端关闭句柄
				ERROR = 1UL<<5UL,	//未知错误
				CPUEVENT = 1UL<<6UL,	//cpu事件
				CANCELED = 1UL<<7UL,	//事件被取消
				SOCKERROR = 1UL<<8UL,   //socket上发生错误事件

				//REACTOR_DESTROY = 1UL<<8UL,	//reactor销毁激活的事件,	下次有需求在加
			};

			//事件回调函数的类型
			typedef void (*cb_t)(IEvent *, void *);
		public:


			/**
			 * @brief 返回事件的句柄
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:18:45
			**/
			virtual int handle() = 0;
			/**
			 * @brief 设置事件的句柄
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:18:52
			**/
			virtual void setHandle(int) = 0;

			/**
			 * @brief 返回事件的reactor
			 *
			 * @return  IReactor*
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:19:03
			**/
			virtual IReactor *reactor() = 0;
			/**
			 * @brief 设置监控这个事件的reactor
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:19:15
			**/
			virtual void setReactor(IReactor *) = 0;

			/**
			 * @brief reactor激活事件后的回调
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:19:29
			**/
			virtual void callback() = 0;
			/**
			 * @brief 设置事件逻辑处理完成有的回调
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
			 * @brief 获取激活的原因，用bit位查询
			 * TIMEOUT = 1UL,	//超时事件
			 * IOREADABLE = 1UL<<1UL,	//io可读事件
			 * IOWRITEABLE = 1UL<<2UL,	//io可写事件
			 * SIGNAL = 1UL<<3UL,	//系统信号事件
			 * CLOSESOCK = 1UL<<4UL,	//对端关闭句柄
			 * ERROR = 1UL<<5UL,	//未知错误
			 * CPUEVENT = 1UL<<6UL,	//cpu事件
			 * CANCELED = 1UL<<7UL,	//事件被取消
			 *
			 * @return  int  返回事件激活原因
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:21:38
			**/
			virtual int result() = 0;

			/**
			 * @brief 设置事件激活原因
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:22:35
			**/
			virtual void setResult(int) = 0;

			/**
			 * @brief 返回事件的类型
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:22:45
			**/
			virtual int type() = 0;
			/**
			 * @brief 设置事件的类型
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:22:59
			**/
			virtual void setType(int) = 0;

			//事件状态
			/**
			 * @brief 返回事件目前的状态
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:23:10
			**/
			virtual int status() = 0;
			/**
			 * @brief 查询事件目前的状态
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:23:19
			**/
			virtual void setStatus(int) = 0;

			/**
			 * @brief 引用计数+1，并返回结果
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:23:28
			**/
			virtual int addRef() = 0;
			/**
			 * @brief 引用计数-1，并返回结果
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:23:43
			**/
			virtual int delRef() = 0;
			/**
			 * @brief 返回引用计数
			 *
			 * @return  int
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:23:57
			**/
			virtual int getRefCnt() = 0;
			/**
			 * @brief 自动释放资源
			 *
			 * @return  bool
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:24:05
			**/
			virtual bool release() = 0;

			/**
			 * @brief 返回用户事件分配的内存池
			 *
			 * @return  bsl::mempool*
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:24:40
			**/
			virtual bsl::mempool * pool() { return NULL; }

			/**
			 * @brief 返回事件的下一个指针
			 *
			 * @return  IEvent*
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:24:59
			**/
			virtual IEvent * next() = 0;	//向后指针
			/**
			 * @brief 设置事件队列指向的下一个指针
			 *
			 * @return  void
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:25:07
			**/
			virtual void setNext(IEvent *) = 0;
			/**
			 * @brief 返回事件的前一个指针
			 *
			 * @return  IEvent*
			 * @retval
			 * @see
			 * @author xiaowei
			 * @date 2009/06/10 13:25:18
			**/
			virtual IEvent * previous() = 0;	//向前指针
			/**
			 * @brief 设置事件队列的前一个指针
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
