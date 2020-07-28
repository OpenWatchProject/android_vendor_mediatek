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
#include <cache_internal.h>
#include <mpu_mtk.h>

#include <dvfs.h>
#include <cache_dram_management.h>
#include <tinysys_config.h>
#include <timers.h>
#include <cache_internal.h>

#define IDX_STACK_PC        (6)
#define IDX_STACK_PSR       (7)
#define NUMBERS_TO_PRINT    (5000)      /* 5000 times */
#define OSTICKS_TO_NS       (77)
#define DURATION_LIMIT      (CFG_IRQ_DURATION_LIMIT * 2)
#define KEEP_DRAM_ON_PERIOD (3) /* 3ms */

#ifdef CFG_DMGR_DEBUG
/*
 * Control DMGR's log by specifying the bits of the control
 * variable, uxLogBitmap, with the enum, logBitPos
 */
enum logBitPos {
    LOG_REGAPI,
    LOG_USER,
    LOG_ONOFF,
    LOG_LATENCY,
    LOG_TRAP,
};

static unsigned long long ullMaxTimeEnDram;
static UBaseType_t uxLogBitmap = CFG_DMGR_DEBUG;
static UBaseType_t uxTrapCounter;
char *pcPrompt = "[DMGR]";

/*
 * Use the flag to categorize the log message according to different operations.
 * Each bit in uxLogBitmap is a switch for printing message specified by the flag.
 * The uxLogBitmap is initialized by the config, CFG_DMGR_DEBUG, and the default
 * value is 9, meaning that log belonged to categories LOG_REGAPI and LOG_LATENCY
 * will be shown on the screen.
 */
#define DMGR_PRINTF(flag, fmt, ...)     \
        if(uxLogBitmap & (1 << flag))   \
            PRINTF_E("%s <%d> " fmt "\n\r", pcPrompt, flag, ##__VA_ARGS__)

void vDmgrMaxDramOnCost() {
    PRINTF_E("Maximal cost of enabling dram, %dns\n\r",
                (int) ullMaxTimeEnDram*OSTICKS_TO_NS);
}

#else
#define DMGR_PRINTF(flag, fmt, x...)
void vDmgrMaxDramOnCost() {}

#endif /* CFG_DMGR_DEBUG */

typedef enum dramMgrAccssType {
    TYPE_INVALID,
    TYPE_NORMAL,
    TYPE_EXCEPTION
} DramMgrAccssType_t;

extern unsigned long long timer_get_global_timer_tick(void);

UBaseType_t xDramOnTimerExpire;

void vDmgrEnableDram(scp_reserve_mem_id_t xId)
{
    dvfs_enable_DRAM_resource(xId);
}

void vDmgrDisableDram(scp_reserve_mem_id_t xId)
{
    dvfs_disable_DRAM_resource(xId);
}

DramMgrAccssType_t prvDramAccessType(uint32_t pulStack[])
{
    uint32_t ulCFSR = SCB->CFSR;
    uint32_t ulMMFAR = SCB->MMFAR;
    uint32_t ulPC = pulStack[IDX_STACK_PC];
    uint32_t ulPSR = pulStack[IDX_STACK_PSR];

    DMGR_PRINTF(LOG_TRAP, "CFSR=0x%08x, MMFAR=0x%08x, pc=0x%08x, psr=0x%08x, sp=0x%08x",
        (unsigned int)ulCFSR, (unsigned int)ulMMFAR, (unsigned int)ulPC,
        (unsigned int)ulPSR, pulStack);

    if((ulPSR & 0xFF) != 0) {
        return TYPE_EXCEPTION;
    }

    /* if instruction accessed located in the range of cachable memory */
    if ((ulCFSR & (1 << 0)) != 0) {
        if (L1C_DRAM_ADDR <= ulPC &&
            ulPC < L1C_DRAM_ADDR + L1C_DRAM_SIZE) {
            return TYPE_NORMAL;
        }
    }

    /* if data accessed located in the range of cachable memory */
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

    switch(prvDramAccessType(pulStack)) {
        case TYPE_NORMAL:
        case TYPE_EXCEPTION:
            DMGR_PRINTF(LOG_ONOFF, "turn dram on");

#ifdef CFG_DMGR_DEBUG
            BaseType_t xTimeOn, xTimeOff, xTimeDiff;

            xTimeOn = timer_get_global_timer_tick();
            dvfs_enable_DRAM_resource_from_isr(DRAM_MANAGER_MEM_ID);
            xTimeOff = timer_get_global_timer_tick();
            xTimeDiff = xTimeOff - xTimeOn;
            configASSERT(xTimeDiff < DURATION_LIMIT);

            ullMaxTimeEnDram =   (xTimeDiff > ullMaxTimeEnDram)
                                ? xTimeDiff
                                : ullMaxTimeEnDram;
            uxTrapCounter++;
            if ((uxTrapCounter % NUMBERS_TO_PRINT) == 0) {
                DMGR_PRINTF(LOG_LATENCY, "max dram on latency = %dns",
                            (int) (ullMaxTimeEnDram*OSTICKS_TO_NS));
            }
#else
            dvfs_enable_DRAM_resource_from_isr(DRAM_MANAGER_MEM_ID);
#endif
            xDramOnTimerExpire = pdTRUE;

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
        /* There are no any dram access in the previous idle time */
        if(xDramOnTimerExpire == pdTRUE) {
            DMGR_PRINTF(LOG_ONOFF, "turn dram off");
            dvfs_disable_DRAM_resource(DRAM_MANAGER_MEM_ID);
            xDramOnTimerExpire = pdFALSE;
        }
    }
    taskEXIT_CRITICAL();
}

BaseType_t xDramManagerInit(void)
{
#ifdef CFG_DMGR_DEBUG
    ullMaxTimeEnDram = 0;
    uxTrapCounter = 0;
#endif
    xDramOnTimerExpire = pdFALSE;
    disable_mpu_region(MPU_ID_DRAM_0);
    disable_mpu_region(MPU_ID_DRAM_1);

    return pdPASS;
}

