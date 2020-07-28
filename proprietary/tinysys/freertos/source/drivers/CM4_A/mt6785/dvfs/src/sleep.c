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
#include <stdio.h>
#include <driver_api.h>
#include <interrupt.h>
#include <mt_reg_base.h>
#include <scp_sem.h>
#include <platform_mtk.h>

#include <scp_ipi.h>
#include <semphr.h>
#include <utils.h>
#include <sys/types.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
#include <feature_manager.h>
#include <tinysys_config.h>
#include <mt_uart.h>
#include <wakelock.h>
#include <dvfs_common.h>
#include <dvfs.h>
#include <sleep.h>
#ifdef CFG_CACHE_SUPPORT
#include <dram_management_api.h>
#include <cache_internal.h>
#include <mpu_mtk.h>
#endif

#define DVFS_DEBUG  0

#if DVFS_DEBUG
#define DVFSLOG(fmt, arg...) PRINTF_W(DVFSTAG fmt, ##arg)
#else
#define DVFSLOG(...)
#endif

#if DVFS_DEBUG
#define DVFSFUC(fmt, arg...) PRINTF_W(DVFSTAG "%s("fmt")\n", __func__, ##arg)
#else
#define DVFSFUC(...)
#endif

#define BUSY_WAIT(x, y) \
    do { \
        uint32_t i = 0; \
        i++; \
        scp_status = DVFS_STATUS_OK; \
        while (x) { \
            if (i > y) { \
                scp_status = DVFS_STATUS_TIMEOUT; \
                break; \
            } \
            i++; \
        } \
    } while (0)

static uint32_t scp_status = DVFS_STATUS_OK;

#define SLEEP_TIMER_CRITERIA        2

static uint32_t g_irq_enable_state = 0;
static uint32_t g_irq_msb_enable_state = 0;

static uint32_t enter_sleep_mode;

static uint32_t wakeup_src_mask = 0;
static uint32_t wakeup_src_msb_mask = 0;

static int scp_sleep_ctrl_flag = SCP_SLEEP_ON;

#if SLEEP_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
TickType_t sleep_start_time, sleep_end_time;
TickType_t wakeup_start_time, wakeup_end_time;
#endif

#if AUTO_DUMP_SLEPP_STATUS
extern unsigned long long timer_get_global_timer_tick(void);

#define STATUS_UPDATE_DURATION_SEC      30
#define SLEEP_BLOCK_CHECK_DURATION_SEC  30

uint32_t g_sleep_cnt = 0, g_sleep_total_cnt = 0;

static unsigned long long STATUS_UPDATE_DURATION = (13000000 * STATUS_UPDATE_DURATION_SEC);
static unsigned long long SLEEP_BLOCK_CHECK_DURATION = (13000000 * SLEEP_BLOCK_CHECK_DURATION_SEC);
static unsigned long long pre_status_update_time = 0;
static unsigned long long pre_sleep_block_time = 0;
static unsigned long sleep_block_cnt[NR_REASONS] = {0};
#endif

extern freq_enum clk_opp_table[CLK_OPP_NUM];

#define SLEEP_READY_CHK_COUNT   100
unsigned int sleep_hw_ready_timeout = 520; /* =32*0xa + 200, unit: us */

#if FAST_WAKEUP_SUPPORT
static unsigned int wakeup_ctrl_flow = SCP_STATE_IDLE;
#endif

void scp_wakeup_src_setup(wakeup_src_id irq, uint32_t enable)
{
    DVFSLOG("%s(%d,%d)\n", __FUNCTION__, (int)irq, (int)enable);

    kal_taskENTER_CRITICAL();

    if (irq >= NR_WAKE_CLK_CTRL) {
        mlprintf(LOG_DVFS_EMERG, "err wakeup_src_id %d\n", irq);
        configASSERT(0);
        kal_taskEXIT_CRITICAL();
        return;
    }

    if (enable) {
        if (irq < 32) {
            wakeup_src_mask |= (1 << irq);
            DRV_WriteReg32(IRQ_SLEEP_EN, wakeup_src_mask);
        } else {
            wakeup_src_msb_mask |= (1 << (irq - 32));
            DRV_WriteReg32(IRQ_SLEEP_EN_MSB, wakeup_src_msb_mask);
        }
    } else {
        if (irq < 32) {
            wakeup_src_mask &= ~(1 << irq);
            DRV_WriteReg32(IRQ_SLEEP_EN, wakeup_src_mask);
        } else {
            wakeup_src_msb_mask &= ~(1 << (irq - 32));
            DRV_WriteReg32(IRQ_SLEEP_EN_MSB, wakeup_src_msb_mask);
        }
    }

#if 0
    DVFSLOG("IRQ_SLEEP_EN=0x%x, IRQ_SLEEP_EN_MSB=0x%x\n",
            DRV_Reg32(IRQ_SLEEP_EN), DRV_Reg32(IRQ_SLEEP_EN_MSB));
#endif

    kal_taskEXIT_CRITICAL();
}

/* Disable SPM controlled Sleep mode. */
void use_SCP_ctrl_sleep_mode(void)
{
    DVFSFUC();
    DRV_ClrReg32(SLEEP_CTRL, 1 << SPM_SLP_MODE_BIT);
}

/*
 * Use only when using SCP internal Sleep Controller.
 * Please disable when using SPM to control SCP Sleep Mode.
 */
void enable_SCP_sleep_mode(void)
{
    /*DVFSFUC();*/
    DRV_SetReg32(SLEEP_CTRL, 1 << SLP_CTRL_EN_BIT);
}

void disable_SCP_sleep_mode(void)
{
    /*DVFSFUC();*/
    DRV_ClrReg32(SLEEP_CTRL, 1 << SLP_CTRL_EN_BIT);
}

void wake_cksw_sel_normal(int val)
{
    DVFSFUC("%d", val);

    DRV_ClrReg32(WAKE_CKSW_SEL, WAKE_CKSW_SEL_NORMAL_MASK << WAKE_CKSW_SEL_NORMAL_BIT);
    DRV_SetReg32(WAKE_CKSW_SEL, val << WAKE_CKSW_SEL_NORMAL_BIT);
}

void wake_cksw_sel_slow(int val)
{
    DVFSFUC("%d", val);

    DRV_ClrReg32(WAKE_CKSW_SEL, WAKE_CKSW_SEL_SLOW_MASK << WAKE_CKSW_SEL_SLOW_BIT);
    DRV_SetReg32(WAKE_CKSW_SEL, val << WAKE_CKSW_SEL_SLOW_BIT);
}

void unmask_all_int(void)
{
    /* restore interrupt setting */
    DRV_WriteReg32(IRQ_ENABLE, g_irq_enable_state);
    DRV_WriteReg32(IRQ_ENABLE_MSB, g_irq_msb_enable_state);
}

void mask_all_int(void)
{
    /* backup interrupt setting */
    g_irq_enable_state = DRV_Reg32(IRQ_ENABLE);
    g_irq_msb_enable_state = DRV_Reg32(IRQ_ENABLE_MSB);

    /* mask all interrupts except CLK_CTRL interrupt */
    DRV_ClrReg32(IRQ_ENABLE, 0xffffffff);
    DRV_ClrReg32(IRQ_ENABLE_MSB, 0xffffffff);
    DRV_SetReg32(IRQ_ENABLE, (1 << CLK_CTRL_IRQ_EN_BIT));
}

void set_cm4_scr(void)
{
    /* DVFSFUC(); */

    /* Selected sleep state is deep sleep */
    DRV_SetReg32(ARMV7_SYS_CTRL, 0x1 << SLEEPDEEP_BIT);
}

void clr_cm4_scr(void)
{
    /* DVFSFUC(); */

    /* Selected sleep state is not deep sleep */
    DRV_ClrReg32(ARMV7_SYS_CTRL, 0x1 << SLEEPDEEP_BIT);
}

void scp_sleep_ctrl(int id, void* data, unsigned int len)
{
    uint8_t *msg = (uint8_t *)data;

    if (*msg == 0) {
        scp_sleep_ctrl_flag = SCP_SLEEP_OFF;
        mlprintf(LOG_DVFS_INFO, "set SCP_SLEEP_OFF\n");
    } else if (*msg == 1) {
        scp_sleep_ctrl_flag = SCP_SLEEP_ON;
        mlprintf(LOG_DVFS_INFO, "set SCP_SLEEP_ON\n");
    } else if (*msg == 2) {
        scp_sleep_ctrl_flag = SCP_SLEEP_NO_WAKEUP;
        mlprintf(LOG_DVFS_INFO, "set SCP_SLEEP_NO_WAKEUP\n");
    } else if (*msg == 3) {
        scp_sleep_ctrl_flag = SCP_SLEEP_NO_CONDITION;
        mlprintf(LOG_DVFS_INFO, "set SCP_SLEEP_FORCE_AND_NO_WAKEUP\n");
    } else
        mlprintf(LOG_DVFS_INFO, "invalid set value %d\n", *msg);
}

#if PMICW_SLEEP_REQ_FLOW
void pmicw_sleep_req(uint32_t enable, uint32_t wait_ack)
{
    freq_enum cur_clk;
    uint32_t count_down;

    DVFSFUC("%d", (int)enable);

    if (enable == SCP_ENABLE) {
        DRV_WriteReg32(PMICW_CTRL, (DRV_Reg32(PMICW_CTRL) |
                                    (1 << PMICW_SLEEP_REQ_BIT)));

        if (wait_ack == NEED_WAIT) {
            cur_clk = get_cur_clk();
            count_down = (cur_clk != FREQ_UNINIT) ?
                         (2000 * cur_clk) : (2000 * clk_opp_table[CLK_OPP_NUM - 1]);

            BUSY_WAIT(((DRV_Reg32(PMICW_CTRL) & (0x1 << PMICW_SLEEP_ACK_BIT)) !=
                       (enable << PMICW_SLEEP_ACK_BIT)), count_down / 6);

            if ((DRV_Reg32(PMICW_CTRL) & (0x1 << PMICW_SLEEP_ACK_BIT)) !=
                    (enable << PMICW_SLEEP_ACK_BIT)) {
                mlprintf(LOG_DVFS_EMERG, "wait ack fail\n");
                configASSERT(0);
            }
        }
    } else {
        DRV_WriteReg32(PMICW_CTRL, (DRV_Reg32(PMICW_CTRL) &
                                    ~(0x1 << PMICW_SLEEP_REQ_BIT)));
    }
}
#endif

#if AUTO_DUMP_SLEPP_STATUS
void dump_sleep_block_reason(void)
{
    int i;

    mlprintf(LOG_DVFS_WARN, "No sleep reasons: tmr=%lu, build=%lu, sema=%lu, lock=%lu, ipi=%lu, flag=%lu, slpbusy=%lu\n",
             sleep_block_cnt[BY_TIMER],
             sleep_block_cnt[BY_COMPILER],
             sleep_block_cnt[BY_SEMAPHORE],
             sleep_block_cnt[BY_WAKELOCK],
             sleep_block_cnt[BY_IPI_BUSY],
             sleep_block_cnt[BY_SLP_DISABLED],
             sleep_block_cnt[BY_SLP_BUSY]);

    if (sleep_block_cnt[BY_WAKELOCK] != 0) {
        wake_lock_list_dump();
        mt_dump_dma_struct();
    }

    /* reset counters of all reasons */
    for (i = 0; i < NR_REASONS; i++)
        sleep_block_cnt[i] = 0;
}
#endif

static int can_enter_sleep(uint32_t modify_time)
{
    int reason = NR_REASONS;
#if AUTO_DUMP_SLEPP_STATUS
    unsigned long long cur_time = 0;
#endif
#ifdef CFG_XGPT_SUPPORT
    uint64_t t1, t2;
    int retry_cnt = 0;
#endif

    /* DVFSFUC(); */

    if (scp_sleep_ctrl_flag == SCP_SLEEP_NO_CONDITION)
        goto out;
    else if (scp_sleep_ctrl_flag ==  SCP_SLEEP_OFF) {
        reason = BY_SLP_DISABLED;
        goto out;
    }

#if (configLOWPOWER_DISABLE == 1)
    reason = BY_COMPILER;
    goto out;
#endif

    if (modify_time < SLEEP_TIMER_CRITERIA) {
        reason = BY_TIMER;
        goto out;
    }

    if (scp_get_awake_semaphore(SEMAPHORE_SCP_A_AWAKE)) {
        reason = BY_SEMAPHORE;
        goto out;
    }

    if (!wake_lock_is_empty()) {
        reason = BY_WAKELOCK;
        goto out;
    }

    if (is_ipi_busy()) {
        reason = BY_IPI_BUSY;
        goto out;
    }

#if SLEEP_WAKEUP_INTERCHANGE_TEST
    {
        static int is_sleep_turn = 0;
        static int continue_times = 0;

        if (continue_times >= INTERCHANGE_INTERVAL) {
            continue_times = 0;
            if (is_sleep_turn) {
                is_sleep_turn = 0;
            } else {
                is_sleep_turn = 1;
            }
        } else {
            continue_times++;
        }

        if (!is_sleep_turn) {
            reason = BY_SLP_DISABLED;
            goto out;
        }
    }
#endif

#if FAST_WAKEUP_SUPPORT
    if (wakeup_ctrl_flow == SCP_STATE_WAKEUP_1) {
        reason = BY_SLP_BUSY;
        mlprintf(LOG_DVFS_DEBUG, "Can't sleep due to only clkctrl1 triggered\n\n");
        goto out;
    }
#endif

    /* Wait until SLEEP_CTRL[31] = 0x1 or timeout happened */
#ifdef CFG_XGPT_SUPPORT
    t1 = timer_get_global_timer_tick();
    while (1) {
        if ((DRV_Reg32(SLEEP_CTRL) & (0x1 << 31)) != 0x0)
            break;

        retry_cnt++;
        if (retry_cnt > SLEEP_READY_CHK_COUNT) {
            retry_cnt = 0;
            t2 = timer_get_global_timer_tick(); /* 13MHz timer */
            if ((t2 - t1) > sleep_hw_ready_timeout * 13) {
                reason = BY_SLP_BUSY;
                mlprintf(LOG_DVFS_WARN, "Can't sleep due to HW not ready(0x%x), %uus\n",
                         DRV_Reg32(RG_SLEEP_STATE), sleep_hw_ready_timeout);
                goto out;
            }
        }
    }
#else
    BUSY_WAIT((DRV_Reg32(SLEEP_CTRL) & (0x1 << 31)) == 0x0, 1000);
    if (scp_status == DVFS_STATUS_TIMEOUT) {
        reason = BY_SLP_BUSY;
        mlprintf(LOG_DVFS_WARN, "Can't sleep due to HW not ready(0x%x), %uus\n",
                 DRV_Reg32(RG_SLEEP_STATE), sleep_hw_ready_timeout);
        goto out;
    }
#endif

out:
#if AUTO_DUMP_SLEPP_STATUS
    /* get current system tick */
#ifdef CFG_XGPT_SUPPORT
    cur_time = timer_get_global_timer_tick();
#endif
    if (pre_status_update_time == 0)
        pre_status_update_time = cur_time;

    if (pre_sleep_block_time == 0)
        pre_sleep_block_time = cur_time;
#endif

    if (reason == NR_REASONS) {
        g_sleep_cnt++;
        g_sleep_total_cnt++;

#if AUTO_DUMP_SLEPP_STATUS
        /* dump sleep status in each STATUS_UPDATE_DURATION */
        if ((cur_time - pre_status_update_time) >= STATUS_UPDATE_DURATION) {
            pre_status_update_time = cur_time;

            /* dump sleep count */
            mlprintf(LOG_DVFS_WARN, "sleep: cnt=%d, total=%d\r\n",
                     (int)g_sleep_cnt, (int)g_sleep_total_cnt);

            g_sleep_cnt = 0;
        }

        /* reset pre_sleep_block_time */
        pre_sleep_block_time = cur_time;
#else
        mlprintf(LOG_DVFS_WARN, "g_sleep_cnt = %d\r\n", (int)g_sleep_cnt);
#endif

        return 1;
    } else {
#if AUTO_DUMP_SLEPP_STATUS
        sleep_block_cnt[reason]++;

        if ((cur_time - pre_sleep_block_time) >= SLEEP_BLOCK_CHECK_DURATION) {
            pre_sleep_block_time = cur_time;
            dump_sleep_block_reason();
        }
#endif

        return 0;
    }
}

void dump_fast_wakeup_setting(void)
{
    mlprintf(LOG_DVFS_NOTE, "dump fast wakeup registers\n");
    mlprintf(LOG_DVFS_NOTE, "[0x%x] = 0x%x\n", SLEEP_CTRL, DRV_Reg32(SLEEP_CTRL));
    mlprintf(LOG_DVFS_NOTE, "[0x%x] = 0x%x\n", CLK_SYS_VAL, DRV_Reg32(CLK_SYS_VAL));
    mlprintf(LOG_DVFS_NOTE, "[0x%x] = 0x%x\n", CLK_HIGH_VAL, DRV_Reg32(CLK_HIGH_VAL));
    mlprintf(LOG_DVFS_NOTE, "[0x%x] = 0x%x\n", CLK_CTRL_SLP_CTRL, DRV_Reg32(CLK_CTRL_SLP_CTRL));
    mlprintf(LOG_DVFS_NOTE, "[0x%x] = 0x%x\n", CLK_ON_CTRL, DRV_Reg32(CLK_ON_CTRL));
    mlprintf(LOG_DVFS_NOTE, "[0x%x] = 0x%x\n", FAST_WAKE_CNT_END, DRV_Reg32(FAST_WAKE_CNT_END));
    mlprintf(LOG_DVFS_NOTE, "[0x%x] = 0x%x\n", WAKE_CKSW_SEL, DRV_Reg32(WAKE_CKSW_SEL));
    mlprintf(LOG_DVFS_NOTE, "[0x%x] = 0x%x\n", CLK_SEL_SLOW, DRV_Reg32(CLK_SEL_SLOW));
    mlprintf(LOG_DVFS_NOTE, "[0x%x] = 0x%x\n", SLEEP_DEBUG, DRV_Reg32(SLEEP_DEBUG));
    mlprintf(LOG_DVFS_NOTE, "[0x%x] = 0x%x\n", CLK_DIV_SEL, DRV_Reg32(CLK_DIV_SEL));
}

void pre_sleep_process(uint32_t modify_time)
{
#if SLEEP_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
    sleep_start_time = timer_get_global_timer_tick();
#endif

    /* DVFSFUC("%u", modify_time); */

    if (can_enter_sleep(modify_time)) {
        enter_sleep_mode = 1;

#ifdef CFG_CACHE_SUPPORT
        vDramManagerIdleHook();
#endif

#if PMICW_SLEEP_REQ_FLOW
        pmicw_sleep_req(SCP_ENABLE, NEED_WAIT);
#endif

        mask_all_int();

        if (scp_sleep_ctrl_flag == SCP_SLEEP_NO_WAKEUP ||
                scp_sleep_ctrl_flag == SCP_SLEEP_NO_CONDITION) {
            /* clear all wakeup sources */
            DRV_ClrReg32(IRQ_SLEEP_EN, 0xffffffff);
            DRV_ClrReg32(IRQ_SLEEP_EN_MSB, 0xffffffff);
        }

        /* Clear sleep mode by set SLEEP_CTRL[0] = 0x0 */
        disable_SCP_sleep_mode();

        /* Enable Sleep mode: SLEEP_CTRL[0] = 0x1 */
        enable_SCP_sleep_mode();

        /* ARMV7 sleep state is deep sleep */
        set_cm4_scr();

        mlprintf(LOG_DVFS_DEBUG, "enter sleep\n");

#if SLEEP_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
        sleep_end_time = timer_get_global_timer_tick();

        mlprintf(LOG_DVFS_INFO, "wakeup sw time: %d => %d = %d\n", wakeup_start_time, wakeup_end_time,
                 wakeup_end_time - wakeup_start_time);
        mlprintf(LOG_DVFS_INFO, "RTOS   sw time: %d => %d = %d\n", wakeup_end_time, sleep_start_time,
                 sleep_start_time - wakeup_end_time);
        mlprintf(LOG_DVFS_INFO, "sleep  sw time: %d => %d = %d\n\n\n", sleep_start_time, sleep_end_time,
                 sleep_end_time - sleep_start_time);
#endif
    } else {
        enter_sleep_mode = 0;
        mlprintf(LOG_DVFS_DEBUG, "enter wfi\n");
    }
}

void post_sleep_process(uint32_t expect_time)
{
#if SLEEP_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
    wakeup_start_time = timer_get_global_timer_tick();
#endif

    if (enter_sleep_mode == 1) {
        mlprintf(LOG_DVFS_DEBUG, "SCP wakeup\n");

        wakeup_source_count(DRV_Reg32(IRQ_STATUS));

#if PMICW_SLEEP_REQ_FLOW
        pmicw_sleep_req(SCP_DISABLE, NEED_WAIT);
#endif

        /* ARMV7 sleep state is NOT deep sleep */
        clr_cm4_scr();

        unmask_all_int();

#if SLEEP_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
        wakeup_end_time = timer_get_global_timer_tick();
#endif
    } else {
        DVFSLOG("exit WFI\n");
    }
}

void clkctrl_isr(void)
{
    DVFSFUC();

#if FAST_WAKEUP_SUPPORT
    wakeup_ctrl_flow = SCP_STATE_WAKEUP_1;
#endif

    /* Read CLK_IRQ_ACK register to acknowledge CLK_CTRL interrupt */
    DRV_Reg32(CLK_IRQ_ACK);

    NVIC_ClearPendingIRQ(CLK_CTRL_IRQn);
}

#if FAST_WAKEUP_SUPPORT
void clkctrl2_isr(void)
{
#if SW_TRIGGER_26M_TO_NORMAL
    unsigned int val1, val2, val3;
#endif

    DVFSFUC();

    if (wakeup_ctrl_flow != SCP_STATE_WAKEUP_1) {
        PRINTF_E("\n\nERROR: clkctrl2_isr: clkctrl1 not triggered\n\n");
        configASSERT(0);
    }

    wakeup_ctrl_flow = SCP_STATE_WAKEUP_2;

#if SW_TRIGGER_26M_TO_NORMAL
    /* SLEEP_CTRL[0] = 0x0 */
    disable_SCP_sleep_mode();
#endif

    /* ACK SLP_IRQ_2 (Write 1 clear) */
    DRV_WriteReg32(SLP_IRQ2, SLP_IRQ2_MASK << SLP_IRQ2_BIT);

    NVIC_ClearPendingIRQ(CLK_CTRL2_IRQn);

#if SW_TRIGGER_26M_TO_NORMAL
    val1 = (0x1 << (DRV_Reg32(CLK_SW_SEL) & 0x3)) << 8;
    val2 = (0x1 << (DRV_Reg32(CLK_DIV_SEL) & 0x3)) << 8;
    val3 = (0x1 << (DRV_Reg32(WAKE_CKSW_SEL) & 0x3)) << 8;
    DVFSLOG("val1=0x%x, val2=0x%x, val3=0x%x\n", val1, val2, val3);

    BUSY_WAIT((DRV_Reg32(CLK_SW_SEL) & 0xf00) != val1 ||
              (DRV_Reg32(CLK_DIV_SEL) & 0xf00) != val2 ||
              (DRV_Reg32(WAKE_CKSW_SEL) & 0xf00) != val3, 500000);

    if ((DRV_Reg32(CLK_SW_SEL) & 0xf00) != val1 ||
            (DRV_Reg32(CLK_DIV_SEL) & 0xf00) != val2 ||
            (DRV_Reg32(WAKE_CKSW_SEL) & 0xf00) != val3) {
        mlprintf(LOG_DVFS_EMERG, "polling fast-wakeup DIV fail\n");
        configASSERT(0);
    }
#endif
}
#endif
