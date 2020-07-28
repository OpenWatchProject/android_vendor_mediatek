#ifndef __DP_LOGGER_H__
#define __DP_LOGGER_H__

#if CONFIG_FOR_OS_WINDOWS
    #include "./windows/logger/DpLogger_Win32.h"
#elif CONFIG_FOR_OS_ANDROID
    #include "./android/logger/DpLogger_Android.h"
#else
    #error "Unknown operation system!\n"
#endif

#endif  // __DP_LOGGER_H__
