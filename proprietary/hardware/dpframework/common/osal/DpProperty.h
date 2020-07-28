#ifndef __DP_PROPERTY_H__
#define __DP_PROPERTY_H__

#include "DpConfig.h"

#if CONFIG_FOR_OS_WINDOWS
    #include "./windows/property/DpProperty_Win32.h"
#elif CONFIG_FOR_OS_ANDROID
    #include "./android/property/DpProperty_Android.h"
#else
    #error "Unknown operation system!\n"
#endif

#endif  // __DP_PROPERTY_H__
