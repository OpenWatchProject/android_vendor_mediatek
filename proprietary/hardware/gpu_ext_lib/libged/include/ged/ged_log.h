/*
 * Copyright (C) 2011-2014 MediaTek Inc.
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

#ifndef __GED_LOG_H__
#define __GED_LOG_H__

#include "ged_type.h"

#if defined (__cplusplus)
extern "C" {
#endif

GED_LOG_HANDLE ged_log_connect(const char* pszName);

void ged_log_disconnect(GED_LOG_HANDLE hLog);

GED_ERROR ged_log_print(GED_LOG_HANDLE hLog, const char *fmt, ...);

/* print with tpt (tpt = time, pid, and tid) */
GED_ERROR ged_log_tpt_print(GED_LOG_HANDLE hLog, const char *fmt, ...);

GED_ERROR ged_log_reset(GED_LOG_HANDLE hLog);

#if defined (__cplusplus)
}
#endif

#endif
