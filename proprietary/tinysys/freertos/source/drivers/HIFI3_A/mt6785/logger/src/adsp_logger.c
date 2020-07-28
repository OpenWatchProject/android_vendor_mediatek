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
#include "timers.h"
#include <string.h>
#include <stdio.h>
#include <mt_reg_base.h>
#include <driver_api.h>
#include <platform_mtk.h>
#include <interrupt.h>
#include <adsp_ipi.h>
#include <hal_cache.h>
#include <exception_handler.h>

#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif

#ifdef CFG_DEBUG_COMMAND_SUPPORT
#include "adsp_dbg.h"
#endif

#define PLT_LOG_ENABLE      0x504C5402 /*magic*/

struct buffer_info_s {
    unsigned int r_pos;
    unsigned char resv1[124]; /* dummy bytes for 128-byte align */
    unsigned int w_pos;
    unsigned char resv2[124]; /* dummy bytes for 128-byte align */
};

struct dram_ctrl_s {
    unsigned int base;
    unsigned int size;
    unsigned int enable;
    unsigned int info_ofs;
    unsigned int buff_ofs;
    unsigned int buff_size;
    unsigned char resv1[104]; /* dummy bytes for 128-byte align */
};

void logger_update(const char *str, int src_len);
static unsigned int logger_dram_inited;
static struct dram_ctrl_s logger_dram_ctrl;


/* should be called in either critical section or ISR (non-preemptable) */
void logger_puts(const char *str, int length)
{

    if (length <= 0) {
        return ;
    }

    logger_update(str, length);
}

/* don't put printf in logger_update to avoid recursive status */
void logger_update(const char *str, int src_len)
{

    unsigned int dst_base, dst_buf, dst_w_pos, dst_r_pos, dst_sz;
    unsigned int dst_len;
    struct buffer_info_s *ctrl;
    UBaseType_t uxSavedInterruptStatus;
    /* clear data move flag */
    if (!logger_dram_ctrl.enable || !logger_dram_inited)
    {
        return ;
    }

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();
    /* sync dram ctrl base */
    dst_base = logger_dram_ctrl.base;

    ctrl = (struct buffer_info_s *)(dst_base + logger_dram_ctrl.info_ofs);
    dst_buf = dst_base + logger_dram_ctrl.buff_ofs;
    dst_sz = logger_dram_ctrl.buff_size;

    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                               HAL_CACHE_OPERATION_INVALIDATE,
                               (void *)(ctrl), sizeof(*ctrl));

    dst_w_pos = ctrl->w_pos;
    dst_r_pos = ctrl->r_pos;

    if (dst_w_pos >= logger_dram_ctrl.buff_size ||
        dst_r_pos >= logger_dram_ctrl.buff_size) {
        logger_dram_inited = 0;
        goto exit;
    }

    /* calculate DRAM logger buf remaining space */
    if (dst_w_pos > dst_r_pos) {
        dst_len = dst_sz - dst_w_pos + dst_r_pos - 1;
    }
    else if (dst_w_pos < dst_r_pos) {
        dst_len = dst_r_pos - dst_w_pos - 1;
    }
    else {
        dst_len = dst_sz - 1;
    }

    /* dram full, drop all */
    if (!dst_len) {
        dst_w_pos += src_len;
        if (dst_w_pos >= dst_sz) {
            dst_w_pos -= dst_sz;
        }
        goto exit;
    }
    if (src_len > dst_len) {
        src_len = dst_len;
    }

    if (src_len != 0) {
        /* dram length need wrap */
        if ((dst_w_pos + src_len) > dst_sz) {
            src_len = dst_sz - dst_w_pos;
        }
    }

    /* memcpy string content to Logger Dram */
    memcpy((void *)(dst_buf + dst_w_pos), str, src_len);

    /* flush cache */
    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                               HAL_CACHE_OPERATION_FLUSH_INVALIDATE,
                               (void *)(dst_buf + dst_w_pos), src_len);

    /* update dst_w_pos, dram write pointer */
    dst_w_pos += src_len;

    if (dst_w_pos >= dst_sz) {
        dst_w_pos -= dst_sz;
    }

    /* update dram write pointer */
    ctrl->w_pos = dst_w_pos;
    /* flush RW cache */
    hal_cache_region_operation(HAL_CACHE_TYPE_DATA,
                               HAL_CACHE_OPERATION_FLUSH,
                               &ctrl->w_pos, sizeof(unsigned int));
exit:

    /* clear data move flag */
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
}


unsigned int logger_dram_register(unsigned int dram_ctrl_phy,
                                  unsigned int limit)
{
    unsigned int addr;
    struct dram_ctrl_s *ctrl;
    unsigned int ofs, size;

    if (!logger_dram_inited) {
        /* sync dram address */
        addr = dram_ctrl_phy;

        ctrl = (struct dram_ctrl_s *) addr;

        if (ctrl->base != PLT_LOG_ENABLE) {
            //PRINTF_E("log!= PLT_LOG_ENABLE\n");
            goto abandon;
        }

        if (ctrl->size != sizeof(*ctrl)) {
            //PRINTF_E("log!= sizeof(*ctrl)\n");
            goto abandon;
        }

        ofs = ctrl->info_ofs;

        if ((ofs + sizeof(struct buffer_info_s)) > limit) {
            //PRINTF_E("logbuf_info_s > limit\n");
            goto abandon;
        }

        logger_dram_ctrl.info_ofs = ofs;
        ofs = ctrl->buff_ofs;
        size = ctrl->buff_size;

        if ((ofs + size) > limit) {
            //PRINTF_E("log:(ofs + size) > limit\n");
            goto abandon;
        }
        logger_dram_ctrl.buff_ofs = ofs;
        logger_dram_ctrl.buff_size = size;

        /* enable logger by mobile log */
        logger_dram_ctrl.base = dram_ctrl_phy;
        logger_dram_inited = 1;

    }
abandon:
    return logger_dram_inited;
}

/* should be called in either critical section or ISR (non-preemptable) */
unsigned int logger_update_enable(unsigned int enable)
{
    enable = (enable) ? 1 : 0;

    logger_dram_ctrl.enable = enable;
    /* clear data move flag */

    return logger_dram_ctrl.enable;
}

static void adsp_logger_enable_handler(int id, void *data, unsigned int len)
{
    unsigned int enable;
    unsigned int ret;
    enable = *(unsigned int *)data;
    ret = logger_update_enable(enable);
}

static void adsp_logger_init_handler(int id, void *data, unsigned int len)
{
    unsigned int ret;
    unsigned int *mem_info = (unsigned int *)data;
    unsigned int addr, size;

    /* 0: logger buf addr, 1: logger buf size */
    /* 2: coredump buf addr, 3: coredump buf size */
    /* 4: debugdump buf addr, 5: debugdump buf size */
    PRINTF_D("%s, logger_dram=0x%x, size=%d, core_dump=0x%x, size=%d, debug_dump=0x%x, size=%d\n",
             __FUNCTION__,
             *(mem_info+0), *(mem_info+1),
             *(mem_info+2), *(mem_info+3),
             *(mem_info+4), *(mem_info+5));

    logger_dram_ctrl = *(struct dram_ctrl_s *)data;
    size = *(mem_info+1);
    ret = logger_dram_register(logger_dram_ctrl.base, size);

    addr = *(mem_info+2);
    size = *(mem_info+3);
    set_exception_context_pointer((void *)addr, size);

#ifdef CFG_DEBUG_COMMAND_SUPPORT
    addr = *(mem_info+4);
    size = *(mem_info+5);
    set_debug_buffer((void *)addr, size);
#endif

    if (ret == 0) {
        //PRINTF_E("log dram init fail\n");
        configASSERT(0);
        return;
    }
}

void logger_init(void)
{

    enum ipi_id logger_init_id;
    unsigned int magic =  PLT_LOG_ENABLE;

    /* config initial */
    logger_dram_inited = 0;

    /* logger ipi hander register */
    if (SCP_CORE == HIFI3_A_ID) {
        adsp_ipi_registration(IPI_LOGGER_INIT_A, adsp_logger_init_handler, "log initA");
    }
    /* cmd for disable/enable log and wakeup flag */

    adsp_ipi_registration(IPI_LOGGER_ENABLE, adsp_logger_enable_handler,
                          "log enable");


    if (SCP_CORE == HIFI3_A_ID) {
        logger_init_id = IPI_LOGGER_INIT_A;
    }
    /* send ipi */
    while (adsp_ipi_send(logger_init_id, &magic, sizeof(magic),
                         1, IPI_ADSP2AP) != DONE);
}


