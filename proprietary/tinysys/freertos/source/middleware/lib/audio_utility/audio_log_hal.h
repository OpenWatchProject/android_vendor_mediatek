#ifndef AUDIO_LOG_HAL_H
#define AUDIO_LOG_HAL_H

#include <audio_log.h>

#define LOG_TAG ""


/* for hal common code wrapper */
#ifdef __FUNCTION__
#undef __FUNCTION__
#endif
#define __FUNCTION__ __func__

#ifdef __unused
#undef __unused
#endif
#define __unused

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) LOG_TAG " " fmt "\n"


#endif /* end of AUDIO_LOG_HAL_H */

