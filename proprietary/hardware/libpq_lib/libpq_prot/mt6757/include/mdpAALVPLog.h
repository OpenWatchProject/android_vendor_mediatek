#ifndef MDPAALVPLOG_H
#define MDPAALVPLOG_H


/*
* header files
*/


#ifdef DRE_ANDROID_PLATFORM
#define LOG_TAG "SCLTM"
#include <android/log.h>
#include <math.h>

#define DRE_LOGD(fp, fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s: " fmt, __FUNCTION__, ##__VA_ARGS__)

#else
#include <math.h>
#include "common.h"
#include "utilities.h"

#define DRE_LOGD(fp, fmt, ...) fprintf(fp, fmt"\n", __VA_ARGS__)

#endif



#endif
