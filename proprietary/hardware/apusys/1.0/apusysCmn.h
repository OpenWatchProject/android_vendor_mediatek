#pragma once

#define LOG_PREFIX "[libapusys]"

#if 0 //def __ANDROID__
#undef LOG_TAG
#define LOG_TAG "apusys"
#include <utils/Trace.h>
#include <cutils/log.h>

#define LOG_VER(x, arg...)      ALOGV_IF(1, LOG_PREFIX " V :(%d)[%s] " x , ::gettid(), __FUNCTION__, ##arg)
#define LOG_DEBUG(x, arg...)    ALOGD_IF(1, LOG_PREFIX " D :(%d)[%s] " x , ::gettid(), __FUNCTION__, ##arg)
#define LOG_INFO(x, arg...)     ALOGI(LOG_PREFIX " I :(%d)[%s] " x , ::gettid(), __FUNCTION__, ##arg)
#define LOG_WARN(x, arg...)     ALOGW(LOG_PREFIX " W :[warn](%d)[%s] " x , ::gettid(), __FUNCTION__, ##arg)
#define LOG_ERR(x, arg...)      ALOGE(LOG_PREFIX " E :[error](%d)[%s] " x , ::gettid(), __FUNCTION__, ##arg)
#else
#define LOG_VER(x, arg...)      printf(LOG_PREFIX" V:[%s]" x , __func__, ##arg)
#define LOG_DEBUG(x, arg...)    printf(LOG_PREFIX" D:[%s] " x , __func__, ##arg)
#define LOG_INFO(x, arg...)     printf(LOG_PREFIX" I:[%s] " x , __func__, ##arg)
#define LOG_WARN(x, arg...)     printf(LOG_PREFIX" W:[%s] " x , __func__, ##arg)
#define LOG_ERR(x, arg...)      printf(LOG_PREFIX" E:[%s] " x , __func__, ##arg)

#define LOG_CON(x, arg...)      printf(x, ##arg);
#endif

#define DEBUG_TAG LOG_DEBUG("%d\n",__LINE__)
