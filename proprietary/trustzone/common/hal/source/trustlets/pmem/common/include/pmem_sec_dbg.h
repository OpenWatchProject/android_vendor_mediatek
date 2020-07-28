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

#ifndef PMEM_SEC_DBG_H
#define PMEM_SEC_DBG_H

/* clang-format off */
#if defined(PMEM_TA_DEBUG_ENABLE) || defined(PMEM_DRVAPI_DEBUG_ENABLE)
#define PMEM_TA_TRACE(fmt, ...) msee_ta_loge("[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#define PMEM_TA_DBG(fmt, ...)   msee_ta_logd("[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#define PMEM_TA_TRACE_FN()      msee_ta_loge("[%s] %d\n", __FUNCTION__, __LINE__)
#else
#define PMEM_TA_TRACE(fmt, ...) do {} while (0)
#define PMEM_TA_DBG(fmt, ...)   do {} while (0)
#define PMEM_TA_TRACE_FN()      do {} while (0)
#endif
#define PMEM_TA_FORCE(fmt, ...) msee_ta_loge("[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#define PMEM_TA_ERR(fmt, ...)   msee_ta_loge("[%s]" fmt, LOG_TAG, ##__VA_ARGS__)

#if defined(PMEM_TDRV_DEBUG_ENABLE)
#define PMEM_TDRV_TRACE(fmt, ...) msee_loge("[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#define PMEM_TDRV_DBG(fmt, ...)   msee_logd("[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#define PMEM_TDRV_TRACE_FN()      msee_loge("[%s] %d\n", __FUNCTION__, __LINE__)
#else
#define PMEM_TDRV_TRACE(fmt, ...) do {} while (0)
#define PMEM_TDRV_DBG(fmt, ...)   do {} while (0)
#define PMEM_TDRV_TRACE_FN()      do {} while (0)
#endif
#define PMEM_TDRV_FORCE(fmt, ...) msee_loge("[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
#define PMEM_TDRV_ERR(fmt, ...)   msee_loge("[%s]" fmt, LOG_TAG, ##__VA_ARGS__)
/* clang-format on */

#ifndef LOW_DWORD
#define LOW_DWORD(x) ((unsigned int)((x)&0xFFFFFFFF))
#endif

/* Add sizeof() checking to get rid of "right shift count >= width of type"
 * warning */
#ifndef HIGH_DWORD
#define HIGH_DWORD(x) \
    (sizeof(x) > 4 ? (unsigned int)(((x) >> 32) & 0xFFFFFFFF) : 0U)
#endif

#define UNUSED(x) (void)x
#define VALID(ptr) (ptr != NULL)
#define INVALID(ptr) (ptr == NULL)

#endif /* end of PMEM_SEC_DBG_H */
