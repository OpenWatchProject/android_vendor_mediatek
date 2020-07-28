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
#include "FreeRTOS.h"
#include "task.h"
#include <scp_ipi.h>

static volatile uint32_t g_ipi_count = 0;
static struct ipi_desc_t ipi_desc[NR_IPI];
static struct share_obj *scp_send_obj, *scp_rcv_obj;
static uint32_t ipi_init_ready = 0;
static uint32_t scp_ipi_owner = 0;
static uint32_t scp_ipi_id_record = 0;
static uint32_t scp_ipi_id_record_count = 0;

void ipi_status_dump(void)
{
    int32_t id;

    PRINTF_E("id\tname\tcount\tlast\twakeup\n\r");
    for (id = 0; id < NR_IPI; id++) {
        PRINTF_E("%u\t%s\t%u\t%u\t%u\t%u\n\r",
                 id, ipi_desc[id].name, ipi_desc[id].irq_count, ipi_desc[id].init_count, ipi_desc[id].last_handled,
                 ipi_desc[id].is_wakeup_src);
    }
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
static void try_to_wakeup_ap(enum ipi_id id)
{
    if (id < NR_IPI)
        if (ipi_desc[id].is_wakeup_src == 1)
            ipi_scp2spm(); //wake APMCU up
}
/*
 * send ipi to ap
@param id:       IPI ID
*/
static void ipi_scp2host(enum ipi_id id)
{
    try_to_wakeup_ap(id);
    SCP_TO_HOST_REG = IPC_SCP2HOST_BIT;
}

void ipi_handler(void)
{
    if (scp_rcv_obj->id >= NR_IPI) {
        PRINTF_E("wrong id:%d\n", scp_rcv_obj->id);
    } else if (ipi_desc[scp_rcv_obj->id].handler) {
        ipi_desc[scp_rcv_obj->id].irq_count ++;
        ipi_desc[scp_rcv_obj->id].last_handled = xTaskGetTickCountFromISR();
        ipi_desc[scp_rcv_obj->id].handler(scp_rcv_obj->id, scp_rcv_obj->share_buf, scp_rcv_obj->len);
    }
    GIPC_TO_SCP_REG = IPC_CLEAR_BIT;
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
        ipi_desc[id].irq_count = 0;
        ipi_desc[id].init_count = 0;
        ipi_desc[id].last_handled = 0;
        ipi_desc[id].handler = NULL;
    }
    memset(scp_send_obj, 0, SHARE_BUF_SIZE);
    // memset(ipi_desc,0, sizeof(ipi_desc));

    GIPC_TO_SCP_REG = IPC_CLEAR_BIT;
    request_irq(IPC_HANDLER_IRQn, ipi_handler, "IPC");
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
        PRINTF_E("[IPI]id:%d, ipi_init_ready:%u", id, ipi_init_ready);
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
        PRINTF_E("[IPI]unregi err, id:%d, init_ready:%u", id, ipi_init_ready);
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
        if (len > sizeof(scp_send_obj->share_buf) || buf == NULL)
            return ERROR;

        taskENTER_CRITICAL();

        /*check if there is already an ipi pending in AP*/
        if (SCP_TO_HOST_REG & IPC_SCP2HOST_BIT) {
            /*If the following conditions meet,
             * 1)there is an ipi pending in AP
             * 2)the coming IPI is a wakeup IPI
             * so it assumes that AP is in suspend state
             * send a AP wakeup request to SPM
             * */
            /*the coming IPI will be checked if it's a wakeup source*/
            try_to_wakeup_ap(id);

            /* avoid scp log print too much
             * %==0 : switch between different IPI ID
             * %==1 : the same IPI ID comes again
             */
            if ((scp_ipi_id_record_count % IPI_PRINT_THRESHOLD == 0) ||
                    (scp_ipi_id_record_count % IPI_PRINT_THRESHOLD == 1)) {
                PRINTF_E("ipi busy,owner:%d\n", scp_ipi_owner);
            }
            taskEXIT_CRITICAL();
            return BUSY;
        }

        memcpy((void *)scp_send_obj->share_buf, buf, len);
        scp_send_obj->len = len;
        scp_send_obj->id = id;
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

/*-----------------------------------------------------------*/
/*
 * SCP awake handler
 */
void scp_awake_handler(void)
{
    GIPC_TO_SCP_REG = AWAKE_IPC_CLEAR_BIT;
    /*PRINTF_D("scp clear awake ipi\n\r");*/
}

void scp_awake_init(void)
{
    /*clear IRQ status to avoid pending interrupt*/
    GIPC_TO_SCP_REG = AWAKE_IPC_CLEAR_BIT;
    /*request irq for awake ipi*/
    request_irq(AWAKE_IPC_HANDLER_IRQn, scp_awake_handler, "IPC");
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
