#define LOG_TAG "utilPthreadCtrl"

#define MTK_LOG_ENABLE 1
#include <stdio.h>
#if defined(__ANDROID__) || defined(ANDROID)
#include <sys/resource.h>
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else // WIN32 or LINUX64
#define LOGD printf
#endif
#include "utilPThreadCtrl.h"

#include <pthread.h>

UTIL_ERRCODE_ENUM UtlSetpThreadProp(pthread_attr_t *pthreadAttr_ptr)
{
#ifndef SIM_MAIN
        if(pthreadAttr_ptr)
        {
            //@see http://www.kernel.org/doc/man-pages/online/pages/man2/sched_setscheduler.2.html
            int const policy    = pthreadAttr_ptr->sched_policy;
            int const priority  = pthreadAttr_ptr->sched_priority;
            struct sched_param sched_p;
            ::sched_getparam(0, &sched_p);

                switch(policy)
                {
                    //non-real-time
                    case SCHED_OTHER:
                    sched_p.sched_priority = 0;
                        sched_setscheduler(0, policy, &sched_p);
                    setpriority(PRIO_PROCESS, 0, priority);    //-20(high)~19(low)
                    break;

                    //real-time
                    case SCHED_FIFO:
                    case SCHED_RR:
                    default:
                      sched_p.sched_priority = priority;    //1(low)~99(high)
                        sched_setscheduler(0, policy, &sched_p);
                }
            //
            #if 0
            ::sched_getparam(0, &sched_p);
            ALOGD(
                "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d-%d)"
                , policy, ::sched_getscheduler(0)
                , priority, getpriority(PRIO_PROCESS, 0), sched_p.sched_priority
            );
            #endif
        }
#endif

    return UTIL_OK;
}

