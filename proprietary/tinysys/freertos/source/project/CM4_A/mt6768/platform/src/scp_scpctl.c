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

#include <FreeRTOS.h>
#include <task.h>
#include <scp_ipi.h>
#include <scp_scpctl.h>
#include <main.h>

enum SCPCTL_STAT_E {
    SCPCTL_STAT_INACTIVE = 0,
    SCPCTL_STAT_ACTIVE,
};

struct scpctl_ctl_s {
    enum SCPCTL_OP_E     op;
    enum SCPCTL_STAT_E   stat;
};


extern void vTaskMonitor(void *pvParameters);

static struct scpctl_ctl_s scpctl;
static char *prompt = "[SCPCTL]";
static TaskHandle_t xMonitorTask = NULL;

/****************************************************************************
 * The ISR is to receive the IPI commands sent by ADB and then modify
 * the operation, op, of the control state.
 ****************************************************************************/
static void scpctl_handler(int id, void *data, unsigned int len)
{
    struct scpctl_cmd_s *cmd = (struct scpctl_cmd_s *)data;
    enum SCPCTL_TYPE_E type = cmd->type;
    enum SCPCTL_OP_E op = cmd->op;

    switch (type) {
        case SCPCTL_TYPE_TMON:
            scpctl.op = (op == SCPCTL_OP_ACTIVE)
                            ? SCPCTL_OP_ACTIVE
                            : SCPCTL_OP_INACTIVE;
            PRINTF_E("%s: type/op=%d/%d,stat=%d\n", prompt, type, op, scpctl.stat);
            break;
        default:
            PRINTF_E("%s: unknown cmd, %d, %d\n", prompt, type, op);
            break;
    }
}

/****************************************************************************
 * The function is called by in vApplicationIdleHook and determine whether
 * to resume or to suspend monitor task depending on the command and the
 * current status.
 *
 * op = active, state = inactive --> resume monitor task.
 * op = inactive, state = active --> suspend monitor task.
 * others, keep the same status.
 ****************************************************************************/
void scpctl_idlehook(void)
{
    int op = scpctl.op;

    if (op == SCPCTL_OP_ACTIVE && scpctl.stat == SCPCTL_STAT_INACTIVE) {
        vTaskResume(xMonitorTask);
        scpctl.stat = SCPCTL_STAT_ACTIVE;
    }
    else if (op == SCPCTL_OP_INACTIVE && scpctl.stat == SCPCTL_STAT_ACTIVE) {
        vTaskSuspend(xMonitorTask);
        scpctl.stat = SCPCTL_STAT_INACTIVE;
    }
}

void scpctl_init(void)
{
    struct scp_region_info_st* region_info_p = INFO_BASE_POINTER;

    xTaskCreate(vTaskMonitor, "TMon", 384, (void*)4, PRI_MONITOR, &xMonitorTask);
    configASSERT(xMonitorTask != NULL);

    if (region_info_p->scpctl & (1 << SCPCTL_TYPE_TMON)) {
        scpctl.stat = SCPCTL_STAT_ACTIVE;
        scpctl.op = SCPCTL_OP_ACTIVE;
    }
    else {  /* monitor task is in suspened state */
        scpctl.stat = SCPCTL_STAT_INACTIVE;
        scpctl.op = SCPCTL_OP_INACTIVE;
        vTaskSuspend(xMonitorTask);
    }

    scp_ipi_registration(IPI_SCPCTL, scpctl_handler, "scpctl");
}

