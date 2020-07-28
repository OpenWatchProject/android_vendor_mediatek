/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#ifndef _ISEE_IMSG_LOG_H_
#define _ISEE_IMSG_LOG_H_

#ifndef IMSG_TAG
#define IMSG_TAG "[ISEE]"
#endif

#if defined(MICROTRUST_TEE_DEBUG_BUILD)
#define IMSG_KLOG       0
#else
#define IMSG_KLOG       0
#endif

#include <android/log.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

__BEGIN_DECLS

void kernel_log_write(const char *fmt, ...);
int imsg_log_write(int prio, const char *tag,  const char *fmt, ...);
static inline double now_ms(void)
{
    struct timespec res;
    clock_gettime(CLOCK_MONOTONIC, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

__END_DECLS

#if IMSG_KLOG == 1
#define IMSG_KLOG_WRITE(fmt, ...)  kernel_log_write(fmt, ##__VA_ARGS__)
#else
#define IMSG_KLOG_WRITE(fmt, ...)  do { } while(0)
#endif

#define IMSG_PRINT_MAINLOG(fmt, ...) \
    do { \
        imsg_log_write(ANDROID_LOG_ERROR, IMSG_TAG, fmt, ##__VA_ARGS__); \
    } while(0)

#define IMSG_PRINT_KLOG(fmt, ...) \
    do { \
        IMSG_KLOG_WRITE("%s: " fmt, IMSG_TAG, ##__VA_ARGS__); \
    } while(0)

#define imsg_print_wrapper(android_level, fmt, ...) \
    do { \
        imsg_log_write(ANDROID_LOG_##android_level, IMSG_TAG, fmt, ##__VA_ARGS__); \
        IMSG_KLOG_WRITE("%s" fmt, IMSG_TAG, ##__VA_ARGS__); \
    } while(0)

#define IMSG_PRINT_ERROR(fmt, ...)         imsg_print_wrapper(ERROR, fmt, ##__VA_ARGS__)
#define IMSG_PRINT_WARN(fmt, ...)          imsg_print_wrapper(WARN, fmt, ##__VA_ARGS__)
#define IMSG_PRINT_INFO(fmt, ...)          imsg_print_wrapper(INFO, fmt, ##__VA_ARGS__)
#define IMSG_PRINT_DEBUG(fmt, ...)         imsg_print_wrapper(DEBUG, fmt, ##__VA_ARGS__)
#define IMSG_PRINT_TRACE(fmt, ...)         imsg_print_wrapper(VERBOSE, fmt, ##__VA_ARGS__)
#define IMSG_PRINT_PROFILE(fmt, ...)       imsg_print_wrapper(VERBOSE, fmt, ##__VA_ARGS__)
#define IMSG_PRINT_ENTER(fmt, ...)         imsg_print_wrapper(VERBOSE, fmt, ##__VA_ARGS__)
#define IMSG_PRINT_LEAVE(fmt, ...)         imsg_print_wrapper(VERBOSE, fmt, ##__VA_ARGS__)
#define IMSG_PRINT(LEVEL, fmt, ...)        IMSG_PRINT_##LEVEL("[%s]: " fmt, #LEVEL, ##__VA_ARGS__)

#define IMSG_PRINT_TIME_S(fmt, ...) \
    double start_time; \
    do { \
        start_time = now_ms(); \
        IMSG_PRINT(PROFILE, fmt" (start:%f)", ##__VA_ARGS__, start_time); \
    } while(0)

#define IMSG_PRINT_TIME_E(fmt, ...) \
    do { \
        double end_time, delta_time; \
        end_time = now_ms(); \
        delta_time = end_time - start_time; \
        IMSG_PRINT(PROFILE, fmt" (end:%f, spend:%f ms)", ##__VA_ARGS__, end_time, delta_time); \
    } while(0)

/*************************************************************************/
/* Declear macros                                                        */
/*************************************************************************/
#define IMSG_WARN(fmt, ...)         IMSG_PRINT(WARN, fmt, ##__VA_ARGS__)
#define IMSG_ERROR(fmt, ...)        IMSG_PRINT(ERROR, fmt, ##__VA_ARGS__)

#if defined(MICROTRUST_TEE_DEBUG_BUILD)
#define IMSG_INFO(fmt, ...)         IMSG_PRINT(INFO, fmt, ##__VA_ARGS__)
#define IMSG_DEBUG(fmt, ...)        IMSG_PRINT(DEBUG, fmt, ##__VA_ARGS__)
#define IMSG_TRACE(fmt, ...)        IMSG_PRINT(TRACE, fmt, ##__VA_ARGS__)
#define IMSG_ENTER()                IMSG_PRINT(ENTER, "%s\n",  __FUNCTION__)
#define IMSG_LEAVE()                IMSG_PRINT(LEAVE, "%s\n",  __FUNCTION__)
#define IMSG_PROFILE_S(fmt, ...)    IMSG_PRINT_TIME_S(fmt, ##__VA_ARGS__)
#define IMSG_PROFILE_E(fmt, ...)    IMSG_PRINT_TIME_E(fmt, ##__VA_ARGS__)
#else
#define IMSG_INFO(fmt, ...)         do { } while(0)
#define IMSG_DEBUG(fmt, ...)        do { } while(0)
#define IMSG_TRACE(fmt, ...)        do { } while(0)
#define IMSG_ENTER()                do { } while(0)
#define IMSG_LEAVE()                do { } while(0)
#define IMSG_PROFILE_S(fmt, ...)    do { } while(0)
#define IMSG_PROFILE_E(fmt, ...)    do { } while(0)
#endif

#endif //_ISEE_IMSG_LOG_H_

