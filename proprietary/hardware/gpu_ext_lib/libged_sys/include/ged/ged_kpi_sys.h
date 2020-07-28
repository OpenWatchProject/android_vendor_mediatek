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

#ifndef __GED_KPI_H__
#define __GED_KPI_H__

#include <stdint.h>
#include "ged.h"
#include "ged_type.h"

GED_ERROR ged_kpi_create_sys(uint64_t surfaceHandle, uint64_t BBQ_id);
void ged_kpi_destroy_sys(uint64_t surfaceHandle);
GED_ERROR ged_kpi_dequeue_buffer_tag_sys(uint64_t surfaceHandle, int32_t fence, intptr_t buffer_addr);
GED_ERROR ged_kpi_queue_buffer_tag_sys(uint64_t surfaceHandle, int32_t fence, int32_t QedBuffer_length, intptr_t buffer_addr);
GED_ERROR ged_kpi_acquire_buffer_tag_sys(uint64_t surfaceHandle, intptr_t buffer_addr);
GED_ERROR ged_kpi_buffer_connect_sys(uint64_t surfaceHandle, int32_t BBQ_api_type, int32_t pid);
GED_ERROR ged_kpi_buffer_disconnect_sys(uint64_t surfaceHandle);
#endif
