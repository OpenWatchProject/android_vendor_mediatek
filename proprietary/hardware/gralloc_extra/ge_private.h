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

#ifndef __GE_PRIVATE_H__
#define __GE_PRIVATE_H__

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#include <linux/ion_drv.h>
#include <linux/mtk_ion.h>
#include <ion/ion.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#ifndef OFFSETOF
#define OFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)
#endif

#define UNUSED(x) UNUSED_ ## x __attribute__((unused))

typedef uint32_t ge_sec_hnd_t;

typedef struct {
	uint64_t _64hnd;
	GEFD ge_fd;
	ion_user_handle_t ion_hnd;
	ge_sec_hnd_t sec_hnd;
} ge_sec_hwc_t;

__END_DECLS

#endif
