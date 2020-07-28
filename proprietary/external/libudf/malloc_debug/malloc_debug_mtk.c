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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unwind.h>
#include <dlfcn.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/system_properties.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/param.h>
#include <sys/sysconf.h>

#include "back_trace.h"
#include "malloc_debug_mtk.h"

#define USE_DL_PREFIX

#if !defined(__LP64__)
#include "../unwind/libudf-unwind/backtrace.h"
#endif

#include "malloc_debug_common.h"
#include "malloc_debug_mtk.h" // Bt table, addr table
#include "sig_handler.h"

// =================================================================
// Global variables defined in malloc_debug_common.c
// =================================================================
/*
 * In a VM process, this is set to 1 after fork()ing out of zygote.
 */
// extern int gMallocLeakZygoteChild;
// extern void (*mspace_malloc_stat) (void *mem, size_t bytes) __attribute__((weak));
// extern void (*mspace_free_stat) (void *mem) __attribute__((weak));

// =================================================================
// Global variables used in this file
// =================================================================
DebugConfig gDebugConfig = DEFAULT_DEBUG_CONFIG;

// for mtk structure
void* gDebugMspaceBase = NULL;
void* gDebugMspace = NULL;

static uint32_t debug15_config = 0;

static int malloc_debug15_enabled = 0;
int debug15_mspace_full = 1;
void malloc_debug_initialize(void);

//malloc function ptr get from libc
static MallocDebugCalloc real_calloc = NULL;
static MallocDebugFree real_free = NULL;
#if 0 /*mask due to no need rehook*/
static MallocDebugMallinfo real_mallinfo = NULL;
#endif
static MallocDebugMalloc real_malloc = NULL;
static MallocDebugMallocUsableSize real_malloc_usable_size = NULL;
static MallocDebugMemalign real_memalign = NULL;
static MallocDebugPosixMemalign real_posix_memalign = NULL;
#if defined(HAVE_DEPRECATED_MALLOC_FUNCS)
static MallocDebugPvalloc real_pvalloc = NULL;
#endif
static MallocDebugRealloc real_realloc = NULL;
#if defined(HAVE_DEPRECATED_MALLOC_FUNCS)
static MallocDebugValloc real_valloc = NULL;
#endif
static MallocIterate real_malloc_iterate = NULL;
static MallocDisable real_malloc_disable;
static MallocEnable real_malloc_enable;

static void *malloc_libc_handle = NULL;
static int malloc_func_hooked = 0;

static char malloc_init_error_msg[128];
static int malloc_init_flag = 0;
//
// return  -1: fail, 0: sucess
static int malloc_func_rehook(void) {
    if (malloc_libc_handle == NULL) {
        malloc_libc_handle = dlopen("libc.so", RTLD_NOW|RTLD_GLOBAL);
        if (malloc_libc_handle == NULL) {
            error_log("Could not open libc.so for malloc debug: %s\n", dlerror());
            return -1;
        }
    }

    real_calloc = (MallocDebugCalloc)dlsym(malloc_libc_handle, "calloc");
    real_free = (MallocDebugFree)dlsym(malloc_libc_handle, "free");
#if 0
    real_mallinfo = (MallocDebugMallinfo)dlsym(malloc_libc_handle, "mallinfo");
#endif
    real_malloc =  (MallocDebugMalloc)dlsym(malloc_libc_handle, "malloc");
    real_malloc_usable_size = (MallocDebugMallocUsableSize)dlsym(malloc_libc_handle, "malloc_usable_size");
    real_memalign = (MallocDebugMemalign)dlsym(malloc_libc_handle, "memalign");
    real_posix_memalign = (MallocDebugPosixMemalign)dlsym(malloc_libc_handle, "posix_memalign");
    real_realloc = (MallocDebugRealloc)dlsym(malloc_libc_handle, "realloc");
#if defined(HAVE_DEPRECATED_MALLOC_FUNCS)
    real_pvalloc = (MallocDebugPvalloc)dlsym(malloc_libc_handle, "pvalloc");
    real_valloc = (MallocDebugValloc)dlsym(malloc_libc_handle, "valloc");
#endif
    real_malloc_iterate = (MallocIterate)dlsym(malloc_libc_handle, "malloc_iterate");
    real_malloc_disable = (MallocDisable)dlsym(malloc_libc_handle, "malloc_disable");
    real_malloc_enable = (MallocEnable)dlsym(malloc_libc_handle, "malloc_enable");

    if (!real_malloc || !real_free || !real_calloc ||
        !real_realloc || !real_memalign ||
#if defined(HAVE_DEPRECATED_MALLOC_FUNCS)
        !real_pvalloc || !real_valloc ||
#endif
        !real_malloc_usable_size || !real_posix_memalign) {
        error_log("%s failed\n", __FUNCTION__);
        return -1;
    }

    malloc_func_hooked = 1;
    return 0;
}

#ifdef DEBUG15_GUARD_CHECK
#define GUARD_CHECK_SUCCESS        0
#define FRONT_GUARD_INVALID       -1
#define FRONT_DOUBLE_FREE         -2
#define REAR_GUARD_OVERFLOW       -3

static volatile int malloc_debug15_version = 0xa1;  // 0xaa mean old guard, 0xa1 new guard

static inline void init_guard_malloc(mtk_hdr_malloc *hdr,  size_t size) {
    hdr->size = size;
    memset(hdr->front_guard_malloc, FRONT_GUARD_MALLOC, FRONT_GUARD_LEN);
    mtk_ftr_t* ftr = (mtk_ftr_t*)((size_t)(hdr + 1) + hdr->size);
    memset(ftr->rear_guard, REAR_GUARD, REAR_GUARD_LEN);
}

static inline void init_guard_memalign(mtk_hdr_memalign_rear *hdr, void* base, size_t size) {
    hdr->base = base;
    hdr->size = size;
    memset(hdr->front_guard_memalign, FRONT_GUARD_MEMALIGN, FRONT_GUARD_LEN);
    mtk_ftr_t* ftr = (mtk_ftr_t*)((size_t)(hdr + 1) + hdr->size);
    memset(ftr->rear_guard, REAR_GUARD, REAR_GUARD_LEN);
}

#define MALLOC_TYPE 1
#define MEMALIGN_TYPE 2
#define ILLEGAL_TYPE -1

static int slim_front_guard_type(const void *mem) {
    mtk_hdr_malloc *hdr_malloc = (mtk_hdr_malloc*)mem - 1;
    mtk_hdr_memalign_rear *hdr_memalign = (mtk_hdr_memalign_rear*)mem - 1;
    int i;
    for (i = 0; i < FRONT_GUARD_LEN; i++) {
        if (hdr_malloc->front_guard_malloc[i] == FRONT_GUARD_MALLOC) {
            continue;
        } else {
            break;
        }
    }
    if (i == FRONT_GUARD_LEN) return MALLOC_TYPE;
    for (i = 0; i < FRONT_GUARD_LEN; i++) {
        if (hdr_memalign->front_guard_memalign[i] == FRONT_GUARD_MEMALIGN) {
            continue;
        } else {
            break;
        }
    }
    if (i == FRONT_GUARD_LEN) return MEMALIGN_TYPE;

    return ILLEGAL_TYPE;
}


static int slim_front_free_guard_type(void *mem) {
    mtk_hdr_malloc *hdr_malloc = (mtk_hdr_malloc*)mem - 1;
    mtk_hdr_memalign_rear *hdr_memalign = (mtk_hdr_memalign_rear*)mem - 1;
    int i;
    for (i = 0; i < FRONT_GUARD_LEN; i++) {
        if (hdr_malloc->front_guard_malloc[i] == FRONT_GUARD_MALLOC_FREE) {
            continue;
        } else {
            break;
        }
    }
    if (i == FRONT_GUARD_LEN) return MALLOC_TYPE;
    for (i = 0; i < FRONT_GUARD_LEN; i++) {
        if (hdr_memalign->front_guard_memalign[i] == FRONT_GUARD_MEMALIGN_FREE) {
            continue;
        } else {
            break;
        }
    }
    if (i == FRONT_GUARD_LEN) return MEMALIGN_TYPE;

    return ILLEGAL_TYPE;
}

static inline void init_slim_guard_free_poison(void *mem, int type) {
    if (type == MALLOC_TYPE) {
        mtk_hdr_malloc *hdr = (mtk_hdr_malloc*)mem - 1;
        size_t hdr_size = hdr->size;
        void *buffer = (void *)(hdr + 1);

        if (val_in_debug15_mspace(hdr_size)) {
            ChunkHashEntry *entry = (ChunkHashEntry *)hdr_size;
            hdr_size = entry->bytes;
        }

        if (hdr_size > POISON_FREE_LEN)
            hdr_size = POISON_FREE_LEN;

        memset(hdr->front_guard_malloc, FRONT_GUARD_MALLOC_FREE, FRONT_GUARD_LEN);
        memset(buffer, POISON_FREE_FILL, hdr_size);
    } else if (type == MEMALIGN_TYPE) {
        mtk_hdr_memalign_rear *hdr = (mtk_hdr_memalign_rear*)mem - 1;
        size_t hdr_size = hdr->size;
        void *buffer = (void *)(hdr + 1);

        if (val_in_debug15_mspace(hdr_size)) {
            ChunkHashEntry *entry = (ChunkHashEntry *)hdr_size;
            hdr_size = entry->bytes;
        }

        if (hdr_size > POISON_FREE_LEN)
            hdr_size = POISON_FREE_LEN;

        memset(hdr->front_guard_memalign, FRONT_GUARD_MEMALIGN_FREE, FRONT_GUARD_LEN);
        memset(buffer, POISON_FREE_FILL, hdr_size);
    }
}

static int slim_guard_check(void *mem, void **Pbase) {
    mtk_ftr_t *ftr = NULL;
    char *r_guard = NULL;
    int front_check_fail = 0;
    // int front_free_poison_detect = 1;
    int rear_check_fail = 0;
    size_t i;
    int malloc_type = 0;
    int memalign_type = 0;
    mtk_hdr_malloc *hdr_malloc = (mtk_hdr_malloc*)mem - 1;
    mtk_hdr_memalign_rear *hdr_memalign = (mtk_hdr_memalign_rear*)mem - 1;
    char *ptr;

#define BUF_SIZE 512
    int len = 0;
    char buf[BUF_SIZE];
    int type = slim_front_guard_type(mem);

    if (type == MALLOC_TYPE) {
        *Pbase = (void *)hdr_malloc;
        size_t hdr_size = hdr_malloc->size;
        if (val_in_debug15_mspace(hdr_size)) {
            ChunkHashEntry *entry = (ChunkHashEntry *)hdr_size;
            hdr_size = entry->bytes;
        }
        ftr = (mtk_ftr_t*)((char *)(hdr_malloc + 1) + hdr_size);
    } else if (type == MEMALIGN_TYPE) {
        *Pbase = hdr_memalign->base;
        size_t hdr_size = hdr_memalign->size;
        if (val_in_debug15_mspace(hdr_size)) {
            ChunkHashEntry *entry = (ChunkHashEntry *)hdr_size;
            hdr_size = entry->bytes;
        }
        ftr = (mtk_ftr_t*)((char *)(hdr_memalign + 1) + hdr_size);
    } else {
        memset(buf, 0x0, BUF_SIZE);
        type = slim_front_free_guard_type(mem);
        if (type == MALLOC_TYPE) {
            ptr = (char *)hdr_malloc;
            len = snprintf(buf, BUF_SIZE, "[ERROR]memory double free, head ptr:%p", hdr_malloc);
            for (i = 0; i < sizeof(mtk_hdr_malloc); i++) {
                if (i%sizeof(size_t) == 0) {
                    len += snprintf(buf+len, BUF_SIZE-len, "\n%p", ptr+i);
                }
                len += snprintf(buf+len, BUF_SIZE-len, " %02x", ptr[i]);
            }
            error_log("%s\n", buf);
        } else if (type == MEMALIGN_TYPE) {
            ptr = (char *)hdr_memalign;
            len = snprintf(buf, BUF_SIZE, "[ERROR]memory double free, head ptr:%p", hdr_memalign);
            for (i = 0; i < sizeof(mtk_hdr_memalign_rear); i++) {
                if (i%sizeof(size_t) == 0) {
                    len += snprintf(buf+len, BUF_SIZE-len, "\n%p", ptr+i);
                }
                len += snprintf(buf+len, BUF_SIZE-len, " %02x", ptr[i]);
            }
            error_log("%s\n", buf);
        } else {
            ptr = (char *)hdr_memalign;
            len = snprintf(buf, BUF_SIZE, "[ERROR] %p front guard overwritten, head ptr:%p", mem, hdr_memalign);
            for (i = 0; i < sizeof(mtk_hdr_memalign_rear); i++) {
                if (i%sizeof(size_t) == 0) {
                    len += snprintf(buf+len, BUF_SIZE-len, "\n%p", ptr+i);
                }
                len += snprintf(buf+len, BUF_SIZE-len, " %02x", ptr[i]);
            }
            error_log("%s\n", buf);
        }

        return ILLEGAL_TYPE;
    }

    r_guard = ftr->rear_guard;
    for (i = 0; i < REAR_GUARD_LEN; i++) {
        if (r_guard[i] != REAR_GUARD) {
            rear_check_fail = 1;
            break;
        }
    }
    if (rear_check_fail) {
        memset(buf, 0x0, BUF_SIZE);
        if (type == MALLOC_TYPE) {
            ptr = (char *)hdr_malloc;
            len = snprintf(buf, BUF_SIZE, "[ERROR]malloc rear guard overwritten, hdr_malloc:%p", hdr_malloc);
            for (i = 0; i < sizeof(mtk_hdr_malloc); i++) {
                if (i%sizeof(size_t) == 0) {
                    len += snprintf(buf+len, BUF_SIZE-len, "\n%p", ptr+i);
                }
                len += snprintf(buf+len, BUF_SIZE-len, " %02x", ptr[i]);
            }
        }
        if (type == MEMALIGN_TYPE) {
            ptr = (char *)hdr_memalign;
            len = snprintf(buf, BUF_SIZE, "[ERROR]memalign rear guard overwritten, hdr_memalign:%p", hdr_memalign);
            for (i = 0; i < sizeof(mtk_hdr_memalign_rear); i++) {
                if (i%sizeof(size_t) == 0) {
                    len += snprintf(buf+len, BUF_SIZE-len, "\n%p", ptr+i);
                }
                len += snprintf(buf+len, BUF_SIZE-len, " %02x", ptr[i]);
            }
        }

        len += snprintf(buf+len, BUF_SIZE-len, "\nrear area should be[0x%02x] but it's:", REAR_GUARD);
        for (i = 0; i < sizeof(mtk_ftr_t); i++) {
            if (i%sizeof(size_t) == 0) {
                len += snprintf(buf+len, BUF_SIZE-len, "\n%p", r_guard+i);
            }
            len += snprintf(buf+len, BUF_SIZE-len, " %02x", r_guard[i]);
        }
        error_log("%s\n", buf);
        return ILLEGAL_TYPE;
    }

    return type;
}

// return  1  value in debug15 mspace
//         0  value not in debug15 mspace
int val_in_debug15_mspace(size_t value) {
    if ((value > (size_t)gDebugMspaceBase) &&
       (value < ((size_t)gDebugMspaceBase + (size_t)gDebugConfig.mDebugMspaceSize)))
        return 1;
    else
        return 0;
}
#endif

//
// must be wrapped with mutex
//
static void clean_exit() {
#if UNMAP_DEBUG_MSPACE
    if(gDebugMspaceBase != NULL) {
        munmap(gDebugMspaceBase, gDebugConfig.mDebugMspaceSize);
        gDebugMspaceBase = NULL;
    }
#endif
    debug15_mspace_full = 1;
    error_log("recover to default allocation APIs\n");
}

static inline int get_backtrace_common(void *fp, intptr_t* addrs) {
    if(gDebugConfig.mBtMethod == FP_BACKTRACE){
        return get_backtrace_fp(fp, addrs, gDebugConfig.mMaxBtDepth);
    }
#if !defined(__LP64__)
#if 0  // mask gcc unwind
    else if(gDebugConfig.mBtMethod == GCC_UNWIND_BACKTRACE){
        return get_backtrace(addrs, gDebugConfig.mMaxBtDepth);
    }
#endif
    else if(gDebugConfig.mBtMethod == GCC_UNWIND_BACKTRACE ||
            gDebugConfig.mBtMethod == CORKSCREW_UNWIND_BACKTRACE) {
        return libudf_unwind_backtrace((uintptr_t *)addrs, 3, gDebugConfig.mMaxBtDepth);
    }
#endif
    else {
        return get_backtrace(addrs, gDebugConfig.mMaxBtDepth);
    }
}

static inline void malloc_record_info(void *buffer, size_t bytes) {
    int recovery = 0;
    if (!buffer || debug15_mspace_full) return;

    if (gDebugMspace == NULL) {
        debug_log("debug mspace is not initialized");
        return;
    }

    intptr_t backtrace[MAX_BACKTRACE_SIZE];
    size_t numEntries = get_backtrace_common(__builtin_frame_address(0), backtrace);
    if (numEntries > MAX_BACKTRACE_SIZE) {
        recovery = 1;
        warn_log("backtrace count exceeds max count.");
        goto EXIT;
    }

    // if (numEntries == 0)
    // debug_log("cannot get back trace\n");

    // record back trace in debug mspace
    //
    BtEntry* bt_entry = record_backtrace(backtrace, numEntries, bytes);
    if (!bt_entry) {
        recovery = 1;
        goto EXIT;
    }

    //
    // record addr and setup the map between addr and backtrace
    //
    ChunkHashEntry *entry = record_chunk_info(bt_entry, buffer, bytes, MALLOC_CHUNK);
    if (!entry) {
        recovery = 1;
        goto EXIT;
    }

// record ChunkHashEntry pointer to hdr->size
#ifdef DEBUG15_GUARD_CHECK
    mtk_hdr_malloc *hdr = (mtk_hdr_malloc*)buffer - 1;
    hdr->size = (size_t)entry;
#endif
EXIT:
    if (recovery)
       clean_exit();
}

static inline void free_remove_info(ChunkHashEntry* entry) {
    int recovery = 0;

    if (!entry || debug15_mspace_full)
        return;

    if (gDebugMspace == NULL) {
        debug_log("debug mspace is not initialized");
        return;
    }

    // move chunk entry to historical alloc table
    // did not delete the back trace entry, until deleting historical chunk.
    intptr_t backtrace[MAX_BACKTRACE_SIZE];
    size_t numEntries = get_backtrace_common(__builtin_frame_address(0), backtrace);
    if (numEntries > MAX_BACKTRACE_SIZE) {
        recovery = 1;
        warn_log("backtrace count exceeds max count.");
        goto EXIT;
    }

    // move chunk from current table to historical alloc table
    if (move_to_historical(entry, backtrace, numEntries) < 0)
        recovery = 1;

EXIT:
    if (recovery)
        clean_exit();
}

DEBUG15_EXPORT
void *malloc(size_t bytes)
{
    void* buffer = NULL;

    if (!malloc_func_hooked) malloc_func_rehook();
    if (malloc_debug15_enabled) {
#ifdef DEBUG15_GUARD_CHECK
        size_t size = sizeof(mtk_hdr_malloc) + bytes + sizeof(mtk_ftr_t);
        mtk_hdr_malloc* hdr = (mtk_hdr_malloc*)real_malloc(size);
        if (hdr) {
            init_guard_malloc(hdr, bytes);
            buffer = (void *)(hdr + 1);
        }
#else
        buffer = real_malloc(bytes);
#endif
        malloc_record_info(buffer, bytes);
        debug_log("%s:%p\n", __FUNCTION__, buffer);
    } else {
        buffer = real_malloc(bytes);
    }

    return buffer;
}

DEBUG15_EXPORT
void free(void *mem)
{
    if (!mem) return;
    void *base = mem;

    if (!malloc_func_hooked) malloc_func_rehook();
    if (malloc_debug15_enabled) {
        size_t slot = 0;
        ChunkHashEntry *entry = NULL;
#ifdef DEBUG15_GUARD_CHECK
        void* Pbase = NULL;
        int type = ILLEGAL_TYPE;

        chunk_mutex_lock();
        type = slim_guard_check(mem, &Pbase);
        if (type == ILLEGAL_TYPE) {
            error_log("guard check fail when free: %p\n", mem);
            abort();
        } else {
            init_slim_guard_free_poison(mem, type);
        }
        chunk_mutex_unlock();

        base = Pbase;
        entry = find_and_delete_current_entry(mem);

        debug_log("%s base:%p, size_used:%zu\n",
            __FUNCTION__, base, ((mtk_hdr_malloc *)mem-1)->size);
#else
        entry = find_and_delete_current_entry(mem);
#endif
        free_remove_info(entry);
        debug_log("%s:%p\n", __FUNCTION__, mem);
    }

EXIT:
    /*  [288308]
       The pthread_exit function will free thread strcutre which we get stack
       base address and size. In this case, we have to record backtrace before
       freeing it. */
    real_free(base);
}

DEBUG15_EXPORT
size_t malloc_usable_size(const void *mem) {
    if (!mem) return 0;

    if (!malloc_func_hooked) malloc_func_rehook();
    if (malloc_debug15_enabled) {
#ifdef DEBUG15_GUARD_CHECK
        int type = slim_front_guard_type(mem);
        if (type == MALLOC_TYPE) {
            mtk_hdr_malloc *hdr = (mtk_hdr_malloc*)mem - 1;
            size_t hdr_size = hdr->size;
            if (val_in_debug15_mspace(hdr_size)) {
                ChunkHashEntry *entry = (ChunkHashEntry *)hdr_size;
                hdr_size = entry->bytes;
            }
            return (hdr_size);
        } else if (type == MEMALIGN_TYPE) {
            mtk_hdr_memalign_rear *hdr = (mtk_hdr_memalign_rear*)mem - 1;
            size_t hdr_size = hdr->size;
            if (val_in_debug15_mspace(hdr_size)) {
                ChunkHashEntry *entry = (ChunkHashEntry *)hdr_size;
                hdr_size = entry->bytes;
            }
            return (hdr_size);
        } else {
            error_log("%s: %p front guard check fail", __FUNCTION__, mem);
            abort();
        }
#else
        debug_log("%s\n", __FUNCTION__);
        return real_malloc_usable_size(mem);
#endif
    } else {
        return real_malloc_usable_size(mem);
    }
}

#if 0
struct mallinfo mallinfo() {
    debug_log("%s\n", __FUNCTION__);
    return real_mallinfo();
}
#endif

DEBUG15_EXPORT
void *calloc(size_t n_elements, size_t elem_size) {
    if (!malloc_func_hooked) malloc_func_rehook();
    void* buffer = NULL;

    if (malloc_debug15_enabled) {
        size_t bytes = n_elements * elem_size;
#ifdef DEBUG15_GUARD_CHECK
        size_t size = sizeof(mtk_hdr_malloc) + bytes + sizeof(mtk_ftr_t);
        if (size < bytes) {  // Overflow
            errno = ENOMEM;
            return NULL;
        }

        mtk_hdr_malloc* hdr = (mtk_hdr_malloc*)real_calloc(1, size);
        if (hdr) {
            init_guard_malloc(hdr, bytes);
            buffer = (void *)(hdr + 1);
        }
#else
        buffer = real_calloc(n_elements, elem_size);
#endif
        malloc_record_info(buffer, bytes);
        debug_log("%s:%p\n", __FUNCTION__, buffer);
    } else {
        buffer = real_calloc(n_elements, elem_size);
    }

    return buffer;
}

DEBUG15_EXPORT
void *realloc(void* oldMem, size_t bytes)
{
    if (!malloc_func_hooked) malloc_func_rehook();
    void *newMem = NULL;

    if (malloc_debug15_enabled) {
#ifdef DEBUG15_GUARD_CHECK
        if (!oldMem && bytes == 0)
            return NULL;

        mtk_hdr_malloc* new_hdr;
        size_t size = sizeof(mtk_hdr_malloc) + bytes + sizeof(mtk_ftr_t);
        if (size < bytes) {  // Overflow
            errno = ENOMEM;
            return NULL;
        }

        if (!oldMem) {
            new_hdr = (mtk_hdr_malloc*)real_malloc(size);
            if (!new_hdr) {
                return NULL;
            }
        }
        else if (bytes == 0) {
            free(oldMem);
            return NULL;
        }
        else {
            mtk_hdr_malloc *hdr = (mtk_hdr_malloc*)oldMem - 1;
            size_t copy_size = hdr->size;
            if (val_in_debug15_mspace(copy_size)) {
                ChunkHashEntry *entry = (ChunkHashEntry *)copy_size;
                copy_size = entry->bytes;
            }

            debug_log("%s: oldMem %p front_guard [%zx] copy_size:%zu\n",
                __FUNCTION__, oldMem, *(size_t *)hdr->front_guard_malloc, copy_size);

            new_hdr = (mtk_hdr_malloc*)real_malloc(size);
            if (!new_hdr) {
                return NULL;
            }
            debug_log("%s: new_hdr %p\n", __FUNCTION__, new_hdr);

            copy_size = copy_size < bytes ? copy_size : bytes;
            memcpy((void *)(new_hdr+1), oldMem, copy_size);

            free(oldMem);
        }

        init_guard_malloc(new_hdr, bytes);
        newMem = (void *)(new_hdr + 1);
#else
        size_t slot = 0;
        ChunkHashEntry *entry = NULL;
        newMem = real_realloc(oldMem, bytes);
        entry = find_and_delete_current_entry(oldMem);
        free_remove_info(entry);
#endif
        malloc_record_info(newMem, bytes);
        debug_log("%s:%p\n", __FUNCTION__, newMem);
    } else {
        newMem = real_realloc(oldMem, bytes);
    }

    return newMem;
}

#ifdef DEBUG15_GUARD_CHECK
static inline void *mtk_guard_memalign(size_t alignment, size_t bytes) {
    void* buffer = NULL;

    // Make the alignment a power of two.
    if (!powerof2(alignment)) {
        alignment = BIONIC_ROUND_UP_POWER_OF_2(alignment);
    }
    size_t size = alignment + bytes + sizeof(mtk_hdr_memalign_rear) + sizeof(mtk_ftr_t) + 2 * sizeof(void *);
    if (size < bytes) { // Overflow.
        return NULL;
    }

    void* base = real_malloc(size);
    if (base != NULL) {
        void *hdr_base = (void*)((size_t*)base + 2);  // 2 pointer for memaglign magic and hdr
        intptr_t ptr = (intptr_t)((mtk_hdr_memalign_rear*)hdr_base + 1);
        debug_log("%s:base %p\n", __FUNCTION__, base);
        if (ptr%alignment) {
            debug_log("%s:ptr %p before align\n", __FUNCTION__, (void *)ptr);
            ptr += ((-ptr) % alignment);  // Align the pointer.
            debug_log("%s:ptr %p after align\n", __FUNCTION__, (void *)ptr);
        }
        mtk_hdr_memalign_rear* hdr = (mtk_hdr_memalign_rear*)ptr - 1;

        *(size_t *)base = MEMALIGN_MAGIC_NO;  // for memalign chunk hdr find
        *((size_t *)base+1) = (size_t)hdr;

        init_guard_memalign(hdr, base, bytes);
        buffer = (void *)(hdr + 1);
    }
    return buffer;
}
#endif

DEBUG15_EXPORT
void *memalign(size_t alignment, size_t bytes) {
    if (!malloc_func_hooked) malloc_func_rehook();
    void* buffer = NULL;

    if (malloc_debug15_enabled) {
    #ifdef DEBUG15_GUARD_CHECK
        if (alignment <= MALLOC_ALIGNMENT)
            return malloc(bytes);
        buffer = mtk_guard_memalign(alignment, bytes);
    #else
        buffer = real_memalign(alignment, bytes);
    #endif
        debug_log("%s:%p\n", __FUNCTION__, buffer);
        malloc_record_info(buffer, bytes);
    } else {
        buffer = real_memalign(alignment, bytes);
    }

    return buffer;
}

DEBUG15_EXPORT
int posix_memalign(void** pp, size_t alignment, size_t bytes) {
    if (!malloc_func_hooked) malloc_func_rehook();
    int ret = -1;

    if (malloc_debug15_enabled) {
        void* buffer = NULL;
    #ifdef DEBUG15_GUARD_CHECK
        if (alignment <= MALLOC_ALIGNMENT) {
            buffer = malloc(bytes);
            ret = (buffer != NULL) ? 0 : ENOMEM;
            if (buffer)
                *pp = buffer;
            return ret;
        }
        *pp = mtk_guard_memalign(alignment, bytes);
        ret = (*pp != NULL) ? 0 : ENOMEM;
 #else
        ret = real_posix_memalign(pp, alignment, bytes);
 #endif
        if (!ret) {
            buffer = *pp;
        }
        debug_log("%s:%p\n", __FUNCTION__, buffer);
        malloc_record_info(buffer, bytes);
    } else {
        ret = real_posix_memalign(pp, alignment, bytes);
    }

    return ret;
}

#if defined(HAVE_DEPRECATED_MALLOC_FUNCS)
DEBUG15_EXPORT
void *pvalloc(size_t bytes){
    if (!malloc_func_hooked) malloc_func_rehook();
    void *buffer = NULL;

    if (malloc_debug15_enabled) {
    #ifdef DEBUG15_GUARD_CHECK
        size_t pagesize = sysconf(_SC_PAGE_SIZE);
        size_t size = BIONIC_ALIGN(bytes, pagesize);
        if (size < bytes) {  // Overflow
            return NULL;
        }
        buffer = mtk_guard_memalign(pagesize, size);
        malloc_record_info(buffer, size);
 #else
        buffer = real_pvalloc(bytes);
        malloc_record_info(buffer, bytes);
 #endif
        debug_log("%s:%p\n", __FUNCTION__, buffer);
    } else {
        buffer = real_pvalloc(bytes);
    }

    return buffer;
}

DEBUG15_EXPORT
void *valloc(size_t bytes) {
    if (!malloc_func_hooked) malloc_func_rehook();
    void *buffer = NULL;
    if (malloc_debug15_enabled) {
    #ifdef DEBUG15_GUARD_CHECK
        size_t pagesize = sysconf(_SC_PAGE_SIZE);
        buffer = mtk_guard_memalign(pagesize, bytes);
 #else
        buffer = real_valloc(bytes);
 #endif
        debug_log("%s:%p\n", __FUNCTION__, buffer);
        malloc_record_info(buffer, bytes);
    } else {
        buffer = real_valloc(bytes);
    }

    return buffer;
}
#endif

// typedef void (*MallocCallBack)(uintptr_t, size_t, void*);
DEBUG15_EXPORT
int malloc_iterate(uintptr_t base, size_t size,
    void (*callback)(uintptr_t base, size_t size, void* arg), void* arg) {
    if (real_malloc_iterate)
        return real_malloc_iterate(base, size, callback, arg);
    else
        return -1;
}

DEBUG15_EXPORT
void malloc_disable() {
    if (real_malloc_disable)
        real_malloc_disable();
    multi_mutex_lock();
}

DEBUG15_EXPORT
void malloc_enable() {
    multi_mutex_unlock();
    if (real_malloc_enable)
        real_malloc_enable();
}

//
// initialize debug config
// 0: success;
// others: fail
//
static void init_debug_config() {
    if (debug15_config) {
        gDebugConfig.mDebugMspaceSize =
            (debug15_config & DEBUG_MSPACE_SIZE_MASK) * DEBUG_MSPACE_SIZE_UNIT;
        gDebugConfig.mHistoricalBufferSize =
            ((debug15_config & HISTORICAL_BUFFER_SIZE_MASK) >> 12) * HISTORICAL_BUFFER_SIZE_UNIT;
        gDebugConfig.mMaxBtDepth =
            ((debug15_config & MAX_BT_DEPTH_MASK) >> 24) * BT_DEPTH_UNIT;

        gDebugConfig.mBtMethod =
            (debug15_config & UNWIND_BT_MASK) >> 29;

        gDebugConfig.mSig = (debug15_config & SIG_HANDLER_MASK) ? 1 : 0;

        // 0: mmap anon; 1: external memory
        gDebugConfig.mDebugMspaceSource =
            (debug15_config & DEBUG_MSPACE_SOURCE_MASK) ? EXTERNAL_MEM : INTERNAL_MEM;
    }

    //
    // simple sanity check
    //
    if (gDebugConfig.mDebugMspaceSize == 0)
        gDebugConfig.mDebugMspaceSize = DEFAULT_DEBUG_MSPACE_SIZE;
    gDebugConfig.mDebugMspaceSize = ALIGN_UP_TO_PAGE_SIZE(gDebugConfig.mDebugMspaceSize);

    if (gDebugConfig.mHistoricalBufferSize == 0)
        gDebugConfig.mHistoricalBufferSize = DEFAULT_HISTORICAL_ALLOC_SIZE;

    if (gDebugConfig.mMaxBtDepth == 0)
        gDebugConfig.mMaxBtDepth = DEFAULT_MAX_BACKTRACE_DEPTH;
    // when using FP,
    // back trace depth cannot be larger than DEFAULT_MAX_BACKTRACE_DEPTH(5),
    if (gDebugConfig.mMaxBtDepth > DEFAULT_MAX_BACKTRACE_DEPTH &&
        gDebugConfig.mBtMethod == FP_BACKTRACE)
        gDebugConfig.mMaxBtDepth = DEFAULT_MAX_BACKTRACE_DEPTH;
    if (gDebugConfig.mMaxBtDepth > MAX_BACKTRACE_SIZE)
        gDebugConfig.mMaxBtDepth = MAX_BACKTRACE_SIZE;

#if 0
    info_log("debug15 config: 0x%x\n", debug15_config);
    info_log("debug mspace size: 0x%x\n", gDebugConfig.mDebugMspaceSize);
    info_log("historical buffer size: %d\n", gDebugConfig.mHistoricalBufferSize);
    info_log("max bt depth: %d\n", gDebugConfig.mMaxBtDepth);
    info_log("bt method: %d\n", gDebugConfig.mBtMethod);
    info_log("sig handler: %d\n", gDebugConfig.mSig);
    info_log("debug mspace source: %s\n", gDebugConfig.mDebugMspaceSource ? "external" : "internal");
#endif
}

#ifdef MTK_USE_RESERVED_EXT_MEM
static void *gMspaceBuffer = NULL;
#endif
static void mtk_malloc_debug_prepare(void) {
    multi_mutex_lock();
    debug_log("parent: prepare is called");
    const char *progname = getprogname();

#ifdef MTK_USE_RESERVED_EXT_MEM
    if (gDebugMspaceBase == NULL)
        return;

    if (gDebugConfig.mDebugMspaceSource == INTERNAL_MEM)
        return;

    // create anon memory
    gMspaceBuffer = mmap(NULL, gDebugConfig.mDebugMspaceSize,
            PROT_READ |PROT_WRITE |PROT_MALLOCFROMBIONIC, MAP_PRIVATE | MAP_ANONYMOUS, /*fd*/-1, 0);
    debug_log("parent: before fork gMspaceBuffer = %p\n", gMspaceBuffer);
    if (gMspaceBuffer == MAP_FAILED) {
        if (strstr(progname, "/system/bin/logd")) {
            snprintf(malloc_init_error_msg, sizeof(malloc_init_error_msg), "parent: Internal mem MAP_FAILED, errno = %d\n", errno);
        } else {
            error_log("parent: Internal mem MAP_FAILED, errno = %d\n", errno);
        }
        return; // #define MAP_FAILED ((void *)-1) in mman.h
    }

    // copy external memory to anon memory
    debug_log("parent: start copy ext to anon");
    memcpy(gMspaceBuffer, gDebugMspaceBase, gDebugConfig.mDebugMspaceSize);
    debug_log("parent: copy done");
#endif
}

static void mtk_malloc_debug_parent(void) {
    debug_log("parent: parent is called");

#ifdef MTK_USE_RESERVED_EXT_MEM
    if (gDebugMspaceBase == NULL)
        goto UNLOCK;

    if (gDebugConfig.mDebugMspaceSource == INTERNAL_MEM)
        goto UNLOCK;

    // free anon memory
    debug_log("parent: free parent's anon memory");
    munmap(gMspaceBuffer, gDebugConfig.mDebugMspaceSize);
#endif
UNLOCK:
    multi_mutex_unlock();
}

static void mtk_malloc_debug_child(void) {
    debug_log("child: child is called");
    const char *progname = getprogname();

#ifdef MTK_USE_RESERVED_EXT_MEM
    if (gDebugMspaceBase == NULL)
        goto UNLOCK;

    if (gDebugConfig.mDebugMspaceSource == INTERNAL_MEM)
        goto UNLOCK;

    void *old_mmap_addr = gDebugMspaceBase;

    // create external memory with map_fixed
    int fd = open(EXM_DEV, O_RDWR);
    if (fd < 0) {
        if (strstr(progname, "/system/bin/logd")) {
            snprintf(malloc_init_error_msg, sizeof(malloc_init_error_msg), "%s cannot open %s", __FUNCTION__, EXM_DEV);
        } else {
            error_log("%s cannot open %s", __FUNCTION__, EXM_DEV);
        }
        gDebugMspaceBase = NULL;
    #ifdef FALLBACK_TO_INTERNAL_MEM
        gDebugConfig.mDebugMspaceSource = INTERNAL_MEM;
    #else
        goto UNLOCK;
    #endif
    } else {
        gDebugMspaceBase = mmap(old_mmap_addr, gDebugConfig.mDebugMspaceSize,
                PROT_READ | PROT_WRITE |PROT_MALLOCFROMBIONIC, MAP_SHARED | MAP_FIXED, fd, 0);
        debug_log("child: mmap EXTERNAL mem %p\n", gDebugMspaceBase);
        close(fd);
        fd = -1;
        if (gDebugMspaceBase == MAP_FAILED) {
            error_log("%s External mem MAP_FAILED, errno = %d", __FUNCTION__, errno);
            gDebugMspaceBase = NULL;
        #ifdef FALLBACK_TO_INTERNAL_MEM
            gDebugConfig.mDebugMspaceSource = INTERNAL_MEM;
        #else
            goto UNLOCK;
        #endif
        }
    }

    // use internal memory
    if (gDebugMspaceBase == NULL || gDebugMspaceBase == MAP_FAILED) {
        gDebugMspaceBase = mmap(old_mmap_addr, gDebugConfig.mDebugMspaceSize,
                 PROT_READ |PROT_WRITE|PROT_MALLOCFROMBIONIC, MAP_PRIVATE | MAP_ANONYMOUS, /*fd*/-1, 0);
        debug_log("child: mmap INTERNAL mem %p\n", gDebugMspaceBase);
        if (gDebugMspaceBase == MAP_FAILED) {
            gDebugMspaceBase = NULL;
            error_log("%s Internal mem MAP_FAILED, errno = %d\n", __FUNCTION__, errno);
            goto UNLOCK;
        }
    }

    // copy anon memory to external memory.
    memcpy(gDebugMspaceBase, gMspaceBuffer, gDebugConfig.mDebugMspaceSize);
    munmap(gMspaceBuffer, gDebugConfig.mDebugMspaceSize);
    debug_log("child: copy done and free anon memory");
#endif
UNLOCK:
    // reinit mutex for child
    multi_mutex_unlock();
}

DEBUG15_EXPORT
void malloc_debug_reg_atfork(void) {
    if (malloc_debug15_enabled) {
        if(pthread_atfork(mtk_malloc_debug_prepare,
                          mtk_malloc_debug_parent,
                          mtk_malloc_debug_child)){
            const char *progname = getprogname();
            if (strstr(progname, "/system/bin/logd")) {
                snprintf(malloc_init_error_msg, sizeof(malloc_init_error_msg), "pthread_atfork fail\n");
            } else {
                error_log("pthread_atfork fail\n");
            }
        }
    }
}

//
// 1. initialize debug mspace
// 2. initialize recorder
//
DEBUG15_EXPORT
void malloc_debug_initialize(void) {
    int malloc_debug_level;
    char env[PROP_VALUE_MAX];
    char debug_program[PROP_VALUE_MAX];
    char debug_config[PROP_VALUE_MAX];
    const char *progname = NULL;
#ifdef MTK_USE_RESERVED_EXT_MEM
    char config_file[PROP_VALUE_MAX];
#endif

    progname = getprogname();
    if (!progname) {
        return;
    }
    if (strstr(progname, "/system/bin/logd")) {
        malloc_init_flag = 1;
    }

    if (!malloc_func_hooked) {
        if (malloc_func_rehook()) {
            if (malloc_init_flag) {
                snprintf(malloc_init_error_msg, sizeof(malloc_init_error_msg), "malloc function rehook fail\n");
            } else {
                error_log("malloc function rehook fail\n");
            }
            return;
        }
    }

#ifdef _MTK_ENG_
    malloc_debug_level = 15;
#else
    malloc_debug_level = 0;
#endif

    if (__system_property_get("persist.vendor.libc.debug.malloc", env)) {
        malloc_debug_level = atoi(env); // overwrite initial value(0, or 15)
    }

    if (malloc_debug_level != 15) {
        return;
    }


    //white list: disable debug 15 process
    if (!strcmp(progname, "sh") ||
        !strcmp(progname, "/system/bin/sh") ||
        strstr(progname, "/system/bin/logcat") ||
        strstr(progname, "/system/bin/installd") ||
#if defined(DEBUG15_GMO_RAM_OPTIMIZE) && !defined(MTK_USE_RESERVED_EXT_MEM) && !defined(__LP64__)
        strstr(progname, "/system/bin/app_process") ||
#endif
        // strstr(progname, "/system/bin/vold") ||
        strstr(progname, "/system/bin/dex2oat") ||
        // strstr(progname, "/system/bin/logd") ||
        strstr(progname, "/system/bin/debuggerd")) {
        malloc_debug_level = 0;
    }

    if (__system_property_get("persist.vendor.libc.debug15.prog", debug_program)) {
        if (strstr(progname, debug_program))
            malloc_debug_level = 15;
        else
            malloc_debug_level = 0;
    }

    if (malloc_debug_level != 15) {
        return;
    }

#ifndef MTK_USE_RESERVED_EXT_MEM
    if (strstr(progname, "/system/bin/app_process") ||
        strstr(progname, "/vendor/bin/hw/camerahalserver")) {
        gDebugConfig.mBtMethod = FP_BACKTRACE;
    } else {
        gDebugConfig.mBtMethod = CORKSCREW_UNWIND_BACKTRACE;
    }
#endif

    if (__system_property_get("persist.vendor.debug15.config",  debug_config)) {
        debug15_config = strtoul(debug_config, (char **)NULL, 16);
    }

#ifdef MTK_USE_RESERVED_EXT_MEM
    if (!__system_property_get("persist.vendor.debug15.config.file", config_file)) {
        strcpy(config_file, "/system/vendor/etc/debug15.conf");
    }

    if (config_file[0]) {
        mtk_malloc_debug_read_config(config_file);
    }

    if (mtk_malloc_debug_set_config(progname)) {
        if (malloc_init_flag) {
            snprintf(malloc_init_error_msg, sizeof(malloc_init_error_msg), "mtk_malloc_debug_set_config fails\n");
        } else {
            error_log("mtk_malloc_debug_set_config fails\n");
        }
        return;
    }
#endif

    init_debug_config();

    if (init_debug_mspace()) {
        if (malloc_init_flag) {
            snprintf(malloc_init_error_msg, sizeof(malloc_init_error_msg), "init_debug_mspace fails\n");
        } else {
            error_log("init_debug_mspace fails\n");
        }
        return;
    }

    if (gDebugConfig.mSig) {
        install_signal();
    }

    // create and initialize three hash tables
    if (!init_recorder()) {
        malloc_debug15_enabled = 1;
        debug_log("%s: using malloc %d, debug15_config %x\n",
                      progname, malloc_debug_level, debug15_config);
    } else {
        if (malloc_init_flag) {
            snprintf(malloc_init_error_msg, sizeof(malloc_init_error_msg), "malloc_debug_initialize fails\n");
        } else {
            error_log("malloc_debug_initialize fails\n");
        }
    }
#ifdef DEBUG15_GUARD_CHECK
    if (!malloc_debug15_version) {
        if (malloc_init_flag) {
            snprintf(malloc_init_error_msg, sizeof(malloc_init_error_msg), "debug15 no guard\n");
        } else {
            error_log("debug15 no guard\n");
        }
    }
#endif
}

#if 0
void malloc_debug_finalize() {
    info_log("malloc_debug_finalize is called\n");
    release_debug_mspace();
}
#endif
