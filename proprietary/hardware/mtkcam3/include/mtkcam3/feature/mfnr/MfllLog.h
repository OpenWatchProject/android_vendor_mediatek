#ifndef __MFLLLOG_H__
#define __MFLLLOG_H__

#include "MfllDefs.h"
#include "MfllProperty.h"

#if MFLL_USING_ALOG
    #include <cutils/log.h>
    #define __LOGV ALOGV
    #define __LOGD ALOGD
    #define __LOGI ALOGI
    #define __LOGW ALOGW
    #define __LOGE ALOGE
    #define __LOGE ALOGF
#else
    #include <mtkcam/utils/std/ULog.h>
    CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_MFNR);
    #define __LOGV CAM_ULOGMV
    #define __LOGD CAM_ULOGMD
    #define __LOGI CAM_ULOGMI
    #define __LOGW CAM_ULOGMW
    #define __LOGE CAM_ULOGME
    #define __LOGF CAM_ULOGM_FATAL
#endif

#include <libladder.h>

#ifndef MFLL_LOG_KEYWORD
    #define MFLL_LOG_KEYWORD "{Mfll}" // Original name!
#endif


/* log level condition, where MfllProperty::getDebugLevel is a static inline, cheap. */
#define mfllLogCond(level)      __builtin_expect( mfll::MfllProperty::getDebugLevel() >= level, false )


#if MFLL_LOG
    #define ATRACE_TAG ATRACE_TAG_CAMERA
    #include <utils/Trace.h>
    #define mfllTraceCall()                     CAM_ULOGM_FUNCLIFE()
    #define mfllTraceName(name)                 CAM_ULOGM_TAGLIFE(name)
    #define mfllTraceBegin(name)                CAM_ULOGM_TAG_BEGIN(name)
    #define mfllTraceEnd()                      CAM_ULOGM_TAG_END()
    #define mfllTraceAsyncBegin(name, cookie)   ATRACE_ASYNC_BEGIN(name, cookie)
    #define mfllTraceAsyncEnd(name, cookie)     ATRACE_ASYNC_END(name, cookie)


    #define mfllLogV(fmt, arg...)   do { __LOGV (MFLL_LOG_KEYWORD fmt "\n", ##arg);} while(0)
    #define mfllLogD(fmt, arg...)   do { __LOGD (MFLL_LOG_KEYWORD fmt "\n", ##arg);} while(0)
    #define mfllLogI(fmt, arg...)   do { __LOGI (MFLL_LOG_KEYWORD fmt "\n", ##arg);} while(0)
    #define mfllLogW(fmt, arg...)   do { __LOGW (MFLL_LOG_KEYWORD "warning:" fmt "\n", ##arg);} while(0)
    #define mfllLogE(fmt, arg...)   do { __LOGE (MFLL_LOG_KEYWORD "@@@ error @@@:" fmt "\n", ##arg); } while(0)
    #define mfllLogF(fmt, arg...)   do { __LOGF (MFLL_LOG_KEYWORD "@@@ fatal @@@:" fmt "\n", ##arg); } while(0)
    #define mfllFunctionIn()        do { mfllLogD3("[+] %s", __FUNCTION__);} while(0)
    #define mfllFunctionInInt(x)    do { mfllLogD3("[+] %s:%d", __FUNCTION__, x);} while(0)
    #define mfllFunctionOut()       do { mfllLogD3("[-] %s", __FUNCTION__);} while(0)
    #define mfllDumpStack(x)        do { std::string callstack; \
                                         UnwindCurThreadBT(&callstack); \
                                         __android_log_write(ANDROID_LOG_DEBUG, LOG_TAG"/" MFLL_LOG_KEYWORD, callstack.c_str()); \
                                    } while(0)

    /* log level */
    #define mfllLogD2(...)  do { if ( mfllLogCond(2) ) mfllLogD(__VA_ARGS__);  } while(0)
    #define mfllLogD3(...)  do { if ( mfllLogCond(3) ) mfllLogD(__VA_ARGS__);  } while(0)

#else
    #define mfllTraceCall()
    #define mfllTraceName(name)
    #define mfllTraceBegin(name)
    #define mfllTraceEnd()
    #define mfllTraceAsyncBegin(name, cookie)
    #define mfllTraceAsyncEnd(name, cookie)

    #define mfllLogV(fmt, arg...)
    #define mfllLogD(fmt, arg...)
    #define mfllLogI(fmt, arg...)
    #define mfllLogW(fmt, arg...)
    #define mfllLogE(fmt, arg...)
    #define mfllLogF(fmt, arg...)
    #define mfllFunctionIn()
    #define mfllFunctionInInt(x)
    #define mfllFunctionOut()
    #define mfllDumpStack(x)

    /* log level */
    #define mfllLogD2(...)          do {} while(0)
    #define mfllLogD3(...)          do {} while(0)
#endif

#define mfllAutoLog(x)              MfllAutoLog _____l(x)
#define mfllAutoLogFunc()           MfllAutoLog _____l(__FUNCTION__)
#define mfllAutoTrace(x)
#define mfllAutoTraceFunc()         mfllTraceCall()

namespace mfll {

struct MfllAutoLog final
{
    const char *txt;
    MfllAutoLog(const char *t) noexcept : txt(nullptr) { mfllLogD3("%s +", t); txt = t; }
    ~MfllAutoLog(){ mfllLogD3("%s -", this->txt); }
};
} // namespace mfll

#endif /* __MFLLLOG_H__ */
