/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2018
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bus_monitor.h"
#include "mt_reg_base.h"
#include "driver_api.h"
#include "hifi3_irq.h"
#include "interrupt.h"
#include "mt_printf.h"
#include "main.h"
#ifdef CFG_WDT_SUPPORT
#include <wdt.h>
#endif

#define BUS_MON_STAGE1_TIMEOUT_VALUE    (0x01DA0A2B)    /* 1.00 sec @ 466MHz, 0.67 sec @ 700MHz, 0.59 sec @ 788MHz (unit: 15 cycle count) */
#define BUS_MON_STAGE2_TIMEOUT_VALUE    (0x03B41456)    /* 2.00 sec @ 466MHz, 1.33 sec @ 700MHz, 1.18 sec @ 788MHz (unit: 15 cycle count) */
#define BUS_MON_FLAG_WATCHPOINT_BIT     (0)
#define BUS_MON_FLAG_CLEAR_STAGE1_BIT   (1)
#define BUS_MON_FLAG_CLEAR_STAGE2_BIT   (2)
#define BUS_MON_FLAG_WATCHPOINT_MASK    (0x1 << BUS_MON_FLAG_WATCHPOINT_BIT)
#define BUS_MON_FLAG_CLEAR_STAGE1_MASK  (0x1 << BUS_MON_FLAG_CLEAR_STAGE1_BIT)
#define BUS_MON_FLAG_CLEAR_STAGE2_MASK  (0x1 << BUS_MON_FLAG_CLEAR_STAGE2_BIT)
#define BUS_MON_STATE_INIT              (0x1)
#define BUS_MON_STATE_1ST_STAGE         (0x2)
#define BUS_MON_STATE_2ND_STAGE         (0x3)
#define NUM_OF_BUS_MON_REGISTERS        (23)
#define NUM_OF_BUS_MON_TOTAL            (1 + 2 * NUM_OF_BUS_MON_REGISTERS)  /* state + 1st stage + 2nd stage */
#define ADSP_BUS_MON_STATE              (*(volatile uint32_t *)(ADSP_BUS_MON_BACKUP_BASE))
#define ADSP_BUS_MON_1ST_STAGE_BASE     (ADSP_BUS_MON_BACKUP_BASE + sizeof(uint32_t))
#define ADSP_BUS_MON_2ND_STAGE_BASE     (ADSP_BUS_MON_1ST_STAGE_BASE + NUM_OF_BUS_MON_REGISTERS * sizeof(uint32_t))

#ifndef NULL
#define NULL 0
#endif

static uint32_t dump_bus_monitor_register (uint32_t ctrl, uint32_t wp_addr, uint32_t wp_mask, uint32_t *p_buf)
{
    uint32_t bus_monitor_flags = 0;
    *p_buf++ = ctrl;
    *p_buf++ = drv_reg32(BUS_DBG_TIMER_CON0);
    *p_buf++ = drv_reg32(BUS_DBG_TIMER_CON1);
    *p_buf++ = drv_reg32(BUS_DBG_TIMER0);
    *p_buf++ = drv_reg32(BUS_DBG_TIMER1);
    *p_buf++ = wp_addr;
    *p_buf++ = wp_mask;
    {   /* Dump read address tracker to DTCM */
        uint32_t idx;
        for (idx = 0; idx < 8; idx++) {
            *p_buf++ = drv_reg32(BUS_DBG_AR_TRACK0_L + 8 * idx);
        }
    }
    {   /* Dump write address tracker to DTCM */
        uint32_t idx;
        for (idx = 0; idx < 8; idx++) {
            *p_buf++ = drv_reg32(BUS_DBG_AW_TRACK0_L + 8 * idx);
        }
    }
    if ((ctrl & (0x1 << 8)) != 0 || (ctrl & (0x1 << 9)) != 0) {
        bus_monitor_flags |= 0x1 << BUS_MON_FLAG_CLEAR_STAGE1_BIT;
    }
    if ((ctrl & (0x1 << 10)) != 0) {
        bus_monitor_flags |= 0x1 << BUS_MON_FLAG_WATCHPOINT_BIT;
    }
    if ((ctrl & (0x1 << 20)) != 0 || (ctrl & (0x1 << 21)) != 0) {
        bus_monitor_flags |= 0x1 << BUS_MON_FLAG_CLEAR_STAGE2_BIT;
    }
    return bus_monitor_flags;
}

static void bus_monitor_1st_stage_handler(void)
{
    ADSP_BUS_MON_STATE = BUS_MON_STATE_1ST_STAGE;
    {   /* Backup bus monitor registers to DTCM */
        uint32_t ctrl = drv_reg32(BUS_DBG_CON);
        uint32_t wp_addr = drv_reg32(BUS_DBG_WP);
        uint32_t wp_mask = drv_reg32(BUS_DBG_WP_MASK);
        uint32_t bus_monitor_flags = dump_bus_monitor_register(ctrl, wp_addr, wp_mask, (uint32_t *)ADSP_BUS_MON_1ST_STAGE_BASE);
        if (bus_monitor_flags & (BUS_MON_FLAG_CLEAR_STAGE1_MASK | BUS_MON_FLAG_WATCHPOINT_MASK)) {
            ctrl |= 0x1 << 7;
        }
        if (bus_monitor_flags & BUS_MON_FLAG_CLEAR_STAGE2_MASK) {
            ctrl |= 0x1 << 23;
        }
        drv_write_reg32(BUS_DBG_CON, ctrl);
    }

#ifdef CFG_WDT_SUPPORT
    /* wake up APMCU */
    ADSP_SPM_WAKEUPSRC = 1;
#ifdef TCM_LAYOUT_VERSION
    mask_irq(HIFI3_WDT_IRQn);
#endif
    mtk_wdt_set_time_out_value(10);/*assign a small value to make ee sooner*/
    mtk_wdt_restart();
#endif
    /*put hifi3 in memw & idle*/
    while (1) {
        __asm(
            "memw;\n"
            "WAITI 15;\n"
        );
    }
}

void bus_monitor_2nd_stage_handler(void)
{
    ADSP_BUS_MON_STATE = BUS_MON_STATE_2ND_STAGE;
    {   /* Backup bus monitor registers to DTCM */
        uint32_t ctrl = drv_reg32(BUS_DBG_CON);
        uint32_t wp_addr = drv_reg32(BUS_DBG_WP);
        uint32_t wp_mask = drv_reg32(BUS_DBG_WP_MASK);
        uint32_t bus_monitor_flags = dump_bus_monitor_register(ctrl, wp_addr, wp_mask, (uint32_t *)ADSP_BUS_MON_2ND_STAGE_BASE);
        if (bus_monitor_flags & (BUS_MON_FLAG_CLEAR_STAGE1_MASK | BUS_MON_FLAG_WATCHPOINT_MASK)) {
            ctrl |= 0x1 << 7;
        }
        if (bus_monitor_flags & BUS_MON_FLAG_CLEAR_STAGE2_MASK) {
            ctrl |= 0x1 << 23;
        }
        drv_write_reg32(BUS_DBG_CON, ctrl);
    }
}

void bus_monitor_init (void)
{
    request_irq(BUS_TRACKER_IRQn, bus_monitor_1st_stage_handler, "BUS");

    /* hw initialize move to ap */
}

void bus_monitor_dump (void)
{
    uint32_t ctrl = drv_reg32(BUS_DBG_CON);
    uint32_t wp_addr = drv_reg32(BUS_DBG_WP);
    uint32_t wp_mask = drv_reg32(BUS_DBG_WP_MASK);
    PRINTF_D("+%s()\n", __FUNCTION__);
    dump_bus_monitor_register(ctrl, wp_addr, wp_mask, (uint32_t *)ADSP_BUS_MON_1ST_STAGE_BASE);
    dump_bus_monitor_register(ctrl, wp_addr, wp_mask, (uint32_t *)ADSP_BUS_MON_2ND_STAGE_BASE);
    PRINTF_D("-%s()\n", __FUNCTION__);
}

int32_t is_bus_monitor_triggered (void)
{
    uint32_t ctrl = drv_reg32(BUS_DBG_CON);
    return ((ctrl & 0x00300300) != 0 || ADSP_BUS_MON_STATE == BUS_MON_STATE_1ST_STAGE ? 1 : 0);
}
