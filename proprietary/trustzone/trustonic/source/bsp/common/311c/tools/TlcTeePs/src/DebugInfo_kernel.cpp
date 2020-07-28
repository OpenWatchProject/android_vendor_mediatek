/*
 * Copyright (c) 2015-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
#include <stdlib.h>
#include <stdint.h>

#include "buildTag.h"
#include "mcLoadFormat.h"
#include "mcinq.h"              // SID_INVALID SID_MCP
#include "log.h"

#include "MobiCore_defines.h"
#include "DebugInfo.h"
#include "Utils.h"

static int cmp_last_modified(
    const void *idx1,
    const void *idx2
) {
    const uint64_t *index1 = reinterpret_cast<const uint64_t*>(idx1);
    const uint64_t *index2 = reinterpret_cast<const uint64_t*>(idx2);

    // /!\ the bigger the last modified field is, the later the thread
    // has been modified
    if (index1[1] > index2[1]) return -1;
    else if (index1[1] == index2[1]) return 0;
    else return 1;
}

void DebugInfo::showBasicKernelInfo() {
    printf("\n" BOLD("TEE kernel info : ") "\n");
    printf(" Max threads=%i, current thread=0x%08x, last thread=0x%08x, last syscall=%s(%i)\n\n",
            debug_info_kernel_->max_threads, debug_info_kernel_->current_thread,
            debug_info_kernel_->last_thread, syscallToString(debug_info_kernel_->last_syscall),
            debug_info_kernel_->last_syscall);
}

void DebugInfo::showKernelInfo(bool ordered) {
    uint64_t temp[MAX_THREADS][2];
    int i;

    showBasicKernelInfo();

    for (i=0; i<MAX_THREADS; i++) {
        temp[i][0] = i;
        temp[i][1] = debug_info_kernel_->threads[i].last_modified_time;
    }

    if (true == ordered) {
        ::qsort(&temp, MAX_THREADS, sizeof(uint32_t[2]), cmp_last_modified);
    }

    for (i=0; i<MAX_THREADS; i++) {
        uint32_t index = static_cast<uint32_t>(temp[i][0]);
        showThreadInfo(index);
    }
}

void DebugInfo::showThreadInfo(uint32_t index) {
    tee_thread_t *t = &debug_info_kernel_->threads[index];

    if (t->threadid != 0 || t->pc != 0) {
        printf("%03x Thread Id=0x%08x at PC=0x%08x, SP=0x%08x\n"
               "\tflags=0x%08x, ipcPartner=0x%08x, last syscall=%s(%i),\n\tlast modified=%lu ms, total user time=%lu µs, total sys time=%lu µs\n\n",
               threadid_get_dbgId(t->threadid), t->threadid,
               t->pc, t->sp,
               t->flags_priority, t->ipc_partner, syscallToString(t->last_syscall), t->last_syscall,
               (u_long)t->last_modified_time, (u_long)t->total_user_time, (u_long)t->total_sys_time);
    }
}

void DebugInfo::showGroupedThreadsInfo() {
    showBasicKernelInfo();

    for (int i=0; i<MAX_SESSIONS; i++) {
        tee_rtm_session_t *s = &debug_info_rtm_->sessions[i];

        if (s->sessionId != SID_INVALID) {
            uint32_t taskid = threadid_get_taskid(s->mainThread);

            showSessionInfo(i);

            if (0 == taskid) {
                /* This session doesn't contain any threads */
                continue;
            }
            for (int j=0; j<MAX_THREADS; j++) {
                tee_thread_t *t = &debug_info_kernel_->threads[j];

                if (threadid_get_taskid(t->threadid) == taskid) {
                    printf("\t");
                    showThreadInfo(j);
                }
            }
        }
    }
}
