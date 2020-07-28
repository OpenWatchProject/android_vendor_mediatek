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

#ifndef __CCCI_RPCD_PLATFORM_H__
#define __CCCI_RPCD_PLATFORM_H__

#include <cutils/log.h>
#include <cutils/properties.h> /* bool type */

#ifndef true
typedef int    bool;
#define true 1
#define false 0
#endif

#define RPC_WAKE_LOCK() rpc_com_wakelock(1)
#define RPC_WAKE_UNLOCK() rpc_com_wakelock(0)

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "ccci_rpcd",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "ccci_rpcd",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "ccci_rpcd",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "ccci_rpcd",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "ccci_rpcd",__VA_ARGS__)

#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX  128
#endif

int mtk_property_get(char *property_name, char *property_val, char *default_val);
void rpc_com_wakelock(int lock);

#endif // __CCCI_RPCD_PLATFORM_H__
