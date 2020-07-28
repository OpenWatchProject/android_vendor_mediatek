

#ifndef _SHOW_LOGO_LOG_H
#define _SHOW_LOGO_LOG_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef  BUILD_LK

#include <debug.h>
#include <lib/zlib.h>

#else

#include <cutils/log.h>
#include "zlib.h"

#endif

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 0
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "libshowlogo"

#ifdef __cplusplus
}
#endif
#endif
