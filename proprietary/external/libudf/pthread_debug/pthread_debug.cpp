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
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#include <pthread_internal.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PTHREADDEBUG_EXPORT __attribute__ ((visibility("default")))

#include <ubrd_config.h>
#include "../include/recorder.h"

#define _LEAK_DETECT_
//#define _HOOK_PTHREAD_EXIT_

PUBRD g_PthreadDebug_PUBRD = NULL;
static int pthread_debug_inited;

typedef int (*PthreadDebug_pthread_create) (pthread_t *, pthread_attr_t const *,
					    void *(*)(void *), void *);
typedef int (*PthreadDebug_pthread_join) (pthread_t, void **);
typedef int (*PthreadDebug_pthread_detach) (pthread_t);

static void *pthread_create_libc_handle;

static PthreadDebug_pthread_create real_pthread_create;
static PthreadDebug_pthread_join real_pthread_join;
static PthreadDebug_pthread_detach real_pthread_detach;

static unsigned long g_tid_offset = (unsigned long) -1;

#ifdef _LEAK_DETECT_
// default off
static int pthread_debug_leak_detect = 1;

// maximum of allowed same backtrace allocations
#define DEFAULT_MAX_ALLOCATIONS 512
#define DEFAULT_MIN_ALLOCATIONS 16
static size_t bt_allocations_limit = DEFAULT_MAX_ALLOCATIONS;

// maximum of depth of recorded backtrace
const static int pthread_debug_bt_max_depth = 10;
#endif

#ifdef _HOOK_PTHREAD_EXIT_
typedef void (*PthreadDebug_pthread_exit) (void*) __noreturn;
static PthreadDebug_pthread_exit real_pthread_exit;
#endif

static int try_read_tid(void *addr) __attribute__((noinline));

static int try_read_tid(void *addr) {
	int val = 0xdeadaee0;
#if defined(__arm__)
	asm volatile ("ldr     %0, [%1, #0]\n"
#elif defined(__aarch64__)
	asm volatile ("ldr     %w0, [%1, #0]\n"
#endif
			"b       1f\n"
			".long   0x75666475\n"
			".long   0x00006477\n"
			"1:\n"
			:"+r"(val):"r"(addr):"memory");
	return val;
}

static void pthread_func_rehook(void)
{
	if (pthread_create_libc_handle == NULL) {
		pthread_create_libc_handle = dlopen("libc.so", RTLD_NOW | RTLD_GLOBAL);
		if (pthread_create_libc_handle == NULL) {
			ubrd_error_log
			    ("[PTHREAD] Could not open libc.so for pthread debug!!: %s\n",
			     dlerror());
			return;
		}
	}

	real_pthread_create =
	    (PthreadDebug_pthread_create) dlsym(pthread_create_libc_handle, "pthread_create");
	if (!real_pthread_create) {
		ubrd_error_log("[PTHREAD] pthread_create ptr get fail: %s\n", dlerror());
	}

	real_pthread_join =
	    (PthreadDebug_pthread_join) dlsym(pthread_create_libc_handle, "pthread_join");
	if (!real_pthread_join) {
		ubrd_error_log("[PTHREAD] pthread_join ptr get fail: %s\n", dlerror());
	}

	real_pthread_detach =
	    (PthreadDebug_pthread_detach) dlsym(pthread_create_libc_handle, "pthread_detach");
	if (!real_pthread_detach) {
		ubrd_error_log("[PTHREAD] pthread_detach ptr get fail: %s\n", dlerror());
	}

#ifdef _HOOK_PTHREAD_EXIT_
	real_pthread_exit =
	    (PthreadDebug_pthread_exit) dlsym(pthread_create_libc_handle, "pthread_exit");
	if (!real_pthread_exit) {
		ubrd_error_log("[PTHREAD] pthread_exit ptr get fail: %s\n", dlerror());
	}
#endif
}

static void pthread_debug_prepare(void)
{
	if (g_PthreadDebug_PUBRD) {
		pthread_mutex_lock(&g_PthreadDebug_PUBRD->mMutex);
	}
}

static void pthread_debug_parent(void)
{
	if (g_PthreadDebug_PUBRD) {
		pthread_mutex_unlock(&g_PthreadDebug_PUBRD->mMutex);
	}
}

static void pthread_debug_child(void)
{
	if (g_PthreadDebug_PUBRD) {
		pthread_mutex_init(&g_PthreadDebug_PUBRD->mMutex, NULL);
	}
}

static void pthread_debug_exit()
{
	pthread_debug_inited = 0;
	g_PthreadDebug_PUBRD->mMspace = NULL;
}

/*
 * This will be used while removing a HashTableEntry
 *
 * "mAddr" is (void *) pthread_t thread_id
 * "mBytes" is (size_t) (void *) start_routine
 *
 */
PTHREADDEBUG_EXPORT
void pthread_debug_initialize()
{
	int pthread_debug_level = 1;
	char env[PROP_VALUE_MAX];
	char debug_config[PROP_VALUE_MAX];
	char debug_program[PROP_VALUE_MAX];
	const char *progname = NULL;
	void *lib = NULL;

	// User-defined configuration is not allowed now
	/*
	   0xc 2 000 010
	   1. debug mspace size: 0x010 == 16 * MB
	   2. historical table size: 0x000 = 0 * K
	   3. Backtrace depth: 0x0 2 000 000 = 2 * 5
	   4. 0xc, 0b1100:
	   b0: sig handler is disable
	   b2_b1:
	   [00: FP]
	   [01: GCC]
	   [10: corkscrew unwind] -- pthread_debug
	 */
	uint64_t debugConfig = 0xc2000010;

	progname = getprogname();
	if (!progname) {
		return;
	}

	if (strstr(progname, "/system/bin/logd")) {
		return;
	}

	if (__system_property_get("persist.vendor.debug.pthread", env)) {
		pthread_debug_level = atoi(env);
	}

	if (!pthread_debug_level) {
		return;
	}

	// Control for only one specific program to enable pthread debug
	if (__system_property_get("persist.vendor.debug.pthread.program", debug_program)) {
		if (strstr(getprogname(), debug_program) || !strcmp("all", debug_program))
			pthread_debug_level = 1;
		else
			pthread_debug_level = 0;
	}

	if (!pthread_debug_level) {
		return;
	}

	if (!real_pthread_create)
		pthread_func_rehook();

#ifdef _LEAK_DETECT_
	if (__system_property_get("persist.vendor.debug.pthread.leakdet", env)) {
		pthread_debug_leak_detect = atoi(env);
	}

	if (__system_property_get("persist.vendor.debug.pthread.max", env)) {
		bt_allocations_limit = (size_t) atoi(env);
		if (bt_allocations_limit < DEFAULT_MIN_ALLOCATIONS || bt_allocations_limit > DEFAULT_MAX_ALLOCATIONS) {
			bt_allocations_limit = DEFAULT_MAX_ALLOCATIONS;
		}
	}
#endif

	/*
	 * Use default compare function, which will only compare mAddr, which is pthread_t
	 */
	g_PthreadDebug_PUBRD = ubrd_init("PthreadDebug", debugConfig, NULL);
	if (g_PthreadDebug_PUBRD) {
		pthread_debug_inited = 1;
		if (pthread_atfork(pthread_debug_prepare, pthread_debug_parent, pthread_debug_child)) {
			ubrd_error_log("[PTHREAD] atfork failed\n");
		}

		pthread_internal_t pit;
		unsigned long addr_pit = (unsigned long)(&pit);
		unsigned long addr_pit_tid = (unsigned long)(&(pit.tid));
		g_tid_offset = addr_pit_tid - addr_pit;
		//ubrd_info_log("[PTHREAD] g_tid_offset = %lu\n", g_tid_offset);
		return;
	}
	// init fail
	ubrd_error_log("[PTHREAD] %s init fail\n", getprogname());
	return;
}

#ifdef _LEAK_DETECT_
static void dump_bt2log(uintptr_t * backtrace, size_t numEntries, void *start_routine)
{
	// 64-bit address will occupy 16 chars + "0x": 2 chars
	char buf[32];
	char tmp[32 * pthread_debug_bt_max_depth];
	size_t i;

	tmp[0] = 0;		// Need to initialize tmp[0] for the first strcat

	for (i = 0; i < numEntries; i++) {
		snprintf(buf, sizeof(buf), "%zu: %p\n", i, (void *)backtrace[i]);
		strlcat(tmp, buf, sizeof(tmp));
	}

	ubrd_error_log("[PTHREAD] start_routine: %p, call stack:\n%s", start_routine, tmp);
}
#endif

/*
 * To maintain original design of "EntryInfo" in UBRD, we will
 * Use thread_id as pEntryInfo->mAddr
 * Use start_routine as size for backtrace entry
 *
 * type of pEntryInfo->mAddr is "void *", and thread_id is "pthread_t" == "(pthread_internal_t *)"
 * type of pEntryInfo->mBytes is "size_t", which is "unsigned long",
 * and type of "pthread_internal_t->tid" is "pid_t", which is "int"
 *
 * type of btEntry->size is "size_t", which is "unsigned long", and start_routine is "void *"
 *
 * So, above type casting should be fine!
 */
static inline void PthreadDebug_btrace_record(pthread_t thread_id, void *start_routine)
{
	if (!thread_id || !g_PthreadDebug_PUBRD)
		return;

	if (!g_PthreadDebug_PUBRD->mMspace) {
		ubrd_error_log("[PTHREAD] pthread_create bt recorder disable\n");
		return;
	}

	if (g_tid_offset == (unsigned long) -1) {
		ubrd_error_log("[PTHREAD] g_tid_offset is not set\n");
		pthread_debug_exit();
		return;
	}

	pthread_internal_t *_thr = (pthread_internal_t *) thread_id;
	size_t _tid = 0;

	pthread_mutex_lock(&g_PthreadDebug_PUBRD->mMutex);

	PUBRD_EntryInfo pEntryInfo = (PUBRD_EntryInfo) mspaceAllocate(g_PthreadDebug_PUBRD, sizeof(UBRD_EntryInfo));
	if (pEntryInfo == NULL) {
		ubrd_error_log("[PTHREAD] allocation from mspace failed\n");
		pthread_debug_exit();
		pthread_mutex_unlock(&g_PthreadDebug_PUBRD->mMutex);
		return;
	}

	// for backward compatibility, type cast pthread_t and tid
	pEntryInfo->mAddr = (void *) thread_id;

	pid_t p_tid = (pid_t) try_read_tid((void *)((unsigned long)thread_id + g_tid_offset));
	if (p_tid == (int) 0xdeadaee0) {
		// the "tid" field is not accessible
		ubrd_error_log("[PTHREAD] pthread_t: 0x%lx, errno: %d, errstr: %s\n", thread_id, errno, strerror(errno));
		mspaceFree(g_PthreadDebug_PUBRD, (void *) pEntryInfo); // free the allocated for this EntryInfo
		pthread_mutex_unlock(&g_PthreadDebug_PUBRD->mMutex);
		return;
	} else {
		pEntryInfo->mBytes = (size_t)(p_tid);
	}

	// record backtrace
	// We use "start_routine" as "size" of UBRD BT entry
	// So, same pthread_create backtrace with same start_routine will be treated as same allocation
	PUBRD_BtEntry pBtEntry = recordBacktrace(g_PthreadDebug_PUBRD, (size_t) start_routine);
	if (pBtEntry == NULL) {
		pthread_debug_exit();
		mspaceFree(g_PthreadDebug_PUBRD, (void *) pEntryInfo); // free the allocated for the EntryInfo
		pthread_mutex_unlock(&g_PthreadDebug_PUBRD->mMutex);
		return;
	}

#ifdef _LEAK_DETECT_
	if ((pthread_debug_leak_detect == 1)
	    && (pBtEntry->allocations > bt_allocations_limit)) {
		ubrd_error_log(
			"[PTHREAD] Allocation of this backtrace exceeds limit: %lu\n",
			(unsigned long) bt_allocations_limit);
		dump_bt2log(pBtEntry->backtrace, pBtEntry->numEntries, start_routine);

		abort();
	}
#endif

	if (recordToHashTable(g_PthreadDebug_PUBRD, (size_t) thread_id, pEntryInfo, pBtEntry) == NULL) {
		ubrd_error_log("[PTHREAD] recordToHashTable failed\n");
		mspaceFree(g_PthreadDebug_PUBRD, (void *) pEntryInfo); // free the allocated for the EntryInfo
		mspaceFree(g_PthreadDebug_PUBRD, (void *) pBtEntry); // free the allocated for the BtEntry
		pthread_debug_exit();
		pthread_mutex_unlock(&g_PthreadDebug_PUBRD->mMutex);
		return;
	}

	pthread_mutex_unlock(&g_PthreadDebug_PUBRD->mMutex);
}


inline static int PthreadDebug_remove_record(pthread_t t)
{
	// invoke _nolock version, we will take care of lock/unlock ourselves
	// (special case for pthread_detach)
	return ubrd_btrace_remove_nolock(g_PthreadDebug_PUBRD, (void *)t, 0, NULL, 0);
}

/*
 * rehook pthread_create function.
 */
PTHREADDEBUG_EXPORT
int pthread_create(pthread_t* thread_out, pthread_attr_t const* attr,
		   void* (*start_routine)(void*), void* arg)
{
	int saved_errno_;
	int result = 0;

	if (!real_pthread_create)
		pthread_func_rehook();

	result = real_pthread_create(thread_out, attr, start_routine, arg);
	if (!pthread_debug_inited)
		return result;

	saved_errno_ = errno;

	if (!result) {
		/*
		 * If pthread_create with attr set as DETACHED,
		 * there will be no leakage in this case
		 */
		if (attr && (attr->flags & PTHREAD_ATTR_FLAG_DETACHED)) {
			errno = saved_errno_;
			return result;
		}

		/*
		 * Created with default detach state (PTHREAD_CREATE_JOINABLE)
		 * Need explicit pthread_join() or pthread_detach()
		 */

		/*
		 * print debug log before and after actually recording the backtrace
		 * to compare the sequence of calling pthread_create() and pthread_detach()
		 */

		// TODO: Remove these logs
		//ubrd_info_log("[PTHREAD][+][C][S] pthread_t: %p start_rt: %p\n", *thread_out, start_routine);
		PthreadDebug_btrace_record(*thread_out, (void *)start_routine);
		//ubrd_info_log("[PTHREAD][+][C][E] pthread_t: %p start_rt: %p\n", *thread_out, start_routine);
	} else if ((result != 0) && (result == EAGAIN)) {
		/*
		 * bionic pthread_create will return EAGAIN if:
		 *
		 * 1. mmap() stack failed
		 * 2. mprotect() the guard bytes failed
		 * 3. clone() filed due to "Too many processes are already running."
		 *
		 * "ENOMEM" returned by clone() should no be our interested case
		 */
		ubrd_error_log(
			"[PTHREAD] pthread_create with EAGAIN, start_routine: %p, errno: %d, saved_errno: %d\n",
			start_routine, errno, saved_errno_);

		// Bypass CTS test case pthread_create_EAGAIN
		size_t _cts_tc_stack_size = (size_t)(-1) & ~(getpagesize() - 1);
		if (attr && (attr->stack_size == _cts_tc_stack_size)) {
			ubrd_error_log("[PTHREAD] This should be CTS test case, errno: %d\n", saved_errno_);
		} else if (attr && attr->stack_size > PTRDIFF_MAX) {
			/*
			 *     AOSP art unit test "art/test/202-thread-oome/" will try to
			 *     allocate 3GB stack size during pthread_create(), which will
			 *     be blocked by 32-bit mmap() in bionic. (64-bit will pass)
			 */
			ubrd_error_log("[PTHREAD] Requested stack size is too large\n");
		} else {
			errno = saved_errno_;
			abort();
		}
	}

	errno = saved_errno_;
	return result;
}

/*
 * rehook pthread_join function.
 *
 * This will be always called "after" pthread_create() "inside" the same
 * calling thread of pthread_create(). Which means the "remove record"
 * will always "after" "btrace_record"!
 */
PTHREADDEBUG_EXPORT
int pthread_join(pthread_t t, void** return_value)
{
	if (!real_pthread_join)
		pthread_func_rehook();

	if (pthread_debug_inited && t) {
		//ubrd_info_log("[PTHREAD][-][J] pthread_t: %p\n", t);
		pthread_mutex_lock(&g_PthreadDebug_PUBRD->mMutex);
		PthreadDebug_remove_record(t); // we invoke _nolock version
		pthread_mutex_unlock(&g_PthreadDebug_PUBRD->mMutex);
	}

	return real_pthread_join(t, return_value);
}

/*
 * rehook pthread_detach function.
 *
 * Be noticed!
 * Since pthread_detach() could be called inside start_routine (pthread_detach(pthread_self())), which
 * means this function could be invoked and finished "before" our hooked pthread_create() returns.
 * (If the pthread_detach() is called from the calling thread, it should be ok.)
 *
 * So, the "remove record" could be invoked before "btrace_record"!
 *
 * Which may lead to:
 * 1. some records are kept, but they should be detached already (false alarm)
 * 2. the access "_thr->tid" in btrace_record" will trigger SIGSEGV!
 *
 * For example, the start_routine finished very quickly, and the calling thread
 * of read_pthread_create() can't get CPU usage, with pthread_detach or DETACHED@create,
 * the associated pthread stack has been tear down, which will cause following access to
 * the thread_id content (fields of pthread_internal_t) will get SIGSEGV!
 *
 * Example CR: ALPS02340427
 */
PTHREADDEBUG_EXPORT
int pthread_detach(pthread_t t)
{
	if (!real_pthread_detach)
		pthread_func_rehook();

	if (pthread_debug_inited && t) {
		//ubrd_info_log("[PTHREAD][-][D] pthread_t: %p\n", t);

		pthread_mutex_lock(&g_PthreadDebug_PUBRD->mMutex);

		// we invoke _nolock version API
		if (PthreadDebug_remove_record(t) == 1) {
			ubrd_info_log("[PTHREAD][-][D] pthread_t: %lu is not found in records\n", (unsigned long) t);
			// We only care the case that pthread_detach() is called inside start_routine.
			if (pthread_equal(t, pthread_self())) {
				/*
				 * This key (t) can't be found in hash table if
				 * 1. "real" pthread_create() is finished and returned
				 * 2. the btrace_record in "hooked" pthread_create() is not complete yet
				 */
				// TODO: Remove this log
				ubrd_info_log("[PTHREAD][-][D] pthread_t: %lu is called inside start_routine itself\n", (unsigned long) t);
			}
		}

		pthread_mutex_unlock(&g_PthreadDebug_PUBRD->mMutex);
	}

	return real_pthread_detach(t);
}

/*
 * rehook pthread_exit function.
 */
#ifdef _HOOK_PTHREAD_EXIT_
PTHREADDEBUG_EXPORT
void pthread_exit(void* return_value)
{
	if (!real_pthread_exit)
		pthread_func_rehook();

	// the pthread_self() should return valid pthread_t
	pthread_t t = pthread_self();
	pthread_internal_t *thread = (pthread_internal_t *)t;

	if (atomic_load(&thread->join_state) == THREAD_DETACHED) {
		ubrd_info_log("[PTHREAD][|][E] pthread_t: %p exit in detached state\n", t);
		/*
		 * If pthread is in detached state, after pthread_exit, the mmap()ed stack space
		 * will be tear down.
		 */
	}
	real_pthread_exit(return_value);
}
#endif // _HOOK_PTHREAD_EXIT_

#ifdef __cplusplus
}
#endif
