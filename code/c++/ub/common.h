#ifndef _COMMON_H_
#define _COMMON_H_
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define CODE_INFO               __FILE__, __PRETTY_FUNCTION__, __LINE__
#define CODE_INFO_FORMAT        "[%s:%s:%u] "
#define _info(fmt, ...)\
({\
    time_t __time_buf__;\
    tm __localtime_buf__;\
    char __strftime_buf__[sizeof("19820429 23:59:59")];\
    time(&__time_buf__);\
    localtime_r(&__time_buf__, &__localtime_buf__);\
    strftime(__strftime_buf__, sizeof("19820429 23:59:59"), "%Y%m%d %H:%M:%S", &__localtime_buf__);\
    fprintf(stderr, "[INFO][%s][%08lX]"CODE_INFO_FORMAT fmt "\n",\
            __strftime_buf__, pthread_self(), CODE_INFO,##__VA_ARGS__);\
})

#define _err(fmt, ...)\
({\
	time_t __time_buf__;\
	tm __localtime_buf__;\
	char __strftime_buf__[sizeof("19820429 23:59:59")];\
	time(&__time_buf__);\
	localtime_r(&__time_buf__, &__localtime_buf__);\
	strftime(__strftime_buf__, sizeof("19820429 23:59:59"), "%Y%m%d %H:%M:%S", &__localtime_buf__);\
	fprintf(stderr, "[ERROR][%s][%08lX]"CODE_INFO_FORMAT fmt "\n",\
			__strftime_buf__, pthread_self(), CODE_INFO,##__VA_ARGS__);\
})

#define _debug(fmt, ...)\
({\
	time_t __time_buf__;\
	tm __localtime_buf__;\
	char __strftime_buf__[sizeof("19820429 23:59:59")];\
	time(&__time_buf__);\
	localtime_r(&__time_buf__, &__localtime_buf__);\
	strftime(__strftime_buf__, sizeof("19820429 23:59:59"), "%Y%m%d %H:%M:%S", &__localtime_buf__);\
	fprintf(stderr, "[DEBUG][%s][%08lX]"CODE_INFO_FORMAT fmt "\n",\
			__strftime_buf__, pthread_self(), CODE_INFO,##__VA_ARGS__);\
})


namespace ub
{


/**
 * @brief 毫秒加上当前时间
 *
 * @param [out] tv   : timeval*
 * @param [in] msec   : int
 * @return  void 
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2009/08/03 15:54:48
**/
inline void tvABS(timeval *tv, int msec)
{
    gettimeofday(tv, NULL);
    tv->tv_sec += msec/1000;
    tv->tv_usec += (msec%1000) * 1000;
    if (tv->tv_usec > 1000000) {
        tv->tv_sec ++;
        tv->tv_usec -= 1000000;
    }
}

/**
 * @brief _1 -= _2的语义
 *
 * @param [in/out] _1   : timeval&
 * @param [in] _2   : const timeval&
 * @return  timeval& 
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2009/08/03 15:55:07
**/
inline timeval & tvSelfSub(timeval &_1, const timeval &_2)
{
    _1.tv_sec -= _2.tv_sec;
    _1.tv_usec -= _2.tv_usec;
    if (_1.tv_usec < 0) {
        -- _1.tv_sec ;
        _1.tv_usec += 1000000;
    }
    return _1;
}
/**
 * @brief _1 += _2 的语义
 *
 * @param [in/out] _1   : timeval&
 * @param [in] _2   : const timeval&
 * @return  timeval& 
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2009/08/03 15:55:31
**/
inline timeval & tvSelfAdd(timeval &_1, const timeval &_2)
{
    _1.tv_sec += _2.tv_sec;
    _1.tv_usec += _2.tv_usec;
    if (_1.tv_usec >= 1000000) {
        ++ _1.tv_sec;
        _1.tv_usec -= 1000000;
    }
    return _1;
}
/**
 * @brief _1 - _2 的语义
 *
 * @param [in] _1   : const timeval&
 * @param [in] _2   : const timeval&
 * @return  const timeval 
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2009/08/03 15:56:14
**/
inline const timeval tvSub(const timeval &_1, const timeval &_2)
{
    timeval tv = _1;
    return tvSelfSub(tv, _2);
}
/**
 * @brief _1 + _2 的语义
 *
 * @param [in] _1   : const timeval&
 * @param [in] _2   : const timeval&
 * @return  const timeval 
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2009/08/03 15:56:31
**/
inline const timeval tvAdd(const timeval &_1, const timeval &_2)
{
    timeval tv = _1;
    return tvSelfAdd(tv, _2);
}
/**
 * @brief 比较 timeval
 *
 * @param [in] _1   : const timeval&
 * @param [in] _2   : const timeval&
 * @return  int     1 表示大于，0表示相等，-1表示等于
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2009/08/03 15:56:47
**/
inline int tvComp(const timeval &_1, const timeval &_2)
{
    if (_1.tv_sec > _2.tv_sec) return 1;
    if (_1.tv_sec < _2.tv_sec) return -1;
    if (_1.tv_usec > _2.tv_usec) return 1;
    if (_1.tv_usec < _2.tv_usec) return -1;
    return 0;
}
/**
 * @brief 把ms转化为timeval
 *
 * @param [in] ms   : int
 * @return  const timeval 
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2009/08/03 15:57:16
**/
inline const timeval tvMs2Tv(int ms)
{
    timeval tv;
    tv.tv_sec = ms/1000;
    tv.tv_usec = (ms%1000) * 1000;
    if (tv.tv_usec >= 1000000) {
        tv.tv_usec -= 1000000;
        tv.tv_sec ++;
    }
    return tv;
}

/**
 * @brief 把tv转化为ms
 *
 * @param [in] tv   : const timeval&
 * @return  const int 
 * @retval   
 * @see 
 * @author xiaowei
 * @date 2009/08/03 15:57:35
**/
inline const int tvTv2Ms(const timeval &tv)
{
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
};

class MLock
{
public:
	inline MLock() {
    	pthread_mutex_init(&_lock, NULL);
	};
    inline ~MLock() {
    	pthread_mutex_destroy(&_lock);
	};
    inline int lock() {
    	return pthread_mutex_lock(&_lock);
    };
    inline int unlock() {
    	return pthread_mutex_unlock(&_lock);
	};
    inline int trylock() {
    	return pthread_mutex_trylock(&_lock);
    };

private:
    pthread_mutex_t _lock;
};

class SpinLock
{
    public:
        inline SpinLock() {
            pthread_spin_init(&_lock, PTHREAD_PROCESS_PRIVATE);
        }
        inline ~SpinLock() {
            pthread_spin_destroy(&_lock);
        }
        inline int lock() {
            return pthread_spin_lock(&_lock);
        }
        inline int unlock() {
            return pthread_spin_unlock(&_lock);
        }
        inline int trylock() {
            return pthread_spin_trylock(&_lock);
        }

    private:
        pthread_spinlock_t _lock;
};

template <typename Lock>
class AutoLock
{
	public:
		inline AutoLock(Lock &l) : _lock(l) {
			l.lock();
		}
		inline ~AutoLock() {
			_lock.unlock();
		}
	private:
		Lock &_lock;
};
typedef AutoLock<SpinLock> AutoSLock;
typedef AutoLock<MLock> AutoMLock;




};


#endif
