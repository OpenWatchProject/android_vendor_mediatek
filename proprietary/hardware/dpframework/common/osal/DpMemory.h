#ifndef __DP_MEMORY_H__
#define __DP_MEMORY_H__

#include "DpConfig.h"

#if CONFIG_FOR_OS_WINDOWS
    #include "./windows/memory/DpMemory_Win32.h"
#elif CONFIG_FOR_OS_ANDROID
    #include "./android/memory/DpMemory_Android.h"
#else
    #error "Unknown operation system!\n"
#endif

#endif  // __DP_MEMORY_H__
