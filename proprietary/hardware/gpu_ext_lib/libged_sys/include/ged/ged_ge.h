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

#ifndef __GED_GE_H__
#define __GED_GE_H__

#include <stdint.h>

#if defined (__cplusplus)
extern "C" {
#endif

typedef int GEFD;

GEFD ged_ge_alloc(int region_num, uint32_t *region_sizes);
int ged_ge_get(GEFD ge_fd, int region_id, int u32_offset, int u32_size, uint32_t *output_data);
int ged_ge_set(GEFD ge_fd, int region_id, int u32_offset, int u32_size, uint32_t *input_data);

typedef enum {
	GE_GET_ID, /* uint64_t */
} GE_QUERY_TYPE;

int ged_ge_query(GEFD ge_fd, GE_QUERY_TYPE type, void *out_data);

#if defined (__cplusplus)
}
#endif

#endif
