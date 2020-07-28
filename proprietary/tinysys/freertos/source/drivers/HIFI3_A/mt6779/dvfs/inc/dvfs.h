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

#ifndef _VCOREDVFS_
#define _VCOREDVFS_

#include <mt_reg_base.h>
#include <main.h>
#include <dma.h>
#include <event_groups.h>

#include <hifi3_irq.h>

/***********************************************************************************
** Definitions
************************************************************************************/
#define AUTO_DUMP_SLEPP_STATUS      1
#define FAST_WAKEUP_SUPPORT         0

#define DVFS_TIMER_PROFILING        0
#define SLEEP_TIMER_PROFILING       0

typedef enum {
    TCK_26M_MX9_CK = 0,
    MAINPLL_D3,
    UNIVPLL_D2_D4,
    UNIVPLL_D2,
    MMPLL_D4,
    ADSPPLL_D4,
    ADSPPLL_D6
}adsp_clock_idx;

enum {
    ADSP_CLK_DIV_1 = 0,
    ADSP_CLK_DIV_2 = 1,
    ADSP_CLK_DIV_4  = 2,
    ADSP_CLK_DIV_8  = 3,
};

typedef enum {
    ADSPPLL_NULL   = 0x0,
    ADSPPLL_480MHz = 0x82127627,
    ADSPPLL_694MHz = 0x821AB13B,
    ADSPPLL_800MHz = 0x821EC4EC
} adsp_freq_enum;

enum {
    ADSP_CPU_CLOCK_HZ_L = 466666463,
    ADSP_CPU_CLOCK_HZ_M = 700000000,
    ADSP_CPU_CLOCK_HZ_H = 788888888
};

enum {
    ADSP_CPU_CLOCK_P95_HZ_L = 422000000,
    ADSP_CPU_CLOCK_P95_HZ_M = 589333333,
    ADSP_CPU_CLOCK_P95_HZ_H = 788888888,
};

enum {
    ADSP_STATUS_RESET   = 0x00,
    ADSP_STATUS_SUSPEND = 0x01,
    ADSP_STATUS_SLEEP   = 0x10,
    ADSP_STATUS_ACTIVE  = 0x11,
};

/* Wakeup Source ID == IRQn_Type */
typedef enum  {
    SPM_WAKE_CLK_CTRL           = 0,
    DEBUG_IRQ_WAKE_CLK_CTRL     = 1,
    OS_TIMER_WAKE_CLK_CTRL      = 2,
    TIMER0_WAKE_CLK_CTRL        = 3,
    TIMER1_WAKE_CLK_CTRL        = 4,
    IPC0_WAKE_CLK_CTRL          = 5,
    AP_AWAKE_CLK_CTRL           = 6,
    SW2_WAKE_CLK_CTRL           = 7,
    CONNSYS1_WAKE_CLK_CTRL      = 8,  //BTCVSD
    CONNSYS2_WAKE_CLK_CTRL      = 9,  // A2DP
    CONNSYS3_WAKE_CLK_CTRL      = 10, // ISO
    USB0_WAKE_CLK_CTRL          = 11,
    USB1_WAKE_CLK_CTRL          = 12,
    CCIF3_WAKE_CLK_CTRL         = 13,
    DMA_WAKE_CLK_CTRL           = 14,
    AUDIO_WAKE_CLK_CTRL         = 15,
    HIFI3_WDT_WAKE_CLK_CTRL     = 16,
    RESERVED_WAKE_CLK_CTRL      = 17,
    TIMER2_WAKE_CLK_CTRL        = 18,
    TIMER3_WAKE_CLK_CTRL        = 19,
    SW3_WAKE_CLK_CTRL           = 20,
    LATENCY_WAKE_CLK_CTRL       = 21,
    BUS_TRACKER_WAKE_CLK_CTRL   = 22,
    WAKEUP_SRC_WAKE_CLK_CTRL    = 23,
    INFRA_HANG_WAKE_CLK_CTRL    = 24,
    WDT_WAKE_CLK_CTRL           = 25,
    NR_WAKE_CLK_CTRL
} wakeup_src_id;

#define MCPS_LB_MUL          (2007)
#define MCPS_LB_SHIFT        (11)
#define MCPS_LB_VALUE(x)     ((x * MCPS_LB_MUL) >> MCPS_LB_SHIFT)
#define MCPS_UB_MUL          (15)
#define MCPS_UB_SHIFT        (4)
#define MCPS_UB_VALUE(x)     ((x * MCPS_UB_MUL) >> MCPS_UB_SHIFT)

#define EVENTBIT_SUSPEND  (1 << 0)

/***********************************************************************************
** Extern Variables
************************************************************************************/
extern EventGroupHandle_t xSysEventGroup;

/*****************************************
 * AP/INFRA Resource APIs
 ****************************************/
/* define null function to avoid link error in ADSP/SCP common code */
#define dvfs_enable_DRAM_resource(...)
#define dvfs_disable_DRAM_resource(...)
#define scp_wakeup_src_setup(...)
#define enable_clk_bus(...)
#define disable_clk_bus(...)
#define enable_clk_bus_from_isr(...)
#define disable_clk_bus_from_isr(...)

#define ADSP_SPM_ALL_REQ        (ADSP_SPM_DEFAULT_REQ | ADSP_SPM_AWAKE_REQ)
#define ADSP_SPM_DEFAULT_REQ    (0x9)  /*0:adsp_infra_req, 3: adsp_srclkena*/
#define ADSP_SPM_AWAKE_REQ      (0x6)  /*1:adsp_vref18, 2:adsp_apsrc_en*/
#define ADSP_MCLK_DIV_MASK      (0x3 << 0)

/*****************************************
 * DVFS Init APIs
 ****************************************/
extern void dvfs_init(boot_mode_t boot_mode);
extern void reset_dvfs_global_variables(void);

void adsp_fix_opp_handler(int id, void *data, unsigned int len);
void adsp_set_freq_handler(int id, void *data, unsigned int len);
void adsp_suspend_handler(int id, void *data, unsigned int len);
void adsp_update_sys_status(uint32_t status);
int32_t update_dvfs_freq(void);
void request_dfs_opp(uint32_t dvfs_opp);
extern void adsp_wakeup_src_setup(wakeup_src_id irq, uint32_t enable);
void adsp_clock_mux_setup(adsp_clock_idx clock) INTERNAL_FUNCTION;
/*****************************************
 * Status APIs
 ****************************************/
uint32_t adsp_get_clock_mux(void) INTERNAL_FUNCTION;

#endif /* _VCOREDVFS_ */
