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

#ifndef __GE_PLATFORM_H__
#define __GE_PLATFORM_H__

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

typedef struct {
int (*gralloc_extra_get_platform_format)(int in_format, uint64_t usage);
} ge_platform_fn;

extern ge_platform_fn ge_platform;

void ge_platform_wrap_init(ge_platform_fn *table);

__END_DECLS

#endif
