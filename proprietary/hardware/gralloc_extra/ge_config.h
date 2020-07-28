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

#ifndef GE_INCLUDE_CONFIG
#error "gralloc_extra_config only can be included by implementation!"
#endif

#ifndef __GE_CONFIG_H__
#define __GE_CONFIG_H__

#define GE_LIST \
	GE_STRUCT(ge_misc_info_t) \
	GE_STRUCT(ge_smvr_info_t) \
	GE_STRUCT(ge_sf_info_t) \
	GE_STRUCT(ge_sec_hwc_t) \
	GE_STRUCT(ge_hdr_info_t) \
	GE_STRUCT(ge_video_info_t) \
	GE_STRUCT(ge_hwc_info_t) \
	GE_STRUCT(ge_timestamp_info_t) \
	GE_STRUCT(ge_pq_scltm_info_t) \
	GE_STRUCT(ge_pq_mira_vision_info_t) \

#undef GE_STRUCT
#define GE_STRUCT(x) sizeof(x),

static uint32_t region_sizes[] = { GE_LIST };
static int region_num = ARRAY_SIZE(region_sizes);

#define _GE_ID_PREFIX(x) GE_ID_##x

#undef GE_STRUCT
#define GE_STRUCT(x) _GE_ID_PREFIX(x),
enum { GE_LIST };

/* Used by santiy, it will check the size of struct in 32/64 arch.
 * Report fail, if you use a arch-dep type (ex: void *).
 * Report pass, otherwise.
 */
#undef GE_STRUCT
#define GE_STRUCT(x) sizeof(x) +
const int _ge_check_size = GE_LIST + 0;

#define OFFSETOF32(TYPE, MEMBER) (OFFSETOF(TYPE, MEMBER)/sizeof(uint32_t))
#define SIZEOF32(TYPE) (sizeof(TYPE)/sizeof(uint32_t))

typedef struct {
	int id;
	int offset;
	int size;
} ge_info;

#define GEI_BY_STRUCT(x) (ge_info){(int)_GE_ID_PREFIX(x), 0, SIZEOF32(x)}
#define GEI_BY_MEMBER(x, m) (ge_info){(int)_GE_ID_PREFIX(x), OFFSETOF32(x, m), SIZEOF32(((x*)0)->m)}

#endif
