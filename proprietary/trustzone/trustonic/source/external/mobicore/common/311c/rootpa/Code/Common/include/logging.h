/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LOGGING_H
#define LOGGING_H

#ifndef LOG_TAG
#define LOG_TAG "RootPA-C"
#endif

#ifdef ANDROID
    #include <android/log.h>
    #define LOGE(...)	__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
    #define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__))
    #define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
    #ifdef __DEBUG
        #define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
    #else
        #define LOGD(scite ...)
    #endif
#else
#ifdef WIN32
    #include <stdio.h>
	#include <windows.h>
    void MyOutputFunction(const char *str, ...);
	void OutputToLogfile(char buf[]);


#ifdef __cplusplus

	extern "C" void MyOutputFunctionC(const char *str, ...);

	#define LOGE(fmt, ...)  MyOutputFunction(fmt "\n", ##__VA_ARGS__)
    #define LOGW(fmt, ...)  MyOutputFunction(fmt "\n", ##__VA_ARGS__)
    #define LOGI(fmt, ...)  MyOutputFunction(fmt "\n", ##__VA_ARGS__)

    #ifdef __DEBUG
        #define LOGD(fmt, ...)  MyOutputFunction(fmt "\n", ##__VA_ARGS__)
    #else
        #define LOGD(fmt, ...)
    #endif
#else

	#define LOGE(fmt, ...)  MyOutputFunctionC(fmt "\n", ##__VA_ARGS__)
    #define LOGW(fmt, ...)  MyOutputFunctionC(fmt "\n", ##__VA_ARGS__)
    #define LOGI(fmt, ...)  MyOutputFunctionC(fmt "\n", ##__VA_ARGS__)

    #ifdef __DEBUG
        #define LOGD(fmt, ...)  MyOutputFunctionC(fmt "\n", ##__VA_ARGS__)
    #else
        #define LOGD(fmt, ...)
    #endif

#endif

#else
#ifdef TIZEN
    #include <stdio.h>
#endif /*Tizen*/
    #define LOGE(fmt, ...)  printf(fmt "\n", ##__VA_ARGS__)
    #define LOGW(fmt, ...)  printf(fmt "\n", ##__VA_ARGS__)
    #define LOGI(fmt, ...)  printf(fmt "\n", ##__VA_ARGS__)
    #ifdef __DEBUG
	    #define LOGD(fmt, ...)  printf(fmt "\n", ##__VA_ARGS__)
    #else
#ifdef TIZEN
        #define LOGD(fmt, ...)  do { } while(0)
#else
        #define LOGD(fmt, ...)
#endif/*Tizen*/
    #endif
#endif // WIN32
#endif // ANDROID
#endif // LOGGING_H
