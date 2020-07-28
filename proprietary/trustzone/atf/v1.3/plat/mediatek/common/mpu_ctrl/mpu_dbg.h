/*
 * Copyright (c) 2018 MediaTek Inc.
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

#ifndef __MPU_DBG_H__
#define __MPU_DBG_H__

#include <arch_helpers.h>
#include <debug.h>

#define MPU_DEBUG_ENABLE (0)
#define MPU_LOG_TAG_ENABLE (1)

#if MPU_LOG_TAG_ENABLE
#define MPU_TAG "[MPU]"
#else
#define MPU_TAG
#endif

#if (1 == TARGET_BUILD_VARIANT_ENG) || (1 == MPU_DEBUG_ENABLE)
#define MPU_DUMP_ENABLE
#endif

/* clang-format off */
#if (1 == MPU_DEBUG_ENABLE)
#define MPU_TRACE(fmt, ...) \
	VERBOSE(MPU_TAG "[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#define MPU_TRACE_FN() \
	VERBOSE(MPU_TAG "[%s] %d\n", __func__, __LINE__)
#define MPU_DBG(fmt, ...) \
	NOTICE(MPU_TAG "[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#else
#define MPU_TRACE(fmt, ...) do {} while (0)
#define MPU_TRACE_FN()      do {} while (0)
#define MPU_DBG(fmt, ...)   do {} while (0)
#endif

#if (1 == TARGET_BUILD_VARIANT_ENG)
#define MPU_DBG_ENG(fmt, ...) \
	NOTICE(MPU_TAG "[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#else
#define MPU_DBG_ENG(fmt, ...)   do {} while (0)
#endif

#define MPU_FORCE(fmt, ...) \
	NOTICE(MPU_TAG "[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#define MPU_ERR(fmt, ...) \
	ERROR(MPU_TAG "[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#define MPU_ERR_FN(fmt, ...) \
	ERROR(MPU_TAG "[%s][%s] %d" fmt, LOG_TAG, \
	      __func__, __LINE__, ##__VA_ARGS__)
/* clang-format on */

#define UNUSED(x) (void)(x)
#define IS_ZERO(val) (val == 0)
#define NOT_ZERO(val) (val != 0)
#define VALID(ptr) (ptr != NULL)
#define INVALID(ptr) (ptr == NULL)

#endif /* __MPU_DBG_H__ */
