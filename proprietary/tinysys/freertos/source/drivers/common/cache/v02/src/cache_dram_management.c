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
#include <platform_mtk.h>
#include <mpu_mtk.h>
#include <tinysys_config.h>
#include <timers.h>
#include <cache_internal.h>
#ifdef CFG_XGPT_SUPPORT
#include <interrupt.h>
#include <mt_gpt.h>
#endif
#include <FreeRTOSConfig.h>
#include <cache_dram_management.h>
#include <dram_management_api.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#else
void dvfs_enable_DRAM_resource_from_isr(scp_reserve_mem_id_t dma_id)
{
    vDmgrDVFSHookDramOn();
}
void dvfs_disable_DRAM_resource(scp_reserve_mem_id_t dma_id)
{
    vDmgrDVFSHookDramOff();
}
int32_t get_dram_ack_from_isr_for_dmgr(void)
{
    vDmgrDVFSHookDramOn();
    return 1;
}
void enable_dram_resource_for_dmgr(scp_reserve_mem_id_t dma_id) {};
#endif

extern void *pxCurrentTCB;
extern UBaseType_t uxCriticalNesting;

enum dramMgrState eExceptState;
char *pcPrompt = "[DMGR]";

#ifdef CFG_DYNAMIC_DRAM_V2
enum dramMgrState eDramMgrState;
static QueueHandle_t xDramMgrRxQueue;
static List_t xDramMgrSuspendList;
static List_t xDramMgrFreeList;
static ListItem_t dramMgrItemPool[MAX_DMGR_LIST_ITEM];
#endif

#ifdef CFG_DMGR_DEBUG
static UBaseType_t uxLogBitmap = CFG_DMGR_DEBUG;
#endif /* CFG_DMGR_DEBUG */

/*****************************************************************************
 * Porting functions
 ****************************************************************************/

__STATIC_INLINE void prvDmgrEnRegionPort(void)
{
#if L1C_DRAM_SIZE_0
    enable_mpu_region(MPU_ID_DRAM_0);
#endif
#if L1C_DRAM_SIZE_1
    enable_mpu_region(MPU_ID_DRAM_1);
#endif
}

__STATIC_INLINE void prvDmgrDisRegionPort(void)
{
#if L1C_DRAM_SIZE_0
    disable_mpu_region(MPU_ID_DRAM_0);
#endif
#if L1C_DRAM_SIZE_1
    disable_mpu_region(MPU_ID_DRAM_1);
#endif
}

/*****************************************************************************
 ****************************************************************************/


#ifdef CFG_DYNAMIC_DRAM_V2
static void prvDmgrHwTimerStart(void)
{
    struct timer_device *pxDev = id_to_dev(DMGR_TIMER);
    unsigned int ulBaseAddr = pxDev->base_addr;

    DRV_WriteReg32(ulBaseAddr + TIMER_RST_VAL, DMGR_TIMER_RSTVAL);
    DRV_SetReg32(ulBaseAddr + TIMER_IRQ_CTRL_REG, TIMER_ENABLE);
    /* select clock source to clk_32k and enable timer */
    DRV_SetReg32(ulBaseAddr + TIMER_EN,
                 (TIMER_CLK_SRC_CLK_32K << TIMER_CLK_SRC_SHIFT) | TIMER_IRQ_ENABLE);
}

static void prvDmgrHwTimerStop(void)
{
    struct timer_device *pxDev = id_to_dev(DMGR_TIMER);
    unsigned int ulBaseAddr = pxDev->base_addr;

    DRV_ClrReg32(ulBaseAddr + TIMER_EN, TIMER_ENABLE);
    DRV_ClrReg32(ulBaseAddr + TIMER_IRQ_CTRL_REG, TIMER_IRQ_ENABLE);
    /* act timer interrupt */
    DRV_SetReg32(ulBaseAddr + TIMER_IRQ_CTRL_REG, TIMER_IRQ_CLEAR);
    DRV_WriteReg32(ulBaseAddr + TIMER_RST_VAL, 0);
}

static void prvDmgrEventSend(TaskHandle_t xHandle, enum dramMgrAction eAction)
{
    struct dramMgrQueueEvent xEvent;
    BaseType_t xHigherPriorityTaskWoken;
    BaseType_t xRet = pdPASS;

    xEvent.xHandle = (TaskHandle_t) xHandle;
    xEvent.eAction = eAction;
    /* send the current tcb to DramMgr to be suspended */
    xRet = xQueueSendFromISR(xDramMgrRxQueue, &xEvent, &xHigherPriorityTaskWoken);
    configASSERT(xRet == pdTRUE);
    /* context switching to dram manager */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void prvDmgrPollingAck(void)
{
    uint32_t ulIsAck = get_dram_ack_from_isr_for_dmgr();

    configASSERT(eDramMgrState == STAT_DRAM_WARM_UP);

    if (ulIsAck) {
        DMGR_PRINTF(LOG_ONOFF, "p:dram on");
        prvDmgrHwTimerStop();
        prvDmgrEventSend((TaskHandle_t)NULL, ACT_DRAM_POWER_ON);
    }
}
#endif

DramMgrAccssType_t prvDramAccessType(uint32_t pulStack[])
{
    uint32_t ulCFSR = SCB->CFSR;
    uint32_t ulMMFAR = SCB->MMFAR;
    uint32_t ulPC = pulStack[IDX_STACK_PC];
    uint32_t ulPSR = pulStack[IDX_STACK_PSR];

    DMGR_PRINTF(LOG_TRAP, "CFSR=0x%08x, MMFAR=0x%08x, pc=0x%08x, psr=0x%08x, sp=0x%08x",
                (unsigned int)ulCFSR, (unsigned int)ulMMFAR, (unsigned int)ulPC,
                (unsigned int)ulPSR, (unsigned int)pulStack);

    /* if ISR execution locates in the range of cacheable memory */
    if ((ulPSR & 0xFF) != 0) {
        DMGR_WARN("Access DRAM in ISR, PC=0x%x, MMFAR=0x%08x!!!",
                  (unsigned int)ulPC, (unsigned int)ulMMFAR);
        return TYPE_EXCEPTION;
    }

    /* if dram access is in critical secton */
    if (uxCriticalNesting > 0) {
        DMGR_WARN("Access DRAM in CRITICAL SECTION, PC=0x%x, MMFAR=0x%08x!!!",
                  (unsigned int)ulPC, (unsigned int)ulMMFAR);
        return TYPE_CRITICAL;
    }

    /* if instruction accessed locates in the range of cacheable memory */
    if ((ulCFSR & (1 << 0)) != 0) {
        if (L1C_DRAM_ADDR <= ulPC &&
                ulPC < L1C_DRAM_ADDR + L1C_DRAM_SIZE) {
            return TYPE_NORMAL;
        }
    }

    /* if data accessed locates in the range of cacheable memory */
    if ((ulCFSR & (1 << 1)) != 0) {
        if (((ulCFSR & (1 << 7)) != 0) &&
                L1C_DRAM_ADDR <= ulMMFAR &&
                ulMMFAR < L1C_DRAM_ADDR + L1C_DRAM_SIZE) {
            return TYPE_NORMAL;
        }
    }

    return TYPE_INVALID;
}

BaseType_t xTrySwitchOnDramPower(uint32_t pulStack[])
{
    BaseType_t xRet = pdPASS;

    switch (prvDramAccessType(pulStack)) {
        case TYPE_NORMAL:
#ifdef CFG_DYNAMIC_DRAM_V2
            /* the access is located in cacheable memory */
            DMGR_PRINTF(LOG_ONOFF, "p:notify dmgr");
            prvDmgrEventSend((TaskHandle_t) pxCurrentTCB, ACT_DRAM_DETECT_ACCESS);
            break;
#endif
        case TYPE_EXCEPTION:
        case TYPE_CRITICAL:
#ifdef CFG_L1C_ISR_SUPPORT
            DMGR_PRINTF(LOG_ONOFF, "e:dram on");
            dvfs_enable_DRAM_resource_from_isr(DRAM_MANAGER_MEM_ID);
            eExceptState = STAT_DRAM_PW_ON;
#else
            /* Don't allow access DRAM in Critical Section and Interrupt */
            configASSERT(0);
#endif
            break;
        default:
            xRet = pdFALSE;
    }

    return xRet;
}

void vDramManagerIdleHook(void)
{
    taskENTER_CRITICAL();
    {
#ifdef CFG_DYNAMIC_DRAM_V2
        if (eDramMgrState == STAT_DRAM_PW_ON) {
            dvfs_disable_DRAM_resource(DRAM_MANAGER_MEM_ID);
            eDramMgrState = STAT_DRAM_PW_OFF;
            DMGR_PRINTF(LOG_ONOFF, "p:dram off");
        }
#endif

        if (eExceptState == STAT_DRAM_PW_ON) {
            dvfs_disable_DRAM_resource(DRAM_MANAGER_MEM_ID);
            eExceptState = STAT_DRAM_PW_OFF;
            DMGR_PRINTF(LOG_ONOFF, "e:dram off");
        }
    }
    taskEXIT_CRITICAL();
}

#ifdef CFG_DYNAMIC_DRAM_V2
static void prvSuspendTask(TaskHandle_t xHandle)
{
    ListItem_t *pxDramMgrItem = NULL;

    configASSERT(listLIST_IS_EMPTY(&xDramMgrFreeList) != pdTRUE);

    /* get a empty item from free list */
    pxDramMgrItem = listGET_HEAD_ENTRY(&xDramMgrFreeList);
    uxListRemove(pxDramMgrItem);

    /* initialize the item and then insert it into suspend list */
    vListInitialiseItem(pxDramMgrItem);
    listSET_LIST_ITEM_VALUE(pxDramMgrItem, (TickType_t)xHandle);
    vListInsertEnd(&xDramMgrSuspendList, pxDramMgrItem);

    /* as we have recorded the task in suspend list, it can be suspended */
    vTaskSuspend(xHandle);
    DMGR_PRINTF(LOG_TASKTRACE, "Suspend %s", pcTaskGetTaskName(xHandle));
}

static void prvResumeTask(TaskHandle_t xHandle)
{
    ListItem_t *pxDramMgrItem = NULL;

    configASSERT(listLIST_IS_EMPTY(&xDramMgrSuspendList) != pdTRUE);

    /* get an item from suspend list */
    pxDramMgrItem = listGET_HEAD_ENTRY(&xDramMgrSuspendList);
    xHandle = (TaskHandle_t) listGET_LIST_ITEM_VALUE(pxDramMgrItem);
    uxListRemove(pxDramMgrItem);

    vTaskResume(xHandle);
    DMGR_PRINTF(LOG_TASKTRACE, "Resume %s", pcTaskGetTaskName(xHandle));

    /* insert the empty item into free list */
    vListInitialiseItem(pxDramMgrItem);
    vListInsertEnd(&xDramMgrFreeList, pxDramMgrItem);
}

static BaseType_t xGetDramAck(void)
{
    int32_t ret;
    kal_taskENTER_CRITICAL();
    ret = get_dram_ack_from_isr_for_dmgr();
    kal_taskEXIT_CRITICAL();

    return (ret == 1) ? pdTRUE : pdFALSE;
}

static void prvTaskDramManager(void *pvParameters)
{
    struct dramMgrQueueEvent xEvent;
    enum dramMgrAction eAction;
    TaskHandle_t xHandle;
    BaseType_t xRet;

    while (1) {
        xRet = xQueueReceive(xDramMgrRxQueue, &xEvent, portMAX_DELAY);
        if (xRet != pdPASS) {
            PRINTF_E("[DMGR] xQueueReceive failed, %ld\n\r", xRet);
            continue;
        }

        xHandle = xEvent.xHandle;
        eAction = xEvent.eAction;

        /*
         * Normal FSM for DRAM Manager
         */
        DMGR_PRINTF(LOG_STATE, "State=%d, Action=%d", eDramMgrState, eAction);

        switch (eDramMgrState) {
            case STAT_DRAM_PW_OFF:
                switch (eAction) {
                    case ACT_DRAM_DETECT_ACCESS:
                        enable_dram_resource_for_dmgr(DRAM_MANAGER_MEM_ID);
                        /* if dram have been on, directly, transit state to
                         * ACT_DRAM_POWER_ON and save the time of 1m polling
                         * action or go into state, STAT_DRAM_WARM_UP, for
                         * DRAM ready.
                         *
                         * Note: we can do this in this stage because
                         * 1. there are no any items in receive queue.
                         * 2. there are no any tasks in suspend list.
                         */
                        if (xGetDramAck() == pdTRUE) {
                            eDramMgrState = STAT_DRAM_PW_ON;
                            DMGR_PRINTF(LOG_ONOFF, "p2:dram on");
                        } else {
                            prvSuspendTask(xHandle);
                            eDramMgrState = STAT_DRAM_WARM_UP;
                            prvDmgrHwTimerStart();
                        }
                        break;
                    default:
                        configASSERT(0);
                }

                break;
            case STAT_DRAM_WARM_UP:
                switch (eAction) {
                    case ACT_DRAM_DETECT_ACCESS:
                        prvSuspendTask(xHandle);
                        break;
                    case ACT_DRAM_POWER_ON:
                        /* Supposed there are no events following POWER_ON events,
                         * so task resuming is only done here.
                         */
                        while (listLIST_IS_EMPTY(&xDramMgrSuspendList) != pdTRUE) {
                            prvResumeTask(xHandle);
                        }
                        eDramMgrState = STAT_DRAM_PW_ON;
                        break;
                    default:
                        configASSERT(0);
                }

                break;
            case STAT_DRAM_PW_ON:
                configASSERT(0);

            default:
                ;
        }
    }
}
#endif

void vDmgrDVFSHookDramOn(void)
{
    prvDmgrEnRegionPort();
    hal_cache_scp_sleep_protect_off();
    hal_cache_invalidate_all_cache_lines_from_isr(CACHE_ICACHE);
}

void vDmgrDVFSHookDramOff(void)
{
    prvDmgrDisRegionPort();
    hal_cache_scp_sleep_protect_on();
}

BaseType_t xDramManagerInit(void)
{
    BaseType_t xRet = pdPASS;

#ifdef CFG_DYNAMIC_DRAM_V2
    uint32_t i;
    struct timer_device *pxDev;

    xDramMgrRxQueue = xQueueCreate(MAX_DMGR_QUEUE_EVENT, sizeof(struct dramMgrQueueEvent));
    vListInitialise(&xDramMgrSuspendList);
    vListInitialise(&xDramMgrFreeList);
    /* insert all list items into free list */

    for (i = 0; i < MAX_DMGR_LIST_ITEM; i++) {
        vListInitialiseItem(&dramMgrItemPool[i]);
        vListInsertEnd(&xDramMgrFreeList, &dramMgrItemPool[i]);
    }

    xRet = kal_xTaskCreate(prvTaskDramManager, "DMgr", 128, NULL, PRI_DMGR, NULL);

    /* timer for polling the dram ack state by every 1ms */
    pxDev = id_to_dev(DMGR_TIMER);
    request_irq(pxDev->irq_id, prvDmgrPollingAck, "DMGR_Timer");
    prvDmgrHwTimerStop();
    eDramMgrState = STAT_DRAM_PW_OFF;
#endif

    eExceptState = STAT_DRAM_PW_OFF;
    prvDmgrDisRegionPort();

    return xRet;
}

