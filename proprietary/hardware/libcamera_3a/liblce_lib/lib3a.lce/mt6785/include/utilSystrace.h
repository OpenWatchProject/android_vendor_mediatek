#ifndef _UTIL_SYSTRACE_H_
#define _UTIL_SYSTRACE_H_


#ifndef ATRACE_TAG
#define ATRACE_TAG                              ATRACE_TAG_CAMERA
#endif
//
#ifdef ANDROID 
#include <utils/Trace.h>
#define UTIL_TRACE_NAME_LENGTH                   32
#define UTIL_TRACE_CALL()                        ATRACE_CALL()
#define UTIL_TRACE_NAME(name)                    ATRACE_NAME(name)
#define UTIL_TRACE_INT(name, value)              ATRACE_INT(name, value)
#define UTIL_TRACE_BEGIN(name)                   ATRACE_BEGIN(name)
#define UTIL_TRACE_END()                         ATRACE_END()
#define UTIL_TRACE_ASYNC_BEGIN(name, cookie)     ATRACE_ASYNC_BEGIN(name, cookie)
#define UTIL_TRACE_ASYNC_END(name, cookie)       ATRACE_ASYNC_END(name, cookie)
#define UTIL_TRACE_FMT_BEGIN(fmt, arg...)                    \
do{                                                          \
    if( ATRACE_ENABLED() )                                   \
    {                                                        \
        char buf[UTIL_TRACE_NAME_LENGTH];                    \
        snprintf(buf, UTIL_TRACE_NAME_LENGTH, fmt, ##arg);   \
        UTIL_TRACE_BEGIN(buf);                               \
    }                                                        \
}while(0)
#define UTIL_TRACE_FMT_END()                     UTIL_TRACE_END()
#else
#define UTIL_TRACE_CALL(...)
#define UTIL_TRACE_NAME(...)
#define UTIL_TRACE_INT(...)
#define UTIL_TRACE_BEGIN(...)
#define UTIL_TRACE_END(...)
#define UTIL_TRACE_ASYNC_BEGIN(...)
#define UTIL_TRACE_ASYNC_END(...)
#define UTIL_TRACE_FMT_BEGIN(...)
#define UTIL_TRACE_FMT_END(...)
#endif

#endif  // _UTIL_SYSTRACE_H_

