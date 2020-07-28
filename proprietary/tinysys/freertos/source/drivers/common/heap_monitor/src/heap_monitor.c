/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 */

#include <string.h>
#include "heap_monitor.h"
#include "task.h"

extern struct heap_monitor_s __heap_mon_struct_start[];
extern struct heap_monitor_s __heap_mon_struct_end[];

/*
* tinysys_heap_monitor_dump : dump heap monitor struct
*/
void tinysys_heap_monitor_dump()
{
    struct heap_monitor_s *heap_mon;

    PRINTF_E("HEAP status dump:\n");
    for (heap_mon = __heap_mon_struct_start; heap_mon < __heap_mon_struct_end; heap_mon++) {
        /*init heap monitor size */
        PRINTF_E("HEAP name=%s,now=%u,limit=%u,peak=%u\n",heap_mon->name, heap_mon->current, heap_mon->limit, heap_mon->peak);
    }
}

/*
* tinysys_Malloc :  heap monitor for malloc
*/
void * tinysys_Malloc(size_t alloc_bytes, char* feature_name)
{
    struct heap_monitor_s *heap_mon;
    int found_heap_monitor = 0;
    size_t align_alloc_bytes;
    void * ret = NULL;

    for (heap_mon = __heap_mon_struct_start; heap_mon < __heap_mon_struct_end; heap_mon++) {

        if (!strncmp(heap_mon->name,feature_name,HEAP_MON_LENGTH)) {
            /*found heap monitor struct here*/
            //PRINTF_E("HEAP name=%s\n",heap_mon->name);
            /*monitor heap size*/
            extern size_t xHeapStructSize;
            align_alloc_bytes = alloc_bytes + xHeapStructSize;
            if( ( align_alloc_bytes & portBYTE_ALIGNMENT_MASK ) != 0x00 ) {
                /* Byte alignment required. */
                align_alloc_bytes += ( portBYTE_ALIGNMENT - ( align_alloc_bytes & portBYTE_ALIGNMENT_MASK ) );
            }
            heap_mon->current = heap_mon->current + align_alloc_bytes;

            /*update peak size*/
            if(heap_mon->peak < heap_mon->current) {
                heap_mon->peak = heap_mon->current;
            }

            if((heap_mon->current) > heap_mon->limit) {
                /* malloc over heap declare size, DUMP and ASSERT*/
                PRINTF_E("HEAP name=%s over limit\n", feature_name);
                tinysys_heap_monitor_dump();
                configASSERT(0);
            }
            /*malloc here*/
            found_heap_monitor = 1;
            ret = pvPortMalloc(alloc_bytes);
            //PRINTF_E("HEAP name:%s limit=%u now=%u\n", (heap_mon->name),(heap_mon->limit),(heap_mon->limit - heap_mon->current));
        }
    }
    if (found_heap_monitor == 0){
        PRINTF_E("HEAP monitor ERR NO Found\n");
        /* not found heap monitor, DUMP and ASSERT*/
        tinysys_heap_monitor_dump();
        configASSERT(0);
    }

    return ret;
}

/*
* tinysys_Free : heap monitor for free alloc
*/
void tinysys_Free(void* alloc_pointer, char* feature_name)
{
    struct heap_monitor_s *heap_mon;
    int found_heap_monitor = 0;
    size_t free_size;

    for (heap_mon = __heap_mon_struct_start; heap_mon < __heap_mon_struct_end; heap_mon++) {
        if (!strncmp(heap_mon->name,feature_name,HEAP_MON_LENGTH)) {
            /*found heap monitor struct here*/
            /*free heap memory here*/
            found_heap_monitor = 1;
            free_size = xPortFree(alloc_pointer);
            /*monitor heap size*/
            heap_mon->current = heap_mon->current - free_size;
            if((heap_mon->current) < 0) {
                /* malloc over heap declare size, DUMP and ASSERT*/
            }
            //PRINTF_D("HEAP monitor feature:%s limit:%u,remain:%u\n", (heap_mon->name),(heap_mon->limit),(heap_mon->limit - heap_mon->current));
        }
    }
    if (found_heap_monitor == 0){
        PRINTF_E("HEAP monitor ERR NO Found\n");
        /* not found heap monitor, DUMP and ASSERT*/
        tinysys_heap_monitor_dump();
        configASSERT(0);
    }

}

/*
* tinysys_heap_monitor_init : init heap monitor struct
*/
void tinysys_heap_monitor_init()
{
    struct heap_monitor_s *heap_mon;

    for (heap_mon = __heap_mon_struct_start; heap_mon < __heap_mon_struct_end; heap_mon++) {
        /*init heap monitor size */
        heap_mon->current = 0;
        heap_mon->peak = 0;
    }
}
