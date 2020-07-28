#ifndef AUDIO_LOG_H
#define AUDIO_LOG_H

#include <stdio.h>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */

#include <FreeRTOSConfig.h>
#include <mt_printf.h>


#define pr_fmt(fmt) fmt


#ifdef AUD_LOG_D
#undef AUD_LOG_D
#endif
#define AUD_LOG_D(fmt, ...) PRINTF_D("[D]" pr_fmt(fmt), ##__VA_ARGS__)


#ifdef AUD_LOG_I
#undef AUD_LOG_I
#endif
#define AUD_LOG_I(fmt, ...) PRINTF_I("[I]" pr_fmt(fmt), ##__VA_ARGS__)


#ifdef AUD_LOG_W
#undef AUD_LOG_W
#endif
#define AUD_LOG_W(fmt, ...) PRINTF_W("[W]" pr_fmt(fmt), ##__VA_ARGS__)


#ifdef AUD_LOG_E
#undef AUD_LOG_E
#endif
#define AUD_LOG_E(fmt, ...) PRINTF_E("[E]" pr_fmt(fmt), ##__VA_ARGS__)



#ifdef AUD_LOG_V
#undef AUD_LOG_V
#endif
#if 0
#define AUD_LOG_V(x...) AUD_LOG_D(x)
#else
#define AUD_LOG_V(x...)
#endif

#ifdef AUD_LOG_VV
#undef AUD_LOG_VV
#endif
#define AUD_LOG_VV(x...)



/* for hal common code wrapper */
#ifdef ALOGV
#undef ALOGV
#endif
#define ALOGV AUD_LOG_V

#ifdef ALOGD
#undef ALOGD
#endif
#define ALOGD AUD_LOG_D

#ifdef ALOGI
#undef ALOGI
#endif
#define ALOGI AUD_LOG_I

#ifdef ALOGW
#undef ALOGW
#endif
#define ALOGW AUD_LOG_W

#ifdef ALOGE
#undef ALOGE
#endif
#define ALOGE AUD_LOG_E



/* for log if */
#ifdef ALOGD_IF
#undef ALOGD_IF
#endif
#define ALOGD_IF(cond, ...) \
    ((cond) \
     ? AUD_LOG_D(__VA_ARGS__) \
     : (void) 0)


#ifdef ALOGW_IF
#undef ALOGW_IF
#endif
#define ALOGW_IF(cond, ...) \
    ((cond) \
     ? AUD_LOG_W(__VA_ARGS__) \
     : (void) 0)


#ifdef ALOGE_IF
#undef ALOGE_IF
#endif
#define ALOGE_IF(cond, ...) \
    ((cond) \
     ? AUD_LOG_E(__VA_ARGS__) \
     : (void) 0)



/* for kernel common code wrapper */
#ifdef pr_debug
#undef pr_debug
#endif
#define pr_debug  AUD_LOG_D

#ifdef pr_info
#undef pr_info
#endif
#define pr_info   AUD_LOG_I

#ifdef pr_notice
#undef pr_notice
#endif
#define pr_notice AUD_LOG_W

#ifdef pr_warn
#undef pr_warn
#endif
#define pr_warn AUD_LOG_W

#ifdef pr_err
#undef pr_err
#endif
#define pr_err AUD_LOG_E




#endif /* end of AUDIO_LOG_H */

