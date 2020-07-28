/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Contains declarations of types and constants used by malloc leak
 * detection code in both, libc and libc_malloc_debug libraries.
 */
#ifndef MALLOC_DEBUG_MTK_H
#define MALLOC_DEBUG_MTK_H

#include <stdint.h>
#include <assert.h>
#include <sys/prctl.h>
#ifdef O0
//#include <private/libc_logging.h>  // bionic/libc/private/libc_logging.h
#else
#include <async_safe/log.h>
#endif

#include "recorder.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG15_EXPORT __attribute__ ((visibility("default")))

// #define LOCAL_DEBUG

#ifdef LOCAL_DEBUG
#ifdef O0
#define debug_log(format, ...)  \
  __libc_format_log(ANDROID_LOG_DEBUG, "malloc_debug_mtk", (format), ##__VA_ARGS__ )
#else
#define debug_log(format, ...)  \
  async_safe_format_log(ANDROID_LOG_DEBUG, "malloc_debug_mtk", (format), ##__VA_ARGS__ )
#endif
#else
#define debug_log(format, ...)  do { } while(0)
#endif

#ifdef O0
#define error_log(format, ...)  \
  __libc_format_log(ANDROID_LOG_ERROR, "malloc_debug_mtk", (format), ##__VA_ARGS__ )
#define warn_log(format, ...)  \
  __libc_format_log(ANDROID_LOG_WARN, "malloc_debug_mtk", (format), ##__VA_ARGS__ )
#define info_log(format, ...)  \
  __libc_format_log(ANDROID_LOG_INFO, "malloc_debug_mtk", (format), ##__VA_ARGS__ )
#else
#define error_log(format, ...)  \
  async_safe_format_log(ANDROID_LOG_ERROR, "malloc_debug_mtk", (format), ##__VA_ARGS__ )
#define warn_log(format, ...)  \
  async_safe_format_log(ANDROID_LOG_WARN, "malloc_debug_mtk", (format), ##__VA_ARGS__ )
#define info_log(format, ...)  \
  async_safe_format_log(ANDROID_LOG_INFO, "malloc_debug_mtk", (format), ##__VA_ARGS__ )
#endif

/* flag definitions, currently sharing storage with "size" */

#define MAX_SIZE_T           (~(size_t)0)

#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT ((size_t)(2 * sizeof(void *)))
#endif
#define GUARD               0x48151642

#define DEBUG_MSPACE_SIZE_UNIT (1024*1024)
#define HISTORICAL_BUFFER_SIZE_UNIT (1024)
#define BT_DEPTH_UNIT (2)

//
// default config values
//
#ifdef MTK_USE_RESERVED_EXT_MEM
#define DEFAULT_DEBUG_MSPACE_SIZE (4*1024*1024)
#define DEFAULT_DEBUG_MSPACE_SOURCE EXTERNAL_MEM

void mtk_malloc_debug_read_config(char *filename);
int mtk_malloc_debug_set_config(const char *name);
#else
#define DEFAULT_DEBUG_MSPACE_SIZE (32*1024*1024)
#define DEFAULT_DEBUG_MSPACE_SOURCE INTERNAL_MEM
#endif
#define DEFAULT_HISTORICAL_ALLOC_SIZE (2046)
#define DEFAULT_MAX_BACKTRACE_DEPTH 5

/*
configurable items:
 1. debug mspace size (default: 32MB)
 2. historical table size (default: 2046)
 3. backtrace method (default: FP_BACKTRACE, 0)
 4. backtrace depth (default: 5)
 5. signal handler (default: off)
 6. debug mspace source (default: mmap anon)


Example A:
default debug15 config: 0x0 1 002 020

Example B: enable debug 16.
commands:
persist.debug15.config 0x2 4 002 020
setprop persist.libc.debug.malloc 16
reboot

 1. debug mspace size: 0x020 == 32 * MB
 2. historical table size: 0x002 = 2 * K
 3. Backtrace depth: 0x0 4 000 000 = 4 * 5
 4. 0x2, 0b0010:
     sig handler is disable, ENABLE_UNWIND_BT

Example C: enable debug 16, and use external memory as debug mspace source.
commands:
persist.debug15.config 0x6 4 002 020
setprop persist.libc.debug.malloc 16
reboot

 1. debug mspace size: 0x020 == 32 * MB
 2. historical table size: 0x002 = 2 * K
 3. Backtrace depth: 0x0 4 000 000 = 4 * 5
 4. 0x6, 0b0110:
     sig handler is disable
     ENABLE_UNWIND_BT
     create debug mspace from external memory

Example D: enable debug 15, and use external memory as debug mspace source.
     0x4 1 002 020: 0x41002020
     0x 4
     0b0100
         sig handler is disable
     unwind is disable
     create debug mspace from external memory
*/
#define DEBUG_MSPACE_SIZE_MASK (0xFFF)
#define HISTORICAL_BUFFER_SIZE_MASK (0xFFF000)
#define MAX_BT_DEPTH_MASK (0x0F000000)

// mostly for testing
#define SIG_HANDLER_MASK (1 << 28)
// bit: 29/30 00: FP; 01: GCC 10: corkscrew
#define UNWIND_BT_MASK (0x60000000)
// bit: 31 0: mmap anon; 1: external memory
#define DEBUG_MSPACE_SOURCE_MASK (1 << 31)

// external memory
#define EXM_DEV "/dev/exm0"
#define EXM_ADDR "/sys/class/exm/exm0/maps/map0/addr"
#define EXM_SIZE "/sys/class/exm/exm0/maps/map0/size"

//
// chunk flag:
// 1. allocated by mspace_malloc or malloc
// 2. inuse or free
//
#define INUSE_CHUNK (1 << 1)
#define MSPACE_MALLOC_CHUNK (1 << 3)
#define MALLOC_CHUNK (1 << 4)
#define MEMALIGN_CHUNK (1 << 5)

// =================================================================
// back trace size
// =================================================================
#define MAX_FP_BACKTRACE_SIZE 5
#define MAX_BACKTRACE_SIZE 32

// =================================================================
// Alignment
// =================================================================
#define SYSTEM_PAGE_SIZE        4096
#define ALIGN_UP_TO_PAGE_SIZE(p) \
    (((size_t)(p) + (SYSTEM_PAGE_SIZE - 1)) & ~(SYSTEM_PAGE_SIZE - 1))

typedef enum{
    FP_BACKTRACE,
    GCC_UNWIND_BACKTRACE,
    CORKSCREW_UNWIND_BACKTRACE
} BACKTRACE_METHOD;

typedef enum{
    INTERNAL_MEM,
    EXTERNAL_MEM
} DEBUG_MSPACE_SOURCE;

typedef struct DebugConfig {
    uint32_t mDebugMspaceSize;
    uint32_t mHistoricalBufferSize;
    uint32_t mMaxBtDepth;
    uint8_t mSig;
    uint8_t mBtMethod;
    uint8_t mDebugMspaceSource;
} DebugConfig, *PDebugConfig;

#define DEFAULT_DEBUG_CONFIG {                              \
    .mDebugMspaceSize = DEFAULT_DEBUG_MSPACE_SIZE,          \
    .mHistoricalBufferSize = DEFAULT_HISTORICAL_ALLOC_SIZE, \
    .mMaxBtDepth = DEFAULT_MAX_BACKTRACE_DEPTH,             \
    .mSig = 0, /* signal handler is disable */              \
    .mBtMethod = FP_BACKTRACE,                              \
    .mDebugMspaceSource = DEFAULT_DEBUG_MSPACE_SOURCE       \
}

extern DebugConfig gDebugConfig;
extern void* gDebugMspaceBase;
extern void* gDebugMspace;
int init_debug_mspace(void);
int init_recorder(void);

#ifdef DEBUG15_GUARD_CHECK

#define BIONIC_ALIGN(value, alignment) \
  (((value) + (alignment) - 1) & ~((alignment) - 1))

#define BIONIC_ROUND_UP_POWER_OF_2(value) \
  (sizeof(value) == 8) \
    ? (1UL << (64 - __builtin_clzl((unsigned long)(value)))) \
    : (1UL << (32 - __builtin_clz((unsigned int)(value))))

#define FRONT_GUARD_MALLOC         0xa1
#define FRONT_GUARD_MALLOC_FREE    0xb1
#define FRONT_GUARD_MEMALIGN         0xa2
#define FRONT_GUARD_MEMALIGN_FREE    0xb2

#define REAR_GUARD          0xbb
#define POISON_FREE_FILL    0xef
#define POISON_FREE_LEN     (sizeof(size_t) << 1)
#if defined(__LP64__)
#define FRONT_GUARD_LEN     (1<<3)
#define REAR_GUARD_LEN      (1<<3)
#define MEMALIGN_MAGIC_NO   0xcececececececece
#else
#define FRONT_GUARD_LEN     (1<<2)
#define REAR_GUARD_LEN      (1<<2)
#define MEMALIGN_MAGIC_NO   0xcececece
#endif

struct MTK_HDR_MALLOC {
    size_t size;
    char front_guard_malloc[FRONT_GUARD_LEN];
};
typedef struct MTK_HDR_MALLOC mtk_hdr_malloc;

struct MTK_HDR_MEMALIGN_REAR{
    void* base;  // Always points to the memory allocated using malloc.
    size_t size;
    char front_guard_memalign[FRONT_GUARD_LEN];
};
typedef struct MTK_HDR_MEMALIGN_REAR mtk_hdr_memalign_rear;

struct MTK_FTR_T{
    char rear_guard[REAR_GUARD_LEN];
} __attribute__((packed));
typedef struct MTK_FTR_T mtk_ftr_t;

int val_in_debug15_mspace(size_t value);
#endif

// if extmem failed, fallback to internal mem
#ifdef MTK_USE_RESERVED_EXT_MEM
#define FALLBACK_TO_INTERNAL_MEM
#endif

#ifdef __cplusplus
};  /* end of extern "C" */
#endif

#endif  // MALLOC_DEBUG_MTK_H
