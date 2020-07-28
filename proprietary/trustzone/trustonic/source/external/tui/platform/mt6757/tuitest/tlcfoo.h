 /*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef TLCFOO_H_
#define TLCFOO_H_

#include "MobiCoreDriverApi.h"

mcResult_t tlcOpen(void);

//mcResult_t add(uint32_t *num1, uint32_t *num2, uint32_t *result);
//mcResult_t minus(uint32_t *num1, uint32_t *num2, uint32_t *result);

void tlcClose(void);

#define LOG_D(...) __android_log_print(ANDROID_LOG_DEBUG , LOG_TAG, __VA_ARGS__)
#define LOG_I(...) __android_log_print(ANDROID_LOG_INFO  , LOG_TAG, __VA_ARGS__)
#define LOG_W(...) __android_log_print(ANDROID_LOG_WARN  , LOG_TAG, __VA_ARGS__)
#define LOG_E(...) __android_log_print(ANDROID_LOG_ERROR , LOG_TAG, __VA_ARGS__)

#endif // TLCFOO_H_
