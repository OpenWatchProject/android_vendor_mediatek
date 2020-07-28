#ifndef AUDIO_LOG_H
#define AUDIO_LOG_H

#ifdef ANDROID
#include <log/log.h>
#else
#include <stdio.h>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */

inline void audio_printf(const char *message, ...) {
    static char printf_msg[512];
    va_list args;
    va_start(args, message);
    vsnprintf(printf_msg, sizeof(printf_msg), message, args);
    printf("%s\n", printf_msg);
    va_end(args);
}

#define ALOGV audio_printf
#define ALOGD audio_printf
#define ALOGI audio_printf
#define ALOGW audio_printf
#define ALOGE audio_printf

#endif /*end of ANDROID */




#ifndef AUD_LOG_VV
#define AUD_LOG_VV(x...)
#endif

#ifndef AUD_LOG_V
#if 0
#define AUD_LOG_V(x...) ALOGD(x)
#else
#define AUD_LOG_V(x...)
#endif
#endif

#ifndef AUD_LOG_D
#define AUD_LOG_D ALOGD
#endif

#ifndef AUD_LOG_I
#define AUD_LOG_I ALOGI
#endif

#ifndef AUD_LOG_W
#define AUD_LOG_W ALOGW
#endif

#ifndef AUD_LOG_E
#define AUD_LOG_E ALOGE
#endif


#endif /* end of AUDIO_LOG_H */

