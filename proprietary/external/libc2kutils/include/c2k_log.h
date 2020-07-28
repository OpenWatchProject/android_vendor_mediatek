#ifndef __INCLUDE_VIA_LOG_H__
#define __INCLUDE_VIA_LOG_H__

#include <log/log.h>//<android/log.h>//<cutils/log.h>

#ifdef RLOGV
#define LOGV RLOGV
#endif
#ifdef RLOGD
#define LOGD RLOGD
#endif
#ifdef RLOGE
#define LOGE RLOGE
#endif
#ifdef RLOGI
#define LOGI RLOGI
#endif
#ifdef RLOGW
#define LOGW RLOGW
#endif


#endif  //end of __INCLUDE_VIA_LOG_H__
