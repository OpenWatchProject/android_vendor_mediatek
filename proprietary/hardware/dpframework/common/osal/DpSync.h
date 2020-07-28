#ifndef __DP_SYNC_H__
#define __DP_SYNC_H__

#include "DpConfig.h"

#if CONFIG_FOR_OS_WINDOWS
    #include "./windows/sync/DpSync_Win32.h"
#elif CONFIG_FOR_OS_ANDROID
    #include "./android/sync/DpSync_Android.h"
#else
    #error "Unknown operation system!\n"
#endif

#endif  // __DP_MUTEX_H__
