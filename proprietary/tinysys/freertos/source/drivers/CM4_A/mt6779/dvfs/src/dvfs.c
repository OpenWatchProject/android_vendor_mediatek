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
    FREQ_110MHZ,
    FREQ_130MHZ,
    FREQ_165MHZ,
    FREQ_330MHZ,
    FREQ_416MHZ
};

#define ULPOSC1_CAL_MIN_VALUE   3
#define ULPOSC1_CAL_MAX_VALUE   60
#define ULPOSC1_CAL_START_VALUE ((ULPOSC1_CAL_MIN_VALUE + ULPOSC1_CAL_MAX_VALUE)/2)

#define ULPOSC2_CAL_MIN_VALUE   3
#define ULPOSC2_CAL_MAX_VALUE   60
#define ULPOSC2_CAL_START_VALUE ((ULPOSC2_CAL_MIN_VALUE + ULPOSC2_CAL_MAX_VALUE)/2)

ulposc_con_t ulposc_setting[ULPOSC_NUM] = {
    {.osc_cali = ULPOSC1_CAL_START_VALUE},
    {.osc_cali = ULPOSC2_CAL_START_VALUE},
};

uint32_t clk_sys_req_cnt = 0;
int clk_sys_is_on_by_dvfs = 0;

extern unsigned int sleep_hw_ready_timeout;

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
        mlprintf(LOG_DVFS_EMERG, "err opp %d\n", clk_opp);
        configASSERT(0);
        return FREQ_UNINIT;
    }

    return clk_opp_table[clk_opp];
}

clk_opp_enum convert_clk_2_opp(freq_enum clk_freq)
{
    int i;

    if (clk_freq == FREQ_UNINIT)
        return CLK_OPP_UNINIT;

    for (i = 0; i < CLK_OPP_NUM; i++)
        if (clk_opp_table[i] == clk_freq)
            break;

    if (i >= CLK_OPP_NUM) {
        mlprintf(LOG_DVFS_EMERG, "err freq %d\n", clk_freq);
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

void ulposc_div_select(ulposc_id_enum ulposc_id, uint32_t val)
{
    DVFSFUC("0x%x", (unsigned int)val);

    if (ulposc_id == ULPOSC_2) {
        DRV_ClrReg32(ULPOSC_CON3, RG_OSC_MOD_MSK << RG_OSC_MOD_SHFT);
        DRV_SetReg32(ULPOSC_CON3, val << RG_OSC_MOD_SHFT);
    } else {
        DRV_ClrReg32(ULPOSC_CON1, RG_OSC_MOD_MSK << RG_OSC_MOD_SHFT);
        DRV_SetReg32(ULPOSC_CON1, val << RG_OSC_MOD_SHFT);
    }
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
        mlprintf(LOG_DVFS_EMERG, "err CLK_SW_SEL=0x%x\n", DRV_Reg32(CLK_SW_SEL));
        configASSERT(0);
    }

    return i;
}

void turn_off_clk_sys(void)
{
    DVFSFUC();

    kal_taskENTER_CRITICAL();

    if (clk_sys_req_cnt == 0) {
        mlprintf(LOG_DVFS_EMERG, "clk_sys_req_cnt==0\n");
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
            mlprintf(LOG_DVFS_EMERG, "ack1=%d, ack2=%d\n", (int)get_clk_sys_ack(), (int)get_clk_sys_safe_ack());
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
        mlprintf(LOG_DVFS_EMERG, "clk_sys_req_cnt==0\n");
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
                mlprintf(LOG_DVFS_EMERG, "ack1=%d, ack2=%d\n", (int)get_clk_sys_ack(), (int)get_clk_sys_safe_ack());
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
    DRV_WriteReg32(CLK_CFG_0_CLR, CLK_SCP_SEL_MSK << CLK_SCP_SEL_SHFT);
    DRV_WriteReg32(CLK_CFG_0_SET, (opp & CLK_SCP_SEL_MSK) << CLK_SCP_SEL_SHFT);
    DRV_WriteReg32(CLK_CFG_UPDATE, 0x1 << SCP_CK_UPDATE_SHFT);
}

void request_clk_sys_opp(uint32_t req_opp)
{
    DVFSFUC("%d", (int)req_opp);

    switch (req_opp) {
        case CLK_OPP0: /* 110M (ulposc2 div 3) */
            pll_opp_switch(7);
            break;
        case CLK_OPP1: /* 130M (ulposc1 div 2) */
            break;
        case CLK_OPP2: /* 165 MHz, U2/2 */
            pll_opp_switch(6);
            break;
        case CLK_OPP3: /* 330 MHz, U2 */
            break;
        case CLK_OPP4: /* 416 MHz, UNIVPLL */
            pll_opp_switch(4);
            break;
        default:
            mlprintf(LOG_DVFS_EMERG, "%s:invalid opp %d\n", __func__, (int)req_opp);
            configASSERT(0);
            break;
    }
}
#endif

void request_dfs_opp(uint32_t req_opp)
{
    DVFSFUC("%d", (int)req_opp);

    switch ((clk_opp_enum)req_opp) {
        case CLK_OPP0: /* 110M (ulposc2 div 3) */
            /* temporarily parking clock to PLL */
            select_clk_source(CLK_SEL_SYS);

            /* turn on ulposc2 */
            turn_on_clk_high(ULPOSC_2);

            /* scp div 3 and switch to ulposc2 */
            clk_div_select(CLK_DIV_3);
            select_clk_source(CLK_SEL_ULPOSC2);

            /* turn off clk-sys if needed */
            if (clk_sys_is_on_by_dvfs == 1) {
                clk_sys_is_on_by_dvfs = 0;
                turn_off_clk_sys_from_isr();
            }
            break;

        case CLK_OPP1: /* 130M (ulposc1 div 2) */
            /* temporarily parking clock to PLL */
            select_clk_source(CLK_SEL_SYS);

            /* scp div 2 and switch to ulposc1 */
            clk_div_select(CLK_DIV_2);
            select_clk_source(CLK_SEL_ULPOSC1);

            /* turn off ulposc2 */
            turn_off_clk_high(ULPOSC_2);

            /* turn off clk-sys if needed */
            if (clk_sys_is_on_by_dvfs == 1) {
                clk_sys_is_on_by_dvfs = 0;
                turn_off_clk_sys_from_isr();
            }
            break;

        case CLK_OPP2: /* 165M (ulposc2 div 2) */
            /* temporarily parking clock to PLL */
            select_clk_source(CLK_SEL_SYS);

            /* turn on ulposc2 */
            turn_on_clk_high(ULPOSC_2);

            /* scp div 2 and switch to ulposc2 */
            clk_div_select(CLK_DIV_2);
            select_clk_source(CLK_SEL_ULPOSC2);

            /* turn off clk-sys if needed */
            if (clk_sys_is_on_by_dvfs == 1) {
                clk_sys_is_on_by_dvfs = 0;
                turn_off_clk_sys_from_isr();
            }
            break;

        case CLK_OPP3: /* 330M (=ulposc2) */
            /* temporarily parking clock to PLL */
            select_clk_source(CLK_SEL_SYS);

            /* turn on ulposc2 */
            turn_on_clk_high(ULPOSC_2);

            /* scp div 1 and switch to ulposc2 */
            clk_div_select(CLK_DIV_1);
            select_clk_source(CLK_SEL_ULPOSC2);

            /* turn off clk-sys if needed */
            if (clk_sys_is_on_by_dvfs == 1) {
                clk_sys_is_on_by_dvfs = 0;
                turn_off_clk_sys_from_isr();
            }
            break;

        case CLK_OPP4: /* 416M (univpll) */
            /* turn on clk-sys */
            if (clk_sys_is_on_by_dvfs == 0) {
                clk_sys_is_on_by_dvfs = 1;
                turn_on_clk_sys_from_isr(NEED_WAIT);
#ifdef CFG_TESTSUITE_SUPPORT
                request_clk_sys_opp(CLK_OPP4);
#endif
            }

            /* scp div1 and switch to clk-sys */
            clk_div_select(CLK_DIV_1);
            select_clk_source(CLK_SEL_SYS);

            /* turn off ulposc2 */
            turn_off_clk_high(ULPOSC_2);
            break;

        default:
            mlprintf(LOG_DVFS_EMERG, "err opp %d\n", (int)req_opp);
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

    mlprintf(LOG_DVFS_INFO, "SCP DVFS: %d MHz => %d MHz\n",
        drv_reg32(CURRENT_FREQ_REG),
        drv_reg32(EXPECTED_FREQ_REG));

    req_opp = get_expect_opp();
    cur_opp = convert_clk_2_opp(get_cur_clk());

    DVFSLOG("DVFS request: %d => %d\n", (int)cur_opp, (int)req_opp);

    if (cur_opp != req_opp || cur_opp == CLK_OPP_UNINIT) {
        request_dfs_opp(req_opp);
        set_cur_opp(req_opp);

#if FAST_WAKEUP_SUPPORT == 0 && SRCLKEN_RC_ENABLE == 0
        if (req_opp == CLK_OPP0)
            set_vreq_count(0x1);
        else
            set_vreq_count(0xA);
#endif

#if FAST_WAKEUP_SUPPORT
        /* FAST_WAKE_CNT_END 0xC40A4[11:0] */
        DRV_ClrReg32(FAST_WAKE_CNT_END, FAST_WAKE_CNT_END_MASK << FAST_WAKE_CNT_END_BIT);
        if (req_opp == CLK_OPP0) {
            DRV_SetReg32(FAST_WAKE_CNT_END, 0xA << FAST_WAKE_CNT_END_BIT);
            sleep_hw_ready_timeout = 32*0xA + 200;
        } else {
            DRV_SetReg32(FAST_WAKE_CNT_END, 0x18 << FAST_WAKE_CNT_END_BIT);
            sleep_hw_ready_timeout = 32*0x18 + 200;
        }
#endif
    }

#if DVFS_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
    dvfs_end_time = timer_get_global_timer_tick();
    mlprintf(LOG_DVFS_INFO, "DVFS profiling: %d => %d = %d\n",
        dvfs_start_time, dvfs_end_time, dvfs_end_time - dvfs_start_time);
#endif
}

void set_ulposc_cali_value(uint32_t cali_val, ulposc_id_enum ulposc)
{
    DVFSFUC();

    if (ulposc == ULPOSC_1) {
        DRV_ClrReg32(ULPOSC_CON0, RG_OSC_CALI_MSK << RG_OSC_CALI_SHFT);
        DRV_SetReg32(ULPOSC_CON0, (cali_val & RG_OSC_CALI_MSK) << RG_OSC_CALI_SHFT);
    } else {
        DRV_ClrReg32(ULPOSC_CON2, RG_OSC_CALI_MSK << RG_OSC_CALI_SHFT);
        DRV_SetReg32(ULPOSC_CON2, (cali_val & RG_OSC_CALI_MSK) << RG_OSC_CALI_SHFT);
    }

#ifdef CFG_XGPT_SUPPORT
    udelay(50);
#endif
}

uint32_t get_ulposc_clk_by_freqmeter(ulposc_id_enum ulposc)
{
    unsigned int i = 0, result = 0;
    unsigned int clk_misc_cfg_0, clk_dbg_cfg, clk_scp_cfg_0, clk_scp_cfg_1;
    unsigned int tmp;
    int is_fmeter_timeout = 0;

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
            DVFSLOG("freq-meter timeout!\n");
            is_fmeter_timeout = 1;
            break;
        }
    }

    /* Disable frequency meter */
    DRV_ClrReg32(CLK_SCP_CFG_0, 0x1<<12);

    /* get freq meter result */
    if (!is_fmeter_timeout)
        result = DRV_Reg32(CLK_SCP_CFG_1) & 0xffff;
    else
        result = 0;

    /* restore freq meter RGs */
    DRV_WriteReg32(CLK_MISC_CFG_0, clk_misc_cfg_0);
    DRV_WriteReg32(CLK_DBG_CFG, clk_dbg_cfg);
    DRV_WriteReg32(CLK_SCP_CFG_0, clk_scp_cfg_0);
    DRV_WriteReg32(CLK_SCP_CFG_1, clk_scp_cfg_1);

    DVFSLOG("freq=%d MHz\n", (result * 26) / 1024);

    return result;
}

uint32_t is_ulposc_calibrated(ulposc_id_enum ulposc_id, freq_enum target_clk)
{
    uint32_t target_val =  target_clk * 1024 / 26;
    uint32_t current_val;

    DVFSFUC();

    current_val = get_ulposc_clk_by_freqmeter(ulposc_id);

    /* check if calibrated value is in the range of target value +- 15% */
    if (current_val > (target_val*930/1000) && current_val < (target_val*1070/1000)) {
        mlprintf(LOG_DVFS_NOTE, "ULPOSC%d is calibrated (%dMHz)\n", ulposc_id+1, (current_val * 26) / 1024);
        return 1;
    } else {
        mlprintf(LOG_DVFS_NOTE, "ULPOSC%d is not calibrated (%dMHz)\n", ulposc_id+1, (current_val * 26) / 1024);
        return 0;
    }
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

        set_ulposc_cali_value(middle, ulposc_id);
        current_val = get_ulposc_clk_by_freqmeter(ulposc_id);

        if (current_val > target_val)
            max = middle;
        else
            min = middle;
    } while (min <= max);

    set_ulposc_cali_value(min, ulposc_id);
    current_val = get_ulposc_clk_by_freqmeter(ulposc_id);
    if (current_val > target_val)
        diff_by_min = current_val - target_val;
    else
        diff_by_min = target_val - current_val;

    set_ulposc_cali_value(max, ulposc_id);
    current_val = get_ulposc_clk_by_freqmeter(ulposc_id);
    if (current_val > target_val)
        diff_by_max = current_val - target_val;
    else
        diff_by_max = target_val - current_val;

    if (diff_by_min < diff_by_max) {
        cal_result = min;
    } else {
        cal_result = max;
    }
    set_ulposc_cali_value(cal_result, ulposc_id);
    current_val = get_ulposc_clk_by_freqmeter(ulposc_id);

    /* check if calibrated value is in the range of target value +- 15% */
    if (current_val > (target_val*850/1000) && current_val < (target_val*1150/1000)) {
        mlprintf(LOG_DVFS_NOTE, "ULPOSC%d calibration done: %dM\n", ulposc_id+1, current_val*26/1024);
    } else {
        mlprintf(LOG_DVFS_EMERG, "ULPOSC%d calibration fail: %dM\n", ulposc_id+1,  current_val*26/1024);
        configASSERT(0);
    }

    return cal_result;
}

void config_ulposc1_registers(void)
{
    DVFSFUC();

    /*
     **** Config ULPOSC_CON0 ***
     */

    /* OSC_RSV= 00000000 */
    DRV_ClrReg32(ULPOSC_CON0, RG_OSC_RSV_MSK << RG_OSC_RSV_SHFT);

    /* OSC_CP_EN = 1 */
    DRV_SetReg32(ULPOSC_CON0, 0x1 << RG_OSC_CP_EN_SHFT);

    /* set OSC_DIV */
    DRV_ClrReg32(ULPOSC_CON0, RG_OSC_DIV_MSK << RG_OSC_DIV_SHFT);
    DRV_SetReg32(ULPOSC_CON0, 12 << RG_OSC_DIV_SHFT);

    /* OSC_FBAND = 0000 */
    DRV_ClrReg32(ULPOSC_CON0, RG_OSC_FBAND_MSK << RG_OSC_FBAND_SHFT);

    /* OSC_IBAND = 0000100 */
    DRV_ClrReg32(ULPOSC_CON0, RG_OSC_IBAND_MSK << RG_OSC_IBAND_SHFT);
    DRV_SetReg32(ULPOSC_CON0, 4 << RG_OSC_IBAND_SHFT);

    /* OSC_CALI */
    DRV_ClrReg32(ULPOSC_CON0, RG_OSC_CALI_MSK << RG_OSC_CALI_SHFT);
    DRV_SetReg32(ULPOSC_CON0,
        (ulposc_setting[ULPOSC_1].osc_cali & RG_OSC_CALI_MSK) << RG_OSC_CALI_SHFT);

    /*
     **** Config ULPOSC_CON1 ***
     */

    /* OSC_DIV2_EN = 1 */
    DRV_SetReg32(ULPOSC_CON1, 0x1 << RG_OSC_DIV2_EN_SHFT);

    /* OSC_MOD = 00 */
    DRV_ClrReg32(ULPOSC_CON1, RG_OSC_MOD_MSK << RG_OSC_MOD_SHFT);

    DVFSLOG("ULPOSC_CON0 = 0x%x, ULPOSC_CON1 = 0x%x\n",
        DRV_Reg32(ULPOSC_CON0), DRV_Reg32(ULPOSC_CON1));
}

void config_ulposc2_registers(void)
{
    DVFSFUC();

    /*
     **** Config ULPOSC_CON2 ***
     */

    /* OSC_RSV= 00000000 */
    DRV_ClrReg32(ULPOSC_CON2, RG_OSC_RSV_MSK << RG_OSC_RSV_SHFT);

    /* OSC_CP_EN = 1 */
    DRV_SetReg32(ULPOSC_CON2, 0x1 << RG_OSC_CP_EN_SHFT);

    /* set OSC_DIV */
    DRV_ClrReg32(ULPOSC_CON2, RG_OSC_DIV_MSK << RG_OSC_DIV_SHFT);
    DRV_SetReg32(ULPOSC_CON2, 16 << RG_OSC_DIV_SHFT);

    /* OSC_FBAND = 0000 */
    DRV_ClrReg32(ULPOSC_CON2, RG_OSC_FBAND_MSK << RG_OSC_FBAND_SHFT);

    /* OSC_IBAND = 0000100 */
    DRV_ClrReg32(ULPOSC_CON2, RG_OSC_IBAND_MSK << RG_OSC_IBAND_SHFT);
    DRV_SetReg32(ULPOSC_CON2, 4 << RG_OSC_IBAND_SHFT);

    /* set OSC_CALI */
    DRV_ClrReg32(ULPOSC_CON2, RG_OSC_CALI_MSK << RG_OSC_CALI_SHFT);
    DRV_SetReg32(ULPOSC_CON2,
        (ulposc_setting[ULPOSC_2].osc_cali & RG_OSC_CALI_MSK) << RG_OSC_CALI_SHFT);

    /*
     **** Config ULPOSC_CON3 ***
     */

    /* OSC_DIV2_EN = 1 */
    DRV_SetReg32(ULPOSC_CON3, 0x1 << RG_OSC_DIV2_EN_SHFT);

    /* OSC_MOD = 00 */
    DRV_ClrReg32(ULPOSC_CON3, RG_OSC_MOD_MSK << RG_OSC_MOD_SHFT);

    DVFSLOG("ULPOSC_CON2 = 0x%x, ULPOSC_CON3 = 0x%x\n",
        DRV_Reg32(ULPOSC_CON2), DRV_Reg32(ULPOSC_CON3));
}

void ulposc_cali_init(void)
{
    /* By default, ULPOSC2 is OFF even though ULPOSC1 is ON */
    DRV_SetReg32(CLK_ON_CTRL, 1<<HIGH_CORE_DIS_SUB_BIT);

#if DO_ULPOSC1_CALIBRATION
    if (is_ulposc_calibrated(ULPOSC_1, FREQ_260MHZ) == 0) {
        /* calibrate ULPOSC1 */
        config_ulposc1_registers();
        turn_on_clk_high(ULPOSC_1);
        ulposc_setting[ULPOSC_1].osc_cali = ulposc_cali_process(ULPOSC_1, FREQ_260MHZ);
    } else {
        turn_on_clk_high(ULPOSC_1);
    }
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
        mlprintf(LOG_DVFS_INFO, "\n%u sec passed\n", i*DVFS_TASK_DELAY/1000);
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

#if PMICW_SLEEP_REQ_FLOW
    /* release the PMICW-sleep-request set by earlier booting code */
    pmicw_sleep_req(SCP_DISABLE, NO_WAIT);
#endif

#if SCP_DDREN_AUTO_MODE
    /* Set scp ddren to HW mode */
    DRV_SetReg32(SYS_CTRL, 0x1<<AUTO_DDREN_BIT);
#endif

    /* set settle time */
    set_clk_sys_val(0x1);
    set_clk_high_val(0x1);

#if SRCLKEN_RC_ENABLE
    set_vreq_count(0x17);
#else
    set_vreq_count(0x1);
#endif

    wake_cksw_sel_normal(WAKE_CKSW_SEL_NORMAL_CLK_HIGH);

#if FAST_WAKEUP_SUPPORT
    mlprintf(LOG_DVFS_NOTE, "use fast-wakeup\n");

    /* CLK_CTRL_SLP_CTRL 0xC40A0 = 0x0 */
    DRV_WriteReg32(CLK_CTRL_SLP_CTRL, 0x0);

    /* HIGH_FINAL_VAL 0xC406C[12:8] = 0x3 */
    DRV_ClrReg32(CLK_ON_CTRL, HIGH_FINAL_VAL_MASK << HIGH_FINAL_VAL_BIT);
    DRV_SetReg32(CLK_ON_CTRL, 0x3 << HIGH_FINAL_VAL_BIT);

    /* FAST_WAKE_CNT_END 0xC40A4[11:0] = 0x18 */
    DRV_ClrReg32(FAST_WAKE_CNT_END, FAST_WAKE_CNT_END_MASK << FAST_WAKE_CNT_END_BIT);
    DRV_SetReg32(FAST_WAKE_CNT_END, 0xA << FAST_WAKE_CNT_END_BIT);

    /* wake_cksw_sel_slow 0xC4040[5:4] = 2b01 */
    wake_cksw_sel_slow(0x1);

    /* CLK_SEL_SLOW 0xC401C[1:0] = 2b11, select clk_high as wakeup clk */
    DRV_ClrReg32(CLK_SEL_SLOW, CLK_SW_SEL_SLOW_MASK << CLK_SW_SEL_SLOW_BIT);
    DRV_SetReg32(CLK_SEL_SLOW, 0x3 << CLK_SW_SEL_SLOW_BIT);

    /* CLK_DIVSW_SEL_SLOW 0xC401C[5:4] = 2b00 */
    DRV_ClrReg32(CLK_SEL_SLOW, CLK_DIVSW_SEL_SLOW_MASK << CLK_DIVSW_SEL_SLOW_BIT);

    /* simultaneously switch CLK_SW_SEL, CLK_DIV_SEL and WAKE_CKSW_SEL */
    DRV_SetReg32(CLK_CTRL_SLP_CTRL, 0x1 << SLOW_CLK_WAKE_INTERLEAVE_DIS_BIT);
#else
    mlprintf(LOG_DVFS_NOTE, "use legacy-wakeup\n");

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
    mlprintf(LOG_DVFS_NOTE, "switch UART clock source to ULPOSC1\n");
    uart_switch_clk(FREQ_260MHZ);
    mlprintf(LOG_DVFS_DEBUG, "UART_CK_SEL = 0x%x\n", DRV_Reg32(UART_CK_SEL));
#endif
#endif

    /* SCP IPI register init */
    scp_ipi_registration(IPI_DVFS_SET_FREQ, (ipi_handler_t)scp_set_opp, "IPI_DVFS_SET_FREQ");
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

