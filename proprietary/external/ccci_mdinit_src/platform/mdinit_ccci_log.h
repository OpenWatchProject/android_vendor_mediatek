/*
* Copyright (C) 2011-2017 MediaTek Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef __CCCI_MDINIT_LOG_H__
#define __CCCI_MDINIT_LOG_H__

#include <android/log.h>

#define CCCI_MD_TAG         "ccci_mdinit"
#define LOG_PRINT_V(...)    __android_log_print(ANDROID_LOG_VERBOSE, CCCI_MD_TAG, __VA_ARGS__)
#define LOG_PRINT_D(...)    __android_log_print(ANDROID_LOG_DEBUG, CCCI_MD_TAG, __VA_ARGS__)
#define LOG_PRINT_I(...)    __android_log_print(ANDROID_LOG_INFO, CCCI_MD_TAG, __VA_ARGS__)
#define LOG_PRINT_W(...)    __android_log_print(ANDROID_LOG_WARN, CCCI_MD_TAG, __VA_ARGS__)
#define LOG_PRINT_E(...)    __android_log_print(ANDROID_LOG_ERROR, CCCI_MD_TAG, __VA_ARGS__)


#endif
