#ifndef __DP_THREAD_H__
#define __DP_THREAD_H__

#include "DpConfig.h"

#if CONFIG_FOR_OS_WINDOWS
    #include "./windows/thread/DpThread_WIN32.h"
#elif CONFIG_FOR_OS_ANDROID
    #include "./android/thread/DpThread_Android.h"
#else
    #error "Unknown operation system!\n"
#endif

#endif  // __DP_THREAD_H__
