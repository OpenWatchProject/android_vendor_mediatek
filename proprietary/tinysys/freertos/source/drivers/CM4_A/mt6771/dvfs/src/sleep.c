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
#include <cache_dram_management.h>
#include <cache_internal.h>
#include <mpu_mtk.h>
#endif

#define DVFS_DEBUG              0
#define SLEEP_TIMER_PROFILING   0

#if DVFS_DEBUG
#define DVFSLOG(fmt, arg...)        \
    do {            \
        PRINTF_D(DVFSTAG fmt, ##arg);   \
    } while (0)
#else
#define DVFSLOG(...)
#endif

#if DVFS_DEBUG
#define DVFSFUC(fmt, arg...)        \
    do {            \
        PRINTF_D(DVFSTAG "%s("fmt")\n", __func__, ##arg);  \
    } while (0)
#else
#define DVFSFUC(...)
#endif

#if DVFS_DEBUG
#define DVFSDBG(fmt, arg...)        \
    do {            \
        PRINTF_W(DVFSTAG fmt, ##arg);   \
    } while (0)
#else
#define DVFSDBG(...)
#endif

#define DVFSERR(fmt, arg...)        \
    do {            \
        PRINTF_E(DVFSTAG fmt, ##arg);  \
    } while (0)

#define BUSY_WAIT(x, y) { \
                uint32_t i = 0;     \
                i++;                       \
                scp_status = DVFS_STATUS_OK;   \
                while (x) {           \
                    if (i > y) {        \
                        scp_status = DVFS_STATUS_TIMEOUT;   \
                        break;          \
                    }                        \
                    i++;                  \
                } \
            }

static uint32_t scp_status = DVFS_STATUS_OK;

#define SLEEP_TIMER_CRITERIA        2
static uint32_t g_logger_enable_dram = 0;
static uint32_t g_irq_enable_state = 0;
static uint32_t g_irq_msb_enable_state = 0;

uint32_t ap_resource_ack_cnt = 0;
static uint32_t ap_resource_req_cnt = 0;
static uint32_t dma_user_list[NUMS_MEM_ID];
static uint32_t infra_req_cnt = 0;
static uint32_t infra_user_list[NUMS_MEM_ID];

static uint32_t enter_sleep_mode;
static uint32_t g_sleep_cnt = 0, g_sleep_total_cnt = 0;
static uint32_t wakeup_src_mask = 0x00000000;
static uint32_t wakeup_src_msb_mask = 0x00000000;

static int scp_sleep_ctrl_flag = SCP_SLEEP_ON;

#if SLEEP_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
TickType_t sleep_start_time, sleep_end_time;
TickType_t wakeup_start_time, wakeup_end_time;
#endif

#if AUTO_DUMP_SLEPP_STATUS
extern unsigned long long timer_get_global_timer_tick(void);

#define STATUS_UPDATE_DURATION_SEC      30
#define SLEEP_BLOCK_CHECK_DURATION_SEC  30

static unsigned long long STATUS_UPDATE_DURATION = (13000000 * STATUS_UPDATE_DURATION_SEC);
static unsigned long long SLEEP_BLOCK_CHECK_DURATION = (13000000 * SLEEP_BLOCK_CHECK_DURATION_SEC);
static unsigned long long pre_status_update_time = 0;
static unsigned long long pre_sleep_block_time = 0;
static unsigned long sleep_block_cnt[NR_REASONS] = {0};
#endif

extern freq_enum clk_opp_table[CLK_OPP_NUM];


/*****************************************
 * AP/INFRA Resource APIs
 ****************************************/
void init_dram_resrc_req_cnt(void)
{
    int i;

    ap_resource_req_cnt = 0;
    infra_req_cnt = 0;

    for (i=0; i<NUMS_MEM_ID; i++) {
        dma_user_list[i] = 0;
        infra_user_list[i] = 0;
    }
}

int32_t get_resource_ack(uint32_t reg_name)
{
    /*DVFSFUC();*/
    return ((DRV_Reg32(reg_name) & 0x100) ? 1 : 0);
}

int32_t wait_for_ack(uint32_t reg_addr)
{
    uint32_t ret, dma_count = 0;

    do {
        ret = get_resource_ack(reg_addr);
        if (ret == 1)
            break;

        dma_count++;
    } while (dma_count < 1000000);

    if (ret != 1) {
        DVFSERR("%s(0x%x) fail\n", __func__, (unsigned int)reg_addr);
        configASSERT(0);
        return ret;
    }

    return 1;
}

int32_t check_dram_ack_status(void)
{
    int32_t ret = 0;

    kal_taskENTER_CRITICAL();
    if (ap_resource_ack_cnt > 0)
        ret = 1;
    else
        ret = 0;
    kal_taskEXIT_CRITICAL();

    return ret;
}

int32_t get_AP_ack_from_isr(scp_reserve_mem_id_t dma_id)
{
    int32_t ap_ack, ret = 0;

    if (ap_resource_ack_cnt == 0) {
        ap_ack = wait_for_ack(AP_RESOURCE);
        if (ap_ack == 1) {
            ap_resource_ack_cnt = 1;
#ifdef CFG_CACHE_SUPPORT
            enable_mpu_region(MPU_ID_DRAM_0);
            enable_mpu_region(MPU_ID_DRAM_1);
            hal_cache_scp_sleep_protect_off();
            hal_cache_invalidate_all_cache_lines_from_isr(CACHE_ICACHE);
#endif
            /*DVFSLOG("access dram resource success.\n");*/
            ret = 0;
        } else
            DVFSLOG("ap_resource_ack_cnt fail\n");
    } else
        DVFSLOG("ap_resource already acked.\n");

    return ret;
}

void dvfs_enable_DRAM_resource(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);

    kal_taskENTER_CRITICAL();
    enable_infra(dma_id, NO_WAIT);
    enable_AP_resource_from_isr(dma_id);
#ifdef CFG_XGPT_SUPPORT
    if (ap_resource_ack_cnt == 0)
        udelay(REQ_DRAM_WAIT_TIME_US);
#endif
    get_AP_ack_from_isr(dma_id);
    kal_taskEXIT_CRITICAL();
}

unsigned int dvfs_enable_DRAM_no_wait_for_logger(scp_reserve_mem_id_t dma_id)
{
    unsigned int ret = 0;

    kal_taskENTER_CRITICAL();
    if (g_logger_enable_dram == 0) {
        enable_infra(dma_id, NO_WAIT);
        enable_AP_resource_from_isr(dma_id);
        g_logger_enable_dram = 1;
        ret = 1;
    } else {
        get_AP_ack_from_isr(dma_id);
    }
    kal_taskEXIT_CRITICAL();

    return ret;
}

void dvfs_disable_DRAM_no_wait_for_logger(scp_reserve_mem_id_t dma_id)
{
    kal_taskENTER_CRITICAL();
    disable_AP_resource(dma_id);
    disable_infra(dma_id);
    g_logger_enable_dram = 0;
    kal_taskEXIT_CRITICAL();
}

void enable_AP_resource_from_isr(scp_reserve_mem_id_t dma_id)
{
    /*DVFSFUC();*/

    if (dma_id >= NUMS_MEM_ID) {
        DVFSERR("%s: invalid id %d\n", __func__, dma_id);
        configASSERT(0);
        return;
    }

    dma_user_list[dma_id]++;
    ap_resource_req_cnt++;

    if (ap_resource_req_cnt == 1) {
        DRV_SetReg32(AP_RESOURCE, 0x1);
#if SCP_DDREN_AUTO_MODE == 0
        DRV_SetReg32(SYS_CTRL, 0x1 << DDREN_FIX_VALUE_BIT);
#endif
    } else
        DVFSLOG("DRAM is already ON.(ap_resource_req_cnt=%d)\n", (int)ap_resource_req_cnt);
}

void disable_AP_resource(scp_reserve_mem_id_t dma_id)
{
    /*DVFSFUC();*/

    if (dma_id >= NUMS_MEM_ID) {
        DVFSERR("%s: invalid id %d\n", __func__, dma_id);
        configASSERT(0);
        return;
    }

    if (dma_user_list[dma_id] == 0) {
        DVFSERR("dma_user_list[%d]==0\n", dma_id);
        configASSERT(0);
        return;
    }

    if (ap_resource_req_cnt == 0) {
        DVFSERR("ap_resource_req_cnt==0,id=%d\n", dma_id);
        configASSERT(0);
        return;
    }

    dma_user_list[dma_id]--;
    ap_resource_req_cnt--;

    if (ap_resource_req_cnt == 0) {
        DRV_ClrReg32(AP_RESOURCE, 0x1);
#if SCP_DDREN_AUTO_MODE == 0
        DRV_ClrReg32(SYS_CTRL, 0x1 << DDREN_FIX_VALUE_BIT);
#endif
        ap_resource_ack_cnt = 0;
#ifdef CFG_CACHE_SUPPORT
        disable_mpu_region(MPU_ID_DRAM_0);
        disable_mpu_region(MPU_ID_DRAM_1);
        hal_cache_scp_sleep_protect_on();
#endif
    }
}

void enable_infra(scp_reserve_mem_id_t dma_id, uint32_t wait_ack)
{
    uint32_t bus_ack;

    /*DVFSFUC("%d, %d", dma_id, wait_ack);*/

    if (dma_id >= NUMS_MEM_ID) {
        DVFSERR("%s:invalid id %d\n", __func__, dma_id);
        configASSERT(0);
        return;
    }

    infra_user_list[dma_id]++;
    infra_req_cnt++;

    if (infra_req_cnt == 1) {
        turn_on_clk_sys_from_isr(NO_WAIT);
        DRV_SetReg32(BUS_RESOURCE, 0x1);

        if (wait_ack == NEED_WAIT) {
#ifdef CFG_XGPT_SUPPORT
            udelay(REQ_INFRA_WAIT_TIME_US);
#endif
            bus_ack = wait_for_ack(BUS_RESOURCE);
            if (bus_ack == 1) {
                DVFSLOG("open clk_bus success.\n");
            } else
                DVFSLOG("wait_for_ack fail,id=%d\n", dma_id);
        }
    } else {
        DVFSLOG("infra is already ON.(infra_req_cnt=%d)\n", (int)infra_req_cnt);
    }
}

void disable_infra(scp_reserve_mem_id_t dma_id)
{
    /*DVFSFUC();*/

    if (dma_id >= NUMS_MEM_ID) {
        DVFSERR("%s:invalid id %d\n", __func__, dma_id);
        configASSERT(0);
        return;
    }

    if (infra_user_list[dma_id] == 0) {
        DVFSERR("infra_user_list[%d]==0\n", dma_id);
        configASSERT(0);
        return;
    }

    if (infra_req_cnt == 0) {
        DVFSERR("infra_req_cnt==0,id=%d\n", dma_id);
        configASSERT(0);
        return;
    }

    infra_user_list[dma_id]--;
    infra_req_cnt--;

    if (infra_req_cnt == 0) {
        DRV_ClrReg32(BUS_RESOURCE, 0x1);
        turn_off_clk_sys_from_isr();
    } else {
        DVFSLOG("keep clk_bus ON because infra_req_cnt %d != 0\n", (int)infra_req_cnt);
    }
}

/*****************************************
 * Sleep Control
 ****************************************/
void scp_wakeup_src_setup(wakeup_src_id irq, uint32_t enable)
{
    DVFSLOG("%s(%d,%d)\n",__FUNCTION__, (int)irq, (int)enable);

    kal_taskENTER_CRITICAL();

    if (irq >= NR_WAKE_CLK_CTRL) {
        DVFSERR("invalid wakeup_src_id %d\n", irq);
        configASSERT(0);
        kal_taskEXIT_CRITICAL();
        return;
    }

    if (enable) {
        if (irq <32)
            wakeup_src_mask |= (1 << irq);
        else
            wakeup_src_msb_mask |= (1 << (irq - 32));
    } else {
        if (irq <32)
            wakeup_src_mask &= ~(1 << irq);
        else
            wakeup_src_msb_mask &= ~(1 << (irq - 32));
    }

    kal_taskEXIT_CRITICAL();
}

/*
 * Only used if SPM Sleep Mode is enabled.
 * Set to allow SCP to enter halt state, but keep clock on always
 */
void set_CM4_CLK_AO(void)
{
    DVFSFUC();
    DRV_SetReg32(SLEEP_CTRL, 1 << SPM_SLP_MODE_CLK_AO_BIT);
}

/* Allow SPM clock request to be inactive when SCP is halted */
void disable_CM4_CLK_AO(void)
{
    DVFSFUC();
    DRV_ClrReg32(SLEEP_CTRL, 1 << SPM_SLP_MODE_CLK_AO_BIT);
}

/*
 * Enable SPM controlled Sleep mode.
 * In this mode, SPM will handle SCP sleep mode, wakeup and SRAM control.
 */
void use_SPM_ctrl_sleep_mode(void)
{
    DVFSFUC();
    DRV_SetReg32(SLEEP_CTRL, 1 << SPM_SLP_MODE_BIT);
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
    DVFSDBG("%s(%d)\n", __FUNCTION__, val);

    DRV_ClrReg32(WAKE_CKSW_SEL, WAKE_CKSW_SEL_NORMAL_MASK << WAKE_CKSW_SEL_NORMAL_BIT);
    DRV_SetReg32(WAKE_CKSW_SEL, val << WAKE_CKSW_SEL_NORMAL_BIT);
}

void wake_cksw_sel_slow(int val)
{
    DVFSDBG("%s(%d)\n", __FUNCTION__, val);

    DRV_ClrReg32(WAKE_CKSW_SEL, WAKE_CKSW_SEL_SLOW_MASK << WAKE_CKSW_SEL_SLOW_BIT);
    DRV_SetReg32(WAKE_CKSW_SEL, val << WAKE_CKSW_SEL_SLOW_BIT);
}

void unmask_all_int(void)
{
    /* restore interrupt setting */
    DRV_WriteReg32(IRQ_ENABLE, g_irq_enable_state);
    DRV_WriteReg32(IRQ_ENABLE_MSB, g_irq_msb_enable_state);

    DRV_ClrReg32(IRQ_SLEEP_EN, 0xffffffff);
    DRV_ClrReg32(IRQ_SLEEP_EN_MSB, 0xffffffff);
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

    /* enable wakeup source */
    if (scp_sleep_ctrl_flag == SCP_SLEEP_NO_WAKEUP || scp_sleep_ctrl_flag == SCP_SLEEP_NO_CONDITION) {
        DRV_ClrReg32(IRQ_SLEEP_EN, 0xffffffff);
        DRV_ClrReg32(IRQ_SLEEP_EN_MSB, 0xffffffff);
    } else {
        DRV_SetReg32(IRQ_SLEEP_EN, wakeup_src_mask);
        DRV_SetReg32(IRQ_SLEEP_EN_MSB, wakeup_src_msb_mask);
    }
}

void set_cm4_scr(void)
{
    /* DVFSFUC(); */

    /* Selected sleep state is deep sleep */
    DRV_SetReg32(ARMV7_SYS_CTRL, 0x1<<SLEEPDEEP_BIT);
}

void clr_cm4_scr(void)
{
    /* DVFSFUC(); */

    /* Selected sleep state is not deep sleep */
    DRV_ClrReg32(ARMV7_SYS_CTRL, 0x1<<SLEEPDEEP_BIT);
}

void scp_sleep_ctrl(int id, void* data, unsigned int len)
{
    uint8_t *msg = (uint8_t *)data;

    if (*msg == 0) {
        scp_sleep_ctrl_flag = SCP_SLEEP_OFF;
        DVFSDBG("set SCP_SLEEP_OFF\n");
    } else if (*msg == 1) {
        scp_sleep_ctrl_flag = SCP_SLEEP_ON;
        DVFSDBG("set SCP_SLEEP_ON\n");
    } else if (*msg == 2) {
        scp_sleep_ctrl_flag = SCP_SLEEP_NO_WAKEUP;
        DVFSDBG("set SCP_SLEEP_NO_WAKEUP\n");
    } else if (*msg == 3) {
        scp_sleep_ctrl_flag = SCP_SLEEP_NO_CONDITION;
        DVFSDBG("set SCP_SLEEP_FORCE_AND_NO_WAKEUP\n");
    } else
        DVFSDBG("invalid set value %d\n", *msg);
}

#if PMICW_SLEEP_REQ_FLOW
void pmicw_sleep_req(uint32_t enable)
{
    freq_enum cur_clk;
    uint32_t count_down;

    DVFSFUC("%d", (int)enable);

    if (enable == SCP_ENABLE) {
        DRV_WriteReg32(PMICW_CTRL, (DRV_Reg32(PMICW_CTRL) |
                                    (1<<PMICW_SLEEP_REQ_BIT)));

        cur_clk = get_cur_clk();
        count_down = (cur_clk != FREQ_UNINIT)?
                        (2000 * cur_clk) : (2000 * clk_opp_table[CLK_OPP_NUM-1]);

        BUSY_WAIT(((DRV_Reg32(PMICW_CTRL) & (0x1 << PMICW_SLEEP_ACK_BIT)) !=
                   (enable << PMICW_SLEEP_ACK_BIT)), count_down/6);

        if ((DRV_Reg32(PMICW_CTRL) & (0x1 << PMICW_SLEEP_ACK_BIT)) !=
            (enable << PMICW_SLEEP_ACK_BIT)) {
            DVFSERR("%s fail\n", __func__);
            configASSERT(0);
        }
    } else {
        DRV_WriteReg32(PMICW_CTRL, (DRV_Reg32(PMICW_CTRL) &
                                    ~(0x1<<PMICW_SLEEP_REQ_BIT)));
    }
}
#endif

#if AUTO_DUMP_SLEPP_STATUS
void dump_sleep_block_reason(void)
{
    int i;

    PRINTF_W("No sleep reasons: tmr=%lu, build=%lu, sema=%lu, lock=%lu, ipi=%lu, flag=%lu, slpbusy=%lu\n",
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

    if( scp_get_awake_semaphore(SEMAPHORE_SCP_A_AWAKE) ) {
        reason = BY_SEMAPHORE;
        goto out;
    }

    if( !wake_lock_is_empty() ) {
        reason = BY_WAKELOCK;
        goto out;
    }

    if ( is_ipi_busy() ) {
        reason = BY_IPI_BUSY;
        goto out;
    }

    /* Wait until SLEEP_CTRL[31] = 0x1 */
    BUSY_WAIT((DRV_Reg32(SLEEP_CTRL)&(0x1<<31)) == 0x0, 1000);
    if (scp_status == DVFS_STATUS_TIMEOUT) {
        reason = BY_SLP_BUSY;
        PRINTF_W("no sleep by sleep busy\n");
        goto out;
    }

out:
#if AUTO_DUMP_SLEPP_STATUS
    /* get current system tick */
    cur_time = timer_get_global_timer_tick();

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
            PRINTF_W("sleep: cnt=%d, total=%d\r\n", (int)g_sleep_cnt, (int)g_sleep_total_cnt);

            g_sleep_cnt = 0;
        }

        /* reset pre_sleep_block_time */
        pre_sleep_block_time = cur_time;
#else
        PRINTF_W("g_sleep_cnt = %d\r\n", (int)g_sleep_cnt);
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
    DVFSLOG("dump fast wakeup registers\n");
    DVFSLOG("[0x%x] = 0x%x\n", SLEEP_CTRL, DRV_Reg32(SLEEP_CTRL));
    DVFSLOG("[0x%x] = 0x%x\n", CLK_SYS_VAL, DRV_Reg32(CLK_SYS_VAL));
    DVFSLOG("[0x%x] = 0x%x\n", CLK_HIGH_VAL, DRV_Reg32(CLK_HIGH_VAL));
    DVFSLOG("[0x%x] = 0x%x\n", CLK_CTRL_SLP_CTRL, DRV_Reg32(CLK_CTRL_SLP_CTRL));
    DVFSLOG("[0x%x] = 0x%x\n", CLK_ON_CTRL, DRV_Reg32(CLK_ON_CTRL));
    DVFSLOG("[0x%x] = 0x%x\n", FAST_WAKE_CNT_END, DRV_Reg32(FAST_WAKE_CNT_END));
    DVFSLOG("[0x%x] = 0x%x\n", WAKE_CKSW_SEL, DRV_Reg32(WAKE_CKSW_SEL));
    DVFSLOG("[0x%x] = 0x%x\n", CLK_SEL_SLOW, DRV_Reg32(CLK_SEL_SLOW));
    DVFSLOG("[0x%x] = 0x%x\n", SLEEP_DEBUG, DRV_Reg32(SLEEP_DEBUG));
    DVFSLOG("[0x%x] = 0x%x\n", CLK_DIV_SEL, DRV_Reg32(CLK_DIV_SEL));
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
        pmicw_sleep_req(SCP_ENABLE);
#endif

        /* disable debug RG */
        DRV_ClrReg32(CM4_CONTROL, 1 << AP_DBG_REQ);

        mask_all_int();

        /* Clear sleep mode by set SLEEP_CTRL[0] = 0x0 */
        disable_SCP_sleep_mode();

        /* Enable Sleep mode: SLEEP_CTRL[0] = 0x1 */
        enable_SCP_sleep_mode();

        /* ARMV7 sleep state is deep sleep */
        set_cm4_scr();

        /* dump_fast_wakeup_setting(); */

#if SLEEP_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
    sleep_end_time = timer_get_global_timer_tick();

    PRINTF_W("wakeup sw time: %d => %d = %d\n", wakeup_start_time, wakeup_end_time, wakeup_end_time - wakeup_start_time);
    PRINTF_W("RTOS   sw time: %d => %d = %d\n", wakeup_end_time, sleep_start_time, sleep_start_time - wakeup_end_time);
    PRINTF_W("sleep  sw time: %d => %d = %d\n\n\n", sleep_start_time, sleep_end_time, sleep_end_time - sleep_start_time);
#endif

        DVFSDBG("SCP sleep\n");
    } else {
        enter_sleep_mode = 0;
        DVFSDBG("enter WFI\n");
    }
}

void post_sleep_process(uint32_t expect_time)
{
#if SLEEP_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
    wakeup_start_time = timer_get_global_timer_tick();
#endif

    if (enter_sleep_mode == 1) {
        DVFSDBG("SCP wakeup\n");

        wakeup_source_count(DRV_Reg32(IRQ_STATUS));

#if PMICW_SLEEP_REQ_FLOW
        pmicw_sleep_req(SCP_DISABLE);
#endif

        /* ARMV7 sleep state is NOT deep sleep */
        clr_cm4_scr();

        unmask_all_int();

#if SLEEP_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
    wakeup_end_time = timer_get_global_timer_tick();
#endif
    } else {
        DVFSDBG("exit WFI\n");
    }
}

void clkctrl_isr(void)
{
    DVFSFUC();

    /* Read CLK_IRQ_ACK register to acknowledge CLK_CTRL interrupt */
    DRV_Reg32(CLK_IRQ_ACK);

    NVIC_ClearPendingIRQ(CLK_CTRL_IRQn);
}

#if FAST_WAKEUP_SUPPORT
void clkctrl2_isr(void)
{
    DVFSFUC();

    /* ACK SLP_IRQ_2 (Write 1 clear) */
    DRV_WriteReg32(SLP_IRQ2, SLP_IRQ2_MASK << SLP_IRQ2_BIT);

    NVIC_ClearPendingIRQ(CLK_CTRL2_IRQn);
}
#endif

void dvfs_enable_DRAM_resource_from_isr(scp_reserve_mem_id_t dma_id)
{
    DVFSFUC("%d", (int)dma_id);
    enable_infra(dma_id, NO_WAIT);
    enable_AP_resource_from_isr(dma_id);
#ifdef CFG_XGPT_SUPPORT
    if (ap_resource_ack_cnt == 0)
        udelay(REQ_DRAM_WAIT_TIME_US);
#endif
    get_AP_ack_from_isr(dma_id);
}

