#ifndef __DP_DRIVER_H__
#define __DP_DRIVER_H__

#include "DpConfig.h"

#if CONFIG_FOR_OS_WINDOWS
    #include "./windows/driver/DpDriver_Win32.h"
#elif CONFIG_FOR_OS_ANDROID
    #include "./android/driver/DpDriver_Android.h"
#else
    #error "Unknown operation system!\n"
#endif

#endif  // __DP_DRIVER_H__
