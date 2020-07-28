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

#ifndef __MT_GPT_H__
#define __MT_GPT_H__

#include <mt_reg_base.h>
#include "main.h"



//#define CFG_FPGA

#define TMR0            0x0
#define TMR1            0x1
#define TMR2            0x2
#define TMR3            0x3
#define NR_TMRS         0x4

#define OSTIMER_CON          (ADSP_TIMER_BASE + 0x80)
#define OSTIMER_INIT_L       (ADSP_TIMER_BASE + 0x84)
#define OSTIMER_INIT_H       (ADSP_TIMER_BASE + 0x88)
#define OSTIMER_CUR_L        (ADSP_TIMER_BASE + 0x8C)
#define OSTIMER_CUR_H        (ADSP_TIMER_BASE + 0x90)
#define OSTIMER_TVAL         (ADSP_TIMER_BASE + 0x94)
#define OSTIMER_IRQ_ACK      (ADSP_TIMER_BASE + 0x98)

#define TIMER_EN             (0x00)
#define TIMER_CLK_SRC        (0x00)
#define TIMER_RST_VAL        (0x04)
#define TIMER_CUR_VAL_REG    (0x08)
#define TIMER_IRQ_CTRL_REG   (0x0C)

#define TIMER_ENABLE            1
#define TIMER_DISABLE           0

#define TIMER_IRQ_ENABLE            1
#define TIMER_IRQ_DISABLE           0

#define TIMER_IRQ_STA                   (0x1 << 4)
#define TIMER_IRQ_CLEAR                 (0x1 << 5)

#define TMR_MCLK_DUAL_CG (1 << 16)
#define TMR_BCLK_DUAL_CG (1 << 17)
/* TODO: Check this setting */

#define TIMER_CLK_SRC_CLK_26M       (0x00)
#define TIMER_CLK_SRC_BCLK          (0x01)
#define TIMER_CLK_SRC_PCLK          (0x02)

#define TIMER_CLK_SRC_MASK          0x3
#define TIMER_CLK_SRC_SHIFT         4

#ifdef CFG_FPGA
#define DELAY_TIMER_1US_TICK       ((unsigned int)10) // (10MHz)
#define DELAY_TIMER_1MS_TICK       ((unsigned int)10000) // (10MHz)
#else
#define DELAY_TIMER_1US_TICK       ((unsigned int)26) // (26MHz)
#define DELAY_TIMER_1MS_TICK       ((unsigned int)26000) // (26MHz)
#endif
#define TIME_TO_TICK_US(us) ((us)*DELAY_TIMER_1US_TICK)
#define TIME_TO_TICK_MS(ms) ((ms)*DELAY_TIMER_1MS_TICK)

#define COUNT_TO_TICK(x) ((x)/26000)

#define US_LIMIT 31 /* udelay's parameter limit*/

typedef unsigned long long mt_time_t;
typedef void (*platform_timer_callback)(void *arg);

struct timer_sys_time_t {
    unsigned long long ts;
    unsigned long long off_ns;
    unsigned long long clk;
    unsigned char insuspend;
};

extern void mdelay(unsigned long msec) INTERNAL_FUNCTION;
extern void udelay(unsigned long usec) INTERNAL_FUNCTION;
extern void mt_platform_timer_init(void) INTERNAL_FUNCTION;
extern int platform_set_periodic_timer(platform_timer_callback callback, void *arg,
                                mt_time_t interval) INTERNAL_FUNCTION;
extern unsigned long long read_xgpt_stamp_ns(void) INTERNAL_FUNCTION;
extern unsigned long long timer_get_global_timer_tick(void) INTERNAL_FUNCTION;
extern int timer_sched_clock(struct timer_sys_time_t *ap_ts) INTERNAL_FUNCTION;
extern unsigned long long timer_get_ap_timestamp(void) INTERNAL_FUNCTION;
extern unsigned char timer_get_ap_suspend(void) INTERNAL_FUNCTION;

#endif /* __MT_GPT_H__ */

