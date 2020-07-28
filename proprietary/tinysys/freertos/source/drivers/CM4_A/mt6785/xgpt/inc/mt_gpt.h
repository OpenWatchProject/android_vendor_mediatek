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

#define TIMER_IN_CLK    (CLK_CTRL_BASE + 0x30)
#define TMR_MCLK_CG (1 << 0)
#define TMR_BCLK_CG (1 << 1)

#define SCP_TIMER_BASE TIMER_BASE

#define TMR0            0x0
#define TMR1            0x1
#define TMR2            0x2
#define TMR3            0x3
#define TMR4            0x4
#define TMR5            0x5
#define NR_TMRS         0x6


/* AP side system counter frequence is 13MHz*/
#define AP_NS_PER_CNT   (1000000000UL)/(13000000UL)

#define TIMER_EN             (0x00)
#define TIMER_CLK_SRC        (0x00)
#define TIMER_RST_VAL        (0x04)
#define TIMER_CUR_VAL_REG    (0x08)
#define TIMER_IRQ_CTRL_REG   (0x0C)

#define TIMER_CLK_SEL_REG           (SCP_TIMER_BASE+0x40)

#define portNVIC_MTK_XGPT_REG               (TIMER_BASE + 0x18)


#define TIMER_ENABLE            1
#define TIMER_DISABLE           0

#define TIMER_IRQ_ENABLE            1
#define TIMER_IRQ_DISABLE           0

#define TIMER_IRQ_STA                   (0x1 << 4)
#define TIMER_IRQ_CLEAR                 (0x1 << 5)


/* TODO: Check this setting */
#define TIMER_CLK_SRC_CLK_32K       (0x00)
#define TIMER_CLK_SRC_CLK_26M       (0x01)
#define TIMER_CLK_SRC_BCLK          (0x02)
#define TIMER_CLK_SRC_PCLK          (0x03)

#define TIMER_CLK_SRC_MASK          0x3
#define TIMER_CLK_SRC_SHIFT         4

#define DELAY_TIMER_1US_TICK       ((unsigned int)1)           //     (32KHz)
#define DELAY_TIMER_1MS_TICK       ((unsigned int)33) //(32KHz)
// 32KHz: 31us = 1 counter
#define TIME_TO_TICK_US(us) ((us)*DELAY_TIMER_1US_TICK)
// 32KHz: 1ms = 33 counter
#define TIME_TO_TICK_MS(ms) ((ms)*DELAY_TIMER_1MS_TICK)

#define COUNT_TO_TICK(x) ((x)/32) //32KHz, 1 tick is 32 counters

#define US_LIMIT 31 /* udelay's parameter limit*/

#define UNUSE1_TIMER         TMR0
#define UNUSE1_TIMER_IRQ     TIMER0_IRQn

//TICK_TIMER used for system tick & softtimer
#define TICK_TIMER          TMR1
#define TICK_TIMER_IRQ      TIMER1_IRQn

//DELAY_TIMER used for mdelay()/udelay()
#define DELAY_TIMER            TMR2
#define DELAY_TIMER_RSTVAL     0xffffffff
#define MAX_RG_BIT         0xFFFFFFFF

#define DMGR_TIMER             TMR3
#define DMGR_TIMER_RSTVAL      TIME_TO_TICK_MS(1)

#define CHRE_TIMER             TMR4
#define CHRE_TIMER_RSTVAL      0xffffffff

#define UNUSE2_TIMER                TMR5
#define UNUSE2_TIMER_RSTVAL         0xffffffff

typedef unsigned long long mt_time_t;
typedef void (*platform_timer_callback)(void *arg);

struct timer_device {
    unsigned int id;
    unsigned int base_addr;
    unsigned int irq_id;
};

/*************************End*****************************************/
void mdelay(unsigned long msec);
void udelay(unsigned long usec);
void mt_platform_timer_init(void);
int platform_set_periodic_timer(platform_timer_callback callback, void *arg, mt_time_t interval);
unsigned long long read_xgpt_stamp_ns(void);
unsigned long long timer_get_global_timer_tick(void);
unsigned long long get_boot_time_ns(void);
unsigned long long timer_get_global_timer_tick(void);
struct timer_device *id_to_dev(unsigned int id);


#define OSTIMER_LATCH_TIME_SUPPORT

#ifdef OSTIMER_LATCH_TIME_SUPPORT
int alloc_latch_time(void);
void free_latch_time(int id);
void enable_latch_time(int id, int irq);
void disable_latch_time(int id);
uint64_t get_latch_time_timestamp(int id);
#else
static inline int alloc_latch_time(void)
{
    return -1;
}
static inline void free_latch_time(int id)
{
}
static inline void enable_latch_time(int id, int irq)
{
}
static inline void disable_latch_time(int id)
{
}
static inline uint64_t get_latch_time_timestamp(int id)
{
    return 0;
}

#endif
#endif  /* !__MT_GPT_H__ */
