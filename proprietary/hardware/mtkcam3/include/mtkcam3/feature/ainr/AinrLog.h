#ifndef __AINRLOG_H__
#define __AINRLOG_H__

#include "AinrDefs.h"
//#include "AinrProperty.h"

#if AINR_USING_ALOG
    #include <cutils/log.h>
    #define __LOGV ALOGV
    #define __LOGD ALOGD
    #define __LOGI ALOGI
    #define __LOGW ALOGW
    #define __LOGE ALOGE
    #define __LOGE ALOGF
#else
    #include <mtkcam/utils/std/Log.h>
    #define __LOGV CAM_LOGV
    #define __LOGD CAM_LOGD
    #define __LOGI CAM_LOGI
    #define __LOGW CAM_LOGW
    #define __LOGE CAM_LOGE
    #define __LOGF CAM_LOGF
#endif

//#include <libladder.h>

#ifndef AINR_LOG_KEYWORD
    #define AINR_LOG_KEYWORD "{Ainr}" // Original name!
#endif


/* log level condition, where AinrProperty::getDebugLevel is a static inline, cheap. */
//#define ainrLogCond(level)      __builtin_expect( ainr::AinrProperty::getDebugLevel() >= level, false )
#define ainrLogCond(level)      __builtin_expect( 3 >= level, false )


#if AINR_LOG
    #define ATRACE_TAG ATRACE_TAG_CAMERA
    #include <utils/Trace.h>
    #define ainrTraceCall()                     ATRACE_CALL()
    #define ainrTraceName(name)                 ATRACE_NAME(name)
    #define ainrTraceBegin(name)                ATRACE_BEGIN(name)
    #define ainrTraceEnd()                      ATRACE_END()
    #define ainrTraceAsyncBegin(name, cookie)   ATRACE_ASYNC_BEGIN(name, cookie)
    #define ainrTraceAsyncEnd(name, cookie)     ATRACE_ASYNC_END(name, cookie)


    #define ainrLogV(fmt, arg...)   do { __LOGV (AINR_LOG_KEYWORD fmt "\n", ##arg);} while(0)
    #define ainrLogD(fmt, arg...)   do { __LOGD (AINR_LOG_KEYWORD fmt "\n", ##arg);} while(0)
    #define ainrLogI(fmt, arg...)   do { __LOGI (AINR_LOG_KEYWORD fmt "\n", ##arg);} while(0)
    #define ainrLogW(fmt, arg...)   do { __LOGW (AINR_LOG_KEYWORD "warning:" fmt "\n", ##arg);} while(0)
    #define ainrLogE(fmt, arg...)   do { __LOGE (AINR_LOG_KEYWORD "@@@ error @@@:" fmt "\n", ##arg); } while(0)
    #define ainrLogF(fmt, arg...)   do { __LOGF (AINR_LOG_KEYWORD "@@@ fatal @@@:" fmt "\n", ##arg); } while(0)
    #define ainrFunctionIn()        do { ainrLogD3("[+] %s", __FUNCTION__);} while(0)
    #define ainrFunctionInInt(x)    do { ainrLogD3("[+] %s:%d", __FUNCTION__, x);} while(0)
    #define ainrFunctionOut()       do { ainrLogD3("[-] %s", __FUNCTION__);} while(0)
    //#define ainrDumpStack(x)        do { std::string callstack; \
    //                                     UnwindCurThreadBT(&callstack); \
    //                                     __android_log_write(ANDROID_LOG_DEBUG, LOG_TAG"/" AINR_LOG_KEYWORD, callstack.c_str()); \
    //                                } while(0)

    /* log level */
    #define ainrLogD2(...)  do { if ( ainrLogCond(2) ) ainrLogD(__VA_ARGS__);  } while(0)
    #define ainrLogD3(...)  do { if ( ainrLogCond(3) ) ainrLogD(__VA_ARGS__);  } while(0)

#else
    #define ainrTraceCall()
    #define ainrTraceName(name)
    #define ainrTraceBegin(name)
    #define ainrTraceEnd()
    #define ainrTraceAsyncBegin(name, cookie)
    #define ainrTraceAsyncEnd(name, cookie)

    #define ainrLogV(fmt, arg...)
    #define ainrLogD(fmt, arg...)
    #define ainrLogI(fmt, arg...)
    #define ainrLogW(fmt, arg...)
    #define ainrLogE(fmt, arg...)
    #define ainrLogF(fmt, arg...)
    #define ainrFunctionIn()
    #define ainrFunctionInInt(x)
    #define ainrFunctionOut()
    #define ainrDumpStack(x)

    /* log level */
    #define ainrLogD2(...)          do {} while(0)
    #define ainrLogD3(...)          do {} while(0)
#endif

#define ainrAutoLog(x)              AinrAutoLog _____l(x)
#define ainrAutoLogFunc()           AinrAutoLog _____l(__FUNCTION__)
#define ainrAutoTrace(x)
#define ainrAutoTraceFunc()         ainrTraceCall()

namespace ainr {

struct AinrAutoLog final
{
    const char *txt;
    AinrAutoLog(const char *t) noexcept : txt(nullptr) { ainrLogD3("%s +", t); txt = t; }
    ~AinrAutoLog(){ ainrLogD3("%s -", this->txt); }
};
} // namespace ainr

#endif /* __AINRLOG_H__ */
