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

#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <mt_reg_base.h>
#include <driver_api.h>
#include <platform_mtk.h>
#include <interrupt.h>
#include <wakelock.h>
#include "FreeRTOS.h"
#include "task.h"
#include <scp_ipi.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif

static volatile uint32_t g_ipi_count = 0;
static struct ipi_desc_t ipi_desc[NR_IPI];
static struct ipc_desc_t ipc_desc[IPC_MAX];
static struct share_obj *scp_send_obj, *scp_rcv_obj;
static uint32_t ipi_init_ready = 0;
static uint32_t scp_ipi_owner = 0;
static uint32_t scp_ipi_id_record = 0;
static uint32_t scp_ipi_id_record_count = 0;
static uint32_t scp_to_ap_ipi_count = 0;
static uint32_t ap_to_scp_ipi_count = 0;
uint32_t scp_current_ipi_id = 0;
uint32_t scp_max_duration_ipc_id = 0;
wakelock_t ap_wakelock;
wakelock_t connsys_wakelock;

void ipi_info_dump(enum ipi_id id)
{
    PRINTF_E("%u\t%u\t%u\t%u\t%u\t%u\t%u\t%s\n\r",
             id,
             (unsigned int)ipi_desc[id].recv_count,
             (unsigned int)ipi_desc[id].init_count,
             (unsigned int)ipi_desc[id].is_wakeup_src,
             (unsigned int)ipi_desc[id].success_count,
             (unsigned int)ipi_desc[id].busy_count,
             (unsigned int)ipi_desc[id].error_count,
             ipi_desc[id].name);
#if IPI_STAMP_DUMP
    /*time stamp*/
    int32_t i;
    for (i = 0; i < SCP_IPI_ID_STAMP_SIZE; i++) {
        if (ipi_desc[id].recv_timestamp[i] != 0) {
            PRINTF_E("send:%u time%llu\n",
                     ipi_desc[id].recv_flag[i],
                     ipi_desc[id].recv_timestamp[i]);
        }
    }
    for (i = 0; i < SCP_IPI_ID_STAMP_SIZE; i++) {
        if (ipi_desc[id].send_timestamp[i] != 0) {
            PRINTF_E("recv:%u time:%llu\n",
                     ipi_desc[id].send_flag[i],
                     ipi_desc[id].send_timestamp[i]);
        }
    }
#endif
}
void ipi_status_dump(void)
{
    int32_t id;

    PRINTF_E("id\trecv\tinit\twake\tsuccess\tbusy\terror\tname\n\r");
    for (id = 0; id < NR_IPI; id++) {
        if (ipi_desc[id].recv_count > 0 || ipi_desc[id].success_count > 0
                || ipi_desc[id].busy_count > 0 || ipi_desc[id].error_count > 0) {
            ipi_info_dump(id);
        }
    }
    PRINTF_E("ap->scp total=%u scp->ap total=%u\n\r",
             (unsigned int) ap_to_scp_ipi_count , (unsigned int) scp_to_ap_ipi_count);
}

void ipi_status_dump_id(enum ipi_id id)
{
    PRINTF_E("id\trecv\tinit\twake\tsuccess\tbusy\terror\tname\n\r");
    ipi_info_dump(id);
}

void ipi_scp2spm(void)
{
    /* wake up APMCU */
    SCP_TO_SPM_REG = IPC_SCP2SPM_BIT;
}

void scp_ipi_wakeup_ap_registration(enum ipi_id id)
{
    if (id < NR_IPI)
        ipi_desc[id].is_wakeup_src = 1;
}

/*
 * check if the ipi id is a wakeup ipi or not
 * if it is a wakeup ipi, request SPM to wakeup AP
@param id:       IPI ID
*/
#if 0
static void try_to_wakeup_ap(enum ipi_id id)
{
    if (id < NR_IPI)
        if (ipi_desc[id].is_wakeup_src == 1)
            ipi_scp2spm(); //wake APMCU up
}
#endif

/*
 * dump spm state and the ipi id passed in.
 *
 * @param id: IPI ID
 */
static void dump_scp2spm_stat(enum ipi_id id)
{
    PRINTF_E("ipi-%d dump spm state, 0x%x\n", id, SCP_TO_SPM_REG);
}

/*
 * send ipi to ap
@param id:       IPI ID
*/
static void ipi_scp2host(enum ipi_id id)
{
    /* History-2
     * always wake up AP when sending IPI, so try_to_wakeup_ap(id) here
     * is substituted by ipi_scp2spm();
     */
    ipi_scp2spm();

    scp_to_ap_ipi_count++;
    /*ipi send success*/
    ipi_desc[id].success_count++;
#ifdef CFG_IPI_STAMP_SUPPORT
    uint32_t flag = 0;

    flag = ipi_desc[id].success_count % SCP_IPI_ID_STAMP_SIZE;
    if (flag < SCP_IPI_ID_STAMP_SIZE) {
        ipi_desc[id].send_flag[flag] = ipi_desc[id].success_count;
#ifdef CFG_XGPT_SUPPORT
        ipi_desc[id].send_timestamp[flag] = timer_get_global_timer_tick();
#endif
    }

#endif
    SCP_TO_HOST_REG = IPC_SCP2HOST_BIT;
}

void ipc_handler(void)
{
    /* the higher the high priority */
    int i;
    uint64_t ipi_duration;

    for (i = (IPC_MAX - 1); i >= IPC0; i--) {
        if (GIPC_TO_SCP_REG & (1 << i)) {
            if (ipc_desc[i].handler) {
#ifdef CFG_XGPT_SUPPORT
                ipc_desc[i].last_enter = timer_get_global_timer_tick();
#endif
                ipc_desc[i].handler();
#ifdef CFG_XGPT_SUPPORT
                ipc_desc[i].last_exit = timer_get_global_timer_tick();
#endif
                /*monitor ipc handler*/
                if (ipc_desc[i].last_exit > ipc_desc[i].last_enter) {
                    scp_max_duration_ipc_id = i;
                    ipi_duration = ipc_desc[i].last_exit - ipc_desc[i].last_enter;
                    if (ipi_duration > ipc_desc[i].max_duration) {
                        ipc_desc[i].max_duration = ipi_duration;
                    }
                }

            }
            /* wlc */
            GIPC_TO_SCP_REG = (1 << i);
        }
    }
}
void ipi_handler(void)
{
    //PRINTF_D("ipi id:%d, reg:0x%x\n\r", scp_rcv_obj->id, GIPC_TO_SCP_REG);
    if (scp_rcv_obj->id >= NR_IPI) {
        PRINTF_E("wrong id:%d\n", scp_rcv_obj->id);
    } else if (ipi_desc[scp_rcv_obj->id].handler) {
        ap_to_scp_ipi_count++;
        scp_current_ipi_id = scp_rcv_obj->id;
        ipi_desc[scp_rcv_obj->id].recv_count ++;
#ifdef CFG_IPI_STAMP_SUPPORT
        uint32_t flag = 0;

        flag = ipi_desc[scp_rcv_obj->id].recv_count % SCP_IPI_ID_STAMP_SIZE;
        if (flag < SCP_IPI_ID_STAMP_SIZE) {
            ipi_desc[scp_rcv_obj->id].recv_flag[flag] = ipi_desc[scp_rcv_obj->id].recv_count;
#ifdef CFG_XGPT_SUPPORT
            ipi_desc[scp_rcv_obj->id].recv_timestamp[flag] = timer_get_global_timer_tick();
#endif
        }

#endif

#ifdef CFG_IPI_STAMP_SUPPORT
        ipi_desc[scp_rcv_obj->id].last_handled = (uint32_t)timer_get_global_timer_tick();
#else
        ipi_desc[scp_rcv_obj->id].last_handled = xTaskGetTickCountFromISR();
#endif
        ipi_desc[scp_rcv_obj->id].handler(scp_rcv_obj->id, scp_rcv_obj->share_buf, scp_rcv_obj->len);
    }
}

/**
 * @brief AP wakeup SCP and keep SCP awake
 */
void infra_irq_handler(void)
{
    unsigned int reg_val;
    reg_val = DRV_Reg32(INFRA_IRQ_SET) & 0xf;

    /* clr interrupt as early as possible to let AP leave busy waiting */
    DRV_WriteReg32(INFRA_IRQ_CLR, reg_val);

    if (reg_val & (1 << AP_AWAKE_LOCK)) {
        wake_lock_FromISR(&ap_wakelock);
    }
    if (reg_val & (1 << AP_AWAKE_UNLOCK)) {
        wake_unlock_FromISR(&ap_wakelock);
    }
    if (reg_val & (1 << CONNSYS_AWAKE_LOCK)) {
        wake_lock_FromISR(&connsys_wakelock);
    }
    if (reg_val & (1 << CONNSYS_AWAKE_UNLOCK)) {
        wake_unlock_FromISR(&connsys_wakelock);
    }
}

#if SCP_IPI_DEBUG
static void scp_ipi_debug(int id, void *data, unsigned int len)
{
    unsigned int buffer;
    buffer = *(unsigned int *)data;
    PRINTF_D("scp get debug ipi buf=%u, id=%u\n", buffer, id);
}
#endif

void scp_ipi_init(void)
{
    int32_t id;
    scp_send_obj = (struct share_obj *)SCP_IPC_SHARE_BUFFER;
    scp_rcv_obj = scp_send_obj + 1;
    PRINTF_E("scp_send_obj = %p \n\r", scp_send_obj);
    PRINTF_E("scp_rcv_obj = %p \n\r", scp_rcv_obj);
    PRINTF_E("scp_rcv_obj->share_buf = %p \n\r", scp_rcv_obj->share_buf);
    /*init ipi_desc*/
    for (id = 0; id < NR_IPI; id++) {
        ipi_desc[id].name = "";
        ipi_desc[id].init_count++;
        ipi_desc[id].handler = NULL;
    }
    /*init ipc_desc*/
    for (id = 0; id < IPC_MAX; id++) {
        ipc_desc[id].name = "";
        ipc_desc[id].max_duration = 0;
        ipc_desc[id].last_exit = 0;
        ipc_desc[id].last_enter = 0;
        ipc_desc[id].handler = NULL;
    }
    memset(scp_send_obj, 0, sizeof(*scp_send_obj));
    // memset(ipi_desc,0, sizeof(ipi_desc));

    GIPC_TO_SCP_REG = IPC_CLEAR_BIT;
    request_irq(IPC_HANDLER_IRQn, ipc_handler, "IPC");
    /* ap awake */
    wake_lock_init(&ap_wakelock, "AP_W");
    wake_lock_init(&connsys_wakelock, "CO_W");
    request_irq(INFRA_IRQn, infra_irq_handler, "INF");

    request_ipc(IPC0, ipi_handler, "IPI");
    ipi_init_ready = 1;
#if SCP_IPI_DEBUG
    scp_ipi_registration(IPI_TEST1, (ipi_handler_t)scp_ipi_debug, "IPIDebug");
#endif
#ifdef IPI_AS_WAKEUP_SRC
    //scp_wakeup_src_setup(MT_IPC_HOST_IRQ_ID, 1); //disable for FPGA
#endif
}

/*
@param id:       IPI ID
@param handler:  IPI handler
@param name:     IPI name
*/
ipi_status scp_ipi_registration(enum ipi_id id, ipi_handler_t handler, const char *name)
{
    if (id < NR_IPI && ipi_init_ready == 1) {
        ipi_desc[id].name = name;

        if (handler == NULL)
            return ERROR;

        ipi_desc[id].handler = handler;
        return DONE;

    } else {
        PRINTF_E("[IPI]id:%d, ipi_init_ready:%u", id, (unsigned int)ipi_init_ready);
        return ERROR;
    }
}

/*
@param id:       IPI ID
*/
ipi_status scp_ipi_unregistration(enum ipi_id id)
{
    if (id < NR_IPI && ipi_init_ready == 1) {
        ipi_desc[id].handler = NULL;
        return DONE;
    } else {
        PRINTF_E("[IPI]unregi err, id:%d, init_ready:%u", id, (unsigned int)ipi_init_ready);
        return ERROR;
    }
}

/*
@param id:       IPI ID
@param buf:      the pointer of data
@param len:      data length
@param wait:     If true, wait (atomically) until data have been gotten by Host
*/
ipi_status scp_ipi_send(enum ipi_id id, void* buf, uint32_t len, uint32_t wait, enum ipi_dir dir)
{
    uint32_t ipi_idx;
    uint16_t *ipi_count = (uint16_t *)scp_send_obj->reserve;;

#ifdef CFG_TESTSUITE_SUPPORT
    /*prevent from infinity wait when run testsuite*/
    wait = 0;
#endif
    /*avoid scp log print too much*/
    if (scp_ipi_id_record == id)
        scp_ipi_id_record_count++;
    else
        scp_ipi_id_record_count = 0;

    scp_ipi_id_record = id;

    if (is_in_isr() && wait) {
        /*prevent from infinity wait when be in isr context*/
        configASSERT(0);
    }
    if (id < NR_IPI) {
        if (len > sizeof(scp_send_obj->share_buf) || buf == NULL) {
            /*ipi send error*/
            ipi_desc[id].error_count++;
            return ERROR;
        }

        taskENTER_CRITICAL();
        /*check if there is already an ipi pending in AP*/
        if (SCP_TO_HOST_REG & IPC_SCP2HOST_BIT) {
            /* History-1:
             * If the following conditions meet,
             * 1)there is an ipi pending in AP
             * 2)the coming IPI is a wakeup IPI
             * so it assumes that AP is in suspend state
             * send a AP wakeup request to SPM.
             * Please reference History-2 as the following.
             */

            /* avoid scp log print too much
             * %==0 : switch between different IPI ID
             * %==1 : the same IPI ID comes again
             */
            if ((scp_ipi_id_record_count % IPI_PRINT_THRESHOLD == 0) ||
                    (scp_ipi_id_record_count % IPI_PRINT_THRESHOLD == 1)) {
                /* History-2:
                 * There is no non-wakeup ipi to AP so far, so
                 * try_to_wakeup_ap(id) is substitued by dump_scp2spm_stat(id)
                 * for supervison only.
                 */
                dump_scp2spm_stat(id);
                PRINTF_E("ipi busy,owner:%d\n", (int)scp_ipi_owner);
            }
            taskEXIT_CRITICAL();
            /*ipi send busy*/
            ipi_desc[id].busy_count++;
            return BUSY;
        }

        memcpy((void *)scp_send_obj->share_buf, buf, len);
        scp_send_obj->len = len;
        scp_send_obj->id = id;
        *ipi_count = (uint16_t)g_ipi_count;
        ipi_scp2host(id);
        /*get ipi owner*/
        scp_ipi_owner = id;
        g_ipi_count++;
        ipi_idx = g_ipi_count;

        taskEXIT_CRITICAL();

        if (wait)
            while ((SCP_TO_HOST_REG & IPC_SCP2HOST_BIT) && (ipi_idx == g_ipi_count));
    } else
        return ERROR;

    return DONE;
}

/**
 * @brief check SCP -> AP IPI is using now
 * @return pdFALSE, IPI is NOT using now
 * @return pdTRUE, IPI is using, and AP does not receive the IPI yet.
 */
uint32_t is_ipi_busy(void)
{
    return (SCP_TO_HOST_REG & IPC_SCP2HOST_BIT) ? pdTRUE : pdFALSE;
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

