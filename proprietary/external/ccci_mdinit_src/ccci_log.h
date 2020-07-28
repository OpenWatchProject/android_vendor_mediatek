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

#ifndef __CCCI_LOG_H__
#define __CCCI_LOG_H__


void ccci_log_print(int lvl, int md_id, const char *fmt, ...);

enum CCCI_LOG_LV {
    LV_V = 0,
    LV_D,
    LV_I,
    LV_W,
    LV_E
};

#define CCCI_LOGV(...)    do{ccci_log_print(LV_V, curr_md_id, __VA_ARGS__);}while(0)
#define CCCI_LOGD(...)    do{ccci_log_print(LV_D, curr_md_id, __VA_ARGS__);}while(0)
#define CCCI_LOGI(...)    do{ccci_log_print(LV_I, curr_md_id, __VA_ARGS__);}while(0)
#define CCCI_LOGW(...)    do{ccci_log_print(LV_W, curr_md_id, __VA_ARGS__);}while(0)
#define CCCI_LOGE(...)    do{ccci_log_print(LV_E, curr_md_id, __VA_ARGS__);}while(0)

#endif
