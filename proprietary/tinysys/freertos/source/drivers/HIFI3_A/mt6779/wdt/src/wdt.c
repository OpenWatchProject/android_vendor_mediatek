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
#include <interrupt.h>
#include <platform_mtk.h>
#include "FreeRTOS.h" /* log */
#include "main.h"
#include <driver_api.h>
#include <mt_reg_base.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
unsigned long long wdt_time_old INTERNAL_NON_INITIAL_DATA = 0;
unsigned long long wdt_time_new INTERNAL_NON_INITIAL_DATA= 0;
unsigned int wdt_timeout_flag INTERNAL_NON_INITIAL_DATA = 0;

void mtk_wdt_handler(void)
{
    while (1) {
    __asm(
        "memw;\n"
        "WAITI 15;\n"
        );
    }
}

void mtk_wdt_disable(void)
{
    DRV_WriteReg32(ADSP_CFGREG_WDT, DISABLE_WDT);
}
void mtk_wdt_enable(void)
{
    DRV_WriteReg32(ADSP_WDT_INIT_VALUE, WDT_TIMEOUT_COUNT);
    DRV_WriteReg32(ADSP_CFGREG_WDT, ENABLE_WDT);
}

int mtk_wdt_set_time_out_value(unsigned int value)
{
    if (value > 0xFFFFFFFF) {
        PRINTF_D("ADSP WDT Timeout value overflow\n");
        return -1;
    }
    mtk_wdt_disable();
    //set value and enable WDT
    DRV_WriteReg32(ADSP_WDT_INIT_VALUE, value);
#ifdef CFG_XGPT_SUPPORT
    udelay(WDT_DELAY_US * 2); //for wdt 32k clock source wait 2T~3T
#endif
    DRV_WriteReg32(ADSP_CFGREG_WDT, ENABLE_WDT);
#ifdef CFG_XGPT_SUPPORT
    udelay(WDT_DELAY_US * 3); //for wdt 32k clock source wait 2T~3T
#endif
    return 0;
}

void mtk_wdt_restart(void)
{
    unsigned int current_value = DRV_Reg32(ADSP_CFGREG_WDT);
    DRV_WriteReg32(ADSP_CFGREG_WDT, current_value | KICK_WDT);
}

void mtk_wdt_restart_interval(unsigned long long interval)
{
#ifdef CFG_XGPT_SUPPORT
    wdt_time_new = read_xgpt_stamp_ns();
#else
    wdt_time_new = 0;
#endif

    if ((wdt_time_new - wdt_time_old) >= 16777215) {
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
    mtk_wdt_enable();
    request_irq(HIFI3_WDT_IRQn, mtk_wdt_handler, "wdt");
}
