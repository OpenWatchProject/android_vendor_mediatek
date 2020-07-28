/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2017. All rights reserved.
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
*
* The following software/firmware and/or related documentation ("MediaTek Software")
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/

#include <xtensa/trax.h>
#include <mt_uart.h>
#include "memory.h"

int func_1(int a)
{
    a = a + 1;
    return a;
}
int func_2(int a)
{
    a = a - 1;
    return a;
}

int foo()
{
    int a = 0, i = 0;
    for (i = 0; i < 3 ; i++) {
        if (a == 0) {
            a = func_1(a);
        }
        if (a > 0) {
            a = func_2(a);
        }
    }
    return 0;
}

int trax_save(trax_context *context)
{
    int  ret_val = 0, count = 0;
    char dstbuf[8192];
    char buf[2048];
    int dma_ch = 0;
    char *ptr = dstbuf;

    /* While there are unread bytes in the trace RAM, allocate a buffer and read the contents of the trace RAM into this buffer. */
    while ((ret_val = trax_get_trace(context, buf, 2048)) > 0) {
        memcpy(ptr, buf, ret_val);
        count += ret_val;
        ptr = dstbuf + count;

        PRINTFD("ptr = %x\n", ptr);
    }
    PRINTFD("[%d]ret_val = %d buf = %x, dstbuf addr = %x\n", count, ret_val, buf,
            dstbuf);
    //flush to dram
    xthal_dcache_all_writeback();
    return 0;
}

int trax_init()
{

    int trig_flag = 0, trig_index = 0;
    int syncval;
    int postsize_count = 0, postsize_unit = 0;
    int ret = 0;
    /* Specify the start address and the end address.
     * In the current test case, we assume that the size of the trace RAM is 2kB, i.e. 512 words.
     * So, the permissible values of start and end addresses would lie in the range of 0 - 511, i.e. 0x0 - 0x1ff.*/
    unsigned startaddr = 0x0;
    unsigned endaddr = 0x1ff;
    unsigned long pcstart = 0, pcend = 0;
    /* Initializing the trax context is the first step */
    trax_context context;

    ret = trax_context_init_eri(&context);
    if (ret < 0) {
        PRINTFD("context_init failed %d\n", ret);
    }
#ifndef ETB_TRAX
    ret = trax_reset(&context);
#endif
    /* The stop trigger is placed on the entry of the function "foo".
     * The appropriate start and end ranges of the stop trigger are specified & verified. */
    //trax_set_pcstop (&context, 0, (unsigned int)(&foo),   (unsigned int) (&foo) + 3, 0);
    //trax_get_pcstop (&context, &trig_index, &pcstart, &pcend, &trig_flag);

    //PRINTFD("Get pcstop: index: 0x%x, pcstart: 0x%x, pcend: 0x%x,flags: 0x%x\n", trig_index, (int)pcstart, (int)pcend, trig_flag);

    /* The start and end addresses are set. If incorrect values, they are set to their default values. This can be verified by the corresponding get functions. */
    ret = trax_set_ram_boundaries(&context, startaddr, endaddr);
    if (ret < 0) {
        PRINTFD("Start or end addresses set incorrectly\n");
    }
    trax_get_ram_boundaries(&context, &startaddr, &endaddr);
    PRINTFD("Start addr: 0x%x, end addr: 0x%x\n", startaddr, endaddr);
    /* Synchronization period is set to 8, i.e. 1 synchronization message is emitted for every 8 messages emitted by the traceport. */
    trax_set_syncper(&context, 256);
    /*syncval = trax_get_syncper (&context);
    PRINTFD("Sync period set : %d\n", syncval);*/
    /* The postsize indicates the amount of tracing to be done post the stop trigger. Here, it is set to 20% of the trace RAM. */
    trax_set_postsize(&context, 20, 2);
    trax_get_postsize(&context, &postsize_count, &postsize_unit);
    PRINTFD("Get postsize: count: %d, unit: %x\n", postsize_count,
            postsize_unit);
    /* After setting all the parameters and observing that all have been set appropriately, the tracing is started. */
    trax_start(&context);
    foo();
    trax_stop_halt(&context, 1);
    /* The captured trace can be saved in the temp_tracefile, using the trax_save utility. */
    if (trax_save(&context) < 0) {
        return -1;
    }
    return 0;


}
