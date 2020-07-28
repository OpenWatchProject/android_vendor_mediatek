/*
 * Copyright (c) 2015 TRUSTONIC LIMITED
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
#include "log.h"

#include "MobiCore_defines.h"
#include "DebugInfo.h"
#include "Utils.h"

void DebugInfo::showFCInfo() {
    printf("\n" BOLD("TEE fastcall info : ") "\n");
    for (int i=0; i<MAX_CORES; i++) {
        tee_fastcall_trace_t *t = &debug_info_kernel_->fc_trace[i];
        uint32_t pEvent = t->write_pos;
        /* Check if core is used */
        uint32_t initValue = 0xFCFC0000 | (i << 8);
        if (((initValue | 0) == t->fc_events[pEvent].r0) &&
                ((initValue | 1) == t->fc_events[pEvent].r1) &&
                ((initValue | 2) == t->fc_events[pEvent].r2) &&
                ((initValue | 3) == t->fc_events[pEvent].r3)) {
            continue;
        }

        printf("Core %d | size=0x%02x write_pos=0x%02x\n",
                i, t->size, t->write_pos);
        /* Print events in reverse order */
        if(t->size != 0) {
            do {
                pEvent = (pEvent-1) % t->size;
                printf(" %s: r1=%08x r2=%08x r3=%08x\n",
                        fcTypeToString(t->fc_events[pEvent].r0),
                        t->fc_events[pEvent].r1,
                        t->fc_events[pEvent].r2,
                        t->fc_events[pEvent].r3);
            } while (pEvent != (t->write_pos+1));
        }
    }
}
