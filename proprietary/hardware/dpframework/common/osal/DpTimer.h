#ifndef __DP_TIMER_H__
#define __DP_TIMER_H__

#include "DpConfig.h"

#if CONFIG_FOR_OS_WINDOWS
    #include "./windows/timer/DpTimer_Win32.h"
#elif CONFIG_FOR_OS_ANDROID
    #include "./android/timer/DpTimer_Android.h"
#else
    #error "Unknown operation system!\n"
#endif

#endif  // __DP_TIMER_H__
