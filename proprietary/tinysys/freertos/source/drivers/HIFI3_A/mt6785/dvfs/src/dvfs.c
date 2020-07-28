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

#include <adsp_ipi.h>
#include <semphr.h>
#include <utils.h>
#include <sys/types.h>
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
#include <tinysys_config.h>
#include <feature_manager.h>
#include <wakelock.h>
#include <dvfs_common.h>
#include <sleep.h>
#include <dvfs.h>
#include <dvfs_define.h>
#include <dvfs_config_parser.h>

#include "xtensa_api.h"
#include "xtensa_timer.h"
#include <main.h>

/****************************************
 * New Implement
 ****************************************/
static const struct vcore_dvfs_opp_tb mt6779_opp_table[DVFS_OPP_NUM] = {
    {DVFS_LEVEL_0,  DDR_RATE_3733, VCORE_0800},
    {DVFS_LEVEL_1,  DDR_RATE_3094, VCORE_0800},
    {DVFS_LEVEL_2,  DDR_RATE_2400, VCORE_0800},
    {DVFS_LEVEL_3,  DDR_RATE_1534, VCORE_0800},
    {DVFS_LEVEL_4,  DDR_RATE_1200, VCORE_0800},
    {DVFS_LEVEL_5,  DDR_RATE_3094, VCORE_0700},
    {DVFS_LEVEL_6,  DDR_RATE_2400, VCORE_0700},
    {DVFS_LEVEL_7,  DDR_RATE_1534, VCORE_0700},
    {DVFS_LEVEL_8,  DDR_RATE_1200, VCORE_0700},
    {DVFS_LEVEL_9,  DDR_RATE_2400, VCORE_0625},
    {DVFS_LEVEL_10, DDR_RATE_1534, VCORE_0625},
    {DVFS_LEVEL_11, DDR_RATE_1200, VCORE_0625},
    {DVFS_LEVEL_12, DDR_RATE_800,  VCORE_0625}
};

/* vcore dvfs table, ddr_rate is controlled by DVFSRC rising in mt6779 */
struct vcore_dvfs_opp_tb vcore_dvfs[DVFSRC_REQ_NUM] = {
    [DVFSRC_REQ_IDX1]    = ADSP_DVFSRC_UNIT(DVFS_LEVEL_12,  DDR_RATE_800,  VCORE_0625, ADSP_CPU_CLOCK_HZ_L),
    [DVFSRC_REQ_IDX2]    = ADSP_DVFSRC_UNIT(DVFS_LEVEL_12,  DDR_RATE_1200, VCORE_0625, ADSP_CPU_CLOCK_HZ_L),
    [DVFSRC_REQ_IDX3]    = ADSP_DVFSRC_UNIT(DVFS_LEVEL_12,  DDR_RATE_1534, VCORE_0625, ADSP_CPU_CLOCK_HZ_L),
    [DVFSRC_REQ_IDX4]    = ADSP_DVFSRC_UNIT(DVFS_LEVEL_12,  DDR_RATE_2400, VCORE_0625, ADSP_CPU_CLOCK_HZ_L),
    [DVFSRC_REQ_IDX5]    = ADSP_DVFSRC_UNIT(DVFS_LEVEL_8,   DDR_RATE_1200, VCORE_0700, ADSP_CPU_CLOCK_HZ_M),
    [DVFSRC_REQ_IDX6]    = ADSP_DVFSRC_UNIT(DVFS_LEVEL_8,   DDR_RATE_2400, VCORE_0700, ADSP_CPU_CLOCK_HZ_M),
    [DVFSRC_REQ_IDX7]    = ADSP_DVFSRC_UNIT(DVFS_LEVEL_8,   DDR_RATE_3094, VCORE_0700, ADSP_CPU_CLOCK_HZ_M),
    [DVFSRC_REQ_IDX8]    = ADSP_DVFSRC_UNIT(DVFS_LEVEL_4,   DDR_RATE_3733, VCORE_0800, ADSP_CPU_CLOCK_HZ_H)
};

struct vcore_threshold_tb vcore_threshold[VCORE_NUM] = {
    /*vcore : upperbound (2%), lowerbound (6.25%)*/
    [VCORE_0625] = VCORE_THR(466, 456, 437),
    [VCORE_0700] = VCORE_THR(700, 686, 656),
    [VCORE_0800] = VCORE_THR(788, 788, 738)
};

unsigned long long g_cpu_clock_hz INTERNAL_INITIAL_DATA = configCPU_CLOCK_HZ;
uint32_t is_freq_fixed = 0;
uint32_t is_latmon_irq_triggered = 0;  /*for module test*/
/***************************************************
** Latency Monitor
****************************************************/
#define LATMON_INIT_MARGIN      0x190
#define LATMON_TIMER_PRD_BIT    (4)
#define LATMON_TIMER_PRD_VALUE  0x8

static uint32_t latmon_mode;
/* RO state query */
static uint32_t latmon_threshold;
static uint32_t latmon_state;
static uint32_t latmon_account;
static void dvfs_bus_latmon_set_mode(uint32_t mode);


#define LATMON_CONT(mode_thr, mode_budget) \
    {.thr= mode_thr, .budget = mode_budget}

struct latmon_mode_tb {
    uint32_t thr;
    uint32_t budget;
};

struct latmon_mode_tb latmon_cont[VCORE_NUM] = {
    /* [mode] = LATMON_CONT(threshold, budget) */
    [VCORE_0625] = LATMON_CONT(0x130, 0x15D),
    [VCORE_0700] = LATMON_CONT(0x180, 0x20D),
    [VCORE_0800] = LATMON_CONT(0x220, 0x24F)
};

void set_dvfs_global_clkrate(unsigned long long freq)
{
    g_cpu_clock_hz = freq;
}

/* internal implement */
uint32_t get_adsp_clksel_by_cpu_rate(unsigned long long freq)
{
    switch(freq){
    case ADSP_CPU_CLOCK_HZ_L:
    case ADSP_CPU_CLOCK_P95_HZ_M:
        return ADSPPLL_D6;
    case ADSP_CPU_CLOCK_HZ_M:
    case ADSP_CPU_CLOCK_P95_HZ_L:
        return ADSPPLL_D4;
    case ADSP_CPU_CLOCK_HZ_H:
        return MMPLL_D4;
    default:
        return TCK_26M_MX9_CK;
    }
}

uint32_t adsp_get_clock_mux(void)
{
    return (drv_reg32(CLK_CFG_11_STA) >> ADSP_SEL_BASE) & 0x7;
}

void adsp_clock_mux_setup(adsp_clock_idx clock)
{
    /* only set div2 when adsppll_d4 in P95 */
    uint32_t mclk_div_val = drv_reg32(ADSP_MCLK_DIV_REG) & ~ADSP_MCLK_DIV_MASK;

    DVFS_LOG_V("%s():clock = %d g_adsppll_unlock=%d\n", __func__, clock, g_adsppll_unlock);
    if (clock != ADSPPLL_D4 && clock != ADSPPLL_D6) {
        drv_write_reg32(ADSP_MCLK_DIV_REG, mclk_div_val);
        /* CLR+SET+UPDATE*/
        drv_write_reg32(CLK_CFG_11_CLR, 0x7 << ADSP_SEL_BASE);
        drv_write_reg32(CLK_CFG_11_SET, clock << ADSP_SEL_BASE);
        drv_write_reg32(CLK_CFG_UPDATE1, ADSP_CK_UPDATE);
        if (g_adsppll_status && g_adsppll_unlock) {
            /*turn off adsppll when ap suspend*/
            adsp_setup_adsppll(false);
        }
    } else {
        if (!g_adsppll_status) {
            adsp_setup_adsppll(true);
        }

        drv_write_reg32(ADSP_MCLK_DIV_REG, mclk_div_val);
        /* CLR+SET+UPDATE*/
        drv_write_reg32(CLK_CFG_11_CLR, 0x7 << ADSP_SEL_BASE);
        drv_write_reg32(CLK_CFG_11_SET, clock << ADSP_SEL_BASE);
        drv_write_reg32(CLK_CFG_UPDATE1, ADSP_CK_UPDATE);
    }
}

uint32_t get_dvfsrcreq_idx_by_level(uint32_t level)
{
    uint32_t i = 0;

    for(i = 0; i < DVFSRC_REQ_NUM; i++) {
        if(level == vcore_dvfs[i].level)
            return i;
    }
    DVFS_LOG_W("%s cannot match dvfsrc req.\n", __FUNCTION__);
    return 0;
}

uint32_t get_level_by_dvfsrcreq(uint32_t index)
{
    return vcore_dvfs[index].level;
}

uint32_t get_ddr_rate_by_level(uint32_t index)
{
    return vcore_dvfs[get_dvfsrcreq_idx_by_level(index)].ddr_rate;
}

uint32_t get_vcore_by_level(uint32_t index)
{
    return vcore_dvfs[get_dvfsrcreq_idx_by_level(index)].vcore;
}

int32_t get_dvfsrc_idx(uint32_t value)
{
    uint32_t i = 0;

    if (value == 0)
        return -1;

    while(!(value & 0x1)) {
        value >>= 1;
        i ++;
    }
    return i;
}

void request_dfs_level(uint32_t dvfs_level)
{
    uint32_t pre_dvfsrc_req = drv_reg32(ADSP_DVFSRC_REQ);
    uint32_t target_dvfsrc_req = 1 << get_dvfsrcreq_idx_by_level(dvfs_level);
    uint32_t target_dvfsrc_state = 1 << dvfs_level;
    int32_t timeout = 1000;

    DVFS_LOG_V("%s() level=%d cur req/state (0x%x/0x%x) -> new req/state (0x%x/0x%x)\n",
                __FUNCTION__,
                dvfs_level,
                drv_reg32(ADSP_DVFSRC_REQ),
                drv_reg32(ADSP_DVFSRC_STATE),
                target_dvfsrc_req,
                target_dvfsrc_state);

    /* put hifi3 in memw before change frequency */
    __asm(
    "memw;\n"
    );
    drv_write_reg32(ADSP_DVFSRC_REQ, pre_dvfsrc_req | target_dvfsrc_req);
    drv_write_reg32(ADSP_DVFSRC_REQ, target_dvfsrc_req);

    while(timeout--) {
        if(drv_reg32(ADSP_DVFSRC_STATE) <= target_dvfsrc_state)
            break;
        udelay(1);
    }

    if (timeout <= 0) {
        DVFS_LOG_W("Fail to set dvfsrc_req to 0x%x, req/state=%x/%x\n",
            target_dvfsrc_req,
            drv_reg32(ADSP_DVFSRC_REQ),
            drv_reg32(ADSP_DVFSRC_STATE));
    } else {
        DVFS_LOG_V("set dvfsrc_req to 0x%x, req/state=%x/%x\n",
            target_dvfsrc_req,
            drv_reg32(ADSP_DVFSRC_REQ),
            drv_reg32(ADSP_DVFSRC_STATE));
    }

}

#define IS_OVER_THR(vcore, value) \
        (value > vcore_threshold[vcore].upper_bound)
#define IS_LOWER_THR(vcore, value) \
        (value < vcore_threshold[vcore].lower_bound)

int32_t update_dvfs_freq(void)
{
    uint32_t new_req_idx, cur_req_idx, cur_vcore, total_mcps;

    if (is_freq_fixed) {
        return 0;
    }
    cur_req_idx = (uint32_t)get_dvfsrc_idx(drv_reg32(ADSP_DVFSRC_REQ));
    cur_vcore = vcore_dvfs[cur_req_idx].vcore;
    total_mcps = calculate_total_mcps();
    new_req_idx = cur_req_idx;

    if (IS_OVER_THR(VCORE_0800, total_mcps)) {
        DVFS_LOG_E("[DVFS] MCPS overflow! target mcps=%d\n", total_mcps);
        configASSERT(0);
        return -1;
    }

    /* decide vcore level */
    switch(cur_vcore) {
    case VCORE_0625:
        if (IS_OVER_THR(VCORE_0700, total_mcps)) {
            new_req_idx = DVFSRC_REQ_IDX8;
        } else if (IS_OVER_THR(VCORE_0625, total_mcps)) {
            new_req_idx = DVFSRC_REQ_IDX5;
        }
        break;
    case VCORE_0700:
        if (IS_OVER_THR(VCORE_0700, total_mcps)) {
            new_req_idx = DVFSRC_REQ_IDX8;
        } else if (IS_LOWER_THR(VCORE_0625, total_mcps)) {
            new_req_idx = DVFSRC_REQ_IDX1;
        }
        break;
    case VCORE_0800:
        if (IS_LOWER_THR(VCORE_0625, total_mcps)) {
            new_req_idx = DVFSRC_REQ_IDX1;
        } else if (IS_LOWER_THR(VCORE_0700, total_mcps)) {
            new_req_idx = DVFSRC_REQ_IDX5;
        }
        break;
    default:
        new_req_idx = DVFSRC_REQ_UNINIT;
        break;
    }

    /* request DVFSRC and modify ADSP CPU freq */
    if (new_req_idx != cur_req_idx) {
        if (new_req_idx == DVFSRC_REQ_IDX8) {
            DVFS_LOG_W("Do not support 0.8V in mt6779 for now.\n");
            configASSERT(0);
            return -2;
        }

        uint32_t new_vcore = vcore_dvfs[new_req_idx].vcore;
        uint32_t new_cpu_freq = vcore_dvfs[new_req_idx].cpu_freq;

        update_system_loading((uint32_t)(vcore_threshold[new_vcore].max_freq / 10));
        if (new_req_idx > cur_req_idx) {   //vcore up
            total_mcps += (vcore_threshold[new_vcore].max_freq - vcore_threshold[cur_vcore].max_freq) / 10;
            /* ToDo: notify and wait AP: switch clock to MMPLL_D4 */
            request_dfs_level(vcore_dvfs[new_req_idx].level);
            /* update ADSP freq */
            g_active_adsp_mux_value = get_adsp_clksel_by_cpu_rate(new_cpu_freq);
            adsp_clock_mux_setup(g_active_adsp_mux_value);
            /*notify AP to turn off A*/
        } else if (new_req_idx < cur_req_idx) {   //vcore down
            total_mcps -= (vcore_threshold[cur_vcore].max_freq - vcore_threshold[new_vcore].max_freq) / 10;
            g_active_adsp_mux_value = get_adsp_clksel_by_cpu_rate(new_cpu_freq);
            adsp_clock_mux_setup(g_active_adsp_mux_value);
            request_dfs_level(vcore_dvfs[new_req_idx].level);
            /* ToDo: notify and wait AP: switch clock back to ADSPPLL_D4 or ADSPPLL_D6 */
        }
        set_dvfs_global_clkrate((unsigned long long)new_cpu_freq);
        _xt_tick_divisor_init();
        dvfs_bus_latmon_set_mode(new_vcore);

        DVFS_LOG_D("Update system loading to %d due to Vcore change\n",
                (uint32_t)(vcore_threshold[new_vcore].max_freq * 0.1));
        DVFS_LOG_D("%s() cur_vcore=%d, total_mcps=%d cur_req_idx=%d, new_req_idx=%d\n",
                __FUNCTION__, cur_vcore, total_mcps, cur_req_idx, new_req_idx);
        DVFS_LOG_V("%s(), new_req_idx=0x%x, _xt_tick_divisor=%d\n",
                  __FUNCTION__,new_req_idx, _xt_tick_divisor);
    }
    return 0;
}

int32_t fix_dvfs_freq(uint32_t idx)
{
    uint32_t cur_req_idx;

    cur_req_idx = (uint32_t)get_dvfsrc_idx(drv_reg32(ADSP_DVFSRC_REQ));
    /* request DVFSRC and modify ADSP CPU freq */
    if (idx != cur_req_idx) {
        if (idx == DVFSRC_REQ_IDX8) {
            DVFS_LOG_W("Do not support 0.8V in mt6779 for now.\n");
            configASSERT(0);
            return -2;
        }

        uint32_t new_cpu_freq = vcore_dvfs[idx].cpu_freq;
        uint32_t new_level = vcore_dvfs[idx].level;
        if (idx > cur_req_idx) {   //vcore up
            request_dfs_level(new_level);
            /* update ADSP freq */
            g_active_adsp_mux_value = get_adsp_clksel_by_cpu_rate(new_cpu_freq);
            adsp_clock_mux_setup(g_active_adsp_mux_value);
        } else if (idx < cur_req_idx) {   //vcore down
            g_active_adsp_mux_value = get_adsp_clksel_by_cpu_rate(new_cpu_freq);
            adsp_clock_mux_setup(g_active_adsp_mux_value);
            request_dfs_level(new_level);
        }
        set_dvfs_global_clkrate((unsigned long long)new_cpu_freq);
        _xt_tick_divisor_init();
        dvfs_bus_latmon_set_mode(vcore_dvfs[idx].vcore);

        DVFS_LOG_D("Fix adsp freq = %d\n", new_cpu_freq);
    }
    return 0;
}

void dvfs_bus_latmon_irq_handler(void)
{
    uint32_t dvfsrc_state = drv_reg32(ADSP_DVFSRC_STATE);
    uint32_t latmon_margin = drv_reg32(ADSP_LATMON_MARGIN);
    latmon_mode = drv_reg32(ADSP_LATMON_DVFS_MODE);
    latmon_threshold = drv_reg32(ADSP_LATMON_THRESHOLD);
    latmon_state = drv_reg32(ADSP_LATMON_STATE);
    latmon_account = drv_reg32(ADSP_LATMON_ACCCNT);
    drv_write_reg32(ADSP_LATMON_BASE, latmon_mode);
    drv_write_reg32(ADSP_LATMON_BASE + 1*4, latmon_threshold);
    drv_write_reg32(ADSP_LATMON_BASE + 2*4, latmon_state);
    drv_write_reg32(ADSP_LATMON_BASE + 3*4, latmon_account);
    drv_write_reg32(ADSP_LATMON_BASE + 4*4, dvfsrc_state);
    drv_write_reg32(ADSP_LATMON_BASE + 5*4, latmon_margin);
    DVFS_LOG_V("%s() dvfs mode=0x%x, state=0x%x, mar=0x%x, thr=0x%x, acc=0x%x, dvfsrc_state=0x%x",
               __FUNCTION__, latmon_mode, latmon_state, latmon_margin,
               latmon_threshold, latmon_account, dvfsrc_state);

}

void dvfs_bus_latmon_init(void)
{
    uint32_t mode = 0;
    uint32_t val = 0;

    /* use margi & threshold to de-bounce latency urgent,
     * both hifi3 irq and dvfsrc are rasied.
     */
    dvfs_bus_latmon_set_mode(0);

    for (mode = 0; mode < VCORE_NUM; mode++) {
        drv_write_reg32(ADSP_LATMON_CONT0 + (4 * mode), (latmon_cont[mode].thr << 16) | latmon_cont[mode].budget);
    }
    //set window 38.46 ns * (timer_prd * 2048) = 630 us, where 38.46 ns = 1 / 26MHz (timer_prd)=8
    val = drv_reg32(ADSP_LATMON_CON2);
    drv_write_reg32(ADSP_LATMON_CON2, ((val & ~0x000001F0)| (LATMON_TIMER_PRD_VALUE << LATMON_TIMER_PRD_BIT)));

    request_irq(LATENCY_MON_IRQn, dvfs_bus_latmon_irq_handler, "LATENCY_MON_IRQn");
    __asm__ __volatile__("isync");
    __asm__ __volatile__("memw");
}

static void dvfs_bus_latmon_set_mode(uint32_t mode)
{
    latmon_mode = mode;
    drv_write_reg32(ADSP_LATMON_DVFS_MODE, mode);
    drv_write_reg32(ADSP_LATMON_MARGIN, latmon_cont[mode].budget); /*1 transaction */
    DVFS_LOG_V("%s() mode=%d, margin=0x%x\n", __FUNCTION__, mode, drv_reg32(ADSP_LATMON_MARGIN));
}

static void dvfs_bus_latmon_set_narrow_margin_fortest(uint32_t mode)
{
    drv_write_reg32(ADSP_LATMON_MARGIN, latmon_cont[mode].thr - 0x8);
    DVFS_LOG_V("%s() mode=%d, margin=0x%x\n",  __FUNCTION__, mode, drv_reg32(ADSP_LATMON_MARGIN));
}

EventGroupHandle_t xSysEventGroup;
void dvfs_init(boot_mode_t boot_mode)
{
    /* request system resource: SPM, CLOCK MUX, DVFSRC */
    g_sleep_flag = 0;
    g_active_adsp_mux_value = ADSPPLL_D6;
    request_dfs_level(vcore_dvfs[DVFSRC_REQ_IDX1].level);
    set_dvfs_global_clkrate((unsigned long long)vcore_dvfs[DVFSRC_REQ_IDX1].cpu_freq);
    _xt_tick_divisor_init();

    if (boot_mode == ADSP_REBOOT) {
        /*Test: add ip here */
#ifdef PARSER_DEMO
        test_case_add_swip();
#endif
        return;
    }

#if configUSE_WAKEUP_SOURCE == 1
    /*register wakeup source IRQ */
    request_irq(WAKEUP_SRC_IRQn, wakeup_src_handler, "WAKEUP");
    mask_irq(WAKEUP_SRC_IRQn);
    /* setup default wakeup source mask */
    drv_write_reg32(ADSP_WAKEUPSRC_MASK, 0xFFFFFFFF);
#endif

    feature_manager_init();
    /* adsp kernel request SPM src before power on adsp. Do not need to
       request again here */
    update_system_loading((uint32_t)(vcore_threshold[VCORE_0625].max_freq / 10)); // 10% of 0.625V MCPS
    config_parser_init();
    dvfs_bus_latmon_init();

    adsp_ipi_registration(IPI_DVFS_FIX_OPP_SET, (ipi_handler_t)adsp_fix_opp_handler, "dvfs_fix_opp");
    adsp_ipi_registration(IPI_DVFS_SET_FREQ, (ipi_handler_t)adsp_set_freq_handler, "dvfs_set_freq");

#ifdef CFG_SYSTEM_OFF_SUPPORT
    xSysEventGroup = xEventGroupCreate();
    adsp_ipi_registration(IPI_DVFS_SUSPEND, (ipi_handler_t)adsp_suspend_handler, "dvfs suspend");
#endif
}

#define ADD_FREQ_CMD            (0x1111)
#define DEL_FREQ_CMD            (0x2222)
#define FREQ_CMD_MASK           (0xFFFF0000)
#define FREQ_VALUE_MASK         (0x0000FFFF)
#define FREQ_CMD_BIT            (16)
#define TRIGGER_LATMON_IRQ_CMD  (0x00010000)
#define TRIGGER_LATMON_CMD_BIT  (16)

extern struct feature_node feature_table[ADSP_NUM_FEATURE_ID] ;

void adsp_fix_opp_handler(int id, void *data, unsigned int len)
{
    uint32_t level;
    level = *(uint32_t *)data;

    switch (level) {
    case 255:
        is_freq_fixed = 0;
        update_dvfs_freq();
        break;
    case 0:
        is_freq_fixed = 1;
        fix_dvfs_freq(DVFSRC_REQ_IDX1);
        break;
    case 1:
        is_freq_fixed = 1;
        fix_dvfs_freq(DVFSRC_REQ_IDX5);
        break;
    case 0x10000:
        is_latmon_irq_triggered = 0;
        dvfs_bus_latmon_set_mode(latmon_mode);
        break;
    case 0x10001:
        is_latmon_irq_triggered = 1;
        dvfs_bus_latmon_set_narrow_margin_fortest(latmon_mode);
        break;
    default:
        break;
    }
}

void adsp_set_freq_handler(int id, void *data, unsigned int len)
{
    uint32_t op = ((*(uint32_t *)data) & FREQ_CMD_MASK) >> FREQ_CMD_BIT;
    uint32_t value = (*(uint32_t *)data) & FREQ_VALUE_MASK;
    uint32_t ret = 0;
    struct feature_node *p_feature_node = &feature_table[SYSTEM_FEATURE_ID];

    if (is_freq_fixed) {
        DVFS_LOG_W("adsp frequency fixed. No use to add/del swip mcps.\n");
        return;
    }
    switch (op) {
    case ADD_FREQ_CMD:
        ret = add_swip_node_into_feature(p_feature_node, SWIP_ID_SYSTEM_TEST, value);
        DVFS_LOG_V("%s() add mcps value=%d (%d)\n", __FUNCTION__, value, ret);
        break;
    case DEL_FREQ_CMD:
        ret = delete_swip_node_from_feature(p_feature_node, SWIP_ID_SYSTEM_TEST, value);
        DVFS_LOG_V("%s() del mcps value=%d (%d)\n", __FUNCTION__, value, ret);
        break;
    default:
        break;
    }
    if (ret != 0) {
        DVFS_LOG_E("%s, op=%d, mcps=%d fail\n",__FUNCTION__, SYSTEM_FEATURE_ID, value);
    } else {
        update_dvfs_freq();
    }
}

uint32_t dump_dvfs_status(char *buf, int size)
{
    uint32_t n = 0, idx = 0, state = 0;

    idx = (uint32_t)get_dvfsrc_idx(drv_reg32(ADSP_DVFSRC_REQ));
    state = (uint32_t)get_dvfsrc_idx(drv_reg32(ADSP_DVFSRC_STATE));

    n += snprintf(buf + n, size - n, "dvfs request index(%u)\n", idx);
    n += snprintf(buf + n, size - n, "    .level     = %u\n", vcore_dvfs[idx].level);
    n += snprintf(buf + n, size - n, "    .ddr_rate  = %u\n", vcore_dvfs[idx].ddr_rate);
    n += snprintf(buf + n, size - n, "    .vcore     = %u (0:0.625V, 1:0.7V, 2:0.8V)\n", vcore_dvfs[idx].vcore);
    n += snprintf(buf + n, size - n, "    .cpu_freq  = %u\n", vcore_dvfs[idx].cpu_freq);

    n += snprintf(buf + n, size - n, "dvfs current\n");
    n += snprintf(buf + n, size - n, "    .level     = %u\n", mt6779_opp_table[state].level);
    n += snprintf(buf + n, size - n, "    .ddr_rate  = %u\n", mt6779_opp_table[state].ddr_rate);
    n += snprintf(buf + n, size - n, "    .vcore     = %u (0:0.625V, 1:0.7V, 2:0.8V)\n", mt6779_opp_table[state].vcore);
    n += snprintf(buf + n, size - n, "    .cpu_freq  = %llu\n", g_cpu_clock_hz);

    n += snprintf(buf + n, size - n, "dvfs fix freq enable = %s\n", is_freq_fixed ? "Yes" : "No");

    return n;
}

#ifdef CFG_SYSTEM_OFF_SUPPORT
/*ipi: IPI_DVFS_DISABLE, HIFI3 suspend, ask hifi3 into WFI */
void adsp_suspend_handler(int id, void *data, unsigned int len)
{
    /* Set event suspend, it will clr at IDLE hook handle */
    xEventGroupSetBits(xSysEventGroup, EVENTBIT_SUSPEND);
}
#endif

void adsp_update_sys_status(uint32_t status) INTERNAL_FUNCTION;
void adsp_update_sys_status(uint32_t status) {
    DRV_WriteReg32(ADSP_SYS_STATUS, status);
}
