#ifndef __DP_CONDITION_H__
#define __DP_CONDITION_H__

#include "DpConfig.h"

#if CONFIG_FOR_OS_WINDOWS
    #include "./windows/thread/DpCondition_Win32.h"
#elif CONFIG_FOR_OS_ANDROID
    #include "./android/thread/DpCondition_Android.h"
#else
    #error "Unknown operation system!\n"
#endif

#endif  // __DP_CONDITION_H__