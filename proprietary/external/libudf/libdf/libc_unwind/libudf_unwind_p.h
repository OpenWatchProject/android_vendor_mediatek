//#if !defined(__LIBUDF_UNWIND_P_H__)

#ifndef _PTRACE_ARCH_H
#define _PTRACE_ARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <async_safe/log.h>

#define LIBUDF_UNWIND_DEBUG 0

#if LIBUDF_UNWIND_DEBUG
#define LIBUDF_LOG(format, ...) \
    async_safe_format_log(ANDROID_LOG_DEBUG, "libudf_unwind", (format), ##__VA_ARGS__ )
#else
#define LIBUDF_LOG(format, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif // _PTRACE_ARCH_H

//#endif
