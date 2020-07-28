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
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <mt_reg_base.h>
#include <driver_api.h>
#include <platform_mtk.h>
#include <interrupt.h>
#include <scp_ipi.h>
#include <dma.h>
#include <wakelock.h>
#ifdef CFG_CACHE_SUPPORT
#include <cache_internal.h>
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif

// .share section should be zero filled

#define SHARESEC __attribute__ ((section (".share")))
#define PLT_LOG_ENABLE      0x504C5402 /*magic*/
#define BUF_LEN             1024       /*1KB*/
#define DRAM_BUF_LEN        0x200000   /*2MB*/
#define LOGGER_MAX_QUEUE    2          /*maximum timer queue number*/

#define SCP_LOGGER_DRAM_OFF_TIME       50000 /* 50 seconds*/
#define SCP_LOGGER_WAIT_TIMEOUT_NS     1000000000 /* 1 second*/

#define SCP_LOGGER_ON_BIT       (1<<0)  /* bit0 = 1, logger is on, else off*/
#define SCP_LOGGER_DRAM_ON_BIT  (1<<1)  /* bit1 = 1, logger_dram_use is on, else off*/
#define SCP_LOGGER_ON_CTRL_BIT    (1<<8)    /* bit8 = 1, enable function (logger/logger dram use) */
#define SCP_LOGGER_OFF_CTRL_BIT   (0<<8)    /* bit8 = 0, disable function */
#define SCP_LOGGER_ON             (SCP_LOGGER_ON_CTRL_BIT | SCP_LOGGER_ON_BIT)  /* let logger on */
#define SCP_LOGGER_OFF            (SCP_LOGGER_OFF_CTRL_BIT | SCP_LOGGER_ON_BIT) /* let logger off */
#define SCP_LOGGER_DRAM_ON        (SCP_LOGGER_ON_CTRL_BIT | SCP_LOGGER_DRAM_ON_BIT)  /* let logger dram use on */
#define SCP_LOGGER_DRAM_OFF       (SCP_LOGGER_OFF_CTRL_BIT | SCP_LOGGER_DRAM_ON_BIT)  /* let logger dram use off */

struct buffer_info_s {
    unsigned int r_pos;
    unsigned int w_pos;
};

struct dram_ctrl_s {
    unsigned int base;
    unsigned int size;
    unsigned int enable;
    unsigned int info_ofs;
    unsigned int buff_ofs;
    unsigned int buff_size;
};
extern UBaseType_t uxCriticalNesting;
void logger_update(void);
static unsigned int logger_dram_inited SHARESEC;
static unsigned int logger_wakeup_ap SHARESEC; /* scp wake up ap flag*/
static struct dram_ctrl_s logger_dram_ctrl SHARESEC;
static unsigned int logger_w_pos SHARESEC; /* next position to write */
static unsigned int logger_r_pos SHARESEC; /* last position to read */
static unsigned char logger_buf[BUF_LEN] SHARESEC;
volatile int logger_data_move_flag SHARESEC;  /* logger data move flag, 0:data need move, 1:data moving*/
wakelock_t scp_logger_wakelock;

/*
 * This structre use to record logger info.
 * need to sync with AP-side
 */
typedef struct {
    unsigned int scp_log_dram_addr;
    unsigned int scp_log_buf_addr;
    unsigned int scp_log_start_addr;
    unsigned int scp_log_end_addr;
    unsigned int scp_log_buf_maxlen;
} SCP_LOG_INFO;

int send_buf_full_ipi_no_wait(void)
{
    unsigned int magic = 0x5A5A5A5A;
    //PRINTF_E("send_buf_full_ipi_no_wait\n");
    if (scp_ipi_send(IPI_LOGGER_WAKEUP, &magic, sizeof(magic), 0, IPI_SCP2AP) == DONE)
        return 1;
    return 0;
}

static BaseType_t _LoggerTimerPendFunction(PendedFunction_t xFunctionToPend, TickType_t xTicksToWait)
{
    BaseType_t ret = pdFALSE;
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED &&
            xTaskGetSchedulerState() != taskSCHEDULER_SUSPENDED) {
        if (is_in_isr() || uxCriticalNesting > 0) {
            /*in isr or in critical section*/
            ret = xTimerPendFunctionCallFromISR(xFunctionToPend,
                                                NULL,
                                                0,
                                                NULL);
        } else {
            /*not in isr*/
            ret = xTimerPendFunctionCall(xFunctionToPend,
                                         NULL,
                                         0,
                                         xTicksToWait);
        }
    }
    return ret;
}

/* should be called in either critical section or ISR (non-preemptable) */
void logger_puts(const char *str, int length)
{
    unsigned int update_w_pos, update_r_pos;
    unsigned int wrap_length, r_need_update;
    BaseType_t ret;
    if (length >= BUF_LEN || length <= 0)
        return ;

    update_w_pos = logger_w_pos + length;
    update_r_pos = logger_r_pos;

    wrap_length = 0;
    r_need_update = 0;

    if (update_w_pos >= BUF_LEN) {
        update_w_pos -= BUF_LEN;

        wrap_length = update_w_pos;/*record overflow warp length*/
        length -= wrap_length;/*record not overflow length*/

        if (logger_w_pos < logger_r_pos || update_w_pos >= update_r_pos)
            r_need_update = 1;
    } else if (logger_w_pos < logger_r_pos && update_w_pos > update_r_pos) {
        r_need_update = 1;
    }

    if (r_need_update) {
        /*buffer overflow, reflesh*/
        update_r_pos = update_w_pos + 1;

        if (update_r_pos >= BUF_LEN)
            update_r_pos -= BUF_LEN;
    }

    if (length)
        memcpy(&logger_buf[logger_w_pos], str, length);

    if (wrap_length)
        memcpy(&logger_buf, str + length, wrap_length);

    logger_w_pos = update_w_pos;
    logger_r_pos = update_r_pos;
    /*verify and send a timer to move logger*/
    unsigned int buffer_length;

    if (logger_w_pos >= logger_r_pos)
        buffer_length = logger_w_pos - logger_r_pos;
    else
        buffer_length = BUF_LEN - (logger_r_pos - logger_w_pos);

    /*if buffer_length >= (BUF_LEN/2), move log to dram */
    if (!logger_dram_ctrl.enable)
        return;

    if (((BUF_LEN >> 1) <= buffer_length) && (logger_data_move_flag < LOGGER_MAX_QUEUE)) {
        ret = _LoggerTimerPendFunction((PendedFunction_t)logger_update, 10);
        if (ret == pdPASS) {
            logger_data_move_flag ++;
        }
    }
}

static unsigned int _logger_update_dma(unsigned int src_buf, unsigned int src_w_pos, unsigned int src_r_pos,
                                       unsigned int dst_buf, unsigned int dst_sz, unsigned int dst_r_pos, unsigned int *dst_w_pos)
{
    unsigned int src_len, dst_len;
    DMA_RESULT ret = 0;

    /*calculate DRAM logger buf remaining space*/
    if (*dst_w_pos > dst_r_pos)
        dst_len = dst_sz - *dst_w_pos + dst_r_pos - 1;
    else if (*dst_w_pos < dst_r_pos)
        dst_len = dst_r_pos - *dst_w_pos - 1;
    else
        dst_len = dst_sz - 1;

    /* sram empty */
    if (src_w_pos == src_r_pos) {
        PRINTF_E("log src_w_pos=src_r_pos\n");
        return 0;
    }
    /*src_len : data move length*/
    if (src_w_pos > src_r_pos)
        src_len = src_w_pos - src_r_pos;
    else
        src_len = BUF_LEN - src_r_pos;

    if (logger_wakeup_ap) {
        /* when DRAM log buffer over DRAM_BUF_LEN/4,
         * send a buffer full IPI
         * to wakeup ap
         */
        if ((DRAM_BUF_LEN >> 2) >= dst_len) {
            _LoggerTimerPendFunction((PendedFunction_t)send_buf_full_ipi_no_wait, 10);
        }
    }
    /* dram full, drop all */
    if (!dst_len) {
        PRINTF_E("log dram full drop all\n");
        return src_len;
    }
    if (src_len > dst_len)
        src_len = dst_len;

    /*moving log to dram buf*/
#ifdef SCP_DMA_VERSION_03
#else
    int dma_ch = LOGGER_DMA_CH;
#endif
    if (src_len != 0) {
        /*dram length need wrap*/
        if ((*dst_w_pos + src_len) > dst_sz) {
            src_len = dst_sz - *dst_w_pos;
        }
#ifdef SCP_DMA_VERSION_03
        ret = scp_dma_transaction(dst_buf + *dst_w_pos, src_buf + src_r_pos, src_len, LOGGER_DMA_ID, NO_RESERVED);
#else
        ret = dma_transaction_manual(dst_buf + *dst_w_pos, src_buf + src_r_pos, src_len, NULL, (uint32_t*)&dma_ch);
#endif
        if (ret != DMA_RESULT_DONE) {
            PRINTF_E("log dma trans fail%u\n", ret);
            return 0;
        }
    }

    /*update dst_w_pos, dram write pointer*/
    *dst_w_pos += src_len;

    if (*dst_w_pos >= dst_sz)
        *dst_w_pos -= dst_sz;

    return src_len;
}


void logger_update(void)
{
    unsigned int len;
    unsigned int buffer_length_now;
    unsigned int dst_base, dst_buf, dst_w_pos, dst_r_pos;
    unsigned int dram_return = 0;
    struct buffer_info_s *ctrl;
    BaseType_t ret;
    /*clear data move flag*/
    logger_data_move_flag = 0;
    //PRINTF_E("%s %x %x\n", __FUNCTION__, logger_dram_ctrl.enable, logger_dram_inited);
    if (!logger_dram_ctrl.enable || !logger_dram_inited)
        /*clear data move flag*/
        return ;

#ifdef CFG_VCORE_DVFS_SUPPORT
    dram_return = dvfs_enable_DRAM_no_wait_for_logger(SCP_A_LOGGER_MEM_ID);
#endif
    /*if dram need time. send a delay again*/
#define LOGGER_WAIT_DRAM 3
    if (dram_return) {
        ret = _LoggerTimerPendFunction((PendedFunction_t)logger_update, LOGGER_WAIT_DRAM);
        if (ret == pdTRUE) {
            /*set data moving flag*/
            logger_data_move_flag = 1;
        }
        /*send a timer and return, if ret = pdFALSE, just skip it. job */
        return;
    }
    taskENTER_CRITICAL();
    /*verify logger length avoid dummy move*/
    if (logger_w_pos >= logger_r_pos)
        buffer_length_now = logger_w_pos - logger_r_pos;
    else
        buffer_length_now = BUF_LEN - (logger_r_pos - logger_w_pos);
    if ((BUF_LEN >> 1) > buffer_length_now) {
        taskEXIT_CRITICAL();
#ifdef CFG_VCORE_DVFS_SUPPORT
        dvfs_disable_DRAM_no_wait_for_logger(SCP_A_LOGGER_MEM_ID);
#endif
        return;
    }

    /* sync dram ctrl base*/
    dst_base = ap_to_scp(logger_dram_ctrl.base);

    ctrl = (struct buffer_info_s *)(dst_base + logger_dram_ctrl.info_ofs);
    dst_buf = dst_base + logger_dram_ctrl.buff_ofs;
    dst_w_pos = ctrl->w_pos;
    dst_r_pos = ctrl->r_pos;

    if (dst_w_pos >= logger_dram_ctrl.buff_size || dst_r_pos >= logger_dram_ctrl.buff_size) {
        PRINTF_E("%s:err r=%u w=%u l=%u\n", __FUNCTION__, dst_w_pos, dst_r_pos, logger_dram_ctrl.buff_size);
        logger_dram_inited = 0;
        goto exit;
    }

    len = _logger_update_dma((unsigned int) &logger_buf[0], logger_w_pos, logger_r_pos, dst_buf,
                             logger_dram_ctrl.buff_size, dst_r_pos, &dst_w_pos);

    if (len) {
        /*update dst_w_pos, dram write pointer*/
        ctrl->w_pos = dst_w_pos;
        logger_r_pos += len;

        if (logger_r_pos >= BUF_LEN)
            logger_r_pos -= BUF_LEN;
    }

exit:
    taskEXIT_CRITICAL();
#ifdef CFG_VCORE_DVFS_SUPPORT
    dvfs_disable_DRAM_no_wait_for_logger(SCP_A_LOGGER_MEM_ID);
#endif
}


unsigned int logger_dram_register(unsigned int dram_ctrl_phy, unsigned int limit)
{
    unsigned int addr;
    struct dram_ctrl_s *ctrl;
    unsigned int ofs, size;

    if (!logger_dram_inited) {
        /*sync dram address*/
        addr = ap_to_scp(dram_ctrl_phy);

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

        /*enable logger by mobile log*/
        //logger_dram_ctrl.enable = 1;
        logger_dram_ctrl.base = dram_ctrl_phy;
        logger_dram_inited = 1;
        /*already get AP side DRAM info. and release wakelock*/
        wake_unlock_FromISR(&scp_logger_wakelock);
    }
abandon:
    return logger_dram_inited;
}

/* should be called in either critical section or ISR (non-preemptable) */
unsigned int logger_update_enable(unsigned int enable)
{
    unsigned int on_logger = enable & (SCP_LOGGER_ON_CTRL_BIT);
    unsigned int func_sel = enable & (~SCP_LOGGER_ON_CTRL_BIT);
    if (on_logger) {
        logger_dram_ctrl.enable |= (func_sel);
    } else {
        logger_dram_ctrl.enable &= (~func_sel);
    }
    return logger_dram_ctrl.enable;
}
/* should be called in either critical section or ISR (non-preemptable) */
unsigned int logger_update_wakeup_ap(unsigned int enable)
{
    enable = (enable) ? 1 : 0;

    logger_wakeup_ap = enable;

    return logger_wakeup_ap;
}

static void scp_logger_wakeup_handler(int id, void *data, unsigned int len)
{
    unsigned int wakeup;
    //unsigned int ret;
    wakeup = *(unsigned int *)data;
    logger_update_wakeup_ap(wakeup);
    //PRINTF_D("log wakeup=%u,update=%u\n",wakeup,ret);
}
static void scp_logger_enable_handler(int id, void *data, unsigned int len)
{
    unsigned int enable;
    enable = *(unsigned int *)data;
    logger_update_enable(enable);
}

static void scp_logger_init_handler(int id, void *data, unsigned int len)
{
    unsigned int ret;
    ret = logger_dram_register(logger_dram_ctrl.base, DRAM_BUF_LEN);
    if (ret == 0) {
        //PRINTF_E("log dram init fail\n");
        configASSERT(0);
        return;
    }
}

#ifdef CFG_LOGGER_BOOTLOG_SUPPORT
static void scp_dram_off_timer(TimerHandle_t xTimer)
{
    taskENTER_CRITICAL();
    logger_update_enable(SCP_LOGGER_DRAM_OFF);
    taskEXIT_CRITICAL();
}

static void wait_scp_logger_init_done(void)
{
    static TimerHandle_t scp_logger_timer;
#ifdef CFG_XGPT_SUPPORT
    unsigned long long timeout = read_xgpt_stamp_ns() + SCP_LOGGER_WAIT_TIMEOUT_NS;
#endif
    while (1) {
        /* wait until AP set it */
        if (logger_dram_ctrl.base != 0)
            break;
#ifdef CFG_XGPT_SUPPORT
        if (read_xgpt_stamp_ns() > timeout) {
            PRINTF_E("wait logger timeout!\n");
            goto _wait_scp_logger_init_end;
        }
#endif  // CFG_XGPT_SUPPORT
    }
    scp_logger_init_handler(0, NULL, 0);
    logger_update_enable(SCP_LOGGER_DRAM_ON);
    /* setup a timer to disable dram useage */
    scp_logger_timer = xTimerCreate("LoggerTimer", pdMS_TO_TICKS(SCP_LOGGER_DRAM_OFF_TIME), pdFALSE, (void *)0,
                                    scp_dram_off_timer);
    xTimerStart(scp_logger_timer, 0);
#ifdef CFG_XGPT_SUPPORT
_wait_scp_logger_init_end:
    PRINTF_E("Logger init done\n");
#endif  // CFG_XGPT_SUPPORT
}
#endif  // CFG_LOGGER_BOOTLOG_SUPPORT

void logger_init(void)
{
    enum ipi_id logger_init_id;
    SCP_LOG_INFO scp_logger_info;
    /*wakelock init*/
    wake_lock_init(&scp_logger_wakelock, "logwk");
    /*logger wake lock until receive AP side DRAM info.*/
    wake_lock(&scp_logger_wakelock);

#ifdef CFG_LOG_FILTER
    scp_ipi_registration(IPI_SCP_LOG_FILTER, set_log_filter, "log_filter");
#endif  // CFG_LOG_FILTER

    /*config initial*/
    logger_dram_inited = 0;
    logger_wakeup_ap = 0;
    logger_data_move_flag = 0;
    scp_ipi_registration(IPI_LOGGER_INIT_A, scp_logger_init_handler, "log initA");
    /*cmd for disable/enable log and wakeup flag*/
    scp_ipi_registration(IPI_LOGGER_ENABLE, scp_logger_enable_handler, "log enable");
    scp_ipi_registration(IPI_LOGGER_WAKEUP, scp_logger_wakeup_handler, "log wakeup");

    /* init scp logger structure
     * sync to AP side
     */
#ifdef SCP_DMA_VERSION_03
    scp_logger_info.scp_log_dram_addr = (unsigned int)&logger_dram_ctrl;
    scp_logger_info.scp_log_buf_addr = (unsigned int)&logger_buf;
    scp_logger_info.scp_log_start_addr = (unsigned int)&logger_r_pos;
    scp_logger_info.scp_log_end_addr = (unsigned int)&logger_w_pos;
    scp_logger_info.scp_log_buf_maxlen = (unsigned int)BUF_LEN;
#else
    scp_logger_info.scp_log_dram_addr = scp_dual_address_append((unsigned int)&logger_dram_ctrl);
    scp_logger_info.scp_log_buf_addr = scp_dual_address_append((unsigned int)&logger_buf);
    scp_logger_info.scp_log_start_addr = scp_dual_address_append((unsigned int)&logger_r_pos);
    scp_logger_info.scp_log_end_addr = scp_dual_address_append((unsigned int)&logger_w_pos);
    scp_logger_info.scp_log_buf_maxlen = (unsigned int)BUF_LEN;
#endif
    PRINTF_E("log_info dram=%x buf=%x start=%x end=%x maxlen=%x\n", scp_logger_info.scp_log_dram_addr,
             scp_logger_info.scp_log_buf_addr,
             scp_logger_info.scp_log_start_addr,
             scp_logger_info.scp_log_end_addr,
             scp_logger_info.scp_log_buf_maxlen);
    /*logger init finished, send buf address to AP*/

#ifdef SCP_DUAL_LOGGER
    if (SCP_CORE == SCP_A_ID)
#endif
        logger_init_id = IPI_LOGGER_INIT_A;
#ifdef SCP_DUAL_LOGGER
    else
        logger_init_id = IPI_LOGGER_INIT_B;
#endif

    /*send ipi */
    while (scp_ipi_send(logger_init_id, &scp_logger_info, sizeof(scp_logger_info), 0, IPI_SCP2AP) != DONE);

#ifdef CFG_LOGGER_BOOTLOG_SUPPORT
    /* wait scp logger ready */
    wait_scp_logger_init_done();
#endif
}


