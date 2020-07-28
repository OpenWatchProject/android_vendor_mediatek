/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MC_DEBUGSESSION_H_
#define MC_DEBUGSESSION_H_

#define DEBUGSESSION_VERSION_MAJOR 0
#define DEBUGSESSION_VERSION_MINOR 1

#define MAX_THREADS         200
#define MAX_CORES           8
#define MAX_FC_EVENTS       64
#define MAX_SESSIONS        64
#define MAX_L2_TABLES       124
#define MAX_EXC_LOG         5

#define MC_UUID_DEBUG_SESSION { { 0x08, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }
#define DEBUG_SESSION_DUMP_FILE "/data/local/tmp/debugsession"

/*
 * Trustonic TEE DebugSession interface
 */
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t size;
    uint32_t no_entries;
    struct {
        uint32_t type;
        uint32_t version;
        uint32_t offset;
        uint32_t size;
    } headers[3];
} debugsession_header_t;

typedef struct {
    char productid[128];
    char productid_porting[128];
} tee_identity_t;

typedef struct {
    uint32_t threadid;
    uint32_t ipc_partner;
    uint32_t flags_priority;
    uint32_t pc;
    uint32_t sp;
    uint32_t last_syscall;
    uint64_t last_modified_time;
    uint64_t total_user_time;
    uint64_t total_sys_time;
} tee_thread_t;

typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
} tee_fastcall_event_t;

typedef struct {
    uint32_t write_pos;
    uint32_t size;
    tee_fastcall_event_t fc_events[MAX_FC_EVENTS];
} tee_fastcall_trace_t;

typedef struct {
    uint32_t current_thread;
    uint32_t last_thread;
    uint32_t last_syscall;
    uint32_t max_threads;
    uint32_t fc_trace_ready;
    uint32_t RFU;
    tee_thread_t threads[MAX_THREADS];
    tee_fastcall_trace_t fc_trace[MAX_CORES];
} tee_kernel_t;

typedef struct {
    uint32_t num_tables;
    uint32_t total_pages;
    uint32_t rfu_padding[2];
    struct {
        uint16_t        cntBlobRO;
        uint16_t        cntBlobRW;
        uint16_t        cntClient;
        uint16_t        cntWsm;

        uint16_t        cntPhys;
        uint16_t        cntHeap;
        uint16_t        cntMisc;
        uint16_t        rfu_padding;
    } area_counters;
} task_memory_t;

typedef struct {
    uint32_t sessionId; // valid
    uint32_t serviceType;
    uint32_t driverId;
    uint32_t sessionState;

    uint32_t isGP;
    uint32_t rfu_padding[3];

    uint8_t  uuid[16];

    uint32_t spid;
    uint32_t rootid;
    uint32_t maxThreads;
    uint32_t numInstances;

    uint32_t serviceFlags;
    uint32_t referencedL2Tables;
    uint32_t referencedPages;
    uint32_t mainThread;

    uint32_t secondThread; // notificationHandler?
    uint32_t rfu_padding1[3];

    task_memory_t task_memory;
    struct {
        uint32_t msgid;
        uint32_t msgLenPages;
        uint32_t mr0;
        uint32_t mr2;
    } last_ipch;
} tee_rtm_session_t;

typedef struct {
    uint32_t receiver;
    uint32_t trapType;
    uint32_t trapMeta;
    uint32_t rfu_padding;
} lastExc_t;

typedef struct {
    uint32_t write_pos;
    uint32_t rfu_padding[3];

    lastExc_t lastExc[MAX_EXC_LOG];
} exch_t;

typedef struct { //type TRTM
    struct {
        uint32_t lastReceiver; //threadids
        uint32_t lastSender;
        uint32_t lastError;
        uint32_t rfu_padding;
    } ipch;
    exch_t exch;
    struct {
        uint32_t lastMcpCommand;
        uint32_t lastOpenedSession;
        uint32_t lastClosedSession;
        uint32_t lastError;
    } msh;
    struct {
        uint32_t sId;
        uint32_t notification;
    } siqh;
    struct { //TODO was empty struct, just padding to compile correctly
        uint32_t rfu_padding[4];
    } pm_inth;
    uint32_t max_sessions;
    uint32_t rfu_padding;
    tee_rtm_session_t sessions[MAX_SESSIONS];
} tee_rtm_t;

#endif /* MC_DEBUGSESSION_H_ */
