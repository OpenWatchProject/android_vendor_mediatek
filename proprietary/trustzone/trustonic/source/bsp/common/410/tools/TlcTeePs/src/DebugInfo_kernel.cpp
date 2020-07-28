/*
 * Copyright (c) 2015-2018 TRUSTONIC LIMITED
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

#define SWD_EXIT_YIELD      0x1
#define SWD_EXIT_IRQ        0x2
#define SWD_ENTRY_EXTFIQ    0x4
#define SWD_ENTRY_FIQ       0x5
#define SWD_ENTRY_NSIQ      0x6
#define SWD_ENTRY_NYIELD    0x7

static const char * getEntryModeStr(uint32_t entryMode)
{
    switch (entryMode) {
        case  SWD_ENTRY_EXTFIQ  : return "eFIQ"; break;
        case  SWD_ENTRY_FIQ     : return "iFIQ"; break;
        case  SWD_ENTRY_NSIQ    : return "nSIQ"; break;
        case  SWD_ENTRY_NYIELD  : return "YIELD"; break;
        default :return "?";
    }
}

static const char * getExitModeStr(uint32_t exitMode)
{
    switch (exitMode) {
        case  SWD_EXIT_YIELD    : return "YIELD"; break;
        case  SWD_EXIT_IRQ      : return "IRQ"; break;
        default :return "?";
    }
}

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
    long long unsigned int last_exit = 0;
    long long unsigned int timeInSwTotal = 0;

    printf("\n" BOLD("TEE kernel info : ") "\n");
    printf("Dumping Kernel entry/exit time (oldest first)\n");
    // trying to align the formating in columns
    printf("\t%16s %16s %8s %8s %8s %16s ","Entry time (us)","Exit time (us)","SWd (us)","Nwd (us)",
                                            "Cycles", "Cpu Freq (~KHz)");
    printf("%7s %7s %3s %8s\n","Entry", "Exit", "Cpu", "Thread ID");
    for (uint32_t i = 0; i < MAX_TIMESTAMPS; ++i)
    {
        // select the oldest one.
        uint32_t idx = 1 + debug_info_kernel_->currentTimestampIdx;
        idx = (idx + i)%MAX_TIMESTAMPS;
        long long unsigned int entry = (debug_info_kernel_->lastSMCTimestamps[idx].entryTimestamp * 1000) / debug_info_kernel_->frequencyKHz;
        long long unsigned int exit = (debug_info_kernel_->lastSMCTimestamps[idx].exitTimestamp *1000) / debug_info_kernel_->frequencyKHz;
        long long unsigned int timeInSw = exit - entry;
        long long unsigned int timeInNw = (i == 0 ) ? 0 : (entry - last_exit); /* First iteration, we don't have the previous exit */
        printf("%3u)   %16llu %16llu %8llu %8llu %8llu %11llu      ", i,
                entry,
                exit,
                timeInSw,
                timeInNw,
                (long long unsigned int)debug_info_kernel_->lastSMCTimestamps[idx].cycles,
                (timeInSw == 0) ? 0 : (long long unsigned int)debug_info_kernel_->lastSMCTimestamps[idx].cycles / timeInSw);

        printf("%7s %7s %3d %8x\n",
                getEntryModeStr((debug_info_kernel_->lastSMCTimestamps[idx].entryMode)>>8),
                getExitModeStr((debug_info_kernel_->lastSMCTimestamps[idx].exitMode)>>8),
                (debug_info_kernel_->lastSMCTimestamps[idx].exitMode)&0xF,
                debug_info_kernel_->lastSMCTimestamps[idx].currentThreadId);
        // use the counter instead of the computed us to minimize approximation
        timeInSwTotal += (debug_info_kernel_->lastSMCTimestamps[idx].exitTimestamp - debug_info_kernel_->lastSMCTimestamps[idx].entryTimestamp);
        last_exit = exit;
    }
    long long unsigned int exit_time = debug_info_kernel_->lastSMCTimestamps[(debug_info_kernel_->currentTimestampIdx)%MAX_TIMESTAMPS].exitTimestamp;
    long long unsigned int entry_time = debug_info_kernel_->lastSMCTimestamps[(1+debug_info_kernel_->currentTimestampIdx)%MAX_TIMESTAMPS].entryTimestamp;
    long long unsigned int total_time = exit_time - entry_time;
    printf("Tee CPU usage on %d samples: %2llu%%\n", MAX_TIMESTAMPS, (timeInSwTotal * 100)/(total_time));

    printf("Dumping crashes (oldest first). Total = %d\n", debug_info_kernel_->totalCrashes);
    // trying to align the formating in columns
    printf("     %10s %10s %10s %10s %10s\n", "Thread ID", "PC", "SP", "taskOff", "mcLibOff");
    for (uint32_t i = 0; i < MAX_CRASH; ++i)
    {
        // select the oldest one.
        uint32_t idx = 1 + debug_info_kernel_->currentCrashIdx;
        idx = (idx + i)%MAX_CRASH;
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wformat"
        #pragma GCC diagnostic ignored "-Wformat-extra-args"
        printf("%u)   %10#x %10#x %10#x %10#x %10#x\n",
               i,
               debug_info_kernel_->crash[idx].threadid,
               debug_info_kernel_->crash[idx].ip,
               debug_info_kernel_->crash[idx].sp,
               debug_info_kernel_->crash[idx].taskOffset,
               debug_info_kernel_->crash[idx].mcLibOffset);
        #pragma GCC diagnostic pop
    }
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
        ::qsort(&temp, MAX_THREADS, sizeof(temp[0]), cmp_last_modified);
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
