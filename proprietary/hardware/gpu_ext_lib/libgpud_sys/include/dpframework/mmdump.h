/*
 * Copyright (C) 2018-2019 MediaTek Inc.
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

#ifndef GPUD_INCLUDE_DPFRAMEWORK_MMDUMP_H_
#define GPUD_INCLUDE_DPFRAMEWORK_MMDUMP_H_

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

typedef void(*mmdump_func)(const int32_t fd, int32_t port, uint32_t size, uint32_t width,
                           uint32_t height, uint32_t color_space, uint32_t color_format,
                           uint32_t hstride, uint32_t vstride, const char *module);

typedef void(*mmdump2_func)(void *va, uint64_t uid, uint32_t size, uint32_t width,
                            uint32_t height, uint32_t color_space, uint32_t color_format,
                            uint32_t hstride, uint32_t vstride, const char *module);

extern void dump(const int32_t fd,       // ION fd to buffer
                 int32_t port,           // ion port (m4u module id)
                 uint32_t size,          // size of content in fd
                 uint32_t width,         // content width
                 uint32_t height,        // content height
                 uint32_t color_space,   // color space enumerator
                 uint32_t color_format,  // color space enumerator
                 uint32_t hstride,       // hstride
                 uint32_t vstride,       // vstride
                 const char *module);    // module name, see blow comment

extern void dump2(void *va,               // raw buffer va
                  uint64_t uid,           // ion iova(mva) or gralloc unique id
                  uint32_t size,          // size of content in fd
                  uint32_t width,         // content width
                  uint32_t height,        // content height
                  uint32_t color_space,   // color space enumerator
                  uint32_t color_format,  // color space enumerator
                  uint32_t hstride,       // hstride
                  uint32_t vstride,       // vstride
                  const char *module);    // module name, see blow comment

__END_DECLS

#endif  // GPUD_INCLUDE_DPFRAMEWORK_MMDUMP_H_
