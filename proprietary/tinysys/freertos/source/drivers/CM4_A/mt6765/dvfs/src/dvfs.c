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
#include <sleep.h>
#include <dvfs.h>
#ifdef CFG_PMIC_WRAP_SUPPORT
#include "pmic_wrap.h"
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

#if DVFS_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
TickType_t dvfs_start_time, dvfs_end_time;
#endif

static uint32_t scp_status = DVFS_STATUS_OK;
int g_scp_dvfs_debug = 0;

freq_enum clk_opp_table[CLK_OPP_NUM] = {
    FREQ_250MHZ,
    FREQ_273MHZ,
    FREQ_330MHZ,
    FREQ_416MHZ
};

#ifdef CFG_TESTSUITE_SUPPORT
static int clk_pll_table[CLK_OPP_NUM] = {
    26,
    273,
    364,
    416
};
#endif

#if defined(MT6761)
#define ULPOSC1_CAL_MIN_VALUE   50
#elif defined(MT6765)
#define ULPOSC1_CAL_MIN_VALUE   6
#else
#error "undefined platform"
#endif

#define ULPOSC1_CAL_MAX_VALUE   120
#define ULPOSC1_CAL_START_VALUE ((ULPOSC1_CAL_MIN_VALUE + ULPOSC1_CAL_MAX_VALUE)/2)

#define ULPOSC2_CAL_MIN_VALUE   6
#define ULPOSC2_CAL_MAX_VALUE   120
#define ULPOSC2_CAL_START_VALUE ((ULPOSC2_CAL_MIN_VALUE + ULPOSC2_CAL_MAX_VALUE)/2)

ulposc_con_t ulposc_setting[ULPOSC_NUM] = {
    {.osc_cali = ULPOSC1_CAL_START_VALUE},
    {.osc_cali = ULPOSC2_CAL_START_VALUE},
};

uint32_t clk_sys_req_cnt = 0;
int clk_sys_is_on_by_dvfs = 0;

#if SW_CTRL_FOR_VCORE_READY
int clk_sys_on_for_workaround_opp2 = 0;
#endif

#ifdef SUPPORT_PMIC_VOW_LP_MODE
static pmic_vow_lpmode_state_t Vow_lpmode_st = {
    VOW_LPMODE_OFF, VOW_LPMODE_OFF };

#ifdef CFG_PMIC_WRAP_SUPPORT
static void set_pmic_sshub_sleep_en(int enable);
#endif

static void set_pmic_vow_lp_mode(int enabled);
#endif

#if AUTO_DUMP_SLEPP_STATUS
extern uint32_t g_sleep_cnt, g_sleep_total_cnt;
#endif

#ifdef CFG_CORE_BENCHMARK_SUPPORT
extern void run_coremark(void);
#endif

/*****************************************
 * CLK Inform Dump APIs
 ****************************************/
freq_enum convert_opp_2_clk(clk_opp_enum clk_opp)
{
    if (clk_opp == CLK_OPP_UNINIT)
        return FREQ_UNINIT;

    if (clk_opp < CLK_OPP0 || clk_opp >= CLK_OPP_NUM) {
        PRINTF_E("err opp %d\n", clk_opp);
        configASSERT(0);
        return FREQ_UNINIT;
    }

    return clk_opp_table[clk_opp];
}

#ifdef CFG_TESTSUITE_SUPPORT
int convert_opp_2_pll(clk_opp_enum clk_opp)
{
    return clk_pll_table[clk_opp];
}
#endif

clk_opp_enum convert_clk_2_opp(freq_enum clk_freq)
{
    int i;

    if (clk_freq == FREQ_UNINIT)
        return CLK_OPP_UNINIT;

    for (i = 0; i < CLK_OPP_NUM; i++)
        if (clk_opp_table[i] == clk_freq)
            break;

    if (i >= CLK_OPP_NUM) {
        PRINTF_E("err freq %d\n", clk_freq);
        configASSERT(0);
    }

    return i;
}

freq_enum get_cur_clk(void)
{
    return DRV_Reg32(CURRENT_FREQ_REG);
}

/*****************************************
 * Clock Control APIs
 ****************************************/
void set_clk_sys_val(unsigned int val)
{
    DVFSFUC("%d", val);

    /* CLK_SYS_VAL */
    DRV_ClrReg32(CLK_SYS_VAL, CLK_SYS_VAL_MASK << CLK_SYS_VAL_BIT);
    DRV_SetReg32(CLK_SYS_VAL, val << CLK_SYS_VAL_BIT);
}

void set_clk_high_val(unsigned int val)
{
    DVFSFUC("%d", val);

    /* CLK_HIGH_VAL */
    DRV_ClrReg32(CLK_HIGH_VAL, CLK_HIGH_VAL_MASK << CLK_HIGH_VAL_BIT);
    DRV_SetReg32(CLK_HIGH_VAL, val << CLK_HIGH_VAL_BIT);
}

uint32_t get_clk_sys_ack(void)
{
    return ((DRV_Reg32(CLK_ACK) & (1 << CLK_SYS_ACK_BIT)) ? 1 : 0);
}

uint32_t get_clk_sys_safe_ack(void)
{
    return ((DRV_Reg32(CLK_SAFE_ACK) & (1 << CLK_SYS_SAFE_ACK_BIT)) ? 1 : 0);
}

/* select clock source */
void select_clk_source(clk_sel_val val)
{
    DVFSFUC("0x%x", val);
    DRV_WriteReg32(CLK_SW_SEL, val);
}

void clk_div_select(uint32_t val)
{
    DVFSFUC("0x%x", (unsigned int)val);
    DRV_WriteReg32(CLK_DIV_SEL, val);
}

freq_enum get_expect_clk(void)
{
    return drv_reg32(EXPECTED_FREQ_REG);
}

clk_opp_enum get_expect_opp(void)
{
    return (convert_clk_2_opp(get_expect_clk()));
}

void set_cur_opp(clk_opp_enum cur_opp)
{
    uint32_t cur_freq;

    cur_freq = convert_opp_2_clk(cur_opp);
    DRV_WriteReg32(CURRENT_FREQ_REG, cur_freq);
}

void turn_on_clk_high(ulposc_id_enum ulposc)
{
    DVFSFUC("%d", (int)ulposc);

    /* turn on ulposc */
    DRV_SetReg32(CLK_ENABLE, (1 << CLK_HIGH_EN_BIT));
    if (ulposc == ULPOSC_2)
        DRV_ClrReg32(CLK_ON_CTRL, (1 << HIGH_CORE_DIS_SUB_BIT));

#ifdef CFG_XGPT_SUPPORT
    /* wait settle time at least 25ms got from DE */
    udelay(REQ_ULPOSC_WAIT_TIME_US);
#endif

    /* turn on CG */
    if (ulposc == ULPOSC_2)
        DRV_SetReg32(CLK_HIGH_CORE, (1 << HIGH_CORE_CG_BIT));
    else
        DRV_SetReg32(CLK_ENABLE, (1 << CLK_HIGH_CG_BIT));

#ifdef CFG_XGPT_SUPPORT
    udelay(50);
#endif
}

void turn_off_clk_high(ulposc_id_enum ulposc)
{
    DVFSFUC("%d", ulposc);

    /* turn off CG */
    if (ulposc == ULPOSC_2)
        DRV_ClrReg32(CLK_HIGH_CORE, (1 << HIGH_CORE_CG_BIT));
    else
        DRV_ClrReg32(CLK_ENABLE, (1 << CLK_HIGH_CG_BIT));

#ifdef CFG_XGPT_SUPPORT
    udelay(50);
#endif

    /* turn off ulposc */
    if (ulposc == ULPOSC_2)
        DRV_SetReg32(CLK_ON_CTRL, (1 << HIGH_CORE_DIS_SUB_BIT));
    else
        DRV_ClrReg32(CLK_ENABLE, (1 << CLK_HIGH_EN_BIT));

#ifdef CFG_XGPT_SUPPORT
    udelay(50);
#endif
}

void turn_on_clk_high_cg(ulposc_id_enum ulposc)
{
    DVFSFUC();

    if (ulposc == ULPOSC_2)
        DRV_SetReg32(CLK_HIGH_CORE, (1 << HIGH_CORE_CG_BIT));
    else
        DRV_SetReg32(CLK_ENABLE, (1 << CLK_HIGH_CG_BIT));

#ifdef CFG_XGPT_SUPPORT
    udelay(50);
#endif
}

void turn_off_clk_high_cg(ulposc_id_enum ulposc)
{
    DVFSFUC();

    if (ulposc == ULPOSC_2)
        DRV_ClrReg32(CLK_HIGH_CORE, (1 << HIGH_CORE_CG_BIT));
    else
        DRV_ClrReg32(CLK_ENABLE, (1 << CLK_HIGH_CG_BIT));

#ifdef CFG_XGPT_SUPPORT
    udelay(50);
#endif
}

uint32_t get_clk_sw_select(void)
{
    int32_t i, clk_val = 0;

    clk_val = (DRV_Reg32(CLK_SW_SEL) >> CKSW_SEL_O_BIT) & CKSW_SEL_O_MASK;

    for (i = 0; i < 4; i++) {
        if (clk_val == (0x1 << i)) {
            break;
        }
    }

    if (i >= 4) {
        PRINTF_E("err CLK_SW_SEL=0x%x\n", DRV_Reg32(CLK_SW_SEL));
        configASSERT(0);
    }

    return i;
}

void turn_off_clk_sys(void)
{
    DVFSFUC();

    kal_taskENTER_CRITICAL();

    if (clk_sys_req_cnt == 0) {
        PRINTF_E("clk_sys_req_cnt==0\n");
        configASSERT(0);
    }

    clk_sys_req_cnt--;

    if (clk_sys_req_cnt == 0) {
        /* Turn off sys Clock Source */
        DRV_ClrReg32(CLK_ENABLE, 1 << CLK_SYS_EN_BIT);
    }

    kal_taskEXIT_CRITICAL();
}

uint32_t turn_on_clk_sys(void)
{
    volatile uint32_t count_down = 0;
    freq_enum cur_clk;

    DVFSFUC();

    kal_taskENTER_CRITICAL();

    clk_sys_req_cnt++;
    if (clk_sys_req_cnt == 1) {
        /* enable sys clock */
        DRV_SetReg32(CLK_ENABLE, 1 << CLK_SYS_EN_BIT);

#ifdef CFG_XGPT_SUPPORT
        udelay(REQ_26M_WAIT_TIME_US);
#endif

        /* max settle time is 5ms to turn on 26MHz */
        cur_clk = get_cur_clk();
        count_down = (cur_clk != FREQ_UNINIT)? (5000 * cur_clk) : (5000 * clk_opp_table[CLK_OPP_NUM-1]);

        BUSY_WAIT((get_clk_sys_ack() == 0 || get_clk_sys_safe_ack() == 0), count_down);
        if (scp_status == DVFS_STATUS_TIMEOUT) {
            PRINTF_E("ack1=%d, ack2=%d\n", (int)get_clk_sys_ack(), (int)get_clk_sys_safe_ack());
            configASSERT(0);
        }
    } else {
        DVFSLOG("clk-sys is already ON.(clk_sys_req_cnt=%d)\n", (int)clk_sys_req_cnt);
    }

    kal_taskEXIT_CRITICAL();

    return scp_status;
}

void turn_off_clk_sys_from_isr(void)
{
    /*DVFSFUC();*/

    if (clk_sys_req_cnt == 0) {
        PRINTF_E("clk_sys_req_cnt==0\n");
        configASSERT(0);
    }

    clk_sys_req_cnt--;

    if (clk_sys_req_cnt == 0) {
        /* Turn off sys Clock Source */
        DRV_ClrReg32(CLK_ENABLE, 1 << CLK_SYS_EN_BIT);
    }
}

uint32_t turn_on_clk_sys_from_isr(uint32_t wait_ack)
{
    volatile uint32_t count_down = 0;
    freq_enum cur_clk;

    /*DVFSFUC();*/

    clk_sys_req_cnt++;
    if (clk_sys_req_cnt == 1) {
        /* enable sys clock */
        DRV_SetReg32(CLK_ENABLE, 1 << CLK_SYS_EN_BIT);

        if (wait_ack == NEED_WAIT) {
#ifdef CFG_XGPT_SUPPORT
            udelay(REQ_26M_WAIT_TIME_US);
#endif

            /* max settle time is 5ms to turn on 26MHz */
            cur_clk = get_cur_clk();
            count_down = (cur_clk != FREQ_UNINIT)? (5000 * cur_clk) : (5000 * clk_opp_table[CLK_OPP_NUM-1]);
            BUSY_WAIT((get_clk_sys_ack() == 0 || get_clk_sys_safe_ack() == 0), count_down);
            if (scp_status == DVFS_STATUS_TIMEOUT) {
                PRINTF_E("ack1=%d, ack2=%d\n", (int)get_clk_sys_ack(), (int)get_clk_sys_safe_ack());
                configASSERT(0);
            }
        }
    } else {
        DVFSLOG("clk-sys is already ON.(clk_sys_req_cnt=%d)\n", (int)clk_sys_req_cnt);
    }

    return scp_status;
}

#ifdef CFG_TESTSUITE_SUPPORT
static void pll_opp_switch(unsigned int opp)
{
    DRV_WriteReg32(CLK_CFG_0_CLR, CLK_SCP_SEL_MSK << CLK_SCP_SEL_BIT);
    DRV_WriteReg32(CLK_CFG_0_SET, (opp & CLK_SCP_SEL_MSK) << CLK_SCP_SEL_BIT);
    DRV_WriteReg32(CLK_CFG_UPDATE, 0x1 << SCP_CK_UPDATE_BIT);
}

void request_clk_sys_opp(uint32_t req_opp)
{
    DVFSFUC("%d", (int)req_opp);

    switch (req_opp) {
        case CLK_OPP0:
            pll_opp_switch(0); /* 26 MHz */
            break;
        case CLK_OPP1:
            pll_opp_switch(3); /* 273 MHz */
            break;
        case CLK_OPP2:
            pll_opp_switch(5); /* 364 MHz */
            break;
        case CLK_OPP3:
            pll_opp_switch(6); /* 416 MHz */
            break;
        default:
            PRINTF_E("err opp %d\n", (int)req_opp);
            configASSERT(0);
            break;
    }
}
#endif

void request_dfs_opp(uint32_t req_opp)
{
    DVFSFUC("%d", (int)req_opp);

    switch ((clk_opp_enum)req_opp) {
        case CLK_OPP0: /* 250M (=ulposc1) */
            /* switch to ulposc1 */
            clk_div_select(CLK_DIV_1);
            select_clk_source(CLK_SEL_ULPOSC1);

            /* turn off ulposc2 */
            turn_off_clk_high(ULPOSC_2);

            /* turn off clk-sys if needed */
            if (clk_sys_is_on_by_dvfs == 1) {
                clk_sys_is_on_by_dvfs = 0;
                turn_off_clk_sys_from_isr();
            }

#if SW_CTRL_FOR_VCORE_READY
            if (clk_sys_on_for_workaround_opp2 == 1) {
                turn_off_clk_sys_from_isr();
                clk_sys_on_for_workaround_opp2 = 0;
            }
#endif
            break;

        case CLK_OPP1: /* 273M (mainpll) */
            /* turn on clk-sys */
            if (clk_sys_is_on_by_dvfs == 0) {
                clk_sys_is_on_by_dvfs = 1;
                turn_on_clk_sys_from_isr(NEED_WAIT);
#ifdef CFG_TESTSUITE_SUPPORT
                request_clk_sys_opp(CLK_OPP1);
#endif
            }

            /* switch to clk-sys */
            clk_div_select(CLK_DIV_1);
            select_clk_source(CLK_SEL_SYS);

            /* turn off ulposc2 */
            turn_off_clk_high(ULPOSC_2);

#if SW_CTRL_FOR_VCORE_READY
            if (clk_sys_on_for_workaround_opp2 == 1) {
                turn_off_clk_sys_from_isr();
                clk_sys_on_for_workaround_opp2 = 0;
            }
#endif
            break;

        case CLK_OPP2: /* 330M (=ulposc2) */
            /* turn on ulposc2 */
            turn_on_clk_high(ULPOSC_2);

            /* switch to ulposc2 */
            clk_div_select(CLK_DIV_1);
            select_clk_source(CLK_SEL_ULPOSC2);

#if SW_CTRL_FOR_VCORE_READY
            if (clk_sys_on_for_workaround_opp2 == 0) {
                turn_on_clk_sys_from_isr(NO_WAIT);
                clk_sys_on_for_workaround_opp2 = 1;
            }
#endif

            /* turn off clk-sys if needed */
            if (clk_sys_is_on_by_dvfs == 1) {
                clk_sys_is_on_by_dvfs = 0;
                turn_off_clk_sys_from_isr();
            }
            break;

        case CLK_OPP3: /* 416M (univpll) */
            /* turn on clk-sys */
            if (clk_sys_is_on_by_dvfs == 0) {
                clk_sys_is_on_by_dvfs = 1;
                turn_on_clk_sys_from_isr(NEED_WAIT);
#ifdef CFG_TESTSUITE_SUPPORT
                request_clk_sys_opp(CLK_OPP3);
#endif
            }

            /* switch to clk-sys */
            clk_div_select(CLK_DIV_1);
            select_clk_source(CLK_SEL_SYS);

            /* turn off ulposc2 */
            turn_off_clk_high(ULPOSC_2);

#if SW_CTRL_FOR_VCORE_READY
            if (clk_sys_on_for_workaround_opp2 == 1) {
                turn_off_clk_sys_from_isr();
                clk_sys_on_for_workaround_opp2 = 0;
            }
#endif
            break;

        default:
            PRINTF_E("err opp %d\n", (int)req_opp);
            configASSERT(0);
            break;
    }
}

void scp_set_opp(void)
{
    uint32_t req_opp;
    clk_opp_enum cur_opp;

#if DVFS_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
    dvfs_start_time = timer_get_global_timer_tick();
#endif

    DVFSFUC();

    req_opp = get_expect_opp();
    cur_opp = convert_clk_2_opp(get_cur_clk());

    DVFSLOG("DVFS request: %d => %d\n", (int)cur_opp, (int)req_opp);

    if (cur_opp != req_opp || cur_opp == CLK_OPP_UNINIT) {
        request_dfs_opp(req_opp);
        set_cur_opp(req_opp);

        if (req_opp == CLK_OPP0)
            set_vreq_count(0x1);
        else
            set_vreq_count(0xA);

#ifdef SUPPORT_PMIC_VOW_LP_MODE
        if (req_opp == clk_opp_table[CLK_OPP0]) {
            if (Vow_lpmode_st.cur_state == VOW_LPMODE_OFF &&
                Vow_lpmode_st.req_state == VOW_LPMODE_ON)
                set_pmic_vow_lp_mode(SCP_ENABLE);
        } else {
            if (Vow_lpmode_st.cur_state == VOW_LPMODE_ON)
                set_pmic_vow_lp_mode(SCP_DISABLE);
        }
#endif
    }

#if DVFS_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
    dvfs_end_time = timer_get_global_timer_tick();
    PRINTF_W("DVFS profiling: %d => %d = %d\n",
        dvfs_start_time, dvfs_end_time, dvfs_end_time - dvfs_start_time);
#endif
}

uint32_t get_ulposc_clk_by_freqmeter(uint32_t cali_val, ulposc_id_enum ulposc)
{
    unsigned int i = 0, result = 0;
    unsigned int clk_misc_cfg_0, clk_dbg_cfg, clk_scp_cfg_0, clk_scp_cfg_1;
    unsigned int tmp;

    DVFSFUC();

    if (ulposc == ULPOSC_1) {
        DRV_ClrReg32(ULPOSC1_CON0, RG_OSC1_CALI_MSK << RG_OSC1_CALI_BIT);
        DRV_SetReg32(ULPOSC1_CON0, (cali_val & RG_OSC1_CALI_MSK) << RG_OSC1_CALI_BIT);
    } else {
        DRV_ClrReg32(ULPOSC2_CON0, RG_OSC2_CALI_MSK << RG_OSC2_CALI_BIT);
        DRV_SetReg32(ULPOSC2_CON0, (cali_val & RG_OSC2_CALI_MSK) << RG_OSC2_CALI_BIT);
    }

#ifdef CFG_XGPT_SUPPORT
    udelay(50);
#endif

    /* backup freq meter RGs */
    clk_misc_cfg_0 = DRV_Reg32(CLK_MISC_CFG_0);
    clk_dbg_cfg = DRV_Reg32(CLK_DBG_CFG);
    clk_scp_cfg_0 = DRV_Reg32(CLK_SCP_CFG_0);
    clk_scp_cfg_1 = DRV_Reg32(CLK_SCP_CFG_1);

    /* Select meter clock input: CLK_DBG_CFG[1:0] = 0x0 */
    DRV_ClrReg32(CLK_DBG_CFG, 0x3);

    if (ulposc == ULPOSC_1)
        tmp = FREQ_METER_ABIST_AD_OSC_CK;
    else
        tmp =FREQ_METER_ABIST_AD_OSC_CK_2;

    /* Select clock source: CLK_DBG_CFG[21:16] */
    DRV_ClrReg32(CLK_DBG_CFG, 0x3f<<16);
    DRV_SetReg32(CLK_DBG_CFG, tmp<<16);

    /* Setup meter div: CLK_MISC_CFG_0[31:24] = 0x0 */
    DRV_ClrReg32(CLK_MISC_CFG_0, 0xff<<24);

    /* Enable frequency meter: CLK_SCP_CFG_0[12] = 0x1 */
    DRV_SetReg32(CLK_SCP_CFG_0, 0x1<<12);

    /* Trigger frequency meter: CLK_SCP_CFG_0[4] = 0x1 */
    DRV_SetReg32(CLK_SCP_CFG_0, 0x1<<4);

#ifdef CFG_XGPT_SUPPORT
    udelay(60);
#endif

    /* wait frequency done */
    while (DRV_Reg32(CLK_SCP_CFG_0) & (0x1<<4)) {
        i++;
        if (i > 1000000) {
            PRINTF_E("freq-meter timeout!\n");
            configASSERT(0);
            break;
        }
    }

    /* Disable frequency meter */
    DRV_ClrReg32(CLK_SCP_CFG_0, 0x1<<12);

    /* get freq meter result */
    result = DRV_Reg32(CLK_SCP_CFG_1) & 0xffff;

    /* restore freq meter RGs */
    DRV_WriteReg32(CLK_MISC_CFG_0, clk_misc_cfg_0);
    DRV_WriteReg32(CLK_DBG_CFG, clk_dbg_cfg);
    DRV_WriteReg32(CLK_SCP_CFG_0, clk_scp_cfg_0);
    DRV_WriteReg32(CLK_SCP_CFG_1, clk_scp_cfg_1);

    DVFSLOG("cali_val=%d, freq=%d MHz\n",
        (unsigned int)cali_val, (result * 26) / 1024);

    return result;
}

uint32_t ulposc_cali_process(ulposc_id_enum ulposc_id, freq_enum target_clk)
{
    uint32_t target_val = 0, current_val = 0;
    uint32_t min, max, middle;
    uint32_t diff_by_min = 0, diff_by_max = 0xffff;
    uint32_t cal_result = 0;

    DVFSFUC();

    if (ulposc_id == ULPOSC_1) {
        min = ULPOSC1_CAL_MIN_VALUE;
        max = ULPOSC1_CAL_MAX_VALUE;
    } else {
        min = ULPOSC2_CAL_MIN_VALUE;
        max = ULPOSC2_CAL_MAX_VALUE;
    }

    target_val = target_clk * 1024 / 26;

    do {
        middle = (min + max) / 2;
        if (middle == min) {
            DVFSLOG("middle(%d) == min(%d)\n", middle, min);
            break;
        }

        current_val = get_ulposc_clk_by_freqmeter(middle, ulposc_id);

        if (current_val > target_val)
            max = middle;
        else
            min = middle;
    } while (min <= max);

    current_val = get_ulposc_clk_by_freqmeter(min, ulposc_id);
    if (current_val > target_val)
        diff_by_min = current_val - target_val;
    else
        diff_by_min = target_val - current_val;

    current_val = get_ulposc_clk_by_freqmeter(max, ulposc_id);
    if (current_val > target_val)
        diff_by_max = current_val - target_val;
    else
        diff_by_max = target_val - current_val;

    if (diff_by_min < diff_by_max) {
        cal_result = min;
        current_val = get_ulposc_clk_by_freqmeter(min, ulposc_id);
    } else {
        cal_result = max;
        current_val = get_ulposc_clk_by_freqmeter(max, ulposc_id);
    }

    /* check if calibrated value is in the range of target value +- 15% */
    if (current_val > (target_val*850/1000) && current_val < (target_val*1150/1000)) {
        PRINTF_W("ULPOSC%d K done: %dM\n", ulposc_id, current_val*26/1024);
    } else {
        PRINTF_E("ULPOSC%d K fail: %dM\n", ulposc_id,  current_val*26/1024);
        configASSERT(0);
    }

    return cal_result;
}

void config_ulposc1_registers(void)
{
    DVFSFUC();

    /*
     **** Config ULPOSC1_CON0 ***
     */

    /* OSC1_CP_EN = 1 */
    DRV_SetReg32(ULPOSC1_CON0, 1 << RG_OSC1_CP_EN_BIT);

    /* OSC1_DIV */
    DRV_ClrReg32(ULPOSC1_CON0, RG_OSC1_DIV_MSK << RG_OSC1_DIV_BIT);
#if defined(MT6761)
    DRV_SetReg32(ULPOSC1_CON0, 9 << RG_OSC1_DIV_BIT);
#elif defined(MT6765)
    DRV_SetReg32(ULPOSC1_CON0, 12 << RG_OSC1_DIV_BIT);
#endif

    /* OSC1_FBAND */
    DRV_ClrReg32(ULPOSC1_CON0, RG_OSC1_FBAND_MSK << RG_OSC1_FBAND_BIT);
#if defined(MT6761)
    DRV_SetReg32(ULPOSC1_CON0, 4 << RG_OSC1_FBAND_BIT);
#elif defined(MT6765)
    DRV_SetReg32(ULPOSC1_CON0, 0 << RG_OSC1_FBAND_BIT);
#endif

    /* OSC1_IBAND = 0000100 */
    DRV_ClrReg32(ULPOSC1_CON0, RG_OSC1_IBAND_MSK << RG_OSC1_IBAND_BIT);
    DRV_SetReg32(ULPOSC1_CON0, 4 << RG_OSC1_IBAND_BIT);

    /* OSC1_CALI */
    DRV_ClrReg32(ULPOSC1_CON0, RG_OSC1_CALI_MSK << RG_OSC1_CALI_BIT);
    DRV_SetReg32(ULPOSC1_CON0,
        (ulposc_setting[ULPOSC_1].osc_cali & RG_OSC1_CALI_MSK) << RG_OSC1_CALI_BIT);

    /*
     **** Config ULPOSC1_CON1 ***
     */

    /* OSC1_CALTAP = 011 */
    DRV_ClrReg32(ULPOSC1_CON1, RG_OSC1_CALTAP_MSK << RG_OSC1_CALTAP_BIT);
    DRV_SetReg32(ULPOSC1_CON1, 3 << RG_OSC1_CALTAP_BIT);

    /* OSC1_CALMOS = 100 */
    DRV_ClrReg32(ULPOSC1_CON1, RG_OSC1_CALMOS_MSK << RG_OSC1_CALMOS_BIT);
    DRV_SetReg32(ULPOSC1_CON1, 4 << RG_OSC1_CALMOS_BIT);

    /* OSC1_DIV2_EN = 1 */
    DRV_SetReg32(ULPOSC1_CON1, 1 << RG_OSC1_DIV2_EN_BIT);

    /* OSC1_MOD = 00 */
    DRV_ClrReg32(ULPOSC1_CON1, RG_OSC1_MOD_MSK << RG_OSC1_MOD_BIT);

    /* OSC1_RSV = 00000000 */
    DRV_ClrReg32(ULPOSC1_CON1, RG_OSC1_RSV_MSK << RG_OSC1_RSV_BIT);

    DVFSLOG("ULPOSC1_CON0 = 0x%x, ULPOSC1_CON1 = 0x%x\n",
        DRV_Reg32(ULPOSC1_CON0), DRV_Reg32(ULPOSC1_CON1));
}

void config_ulposc2_registers(void)
{
    DVFSFUC();

	/*
     **** Config ULPOSC2_CON0 ***
     */

    /* OSC2_CP_EN = 1 */
    DRV_SetReg32(ULPOSC2_CON0, 1 << RG_OSC2_CP_EN_BIT);

    /* set OSC2_DIV */
    DRV_ClrReg32(ULPOSC2_CON0, RG_OSC2_DIV_MSK << RG_OSC2_DIV_BIT);
#if defined(MT6761)
    DRV_SetReg32(ULPOSC2_CON0, 13 << RG_OSC2_DIV_BIT);
#elif defined(MT6765)
    DRV_SetReg32(ULPOSC2_CON0, 16 << RG_OSC2_DIV_BIT);
#endif

    /* OSC2_FBAND */
    DRV_ClrReg32(ULPOSC2_CON0, RG_OSC2_FBAND_MSK << RG_OSC2_FBAND_BIT);
#if defined(MT6761)
    DRV_SetReg32(ULPOSC2_CON0, 12 << RG_OSC2_FBAND_BIT);
#elif defined(MT6765)
    DRV_SetReg32(ULPOSC2_CON0, 0 << RG_OSC2_FBAND_BIT);
#endif

    /* OSC2_IBAND = 0000100 */
    DRV_ClrReg32(ULPOSC2_CON0, RG_OSC2_IBAND_MSK << RG_OSC2_IBAND_BIT);
    DRV_SetReg32(ULPOSC2_CON0, 4 << RG_OSC2_IBAND_BIT);

    /* OSC2_CALI */
    DRV_ClrReg32(ULPOSC2_CON0, RG_OSC2_CALI_MSK << RG_OSC2_CALI_BIT);
    DRV_SetReg32(ULPOSC2_CON0,
        (ulposc_setting[ULPOSC_2].osc_cali & RG_OSC2_CALI_MSK) << RG_OSC2_CALI_BIT);

    /*
     **** Config ULPOSC2_CON1 ***
     */

    /* OSC2_CALTAP = 011 */
    DRV_ClrReg32(ULPOSC2_CON1, RG_OSC2_CALTAP_MSK << RG_OSC2_CALTAP_BIT);
    DRV_SetReg32(ULPOSC2_CON1, 3 << RG_OSC2_CALTAP_BIT);

    /* OSC2_CALMOS = 100 */
    DRV_ClrReg32(ULPOSC2_CON1, RG_OSC2_CALMOS_MSK << RG_OSC2_CALMOS_BIT);
    DRV_SetReg32(ULPOSC2_CON1, 4 << RG_OSC2_CALMOS_BIT);

    /* OSC2_DIV2_EN = 1 */
    DRV_SetReg32(ULPOSC2_CON1, 1 << RG_OSC2_DIV2_EN_BIT);

    /* OSC2_MOD = 00 */
    DRV_ClrReg32(ULPOSC2_CON1, RG_OSC2_MOD_MSK << RG_OSC2_MOD_BIT);

    /* OSC2_RSV = 00000000 */
    DRV_ClrReg32(ULPOSC2_CON1, RG_OSC2_RSV_MSK << RG_OSC2_RSV_BIT);

    DVFSLOG("ULPOSC2_CON0 = 0x%x, ULPOSC2_CON1 = 0x%x\n",
        DRV_Reg32(ULPOSC2_CON0), DRV_Reg32(ULPOSC2_CON1));
}

void ulposc_cali_init(void)
{
    /* By default, ULPOSC2 is OFF even though ULPOSC1 is ON */
    DRV_SetReg32(CLK_ON_CTRL, 1<<HIGH_CORE_DIS_SUB_BIT);

#if DO_ULPOSC1_CALIBRATION
    /* calibrate ULPOSC1 */
    config_ulposc1_registers();
    turn_on_clk_high(ULPOSC_1);
    ulposc_setting[ULPOSC_1].osc_cali = ulposc_cali_process(ULPOSC_1, FREQ_250MHZ);
#else
    turn_on_clk_high(ULPOSC_1);
#endif

#if DO_ULPOSC2_CALIBRATION
    /* calibrate ULPOSC2 */
    config_ulposc2_registers();
    turn_on_clk_high(ULPOSC_2);
    ulposc_setting[ULPOSC_2].osc_cali = ulposc_cali_process(ULPOSC_2, FREQ_330MHZ);
#else
    turn_on_clk_high(ULPOSC_2);
#endif

#ifndef CFG_TESTSUITE_SUPPORT
    request_dfs_opp(CLK_OPP0);
    set_cur_opp(CLK_OPP_UNINIT);
#endif
}

#if DUMMY_TASK_TEST
static void vDvfsMainTask(void *pvParameters)
{
#ifdef CFG_CORE_BENCHMARK_SUPPORT
    taskENTER_CRITICAL();
    while (1) {
        run_coremark();
    }
    taskEXIT_CRITICAL();
#else
    int i = 0;

    while (1) {
        vTaskDelay(DVFS_TASK_DELAY / portTICK_RATE_MS);
        i++;
        PRINTF_W("\n%u sec passed\n", i*DVFS_TASK_DELAY/1000);
    }
#endif
}
#endif

void set_vreq_count(unsigned int val)
{
    DVFSFUC();

    DRV_ClrReg32(SLEEP_CTRL, VREQ_COUNT_MASK << VREQ_COUNT_BIT);
    DRV_SetReg32(SLEEP_CTRL, val << VREQ_COUNT_BIT);
}

void reset_dvfs_global_variables(void)
{
    /* no need to reset global variables anymore */
}

void reset_dvfs_hw_registers(void)
{
    DRV_WriteReg32(CLK_SW_SEL, 0x0);
    DRV_WriteReg32(CLK_DIV_SEL, 0x0);
}

void dvfs_init(void)
{
    DVFSFUC();

    /* SCP HW RG will not be reset by scp_reset mechanism, so SW
       must reset these clkctrl registers before dvfs init; otherwise,
       SCP will hang once if clk_high2 is gated and CM4 still uses it. */
    reset_dvfs_hw_registers();

    /* disable debug RG; otherwise, SCP will not enter sleep */
    DRV_ClrReg32(CM4_CONTROL, 1 << JTAG_DBG_REQ_BIT);

    /* set Mux of v-req to PMIC and DVFSRC */
    DRV_WriteReg32(CPU_VREQ_CTRL, 0x10001);
    DRV_ClrReg32(SCP_SECURE_CRTL, 0x1<<SCP_ENABLE_SPM_MASK_VREQ_BIT);

    /* set Mux of v-req to PMICW */
    DRV_ClrReg32(CLK_CTRL_GENERAL_CTRL, 0x1<<1);

    /* release the PMICW-sleep-request set by earlier booting code */
    pmicw_sleep_req(SCP_DISABLE, NO_WAIT);

#if SCP_DDREN_AUTO_MODE
    /* Set scp ddren to HW mode */
    DRV_SetReg32(SYS_CTRL, 0x1<<AUTO_DDREN_BIT);
#endif

    /* set settle time */
    set_clk_sys_val(0x1);
    set_clk_high_val(0x1);
    set_vreq_count(0x1);

    wake_cksw_sel_normal(WAKE_CKSW_SEL_NORMAL_CLK_HIGH);

#if FAST_WAKEUP_SUPPORT
    PRINTF_W("use fast-wakeup\n");

    /* CLK_CTRL_SLP_CTRL 0xC40A0 = 0x0 */
    DRV_WriteReg32(CLK_CTRL_SLP_CTRL, 0x0);

    /* HIGH_FINAL_VAL 0xC406C[12:8] = 0x3 */
    DRV_ClrReg32(CLK_ON_CTRL, HIGH_FINAL_VAL_MASK << HIGH_FINAL_VAL_BIT);
    DRV_SetReg32(CLK_ON_CTRL, 0x3 << HIGH_FINAL_VAL_BIT);

    /* FAST_WAKE_CNT_END 0xC40A4[11:0] = 0x18 */
    DRV_ClrReg32(FAST_WAKE_CNT_END, FAST_WAKE_CNT_END_MASK << FAST_WAKE_CNT_END_BIT);
    DRV_SetReg32(FAST_WAKE_CNT_END, 0x18 << FAST_WAKE_CNT_END_BIT);

    /* wake_cksw_sel_slow 0xC4040[5:4] = 2b01 */
    wake_cksw_sel_slow(0x1);

    /* CLK_SEL_SLOW 0xC401C[1:0] = 2b11, select clk_high as wakeup clk */
    DRV_ClrReg32(CLK_SEL_SLOW, CLK_SW_SEL_SLOW_MASK << CLK_SW_SEL_SLOW_BIT);
    DRV_SetReg32(CLK_SEL_SLOW, 0x3 << CLK_SW_SEL_SLOW_BIT);

    /* CLK_DIVSW_SEL_SLOW 0xC401C[5:4] = 2b00 */
    DRV_ClrReg32(CLK_SEL_SLOW, CLK_DIVSW_SEL_SLOW_MASK << CLK_DIVSW_SEL_SLOW_BIT);
#else
    PRINTF_W("use legacy-wakeup\n");

    /* CLK_CTRL_SLP_CTRL 0xC40A0 = 0x1 */
    DRV_WriteReg32(CLK_CTRL_SLP_CTRL, 0x1);
#endif

    /* set sleep control */
    use_SCP_ctrl_sleep_mode();
    disable_SCP_sleep_mode();

#if CLK_HIGH_ALWAYS_ON
    DRV_SetReg32(CLK_ON_CTRL, (0x1 << HIGH_AO_BIT) | (0x1 << HIGH_CG_AO_BIT));
#endif

#if CLK_HIGH_CORE_ALWAYS_ON
    DRV_SetReg32(CLK_ON_CTRL, (0x1 << HIGH_CORE_AO_BIT) | (0x1 << HIGH_CORE_CG_AO_BIT));
#endif

    request_irq(CLK_CTRL_IRQn, clkctrl_isr, "clkctrl_isr");
#if FAST_WAKEUP_SUPPORT
    request_irq(CLK_CTRL2_IRQn, clkctrl2_isr, "clkctrl2_isr");
#endif

    ulposc_cali_init();

#if SWITCH_UART_CLK_TO_ULPOSC
#ifdef CFG_UART_SUPPORT
    PRINTF_W("switch UART clock to ULPOSC1(%dMHz)\n", FREQ_250MHZ);
    uart_switch_clk(FREQ_250MHZ);
    PRINTF_W("UART_CK_SEL = 0x%x\n", DRV_Reg32(UART_CK_SEL));
#endif
#endif

    /* SCP IPI register init */
    scp_ipi_registration(IPI_DVFS_DEBUG, (ipi_handler_t)dvfs_debug_set, "IPI_DVFS_DEBUG");
    scp_ipi_registration(IPI_DVFS_SLEEP, (ipi_handler_t)scp_sleep_ctrl, "IPI_DVFS_SLEEP");

#if DUMMY_TASK_TEST
#ifdef CFG_CORE_BENCHMARK_SUPPORT
    kal_xTaskCreate(vDvfsMainTask, "dvfs", 3840, NULL, 1, NULL);
#else
    kal_xTaskCreate(vDvfsMainTask, "dvfs", 256, NULL, 1, NULL);
#endif
#endif

    DVFSLOG(DVFSTAG "%s done\n", __func__);
}

#ifdef SUPPORT_PMIC_VOW_LP_MODE
#ifdef CFG_PMIC_WRAP_SUPPORT
static void set_pmic_sshub_sleep_en(int enable)
{
    unsigned int rdata1 = 0, rdata2 = 0;

    pwrap_scp_read(PMIC_RG_BUCK_VCORE_SSHUB_SLEEP_VOSEL_EN_ADDR, &rdata1);
    pwrap_scp_read(PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_SLEEP_VOSEL_EN_ADDR, &rdata2);

    if (enable == SCP_ENABLE) {
        pwrap_scp_write(PMIC_RG_BUCK_VCORE_SSHUB_SLEEP_VOSEL_EN_ADDR,
            (rdata1 |
             (PMIC_RG_BUCK_VCORE_SSHUB_SLEEP_VOSEL_EN_MASK <<
              PMIC_RG_BUCK_VCORE_SSHUB_SLEEP_VOSEL_EN_SHIFT)));
        pwrap_scp_write(PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_SLEEP_VOSEL_EN_ADDR,
            (rdata2 |
             (PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_SLEEP_VOSEL_EN_MASK <<
              PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_SLEEP_VOSEL_EN_SHIFT)));
    } else {
        pwrap_scp_write(PMIC_RG_BUCK_VCORE_SSHUB_SLEEP_VOSEL_EN_ADDR,
            (rdata1 &
             ~(PMIC_RG_BUCK_VCORE_SSHUB_SLEEP_VOSEL_EN_MASK <<
               PMIC_RG_BUCK_VCORE_SSHUB_SLEEP_VOSEL_EN_SHIFT)));
        pwrap_scp_write(PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_SLEEP_VOSEL_EN_ADDR,
            (rdata2 &
             ~(PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_SLEEP_VOSEL_EN_MASK <<
               PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_SLEEP_VOSEL_EN_SHIFT)));
    }
}
#endif

static void set_pmic_vow_lp_mode(int enabled)
{
    if (enabled == SCP_ENABLE) {
#ifdef CFG_PMIC_WRAP_SUPPORT
        /* enable PMIC's VOW low power mode */
        set_pmic_sshub_sleep_en(SCP_ENABLE);
#endif

        /* request PMICW in sleep mode */
        pmicw_sleep_req(SCP_ENABLE, NEED_WAIT);

        /* set v_req to PMIC in SW mode with pulled-low */
        DRV_WriteReg32(CPU_VREQ_CTRL, 0x10000);

        Vow_lpmode_st.cur_state = VOW_LPMODE_ON;

    } else {
        /* set v_req to PMIC in HW mode */
        DRV_WriteReg32(CPU_VREQ_CTRL, 0x10001);

        /* release PMICW in sleep mode */
        pmicw_sleep_req(SCP_DISABLE, NEED_WAIT);

#ifdef CFG_PMIC_WRAP_SUPPORT
        /* disable PMIC's VOW low power mode */
        set_pmic_sshub_sleep_en(SCP_DISABLE);
#endif

        Vow_lpmode_st.cur_state = VOW_LPMODE_OFF;
    }
}

int request_pmic_lp_mode_for_vow_from_isr(int enabled)
{
    int ret = 0;

    DVFSFUC();

    if (enabled == SCP_ENABLE) {
        Vow_lpmode_st.req_state = VOW_LPMODE_ON;

        if (drv_reg32(CURRENT_FREQ_REG) != clk_opp_table[CLK_OPP0]) {
            DVFSLOG("Not allow to enter PMIC VOW LP mode due to DVFS == OPP0\n");
            return -1;
        }
    } else
        Vow_lpmode_st.req_state = VOW_LPMODE_OFF;

    set_pmic_vow_lp_mode(enabled);

    return ret;
}

int request_pmic_lp_mode_for_vow(int enabled)
{
    int ret = 0;

    DVFSFUC();

    kal_taskENTER_CRITICAL();

    if (enabled == SCP_ENABLE) {
        Vow_lpmode_st.req_state = VOW_LPMODE_ON;

        if (drv_reg32(CURRENT_FREQ_REG) != clk_opp_table[CLK_OPP0]) {
            DVFSLOG("Not allow to enter PMIC VOW LP mode due to DVFS==OPP0\n");
            kal_taskEXIT_CRITICAL();
            return -1;
        }
    } else
        Vow_lpmode_st.req_state = VOW_LPMODE_OFF;

    set_pmic_vow_lp_mode(enabled);

    kal_taskEXIT_CRITICAL();

    return ret;
}

void get_vow_lpmode_state_from_isr(int *req_state, int *cur_state)
{
    *req_state = Vow_lpmode_st.req_state;
    *cur_state = Vow_lpmode_st.cur_state;
}

void get_vow_lpmode_state(int *req_state, int *cur_state)
{
    kal_taskENTER_CRITICAL();

    *req_state = Vow_lpmode_st.req_state;
    *cur_state = Vow_lpmode_st.cur_state;

    kal_taskEXIT_CRITICAL();
}
#endif
