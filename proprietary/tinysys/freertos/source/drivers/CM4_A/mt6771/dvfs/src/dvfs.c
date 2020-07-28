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

#define DVFS_DEBUG              0
#define DVFS_TIMER_PROFILING    0

#if DUMMY_TASK_TEST
#define DVFS_TASK_DELAY         200
#endif

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

#if DVFS_TIMER_PROFILING && defined(CFG_XGPT_SUPPORT)
TickType_t dvfs_start_time, dvfs_end_time;
#endif

static uint32_t scp_status = DVFS_STATUS_OK;
int g_scp_dvfs_debug = 0;

freq_enum clk_opp_table[CLK_OPP_NUM] = {
    FREQ_125MHZ,
    FREQ_330MHZ,
    FREQ_416MHZ
};

#ifdef CFG_TESTSUITE_SUPPORT
static int clk_pll_table[CLK_OPP_NUM] = {
    78,
    364,
    416
};
#endif

ulposc_con_t ulposc1_setting[1] = {
    {.osc_div = 12, .cali_val = 32},
};

ulposc_con_t ulposc2_setting[1] = {
    {.osc_div = 16, .cali_val = 32},
};

uint32_t clk_sys_req_cnt = 0;
int clk_sys_is_on_by_dvfs = 0;

#ifdef SUPPORT_PMIC_VOW_LP_MODE
static pmic_vow_lpmode_state_t Vow_lpmode_st = {
    VOW_LPMODE_OFF, VOW_LPMODE_OFF };

#ifdef CFG_PMIC_WRAP_SUPPORT
static void set_pmic_sshub_sleep_en(int enable);
#endif

static void set_pmic_vow_lp_mode(int enabled);
#endif


/*****************************************
 * CLK Inform Dump APIs
 ****************************************/
freq_enum convert_opp_2_clk(clk_opp_enum clk_opp)
{
    if (clk_opp == CLK_OPP_UNINIT)
        return FREQ_UNINIT;

    if (clk_opp < CLK_OPP0 || clk_opp >= CLK_OPP_NUM) {
        DVFSERR("%s: invalid opp %d\n", __func__, clk_opp);
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
        DVFSERR("%s: invalid freq %d\n", __func__, clk_freq);
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

int32_t sshub_get_AP_ack_nonblock(scp_reserve_mem_id_t dma_id)
{
    int32_t ret = 0;

    kal_taskENTER_CRITICAL();
    ret = get_AP_ack_from_isr(dma_id);
    kal_taskEXIT_CRITICAL();

    return ret;
}

void sshub_enable_AP_resource_nonblock(scp_reserve_mem_id_t dma_id)
{
    kal_taskENTER_CRITICAL();
    enable_infra(dma_id, NO_WAIT);
    enable_AP_resource_from_isr(dma_id);
    kal_taskEXIT_CRITICAL();
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
        DVFSERR("CLK_SW_SEL=0x%x is invalid\n", DRV_Reg32(CLK_SW_SEL));
        configASSERT(0);
    }

    return i;
}

void turn_off_clk_sys(void)
{
    DVFSFUC();

    kal_taskENTER_CRITICAL();

    if (clk_sys_req_cnt == 0) {
        DVFSERR("%s: clk_sys_req_cnt==0\n", __func__);
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
            DVFSERR("%s fail, ack1=%d, ack2=%d\n", __func__, (int)get_clk_sys_ack(), (int)get_clk_sys_safe_ack());
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
        DVFSERR("%s: clk_sys_req_cnt==0\n", __func__);
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
                DVFSERR("%s fail, ack1=%d, ack2=%d\n", __func__, (int)get_clk_sys_ack(), (int)get_clk_sys_safe_ack());
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
    DRV_WriteReg32(CLK_CFG_8_CLR, CLK_TOP_SCP_SEL_MSK << CLK_TOP_SCP_SEL_SHFT);
    DRV_WriteReg32(CLK_CFG_8_SET, (opp & CLK_TOP_SCP_SEL_MSK) << CLK_TOP_SCP_SEL_SHFT);
}

void request_clk_sys_opp(uint32_t req_opp)
{
    DVFSFUC("%d", (int)req_opp);

    switch (req_opp) {
        case CLK_OPP0:
            pll_opp_switch(1); /* 78 MHz */
            break;
        case CLK_OPP1:
            pll_opp_switch(5); /* 364 MHz */
            break;
        case CLK_OPP2:
            pll_opp_switch(6); /* 416 MHz */
            break;
        default:
            DVFSERR("%s:invalid opp %d\n", __func__, (int)req_opp);
            configASSERT(0);
            break;
    }
}
#endif

void request_dfs_opp(uint32_t req_opp)
{
    DVFSFUC("%d", (int)req_opp);

    switch ((clk_opp_enum)req_opp) {
        case CLK_OPP0:
            /* switch to ulposc1 */
            clk_div_select(CLK_DIV_2);
            select_clk_source(CLK_SEL_ULPOSC1);

            /* turn off ulposc2 */
            turn_off_clk_high(ULPOSC_2);

            /* turn off clk-sys */
            if (clk_sys_is_on_by_dvfs == 1) {
                clk_sys_is_on_by_dvfs = 0;
                turn_off_clk_sys_from_isr();
            }
            break;

        case CLK_OPP1:
            /* turn on ulposc2 */
            turn_on_clk_high(ULPOSC_2);

            /* switch to ulposc2 */
            clk_div_select(CLK_DIV_1);
            select_clk_source(CLK_SEL_ULPOSC2);

            /* turn off clk-sys */
            if (clk_sys_is_on_by_dvfs == 1) {
                clk_sys_is_on_by_dvfs = 0;
                turn_off_clk_sys_from_isr();
            }
            break;

        case CLK_OPP2:
            /* turn on clk-sys */
            if (clk_sys_is_on_by_dvfs == 0) {
                clk_sys_is_on_by_dvfs = 1;
                turn_on_clk_sys_from_isr(NEED_WAIT);
#ifdef CFG_TESTSUITE_SUPPORT
                request_clk_sys_opp(CLK_OPP2);
#endif
            }

            /* switch to clk-sys */
            clk_div_select(CLK_DIV_1);
            select_clk_source(CLK_SEL_SYS);

            /* turn off ulposc2 */
            turn_off_clk_high(ULPOSC_2);
            break;

        default:
            DVFSERR("%s:invalid opp %d\n", __func__, (int)req_opp);
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

    DVFSDBG("DVFS request: %d => %d\n", (int)cur_opp, (int)req_opp);

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
        DRV_ClrReg32(PLL_ULPOSC_CON0, RG_OSC_CALI_MSK << RG_OSC_CALI_SHFT);
        DRV_SetReg32(PLL_ULPOSC_CON0, (cali_val & RG_OSC_CALI_MSK) << RG_OSC_CALI_SHFT);
    } else {
        DRV_ClrReg32(PLL_ULPOSC_CON2, RG_OSC_CALI_MSK << RG_OSC_CALI_SHFT);
        DRV_SetReg32(PLL_ULPOSC_CON2, (cali_val & RG_OSC_CALI_MSK) << RG_OSC_CALI_SHFT);
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

    udelay(60);

    /* wait frequency done */
    while (DRV_Reg32(CLK_SCP_CFG_0) & (0x1<<4)) {
        i++;
        if (i > 1000000) {
            DVFSERR("freq-meter timeout!\n");
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

    DVFSDBG("cali_val=0x%x, result=0x%x, %d MHz\n",
        (unsigned int)cali_val, result, (result * 26) / 1024);

    return result;
}

uint32_t ulposc_cali_process(ulposc_id_enum ulposc_id, freq_enum target_clk)
{
    uint32_t target_val = 0, current_val = 0;
    uint32_t min = 0, max = 0x3f, middle;
    uint32_t diff_by_min = 0, diff_by_max = 0xffff;
    uint32_t cal_result = 0;

    DVFSFUC();

    target_val = target_clk * 1024 / 26;

    do {
        middle = (min + max) / 2;
        if (middle == min)
            break;

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
        PRINTF_W("ULPOSC-%d calibration done: %d MHz\n", ulposc_id, current_val*26/1024);
    } else {
        PRINTF_E("ULPOSC-%d calibration fail: %d MHz\n", ulposc_id,  current_val*26/1024);
        configASSERT(0);
    }

    return cal_result;
}

void config_ulposc1_registers(void)
{
    DVFSFUC();

    /* OSC_RSV= 00000000 */
    DRV_ClrReg32(PLL_ULPOSC_CON0, RG_OSC_RSV_MSK << RG_OSC_RSV_SHFT);

    /* OSC_CP_EN = 1 */
    DRV_SetReg32(PLL_ULPOSC_CON0, RG_OSC_CP_EN_MSK << RG_OSC_CP_EN_SHFT);

    /* set OSC_DIV */
    DRV_ClrReg32(PLL_ULPOSC_CON0, RG_OSC_DIV_MSK << RG_OSC_DIV_SHFT);
    DRV_SetReg32(PLL_ULPOSC_CON0,
        (ulposc1_setting[0].osc_div & RG_OSC_DIV_MSK) << RG_OSC_DIV_SHFT);

    /* OSC_FBAND = 0000 */
    DRV_ClrReg32(PLL_ULPOSC_CON0, RG_OSC_FBAND_MSK << RG_OSC_FBAND_SHFT);

    /* OSC_IBAND = 0000100 */
    DRV_ClrReg32(PLL_ULPOSC_CON0, RG_OSC_IBAND_MSK << RG_OSC_IBAND_SHFT);
    DRV_SetReg32(PLL_ULPOSC_CON0, 0x4 << RG_OSC_IBAND_SHFT);

    /* set OSC_CALI */
    DRV_ClrReg32(PLL_ULPOSC_CON0, RG_OSC_CALI_MSK << RG_OSC_CALI_SHFT);
    DRV_SetReg32(PLL_ULPOSC_CON0,
        (ulposc1_setting[0].cali_val & RG_OSC_CALI_MSK) << RG_OSC_CALI_SHFT);

    /* OSC_DIV2_EN = 1 */
    DRV_SetReg32(PLL_ULPOSC_CON1, RG_OSC_DIV2_EN_MSK << RG_OSC_DIV2_EN_SHFT);

    DVFSDBG("PLL_ULPOSC_CON0 = 0x%x, PLL_ULPOSC_CON1 = 0x%x\n",
        DRV_Reg32(PLL_ULPOSC_CON0), DRV_Reg32(PLL_ULPOSC_CON1));
}

void config_ulposc2_registers(void)
{
    DVFSFUC();

    /* OSC_RSV= 00000000 */
    DRV_ClrReg32(PLL_ULPOSC_CON2, RG_OSC_RSV_MSK << RG_OSC_RSV_SHFT);

    /* OSC_CP_EN = 1 */
    DRV_SetReg32(PLL_ULPOSC_CON2, RG_OSC_CP_EN_MSK << RG_OSC_CP_EN_SHFT);

    /* set OSC_DIV */
    DRV_ClrReg32(PLL_ULPOSC_CON2, RG_OSC_DIV_MSK << RG_OSC_DIV_SHFT);
    DRV_SetReg32(PLL_ULPOSC_CON2,
        (ulposc2_setting[0].osc_div & RG_OSC_DIV_MSK) << RG_OSC_DIV_SHFT);

    /* OSC_FBAND = 0000 */
    DRV_ClrReg32(PLL_ULPOSC_CON2, RG_OSC_FBAND_MSK << RG_OSC_FBAND_SHFT);

    /* OSC_IBAND = 0000100 */
    DRV_ClrReg32(PLL_ULPOSC_CON2, RG_OSC_IBAND_MSK << RG_OSC_IBAND_SHFT);
    DRV_SetReg32(PLL_ULPOSC_CON2, 0x4 << RG_OSC_IBAND_SHFT);

    /* set OSC_CALI */
    DRV_ClrReg32(PLL_ULPOSC_CON2, RG_OSC_CALI_MSK << RG_OSC_CALI_SHFT);
    DRV_SetReg32(PLL_ULPOSC_CON2,
        (ulposc2_setting[0].cali_val & RG_OSC_CALI_MSK) << RG_OSC_CALI_SHFT);

    /* OSC_DIV2_EN = 1 */
    DRV_SetReg32(PLL_ULPOSC_CON3, RG_OSC_DIV2_EN_MSK << RG_OSC_DIV2_EN_SHFT);

    DVFSDBG("PLL_ULPOSC_CON2 = 0x%x, PLL_ULPOSC_CON3 = 0x%x\n",
        DRV_Reg32(PLL_ULPOSC_CON2), DRV_Reg32(PLL_ULPOSC_CON3));
}

void ulposc_cali_init(void)
{
    /* By default, ULPOSC2 is OFF even though ULPOSC1 is ON */
    DRV_SetReg32(CLK_ON_CTRL, 1<<HIGH_CORE_DIS_SUB_BIT);

#if DO_ULPOSC1_CALIBRATION
    /* calibrate ULPOSC1 */
    config_ulposc1_registers();
    turn_on_clk_high(ULPOSC_1);
    ulposc1_setting[0].cali_val = ulposc_cali_process(ULPOSC_1, FREQ_250MHZ);
#else
    turn_on_clk_high(ULPOSC_1);
#endif

#if DO_ULPOSC2_CALIBRATION
    /* calibrate ULPOSC2 */
    config_ulposc2_registers();
    turn_on_clk_high(ULPOSC_2);
    ulposc2_setting[0].cali_val = ulposc_cali_process(ULPOSC_2, FREQ_330MHZ);
#else
    turn_on_clk_high(ULPOSC_2);
#endif

    request_dfs_opp(CLK_OPP0);
    set_cur_opp(CLK_OPP_UNINIT);
}

#if DUMMY_TASK_TEST
static void vDvfsMainTask(void *pvParameters)
{
    int i = 0;

    while (1) {
        vTaskDelay(DVFS_TASK_DELAY / portTICK_RATE_MS);
        i++;
        PRINTF_W("\n%u sec passed\n", i*DVFS_TASK_DELAY/1000);
    }
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

    /* V_req ctrl */
    DRV_WriteReg32(CPU_VREQ_CTRL, 0x10001);
    DRV_ClrReg32(SCP_SECURE_CRTL, 0x1<<SCP_ENABLE_SPM_MASK_VREQ_BIT);

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
    PRINTF_W("set fast-wakeup\n");

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

    /* CLK_DIVSW_SEL_SLOW 0xC401C[5:4] = 2b0 */
    DRV_ClrReg32(CLK_SEL_SLOW, CLK_DIVSW_SEL_SLOW_MASK << CLK_DIVSW_SEL_SLOW_BIT);
#else
    /* CLK_CTRL_SLP_CTRL 0xC40A0 = 0x1 */
    DRV_WriteReg32(CLK_CTRL_SLP_CTRL, 0x1);
#endif
    PRINTF_W("set legacy-wakeup\n");

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

    init_dram_resrc_req_cnt();

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
    kal_xTaskCreate(vDvfsMainTask, "dvfs", 256, NULL, 1, NULL);
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
        pmicw_sleep_req(SCP_ENABLE);

        /* set v_req to PMIC in SW mode with pulled-low */
        DRV_WriteReg32(CPU_VREQ_CTRL, 0x10000);

        Vow_lpmode_st.cur_state = VOW_LPMODE_ON;

    } else {
        /* set v_req to PMIC in HW mode */
        DRV_WriteReg32(CPU_VREQ_CTRL, 0x10001);

        /* release PMICW in sleep mode */
        pmicw_sleep_req(SCP_DISABLE);

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
