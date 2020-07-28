#ifndef _TIME_PROF_H_
#define _TIME_PROF_H_

#include <stdio.h>
#define TIME_PROF
#define LOG_TAG "AppDSDN"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(ANDROID)
#include <sys/time.h>
#include <android/log.h>
#include <sys/system_properties.h>
#include <pthread.h>
#define LOGD(...)   __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...)   __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define PROPERTY_VALUE_MAX PROP_VALUE_MAX
#else
#define LOGD(fmt, ...) printf("[%s] " fmt, LOG_TAG, ##__VA_ARGS__)
#define LOGE(fmt, ...) printf("[%s] Error(%d) " fmt, LOG_TAG, __LINE__, ##__VA_ARGS__)
#define PROPERTY_VALUE_MAX (255)
#define __system_property_set(...) (0)
#define __system_property_get(...) (0)
#endif


#if defined(TIME_PROF)

#ifndef SIM_MAIN
#include <sys/time.h>
static timeval start_time, end_time;
static int temp_time;

static timeval time1, time2;
static int timeDiff;

#define MY_GET_TIME gettimeofday
#define MY_DIFF_TIME time_diff
#define MY_DISPLAY_TIME time_display
#define MY_PRINTF printf
#define MY_FOPEN(fp, filename, attrib) fp = fopen(filename, attrib);
#define MY_FPRINTF fprintf
static void time_diff(int *elapse_t, timeval start_t, timeval end_t)
{
    *elapse_t = (end_t.tv_sec - start_t.tv_sec)*1000000 + (end_t.tv_usec - start_t.tv_usec);
}
static void time_display(int elapse_t, const char *string)
{
    LOGD("%s: %8d(us) = %5.4f(ms)\n", string, elapse_t, elapse_t/1000.0f);
}
#else
#include <time.h>
static clock_t start_time, end_time;
static int temp_time;

static clock_t time1, time2;
static int timeDiff;

#define MY_GET_TIME(a, b) *a = clock();
#define MY_DIFF_TIME time_diff
#define MY_DISPLAY_TIME time_display
#define MY_PRINTF printf
#define MY_FOPEN(fp, filename, attrib) fp = fopen(filename, attrib);
#define MY_FPRINTF fprintf
static void time_diff(int *elapse_t, clock_t start_t, clock_t end_t)
{
    *elapse_t = (long long)(end_t - start_t)*1000000/CLOCKS_PER_SEC;
}
static void time_display(int elapse_t, const char *string)
{
    LOGD("%s: %8d(us) = %5.4f(ms)\n", string, elapse_t, elapse_t/1000.0f);
}
#endif

#else

#ifndef SIM_MAIN
#include <sys/time.h>
static timeval start_time, end_time;
static int temp_time;

static timeval time1, time2;
static int timeDiff;
#else
#include <time.h>
static clock_t start_time, end_time;
static int temp_time;

static clock_t time1, time2;
static int timeDiff;
#endif

#define MY_GET_TIME(a, b)
#define MY_DIFF_TIME
#define MY_DISPLAY_TIME
#define MY_PRINTF
#define MY_FOPEN(fp, filename, attrib)
#define MY_FPRINTF
#endif

#endif /* _REFOCUS_PROF_H_ */

