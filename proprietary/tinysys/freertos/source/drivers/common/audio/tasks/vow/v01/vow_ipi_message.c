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

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        Header Files
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <mt_reg_base.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <scp_ipi.h>
#include "audio_task_vow.h"
#include "vow_ipi_message.h"

QueueHandle_t xIpiSendQueue;

struct ipiCmd {
    unsigned int ipi_msgid;
    unsigned int ipi_ret;
    unsigned int para1;
    unsigned int para2;
    unsigned int para3;
    unsigned int para4;
};

static void vow_ipi_task(void *pvParameters);

/*
============================================================================================================
------------------------------------------------------------------------------------------------------------
||                        IPI Hander
------------------------------------------------------------------------------------------------------------
============================================================================================================
*/
void vow_ipi_init(void)
{
    scp_ipi_registration(IPI_VOW, vow_ipi_handler, "vow");
    scp_ipi_wakeup_ap_registration(IPI_VOW);
    VOW_IPI_LOG("register vow ipi\n\r");

    xIpiSendQueue = xQueueCreate( 15, sizeof(struct ipiCmd));
    if (xIpiSendQueue == NULL) {
        // The semaphore was created failly.
        // The semaphore can not be used.
        VOW_IPI_ERR("Ipi_S Fail\n\r");
    }
    kal_xTaskCreate(vow_ipi_task, "ipi_send_task", 250, (void *)0, 3, NULL);
}

/***********************************************************************************
** vow_ipi_sendmsg - send message to AP side
************************************************************************************/
int vow_ipi_sendmsg(vow_ipi_msgid_t id, void *buf, vow_ipi_result result, unsigned int size, unsigned int wait)
{
    ipi_status status;
    short ipibuf[24];

    ipibuf[0] = id;
    ipibuf[1] = size;
    ipibuf[2] = result;

    if (size != 0) {
        memcpy((void *)&ipibuf[3], buf, size);
    }
    status = scp_ipi_send(IPI_VOW, (void*)ipibuf, size + 6, wait, IPI_SCP2AP);
    // VOW_IPI_LOG("IPI Send:%x %x %x %x\n\r", status, id, size, result);
    if (status == BUSY) {
        // VOW_IPI_LOG("IPI Send Fail:%x %x %x %x\n\r", status, id, size, result);
        return false;
    } else if (status == ERROR) {
        VOW_IPI_ERR("IPI ERROR NEED CHECK\n\r");
        return -1;
    }

    return true;
}

/***********************************************************************************
** vow_ipi_handler - message handler from AP side
************************************************************************************/

void vow_ipi_handler(int id, void * data, unsigned int len)
{
    short *vowmsg;
    // short msglen;
    struct ipiCmd ipiCmdInfo;
    static BaseType_t xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;
    vowmsg = (short*)data;
    // msglen = vowmsg[1];
    // (void)msglen;
    switch (*vowmsg) {
        case AP_IPIMSG_VOW_ENABLE:
            // VOW_LOG("AP_IPIMSG_VOW_ENABLE\n\r");
            ipiCmdInfo.ipi_msgid = SCP_IPIMSG_VOW_ENABLE_ACK;
            ipiCmdInfo.ipi_ret   = 0;
            ipiCmdInfo.para1     = 0;
            ipiCmdInfo.para2     = 0;
            ipiCmdInfo.para3     = 0;
            ipiCmdInfo.para4     = 0;
            xQueueSendToBackFromISR(xIpiSendQueue, &ipiCmdInfo, &xHigherPriorityTaskWoken);
            break;
        case AP_IPIMSG_VOW_DISABLE:
            // VOW_LOG("AP_IPIMSG_VOW_DISABLE\n\r");
            ipiCmdInfo.ipi_msgid = SCP_IPIMSG_VOW_DISABLE_ACK;
            ipiCmdInfo.ipi_ret   = 0;
            ipiCmdInfo.para1     = 0;
            ipiCmdInfo.para2     = 0;
            ipiCmdInfo.para3     = 0;
            ipiCmdInfo.para4     = 0;
            xQueueSendToBackFromISR(xIpiSendQueue, &ipiCmdInfo, &xHigherPriorityTaskWoken);
            break;
        case AP_IPIMSG_VOW_SETMODE:
            // VOW_LOG("AP_IPIMSG_VOW_SETMODE\n\r");
            ipiCmdInfo.ipi_msgid = SCP_IPIMSG_VOW_SETMODE_ACK;
            ipiCmdInfo.ipi_ret   = 0;
            ipiCmdInfo.para1     = vowmsg[2];
            ipiCmdInfo.para2     = 0;
            ipiCmdInfo.para3     = 0;
            ipiCmdInfo.para4     = 0;
            xQueueSendToBackFromISR(xIpiSendQueue, &ipiCmdInfo, &xHigherPriorityTaskWoken);
            break;
        case AP_IPIMSG_VOW_APREGDATA_ADDR: {
            int *msg;
            // VOW_LOG("AP_IPIMSG_VOW_APREGDATA_ADDR\n\r");
            msg = (int*)vowmsg;
            msg++;
            ipiCmdInfo.ipi_msgid = SCP_IPIMSG_VOW_APREGDATA_ADDR_ACK;
            ipiCmdInfo.ipi_ret   = 0;
            ipiCmdInfo.para1     = *msg++;
            ipiCmdInfo.para2     = 0;
            ipiCmdInfo.para3     = 0;
            ipiCmdInfo.para4     = 0;
            xQueueSendToBackFromISR(xIpiSendQueue, &ipiCmdInfo, &xHigherPriorityTaskWoken);
            break;
        }
        case AP_IPIMSG_SET_VOW_MODEL: {
            int *msg;
            // VOW_LOG("AP_IPIMSG_SET_VOW_MODEL\n\r");
            msg  = (int*)vowmsg;
            msg++;
            ipiCmdInfo.ipi_msgid = SCP_IPIMSG_SET_VOW_MODEL_ACK;
            ipiCmdInfo.ipi_ret   = 0;
            ipiCmdInfo.para1     = *msg++;
            ipiCmdInfo.para2     = *msg++;
            ipiCmdInfo.para3     = *msg++;
            ipiCmdInfo.para4     = *msg++;
            xQueueSendToBackFromISR(xIpiSendQueue, &ipiCmdInfo, &xHigherPriorityTaskWoken);
            break;
        }
        case AP_IPIMSG_VOW_SET_FLAG: {
            int *msg;
            // VOW_LOG("AP_IPIMSG_VOW_SET_FLAG\n\r");
            msg = (int*)vowmsg;
            msg++;
            ipiCmdInfo.ipi_msgid = SCP_IPIMSG_SET_FLAG_ACK;
            ipiCmdInfo.ipi_ret   = 0;
            ipiCmdInfo.para1     = *msg++;
            ipiCmdInfo.para2     = *msg++;
            ipiCmdInfo.para3     = 0;
            ipiCmdInfo.para4     = 0;
            xQueueSendToBackFromISR(xIpiSendQueue, &ipiCmdInfo, &xHigherPriorityTaskWoken);
            break;
        }
        case AP_IPIMSG_VOW_DATAREADY_ACK:
            vow_sync_data_ready();
            break;
        case AP_IPIMSG_VOW_SET_SMART_DEVICE: {
            int *msg;
            // VOW_LOG("AP_IPIMSG_VOW_SET_SMART_DEVICE\n\r");
            msg = (int*)vowmsg;
            msg++;
            ipiCmdInfo.ipi_msgid = SCP_IPIMSG_VOW_SET_SMART_DEVICE_ACK;
            ipiCmdInfo.ipi_ret   = 0;
            ipiCmdInfo.para1     = *msg++;
            ipiCmdInfo.para2     = *msg++;
            ipiCmdInfo.para3     = 0;
            ipiCmdInfo.para4     = 0;
            xQueueSendToBackFromISR(xIpiSendQueue, &ipiCmdInfo, &xHigherPriorityTaskWoken);
            break;
        }
        default:
            break;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/***********************************************************************************
** vow_ipi_task - Process VOW IPI send command
************************************************************************************/
static void vow_ipi_task(void *pvParameters)
{
    struct ipiCmd ipiCmdInfo;
    while (1) {
        xQueueReceive(xIpiSendQueue, &ipiCmdInfo, portMAX_DELAY);

        switch (ipiCmdInfo.ipi_msgid) {
            case SCP_IPIMSG_VOW_ENABLE_ACK:
                vow_enable();
                VOW_IPI_DBG("SET_EN_ACK\n\r");
                vow_ipi_sendmsg(ipiCmdInfo.ipi_msgid, &(ipiCmdInfo.ipi_ret), VOW_IPI_SUCCESS, 0, 1);
                break;
            case SCP_IPIMSG_VOW_DISABLE_ACK:
                vow_disable();
                VOW_IPI_DBG("SET_DIS_ACK\n\r");
                vow_ipi_sendmsg(ipiCmdInfo.ipi_msgid, &(ipiCmdInfo.ipi_ret), VOW_IPI_SUCCESS, 0, 1);
                break;
            case SCP_IPIMSG_VOW_SETMODE_ACK:
                vow_setmode((vow_mode_t)ipiCmdInfo.para1);
                VOW_IPI_LOG("SET_MODE_ACK\n\r");
                vow_ipi_sendmsg(ipiCmdInfo.ipi_msgid, &(ipiCmdInfo.ipi_ret), VOW_IPI_SUCCESS, 0, 1);
                break;
            case SCP_IPIMSG_VOW_APREGDATA_ADDR_ACK:
                vow_setapreg_addr(ipiCmdInfo.para1);
                ipiCmdInfo.ipi_ret = vow_gettcmreg_addr();
                VOW_IPI_DBG("SET_AP_ADR_ACK=0x%x\n\r", ipiCmdInfo.ipi_ret);
                vow_ipi_sendmsg(ipiCmdInfo.ipi_msgid, &(ipiCmdInfo.ipi_ret), VOW_IPI_SUCCESS, 4, 1);
                break;
            case SCP_IPIMSG_SET_VOW_MODEL_ACK:
                vow_setModel((vow_event_info_t)ipiCmdInfo.para1, ipiCmdInfo.para2, ipiCmdInfo.para3, ipiCmdInfo.para4);
                vow_ipi_sendmsg(ipiCmdInfo.ipi_msgid, &(ipiCmdInfo.ipi_ret), VOW_IPI_SUCCESS, 0, 1);
                VOW_IPI_DBG("SET_MODEL_ACK\n\r");
                break;
            case SCP_IPIMSG_SET_FLAG_ACK:
                vow_set_flag((vow_flag_t)ipiCmdInfo.para1, (short)ipiCmdInfo.para2);
                VOW_IPI_DBG("SET_FG_ACK\n\r");
                vow_ipi_sendmsg(ipiCmdInfo.ipi_msgid, &(ipiCmdInfo.ipi_ret), VOW_IPI_SUCCESS, 0, 1);
                break;
            case SCP_IPIMSG_VOW_SET_SMART_DEVICE_ACK:
                VOW_IPI_DBG("SET_SM_ACK\n\r");
                vow_setSmartDevice((bool)ipiCmdInfo.para1, (unsigned int)ipiCmdInfo.para2);
                vow_ipi_sendmsg(ipiCmdInfo.ipi_msgid, &(ipiCmdInfo.ipi_ret), VOW_IPI_SUCCESS, 0, 1);
                break;
            default:
                break;
        }
    }
}
