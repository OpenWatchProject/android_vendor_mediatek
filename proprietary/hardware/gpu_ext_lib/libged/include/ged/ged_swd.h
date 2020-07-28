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

#ifndef __GED_SWD_H__
#define __GED_SWD_H__

#include <stdint.h>
#include "ged_type.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef	enum GED_SWD_FENCE_FROM_TAG
{
	GED_SWD_FENCE_FROM_QUEUE_BUFFER,
	GED_SWD_FENCE_FROM_DEQUEUE_BUFFER,
} GED_SWD_FENCE_FROM_TYPE;

GED_SWD_HANDLE ged_swd_create(int32_t i32MaxQueueCount, const char *szName);

void ged_swd_destroy(GED_SWD_HANDLE hHandle);

GED_ERROR ged_swd_push_fence(GED_SWD_HANDLE hHandle, GED_SWD_FENCE_FROM_TYPE type, int fence);

GED_ERROR ged_boost_host_event(GED_SWD_HANDLE hHandle, bool bSwitch);

#if defined (__cplusplus)
}
#endif

#endif
