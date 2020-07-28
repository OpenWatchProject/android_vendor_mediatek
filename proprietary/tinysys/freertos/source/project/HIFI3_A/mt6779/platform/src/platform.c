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
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "timers.h"
#include <stdio.h>
#include <string.h>
#include "interrupt.h"
#include "driver_api.h"
#include "mt_reg_base.h"
#include "main.h"

#ifdef CFG_DEBUG_COMMAND_SUPPORT
#include "adsp_dbg.h"
#endif

#ifdef CFG_AUDIO_SUPPORT
#include <audio.h>
#endif
#ifdef CFG_WDT_SUPPORT
#include <wdt.h>
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif

#ifdef CFG_TASK_MONITOR
#define mainCHECK_DELAY                      ( ( portTickType) 40000 / portTICK_RATE_MS )
#define TASK_MONITOR_DEFAULT_ENABLE
#if configUSE_TICKLESS_IDLE == 1
extern void print_tickless_status (void);
#endif
TimerHandle_t xTaskMonTimer;
void vTaskMonitorTimerCallback(TimerHandle_t xTimer)
{
    unsigned int ix;
    char *list_buffer;

    ix = (unsigned int)pvTimerGetTimerID(xTimer);
    list_buffer = kal_pvPortMalloc(512);

    vTaskList(list_buffer);
    PRINTF_D("[%d]Heap:free/total size:%d/%d\n", ix, xPortGetFreeHeapSize(),
             configTOTAL_HEAP_SIZE);
    PRINTF_D("Task Status:\n\r%s", list_buffer);
#if configUSE_TICKLESS_IDLE == 1
    print_tickless_status();
#endif
    vTimerSetTimerID(xTimer, (void *)++ix);
    kal_vPortFree(list_buffer);
}

static void vInitTaskMonitor(void)
{
    xTaskMonTimer = xTimerCreate("TaskMonitor", mainCHECK_DELAY, pdTRUE,
                                 (void *)0, vTaskMonitorTimerCallback);

    if (xTaskMonTimer == NULL) {
        PRINTF_W("%s(), xTaskMonTimer not Create\n", __func__);
        return;
    }
#ifdef TASK_MONITOR_DEFAULT_ENABLE
    if (xTimerStart(xTaskMonTimer, 0) != pdPASS)
        PRINTF_W("%s(), xTaskMonTimer not Start\n", __func__);

#endif
    return;
}
#endif

static void level4_interrupt_dummy_handler(void)
{
    /* Dummy handler to register irq */
    /* After adsp resume, the driver will base on registered irq to enable & unmask interrupts */
}

static void level4_interrupt_init (void)
{
    request_irq(INFRA_HANG_IRQn, level4_interrupt_dummy_handler, "INF");
    request_irq(WDT_IRQn, level4_interrupt_dummy_handler, "SYS");
    memset((void *)ADSP_INFRA_BUS_DUMP_BASE, 0, NUM_OF_INFRA_BUS_HANG_REGISTERS * sizeof(uint32_t));
}

void platform_init(void)
{
#ifdef CFG_TASK_MONITOR
    vInitTaskMonitor();
#endif

#ifdef CFG_DEBUG_COMMAND_SUPPORT
    dbg_cmds_init();
#endif

#ifdef CFG_AUDIO_SUPPORT
    audio_init();
#endif

    level4_interrupt_init();

    irq_status_dump();
    return;
}

