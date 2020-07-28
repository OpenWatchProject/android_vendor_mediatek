#ifndef __VPU_COMMON_H__
#define __VPU_COMMON_H__

#define LOG_TAG "VpuStream"

#ifndef ATRACE_TAG
#define ATRACE_TAG                              ATRACE_TAG_ALWAYS
#endif

#ifdef __ANDROID__
#define VPU_LOGV ALOGV
#define VPU_LOGD ALOGD
#define VPU_LOGI ALOGI
#define VPU_LOGW ALOGW
#define VPU_LOGE ALOGE
#define VPU_LOGA ALOGA
#define VPU_LOGF ALOGF
#else
#define VPU_LOGV printf
#define VPU_LOGD printf
#define VPU_LOGI printf
#define VPU_LOGW printf
#define VPU_LOGE printf
#define VPU_LOGA printf
#define VPU_LOGF printf
#endif

#define MY_LOGV(fmt, arg...)        ALOGV_IF(getVpuLogLevel(), LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        ALOGD_IF(getVpuLogLevel(), LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        ALOGI(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        ALOGW(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        ALOGE(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        ALOGA(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        ALOGF(LOG_TAG "(%d)[%s] " fmt "\n", ::gettid(), __FUNCTION__, ##arg)

#define PROPERTY_DEBUG_LOGLEVEL "debug.vpustream.loglevel"
static int gVpuLogLevel=0;

int getVpuLogLevel(void);


//#define VPU_EARA_TEST
#define PROPERTY_DEBUG_EARATIME "debug.apu.earatime"
#define PROPERTY_DEBUG_EARAPRIORITY "debug.apu.earapriority"
#define PROPERTY_DEBUG_EARABOOSTVALUE "debug.apu.userboostvalue"

static int gEaraDisable = 0;
#define PROPERTY_DEBUG_EARADISABLE "debug.apu.earadisable"

#endif
