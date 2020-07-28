/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include <sys/types.h>
#include <tz_private/mtee_sys_fd.h>
//#include <interface/uuid/uuid.h>


enum user_mm_cmd {
        MM_FUNC_IOMMAP,
        MM_FUNC_IOMUNMAP,
        MM_FUNC_USER_VA_TO_PA,
        MM_FUNC_CLEAN_DCACHE_ALL,
        MM_FUNC_CLEAN_INVALIDATE_DCACHE_ALL,
        MM_FUNC_CLEAN_DCACHE_RANGE,
        MM_FUNC_CLEAN_INVALIDATE_DCACHE_RANGE,
        MM_FUNC_MMAP,
        MM_FUNC_MUNMAP,
        MM_FUNC_DUMP_USER_TABLE,
        MM_FUNC_INV_DCACHE_RANGE_UT,
        MM_FUNC_CLEAN_DCACHE_RANGE_UT,
		MM_FUNC_CACHEON_KERNEL_CACHEOFF_INIT,
		MM_FUNC_CACHEON_KERNEL_CACHEOFF_INV_CACHE_RANGE_01,
		MM_FUNC_CACHEON_KERNEL_CACHEOFF_INV_CACHE_RANGE_02,
		MM_FUNC_CACHEON_KERNEL_CACHEOFF_CLEAN_CACHE_RANGE,
		MM_FUNC_CACHEON_KERNEL_CACHEOFF_FLUSH,
};

typedef struct user_mm
{
        uint64_t pa;
        uint32_t size;
        uint32_t flags;
        uuid_t uuid;
        uint64_t va;
} user_mm_t;
