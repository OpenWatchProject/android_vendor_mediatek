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

#include <wdt.h>
#include <driver_api.h>
#include <mt_reg_base.h>
#include <FreeRTOS.h> /* log */
#include <utils.h>
#include <interrupt.h> /*for debug*/
#include <string.h>
#ifdef CFG_VCORE_DVFS_SUPPORT
#include <dvfs.h>
#endif
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif

#define SHARESEC     __attribute__ ((section (".share")))
static unsigned long long wdt_time_old = 0;
static unsigned long long wdt_time_new = 0;
static unsigned int wdt_timeout_flag = 0;

/*
 * special function to make sure
 * cpu is idle state when reset
 * wdt interrupt will higher than others
 */
void mtk_wdt_isr(void)
{
    /* set reboot flag to let AP know we are ready to reboot*/
    *(volatile unsigned int *)SCP_GPR_CM4_A_REBOOT = 0x34;
    /*disable INTC interrupt*/
    DRV_WriteReg32(SCP_INTC_IRQ_ENABLE, 0x00000000);
    DRV_WriteReg32(SCP_INTC_IRQ_ENABLE_MSB, 0x00000000 | DBG_IRQ_EN | WDT_IRQ_EN);
    while (1) {
        __asm volatile("dsb");
        __asm volatile("wfi");
    }
    return;
}

void kick_timer_isr(void)
{
    mtk_wdt_restart();
    return;
}

void mtk_wdt_disable(void)
{
    DRV_WriteReg32(WDT_CFGREG, DISABLE_WDT);
}
void mtk_wdt_enable(void)
{
    DRV_WriteReg32(WDT_CFGREG, START_WDT);
}

int mtk_wdt_set_time_out_value(unsigned int value)
{
    if (value > 0xFFFFF) {
        PRINTF_D("SCP WDT Timeout value overflow\n");
        return -1;
    }
    mtk_wdt_disable();
    DRV_WriteReg32(WDT_CFGREG, 1 << WDT_EN | value);
    return 0;
}

void mtk_wdt_restart(void)
{
    DRV_WriteReg32(WDT_KICKREG, KICK_WDT);
}

void mtk_wdt_restart_interval(unsigned long long interval)
{
#ifdef CFG_XGPT_SUPPORT
    wdt_time_new = read_xgpt_stamp_ns();
#else
    wdt_time_new = 0;
#endif

    /* 30 seconds, for debugging only */
    if ((wdt_time_new - wdt_time_old) >= 30000000000) {
        wdt_timeout_flag = 1;
    }

    if ((wdt_time_new - wdt_time_old) > interval) {
        mtk_wdt_restart();
        wdt_time_old = wdt_time_new;
    }
}

void mtk_wdt_init(void)
{
    mtk_wdt_disable();
    /* needs 70us to reload counter value */
    udelay(70);
#ifdef CFG_VCORE_DVFS_SUPPORT
    scp_wakeup_src_setup(WDT_WAKE_CLK_CTRL, 1);
#endif

    NVIC_ClearPendingIRQ(WDT_IRQn);
    /* give WDT the ability to trigger when IRQ disable */
    NVIC_SetPriority(WDT_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY - 1);
    NVIC_EnableIRQ(WDT_IRQn);

    mtk_wdt_enable();
    mtk_wdt_restart(); /* restart wdt after wdt reset */
    PRINTF_D("SCP mtk_wdt_init: WDT_CFGREG=0x%x!!\n", DRV_Reg32(WDT_CFGREG));
}
