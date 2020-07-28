#ifndef __PQ_LOG_H__
#define __PQ_LOG_H__

#include <log/log.h>

#define PQ_LOGD(fmt, arg...) ALOGD(fmt, ##arg)
#define PQ_LOGE(fmt, arg...) ALOGE(fmt, ##arg)
#define PQ_LOGI(fmt, arg...) \
    do { \
       ALOGI_IF (__android_log_is_loggable(ANDROID_LOG_VERBOSE, "PQ", ANDROID_LOG_DEBUG), "[PQ]" fmt, ##arg); \
    }while(0)

#endif
