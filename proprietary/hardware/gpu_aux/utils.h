
#ifndef __GPUAUX_UTILS_H__
#define __GPUAUX_UTILS_H__

#define MTK_UNUSED_PARAMETER(x) ((void)x)
#define ALIGN(x,a)      (((x) + (a) - 1L) & ~((a) - 1L))

#define LOG_NDEBUG 0

#include <cutils/log.h>

//#define GPUAUX_LOG_PRINTF

#ifndef GPUAUX_LOG_PRINTF
#define MTK_LOGV(fmt, ...)      ALOGV("[AUX]%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTK_LOGD(fmt, ...)      ALOGD("[AUX]%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTK_LOGI(fmt, ...)      ALOGI("[AUX]%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTK_LOGW(fmt, ...)      ALOGW("[AUX]%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define MTK_LOGE(fmt, ...)      ALOGE("[AUX]%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define MTK_LOGV(fmt, ...)      do { ALOGV("[AUX]%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); printf("V: [AUX]%s:%d: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); } while(0)
#define MTK_LOGD(fmt, ...)      do { ALOGD("[AUX]%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); printf("D: [AUX]%s:%d: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); } while(0)
#define MTK_LOGI(fmt, ...)      do { ALOGI("[AUX]%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); printf("I: [AUX]%s:%d: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); } while(0)
#define MTK_LOGW(fmt, ...)      do { ALOGW("[AUX]%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); printf("W: [AUX]%s:%d: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); } while(0)
#define MTK_LOGE(fmt, ...)      do { ALOGE("[AUX]%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); printf("E: [AUX]%s:%d: "fmt"\n", __FUNCTION__, __LINE__, ##__VA_ARGS__); } while(0)
#endif

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <utils/Trace.h>

#define MTK_ATRACE_BEGIN(name)  atrace_begin(ATRACE_TAG, name)
#define MTK_ATRACE_END(name)    atrace_end(ATRACE_TAG)

#endif
