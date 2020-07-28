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
#include "mcinq.h"      // SID_INVALID SID_MCP
#include "log.h"

#include "MobiCore_defines.h"
#include "DebugInfo.h"
#include "Utils.h"

void DebugInfo::showRTMInfo() {
    printf("\n" BOLD("RTM IPCH info : ") "\n");
    printf(" last receiver=0x%x, last sender=0x%x, last error=0x%x\n",
            debug_info_rtm_->ipch.lastReceiver, debug_info_rtm_->ipch.lastSender,
            debug_info_rtm_->ipch.lastError);

    printf("\n" BOLD("RTM EXCH info : ") "\n");
    /* Print exceptions in reverse order */
    int pExc = debug_info_rtm_->exch.write_pos;
    do {
        pExc--;
        if (pExc<0) pExc = MAX_EXC_LOG-1;
        if (debug_info_rtm_->exch.lastExc[pExc].receiver != 0)
            printf(" %x: trap type=%s, data=%08x\n",
                    debug_info_rtm_->exch.lastExc[pExc].receiver,
                    exchTypeToString((trapTypes_t)debug_info_rtm_->exch.lastExc[pExc].trapType),
                    debug_info_rtm_->exch.lastExc[pExc].trapMeta);
    } while (static_cast<unsigned int>(pExc) != debug_info_rtm_->exch.write_pos);

    printf("\n" BOLD("RTM MSH info : ") "\n");
    printf(" last MCP command=0x%x, last opened session=0x%x,"
            " last closed session=0x%x, last error=0x%x\n",
            debug_info_rtm_->msh.lastMcpCommand, debug_info_rtm_->msh.lastOpenedSession,
            debug_info_rtm_->msh.lastClosedSession, debug_info_rtm_->msh.lastError);

    printf("\n" BOLD("RTM SIQH info : ") "\n");
    printf(" last notification=0x%x, to session=0x%x\n",
            debug_info_rtm_->siqh.notification, debug_info_rtm_->siqh.sId);

    mem_usage_t *mem_usage = &debug_info_rtm_->mem_usage;
    printf("\n" BOLD("RTM memory usage info : ") "\n");
    printf(" RTM heap usage: %u of %u bytes (alloc/free diff count: %u)\n",
            mem_usage->heapUsage,
            mem_usage->heapSize,
            mem_usage->pending_free_counter);
    printf(" table pool usage: %up/%iKB of %up/%iKB\n",
            mem_usage->l2TablePoolUsage, mem_usage->l2TablePoolUsage * 4,
            mem_usage->l2TablePoolMax, mem_usage->l2TablePoolMax * 4);
    printf(" page pool usage: %up/%iKB of %up/%iKB\n",
            mem_usage->pTablePoolUsage, mem_usage->pTablePoolUsage * 4,
            mem_usage->pTablePoolMax, mem_usage->pTablePoolMax * 4);

    printf("\n" BOLD("RTM session info : ") "\n");
    for (int i=0; i<MAX_SESSIONS; i++) {
        tee_rtm_session_t *s = &debug_info_rtm_->sessions[i];

        if (s->sessionId == SID_INVALID || s->sessionId == SID_MCP) {
            continue;
        }
        showSessionInfo(i);
    }
}

void DebugInfo::showSessionInfo(uint32_t index) {
        tee_rtm_session_t *s = &debug_info_rtm_->sessions[index];

        switch (s->serviceType) {
            case SERVICE_TYPE_DRIVER:
            case SERVICE_TYPE_MIDDLEWARE:
                printf("%03x: %s, ID=0x%x/%i\n",
                        s->sessionId,
                        smgntTypeToString((serviceType_t)s->serviceType),
                        s->driverId, s->driverId);
                break;
            default:
                //if (pRegEntry->flags.is_sd) {s = "Security Domain";
                printf("%03x: %s\n", s->sessionId, smgntTypeToString((serviceType_t)s->serviceType));
        }

        printf(" SPID=0x%08x, UUID=%02X%02X%02X%02X-%02X%02X-%02X%02X-"
                "%02X%02X-%02X%02X%02X%02X%02X%02X\n",
                s->spid,
                s->uuid[ 0],
                s->uuid[ 1],
                s->uuid[ 2],
                s->uuid[ 3],
                s->uuid[ 4],
                s->uuid[ 5],
                s->uuid[ 6],
                s->uuid[ 7],
                s->uuid[ 8],
                s->uuid[ 9],
                s->uuid[10],
                s->uuid[11],
                s->uuid[12],
                s->uuid[13],
                s->uuid[14],
                s->uuid[15]);
        printf(" State=%s, flags=0x%08x, max instances=%u\n",
                smgntStateToString((sessionState_t)s->sessionState),
                s->serviceFlags, s->numInstances);

        printf(" Uses a total of %u pages (%i KB), %i tables:\n",
                s->task_memory.total_pages, s->task_memory.total_pages*4, s->task_memory.num_tables);
        printf("  BlobRO=%up/%iKB, BlobRW=%up/%iKB, Client=%up/%iKB,\n"
               "  WSM=%up/%iKB, Phys=%up/%iKB, Heap=%up/%iKB, Misc=%up/%iKB\n",
                s->task_memory.area_counters.cntBlobRO   , s->task_memory.area_counters.cntBlobRO   *4,
                s->task_memory.area_counters.cntBlobRW   , s->task_memory.area_counters.cntBlobRW   *4,
                s->task_memory.area_counters.cntClient   , s->task_memory.area_counters.cntClient   *4,
                s->task_memory.area_counters.cntWsm      , s->task_memory.area_counters.cntWsm      *4,
                s->task_memory.area_counters.cntPhys     , s->task_memory.area_counters.cntPhys     *4,
                s->task_memory.area_counters.cntHeap     , s->task_memory.area_counters.cntHeap     *4,
                s->task_memory.area_counters.cntMisc     , s->task_memory.area_counters.cntMisc     *4);

        printf(" IPCH info: ID=%i, len pages=%i, mr0=0x%x, mr2=0x%x\n\n",
                s->last_ipch.msgid, s->last_ipch.msgLenPages,
                s->last_ipch.mr0, s->last_ipch.mr2);
}
