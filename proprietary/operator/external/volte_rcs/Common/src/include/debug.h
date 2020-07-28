#ifndef __RCS_DEBUG_H__
#define __RCS_DEBUG_H__

#include <stdio.h>

#define SYSLOG
#define TAG         "RCS_UA"

#ifdef CPE

#ifdef SYSLOG
#include <syslog.h>
#define TrcMsg(f, a...)     syslog(LOG_DEBUG, "[" TAG "-TRC] %s:%d: " f "", &__FILE__[0], __LINE__, ## a)
#define DbgMsg(f, a...)     syslog(LOG_INFO,  "[" TAG "-DBG] %s:%d: " f "", &__FILE__[0], __LINE__, ## a)
#define ErrMsg(f, a...)     do { printf("\n[" TAG "-ERR] %s:%d: " f "", &__FILE__[0], __LINE__,  ## a); syslog(LOG_ERR,  "[" TAG "-ERR] %s:%d: " f "\n", &__FILE__[0], __LINE__, ## a); } while (0)
#define SysMsg(f, a...)     do { printf("\n[" TAG "-SYS] %s:%d: " f "", &__FILE__[0], __LINE__,  ## a); syslog(LOG_INFO,  "[" TAG "-SYS] %s:%d: " f "\n", &__FILE__[0], __LINE__, ## a); } while (0)
#define AutoTestMsg(tag, message, ...)
#else /* SYSLOG */ 
#define TrcMsg(f, a...)     printf("\n[" TAG "-TRC] %s:%d: " f "", &__FILE__[0], __LINE__,  ## a)
#define DbgMsg(f, a...)     printf("\n[" TAG "-DBG] %s:%d: " f "", &__FILE__[0], __LINE__,  ## a)
#define ErrMsg(f, a...)     printf("\n[" TAG "-ERR] %s:%d: " f "", &__FILE__[0], __LINE__,  ## a)
#define SysMsg(f, a...)     printf("\n[" TAG "-SYS] %s:%d: " f "", &__FILE__[0], __LINE__,  ## a)
#define AutoTestMsg(tag, message, ...)
#endif /* SYSLOG */ 

#elif __ANDROID__

#include <android/log.h>
#define TrcMsg(...)         __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define DbgMsg(...)         __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define ErrMsg(...)         __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define SysMsg(...)         __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define AutoTestMsg(tag, message, ...) __android_log_print(ANDROID_LOG_INFO,    "RCS_Auto_Testing", "[%d][%s] " message, imcb_auto_testing_get_id(), tag, ##__VA_ARGS__)

#else 

#define TrcMsg(f, a...)     printf("\n[" TAG "-TRC] %s:%d: " f "", &__FILE__[0], __LINE__,  ## a)
#define DbgMsg(f, a...)     printf("\n[" TAG "-DBG] %s:%d: " f "", &__FILE__[0], __LINE__,  ## a)
#define ErrMsg(f, a...)     printf("\n[" TAG "-ERR] %s:%d: " f "", &__FILE__[0], __LINE__,  ## a)
#define SysMsg(f, a...)     printf("\n[" TAG "-SYS] %s:%d: " f "", &__FILE__[0], __LINE__,  ## a)
#define AutoTestMsg(tag, message, ...)

#endif 

#endif /* __RCS_DEBUG_H__ */ 

