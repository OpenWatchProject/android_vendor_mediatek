#define LOG_TAG "utilMetCtrl"
#define MTK_LOG_ENABLE 1
#include <stdio.h>
#include <string.h>
#include "utilMetCtrl.h"

#ifdef _CFDK_ANALYSIS_
#define MET_USER_EVENT_SUPPORT
#include "met_tag.h"
#endif

#if defined(ANDROID)
#include <android/log.h>
#define MY_LOGD(...)   __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define MY_LOGE(...)   __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
#define MY_LOGD(fmt, ...) printf("[%s] " fmt, LOG_TAG, ##__VA_ARGS__)
#define MY_LOGE(fmt, ...) printf("[%s] Error(%d) " fmt, LOG_TAG, __LINE__, ##__VA_ARGS__)
#endif

UTIL_ERRCODE_ENUM UtlMetTagInit(void)
{
#ifdef _CFDK_ANALYSIS_
    if(met_tag_init()<0)
    {
        MY_LOGD("MET TAG Init Fail\n");
        return UTIL_COMMON_ERR_INVALID_PARAMETER;
    }
    else
    {
        MY_LOGD("MET TAG Init Succeed\n");
        return UTIL_OK;
    }
#else
    MY_LOGD("MET TAG is Disabled\n");
    return UTIL_OK;
#endif
}

void UtlMetTagUninit(void)
{
#ifdef _CFDK_ANALYSIS_
    met_tag_uninit();
#endif
}

void UtlMetTagStart(unsigned int class_id, const char *name)
{
#ifdef _CFDK_ANALYSIS_
    met_record_on();
    met_tag_start(class_id, name);
#endif
}

void UtlMetTagEnd(unsigned int class_id, const char *name)
{

#ifdef _CFDK_ANALYSIS_
    met_tag_end(class_id, name);
#endif
}

void UtlMetTagOneShot(unsigned int class_id, const char *name, unsigned int value)
{

#ifdef _CFDK_ANALYSIS_
    met_tag_oneshot(class_id, name, value);
#endif
}

