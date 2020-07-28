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
#include <stdio.h>
#include <mt_reg_base.h>
#include <driver_api.h>
#include <platform_mtk.h>
#include <interrupt.h>
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include <adsp_ipi.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
#ifdef CFG_WDT_SUPPORT
#include <wdt.h>
#endif

#include <adsp_ipi_queue.h>
#include <audio_messenger_ipi.h>

static struct ipi_desc_t ipi_desc[NR_IPI];
static struct ipc_desc_t ipc_desc[IPC_MAX];
static struct share_obj *adsp_send_obj, *adsp_rcv_obj;
static uint32_t ipi_init_ready = 0;
static uint32_t adsp_ipi_owner = 0;

#if ADSP_IPI_DEBUG
#define MAGIC_NUM 0x5A5A
#define ASSERT_TEST 0xEE
#define WDT_TEST     0xD
#define HEAP_DETAIL  3
#define ENABLE_UART  1
#define DISABLE_UART 0

static void adsp_ipi_debug(int id, void *data, unsigned int len)
{
    unsigned int msg;
    msg = *(unsigned int *)data;

    switch (msg) {
    case MAGIC_NUM :
        PRINTF_D("adsp get debug ipi msg=%u, id=%u\n", msg, id);
        break;
    case ASSERT_TEST :
        PRINTF_D("Assert Test!\n");
        configASSERT(0);
        break;
    case WDT_TEST :
        PRINTF_D("WDT Test!\n");
#ifdef CFG_WDT_SUPPORT
        /* wake up APMCU */
        mtk_wdt_set_time_out_value(10);/*assign a small value to make ee sooner*/
        mtk_wdt_restart();
#endif
        break;
    case ENABLE_UART :
        set_uart_switch(ENABLE_UART);
        break;
    case DISABLE_UART :
        set_uart_switch(DISABLE_UART);
        break;
    }
    PRINTF_D("adsp get debug ipi msg=%x, id=%u\n", msg, id);
}
#endif

void ipi_adsp2spm(void)
{
    /* wake up APMCU */
    ADSP_SPM_WAKEUPSRC = IPC_ADSP2SPM_BIT;
}

void adsp_ipi_wakeup_ap_registration(enum ipi_id id)
{
    if (id < NR_IPI) {
        ipi_desc[id].is_wakeup_src = 1;
    }
}

/*
 * check if the ipi id is a wakeup ipi or not
 * if it is a wakeup ipi, request SPM to wakeup AP
@param id:       IPI ID
*/
static void try_to_wakeup_ap(enum ipi_id id)
{
    if (id < NR_IPI)
        if (ipi_desc[id].is_wakeup_src == 1) {
            ipi_adsp2spm(); // wake APMCU up
        }
}
/*
 * send ipi to ap
@param id:       IPI ID
*/
static void ipi_adsp2host(enum ipi_id id)
{
    try_to_wakeup_ap(id);
    ADSP_TO_HOST_REG = IPC_ADSP2HOST_BIT;
}

void ipc_handler(void)
{
    uint32_t sw_int_status = ADSP_SW_INT_SET;
    if (sw_int_status & (1 << 0)) {
        if (ipc_desc[0].handler) {
            ipc_desc[0].handler();
        }
        ADSP_SW_INT_CLR = (1 << 0); /* wlc */
    } else {
        PRINTF_W("Invalid SW INT status: 0x%X\n", sw_int_status);
    }
}

void ipi_handler(void)
{
    if (adsp_rcv_obj->id >= NR_IPI) {
        PRINTF_E("wrong id:%d\n", adsp_rcv_obj->id);
        return;
    }

    if (ipi_desc[adsp_rcv_obj->id].handler == NULL) {
        PRINTF_E("id:%d hdl null\n", adsp_rcv_obj->id);
        return;
    }
    {
        int ret = scp_dispatch_ipi_hanlder_to_queue(
            adsp_rcv_obj->id,
            adsp_rcv_obj->share_buf,
            adsp_rcv_obj->len,
            ipi_desc[adsp_rcv_obj->id].handler);
        if (ret != 0) {
            PRINTF_W("ipi dispatch fail = %d\n", ret);
        }
    }
}


void adsp_ipi_init(void)
{
    int32_t id;

    // clean up ipi irq
    ADSP_SW_INT_CLR = 0xf;

    scp_ipi_queue_init();

    adsp_send_obj = (struct share_obj *)ADSP_IPC_BUF_BASE;
    adsp_rcv_obj = adsp_send_obj + 1;
    PRINTF_E("adsp_send_obj = %p \n", adsp_send_obj);
    PRINTF_E("adsp_rcv_obj = %p \n", adsp_rcv_obj);
    PRINTF_E("adsp_rcv_obj->share_buf = %p \n", adsp_rcv_obj->share_buf);
    /*init ipi_desc*/
    for (id = 0; id < NR_IPI; id++) {
        ipi_desc[id].name = "";
        ipi_desc[id].handler = NULL;
    }
    /*init ipc_desc*/
    for (id = 0; id < IPC_MAX; id++) {
        ipc_desc[id].name = "";
        ipc_desc[id].handler = NULL;
    }
    memset(adsp_send_obj, 0, SHARE_BUF_SIZE);
    // memset(ipi_desc,0, sizeof(ipi_desc));
    request_irq(IPC0_IRQn, ipc_handler, "IPC0"); //SW int0 from AP to SCP
    request_ipc(IPC0, ipi_handler, "IPI");
    ipi_init_ready = 1;
#if ADSP_IPI_DEBUG
    adsp_ipi_registration(IPI_TEST1, (ipi_handler_t)adsp_ipi_debug, "IPIDebug");
#endif
}

/*
@param id:       IPI ID
@param handler:  IPI handler
@param name:     IPI name
*/
ipi_status adsp_ipi_registration(enum ipi_id id, ipi_handler_t handler, const char *name)
{
    if (id < NR_IPI && ipi_init_ready == 1) {
        ipi_desc[id].name = name;

        if (handler == NULL) {
            return ERROR;
        }

        ipi_desc[id].handler = handler;
        return DONE;

    } else {
        PRINTF_E("[IPI]id:%d, ipi_init_ready:%u\n", id, ipi_init_ready);
        return ERROR;
    }
}

/*
@param id:       IPI ID
*/
ipi_status adsp_ipi_unregistration(enum ipi_id id)
{
    if (id < NR_IPI && ipi_init_ready == 1) {
        ipi_desc[id].handler = NULL;
        return DONE;
    } else {
        PRINTF_E("[IPI]unregi err, id:%d, init_ready:%u\n", id, ipi_init_ready);
        return ERROR;
    }
}

/*
@param id:       IPI ID
@param buf:      the pointer of data
@param len:      data length
@param wait:     If true, wait (atomically) until data have been gotten by Host
*/
ipi_status adsp_ipi_send(enum ipi_id id, void *buf, uint32_t len, uint32_t wait, enum ipi_dir dir)
{
    //return adsp_ipi_send_ipc(id, buf, len, wait, dir);
    return (scp_send_msg_to_queue(id, buf, len) == 0) ? DONE : ERROR;
}


ipi_status adsp_ipi_send_ipc(enum ipi_id id, void *buf, uint32_t len, uint32_t wait, enum ipi_dir dir)
{
    struct ipi_msg_t *p_ipi_msg = NULL;
    uint64_t time_ipc_ns = 0;
    static bool busy_log_flag;

    UBaseType_t uxSavedInterruptStatus;

    if (is_in_isr() && wait) {
        /* prevent from infinity wait when be in isr context */
        configASSERT(0);
    }
    if (id >= NR_IPI) {
        PRINTF_E("wrong id: %d\n", id);
        return ERROR;
    }
    if (len > sizeof(adsp_send_obj->share_buf) || buf == NULL) {
        /* ipi send error */
        return ERROR;
    }

    uxSavedInterruptStatus = portSET_INTERRUPT_MASK_FROM_ISR();

    /* check if there is already an ipi pending in AP */
    if (!(ADSP_TO_HOST_REG & IPC_ADSP2HOST_BIT)) { // liang modi: 0 to assert irq
        if (busy_log_flag == false) {
            busy_log_flag = true;
            p_ipi_msg = (struct ipi_msg_t *)adsp_send_obj->share_buf;
            if (p_ipi_msg->magic == IPI_MSG_MAGIC_NUMBER) {
                DUMP_IPI_MSG("busy. ipc owner", p_ipi_msg);
            } else {
                PRINTF_E("ipi busy,last owner:%d\n", adsp_ipi_owner);
            }
        }
        portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);
        return BUSY;
    }
    busy_log_flag = false;

    /* get ipi owner */
    adsp_ipi_owner = id;

    memcpy((void *)adsp_send_obj->share_buf, buf, len);
    adsp_send_obj->len = len;
    adsp_send_obj->id = id;
    ipi_adsp2host(id);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(uxSavedInterruptStatus);

    if (wait) {
        time_ipc_ns = timer_get_ap_timestamp();
        while (!(ADSP_TO_HOST_REG & IPC_ADSP2HOST_BIT)) {
            time_ipc_ns = timer_get_ap_timestamp() - time_ipc_ns;
            if (time_ipc_ns > 1000000) { /* 1 ms */
                break;
            }
        }
    }

    return DONE;
}

/**
 * @brief check ADSP -> AP IPI is using now
 * @return pdFALSE, IPI is NOT using now
 * @return pdTRUE, IPI is using, and AP does not receive the IPI yet.
 */
uint32_t is_ipi_busy(void)
{
    return (ADSP_TO_HOST_REG & IPC_ADSP2HOST_BIT) ? pdFALSE : pdTRUE;
}

/** register a ipc handler
*
*  @param ipc number
*  @param ipc handler
*  @param ipc name
*
*  @returns
*    no return
*/
void request_ipc(uint32_t ipc_num, ipc_handler_t handler, const char *name)
{
    if (ipc_num < IPC_MAX) {
        ipc_desc[ipc_num].handler = handler;
        ipc_desc[ipc_num].name = name;
    }
}

