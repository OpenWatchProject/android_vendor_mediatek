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

#ifndef PMEM_DBG_H
#define PMEM_DBG_H

#include <iostream>

enum PMEM_ERROR {
    PMEM_SUCCESS = 0,
    PMEM_FAIL = 1,
    PMEM_NOT_IMPLEMENTED = 2,
    PMEM_OPEN_TDRIVER_FAIL = 3,
    PMEM_OPEN_TA_FAIL = 4,
    PMEM_CLOSE_TDRIVER_FAIL = 5,
    PMEM_CLOSE_TA_FAIL = 6,
    PMEM_INVOKE_CMD_FAIL = 7,
    PMEM_PATTERN_CHECK_FAIL = 8,
    PMEM_PROC_ENTRY_DISABLE = 9,
    PMEM_PROC_READ_FAIL = 10,
    PMEM_PROC_MOCK_GZ = 11,

    PMEM_ERR_MAX = 0xFFFFFFFF
};

#define UNUSED(x) (void)x
#define VALID(ptr) (ptr != NULL)
#define INVALID(ptr) (ptr == NULL)
#define IS_ZERO(val) (0 == val)

// clang-format off
#ifdef PMEM_DEBUG_BUILD
#define PMEM_DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define PMEM_INFO(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#else
#define PMEM_DEBUG(...) do {} while (0)
#define PMEM_INFO(...)  do {} while (0)
#endif
#define PMEM_WARN(...)  __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define PMEM_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using std::cout;
#define PRINT_MSG_CONSOLE(msg) std:cout << msg

static void CONSOLE_LOG()
{
    std::cout << std::endl;
}

template<typename Head, typename... Args>
static void CONSOLE_LOG(const Head& head, const Args&... args)
{
    std::cout << head << " ";
    CONSOLE_LOG(args...);
}

#define PRINT_TAG_DBG    "[ DEBUG    ]"
#define PRINT_TAG_INF    "[ INFO     ]"
#define PRINT_TAG_ERR    "[ ERROR    ]"
#define PRINT_TAG_FORCE  "[ FORCE    ]"

#ifdef PMEM_DEBUG_BUILD
#define CONSOLE_DBG(...)    CONSOLE_LOG(PRINT_TAG_DBG, __VA_ARGS__)
#define CONSOLE_INFO(...)   CONSOLE_LOG(PRINT_TAG_INF, __VA_ARGS__)
#else
#define CONSOLE_DBG(...)  do {} while (0)
#define CONSOLE_INFO(...) do {} while (0)
#endif
#define CONSOLE_ERR(...)    CONSOLE_LOG(PRINT_TAG_ERR, __VA_ARGS__)
#define CONSOLE_FORCE(...)  CONSOLE_LOG(PRINT_TAG_FORCE, __VA_ARGS__)

// clang-format on

#endif /* end of PMEM_DBG_H */
