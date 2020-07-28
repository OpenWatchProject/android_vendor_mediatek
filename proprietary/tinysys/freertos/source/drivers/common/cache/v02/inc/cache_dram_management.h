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
 */

#ifndef __CACHE_DRAM_MANAGEMENT_H__
#define __CACHE_DRAM_MANAGEMENT_H__

#include <stdint.h>
#include <dma.h>

#define IDX_STACK_PC            (6)
#define IDX_STACK_PSR           (7)

#define MAX_DMGR_QUEUE_EVENT    (10)
#define MAX_DMGR_LIST_ITEM      MAX_DMGR_QUEUE_EVENT

#define OSTICKS_TO_NS           (77)
#define DURATION_LIMIT          (CFG_IRQ_DURATION_LIMIT * 2)

#ifdef CFG_DMGR_DEBUG
/*
 * Use the flag to categorize the log message according to different operations.
 * Each bit in uxLogBitmap is a switch for printing message specified by the flag.
 * The uxLogBitmap is initialized by the config, CFG_DMGR_DEBUG, and the default
 * value is 9, meaning that log belonged to categories LOG_REGAPI and LOG_LATENCY
 * will be shown on the screen.
 */
#define DMGR_PRINTF(flag, fmt, ...)                                             \
        do {                                                                    \
            if(uxLogBitmap & (1 << flag))                                       \
                PRINTF_E("%s <%d> " fmt "\n", pcPrompt, flag, ##__VA_ARGS__); \
        } while(0)
#else
#define DMGR_PRINTF(flag, fmt, ...)
#endif
#define DMGR_ERROR(fmt, ...)                                        \
        PRINTF_E("%s Err, " fmt "\n", pcPrompt, ##__VA_ARGS__);     \
        configASSERT(0);
#define DMGR_WARN(fmt, ...)                                         \
        PRINTF_E("%s WARN, " fmt "\n", pcPrompt, ##__VA_ARGS__);


enum dramMgrState {
    STAT_DRAM_PW_OFF,
    STAT_DRAM_WARM_UP,
    STAT_DRAM_PW_ON,
};

enum dramMgrAction {
    ACT_DRAM_DETECT_ACCESS,
    ACT_DRAM_POWER_ON,
    ACT_DRAM_POWER_OFF,

    ACT_EXCEPT_POWER_ON,
};

typedef enum dramMgrAccssType {
    TYPE_INVALID,
    TYPE_NORMAL,
    TYPE_EXCEPTION,
    TYPE_CRITICAL
} DramMgrAccssType_t;

struct dramMgrQueueEvent {
    enum dramMgrAction eAction;
    TaskHandle_t xHandle;
};

#ifdef CFG_DMGR_DEBUG
/*
 * Control DMGR's log by specifying the bits of the control
 * variable, uxLogBitmap, with the enum, logBitPos
 */
enum logBitPos {
    LOG_TASKTRACE,
    LOG_ONOFF,
    LOG_LATENCY,
    LOG_TRAP,
    LOG_STATE,
};
#endif

extern char *pcPrompt;

BaseType_t xTrySwitchOnDramPower(uint32_t ulStack[]);
BaseType_t xDramManagerInit(void);
void vDramManagerIdleHook(void);

#endif
