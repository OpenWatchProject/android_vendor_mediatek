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

GED_KPI_HANDLE ged_kpi_create(uint64_t BBQ_id);
void ged_kpi_destroy(GED_KPI_HANDLE hKPI);
GED_ERROR ged_kpi_dequeue_buffer_tag(GED_KPI_HANDLE hKPI, int32_t BBQ_api_type, int32_t fence, int32_t pid, intptr_t buffer_addr);
GED_ERROR ged_kpi_queue_buffer_tag(GED_KPI_HANDLE hKPI, int32_t BBQ_api_type, int32_t fence, int32_t pid, int32_t QedBuffer_length, intptr_t buffer_addr);
GED_ERROR ged_kpi_acquire_buffer_tag(GED_KPI_HANDLE hKPI, int32_t pid, intptr_t buffer_addr);
#endif
