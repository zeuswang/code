#ifndef _ODS_UTILS_DEBUG_
#define _ODS_UTILS_DEBUG_

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

#endif 
