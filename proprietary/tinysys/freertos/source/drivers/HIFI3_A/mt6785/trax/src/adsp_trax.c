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

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include <stdio.h>
#include <string.h>
#include <task.h>
#include <queue.h>
#include <adsp_excep.h>
#include <adsp_trax.h>
#include <mt_reg_base.h>
#include <adsp_ipi.h>
#include <hal_cache.h>
#include <interrupt.h>

#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif

// .share section should be zero filled

#define TRACE_DRAM_BUFFER        0x800   /*2KB*/
#define MAX_MSG_QUEUE_SIZE       2

enum trax_id {
    TRAX_ENABLE,
    TRAX_DISABLE,
};

struct trax_dram_ctrl_s {
    unsigned int base;
    unsigned int size;
    unsigned int enable;
};

static struct trax_dram_ctrl_s trax_dram_ctrl;
static QueueHandle_t msg_queue;
trax_context context;

static int adsp_trax_reset()
{
    int ret = 0;
    /* Specify the start address and the end address.
     * In the current test case, we assume that the size of the trace RAM is 2kB, i.e. 512 words.
     * So, the permissible values of start and end addresses would lie in the range of 0 - 511, i.e. 0x0 - 0x1ff.*/
    unsigned startaddr = 0x0;
    unsigned endaddr = 0x1ff;

    ret = trax_context_init_eri(&context);
    if (ret < 0) {
        PRINTF_E("context_init failed %d\n", ret);
    }
    ret = trax_reset(&context);
    /* The start and end addresses are set. If incorrect values, they are set to their default values. This can be verified by the corresponding get functions. */
    ret = trax_set_ram_boundaries(&context, startaddr, endaddr);
    if (ret < 0) {
        PRINTF_E("Start or end addresses set incorrectly\n");
    }
    trax_get_ram_boundaries(&context, &startaddr, &endaddr);
    PRINTF_D("Start addr: 0x%x, end addr: 0x%x\n", startaddr, endaddr);
    /* Synchronization period is set to 8, i.e. 1 synchronization message is emitted for every 8 messages emitted by the traceport. */
    trax_set_syncper(&context, 1);
    return ret;

}

int trax_save(trax_context *context)
{
    int  ret_val = 0, count = 0;
    char *dst_buf = (char *)trax_dram_ctrl.base; //Non-cacheable Reserved TRAX Dram
    char src_buf[256];   // add backup bit
    char *ptr = dst_buf;
    UBaseType_t uxSavedInterruptStatus;

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    /* While there are unread bytes in the trace RAM, allocate a buffer and read the contents of the trace RAM into this buffer. */
    while ((ret_val = trax_get_trace(context, src_buf, sizeof(src_buf))) > 0) {
        memcpy(ptr, src_buf, ret_val);
        count += ret_val;
        ptr = dst_buf + count;
    }
    /*flush cache toDram*/
    PRINTF_D("%s, addr = %p c = %d\n", __FUNCTION__, dst_buf, count);

    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                               HAL_CACHE_OPERATION_FLUSH_INVALIDATE,
                               (void *)dst_buf, TRACE_DRAM_BUFFER);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
    return count;
}

static void vTaskTraxLoop(void *pvParameters)
{
    uint8_t local_queue_id;
    int len = 0;

    while (1) {
        if (xQueueReceive(msg_queue, &local_queue_id,
                          portMAX_DELAY) == pdTRUE) {
            switch (local_queue_id) {
                case TRAX_ENABLE:
                    adsp_trax_reset(); // reset for pc stop trigger
                    trax_start(&context);
                break;
                case TRAX_DISABLE: {
                    trax_stop_halt(&context, 1);
                    len = trax_save(&context);
                    if (len < 0) {
                        PRINTF_E("trax_save error!\n");
                    }
                    else {
                        /* send save done ipi and content length back */
                        enum ipi_id trax_id = IPI_TRAX_DONE;
                        while (adsp_ipi_send(trax_id, &len, sizeof(len),
                                             0, IPI_ADSP2AP) != DONE);
                        /*after dump done, re-init the pc stop position*/
                        adsp_trax_reset();
                        /* Initializing the trax context is the first step */
                        trax_set_pcstop(&context, 0, (unsigned int)(&adsp_excep_handler),
                                        (unsigned int)(&adsp_excep_handler) + 3, 0);
                        trax_start(&context);
                    }
                }
                break;
            }
        }
    }

}
/* should be called in either critical section or ISR (non-preemptable) */
static void adsp_trax_enable_handler(int id, void *data, unsigned int len)
{
    static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    uint8_t queue_id = TRAX_DISABLE;

    queue_id = *(unsigned int *)data ? TRAX_ENABLE : TRAX_DISABLE;
    if (xQueueSendToBackFromISR(msg_queue, &queue_id,
                                &xHigherPriorityTaskWoken) != pdTRUE) {
        return;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void adsp_trax_init_handler(int id, void *data, unsigned int len)
{
    trax_dram_ctrl.base = *(unsigned int *)data;
}

int adsp_trax_init()
{
    enum ipi_id trax_init_id;
    unsigned int magic =  0xAAAABBBB;

    adsp_trax_reset();
    /* Initializing the trax context is the first step */
    trax_set_pcstop(&context, 0, (unsigned int)(&adsp_excep_handler),
                    (unsigned int)(&adsp_excep_handler) + 3, 0);

    /*logger ipi hander register*/
    if (SCP_CORE == HIFI3_A_ID) {
        adsp_ipi_registration(IPI_TRAX_INIT_A, adsp_trax_init_handler, "trax initA");
    }
    /*cmd for disable/enable log and wakeup flag*/

    adsp_ipi_registration(IPI_TRAX_ENABLE, adsp_trax_enable_handler,
                          "trax enable");
    trax_set_postsize(&context, 20, 2);

    msg_queue = xQueueCreate(MAX_MSG_QUEUE_SIZE, sizeof(uint32_t));
    xTaskCreate(vTaskTraxLoop, "Trax", 4096, (void *)4, 0, NULL);

    if (SCP_CORE == HIFI3_A_ID) {
        trax_init_id = IPI_TRAX_INIT_A;
    }
    /*send ipi */
    while (adsp_ipi_send(trax_init_id, &magic, sizeof(magic),
                         0, IPI_ADSP2AP) != DONE);
    /* start trax for catch exception*/
    trax_start(&context);
    return 0;
}


