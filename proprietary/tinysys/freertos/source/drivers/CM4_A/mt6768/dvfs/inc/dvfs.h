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
#include <dma.h>
#include <dvfs_common.h>

/***********************************************************************************
** Definitions
************************************************************************************/

#define DVFSTAG

#ifdef BUILD_SLT
#define SCP_WAKEUP_BY_TIMER_TEST
#endif

#define DO_ULPOSC1_CALIBRATION      1
#define DO_ULPOSC2_CALIBRATION      1

#define PMICW_SLEEP_REQ_FLOW        1

#define AUTO_DUMP_SLEPP_STATUS      1
#define FAST_WAKEUP_SUPPORT         0

#define SRCLKEN_RC_ENABLE           0

#define SCP_DDREN_AUTO_MODE         1

#define SWITCH_UART_CLK_TO_ULPOSC   1

#define DUMMY_TASK_TEST             0
#if DUMMY_TASK_TEST
#define DVFS_TASK_DELAY            5000
#define SLEEP_WAKEUP_INTERCHANGE_TEST   0
#define INTERCHANGE_INTERVAL            1
#endif

/*#define SUPPORT_PMIC_VOW_LP_MODE */

#define CLK_HIGH_ALWAYS_ON          0
#define CLK_HIGH_CORE_ALWAYS_ON     0

#define DVFS_TIMER_PROFILING        0
#define SLEEP_TIMER_PROFILING       0

#ifdef SUPPORT_PMIC_VOW_LP_MODE
#define MT6358_PMIC_REG_BASE (0x0)
#define MT6358_BUCK_VCORE_SSHUB_CON0        ((unsigned int)(MT6358_PMIC_REG_BASE+0x14a4))
#define MT6358_LDO_VSRAM_OTHERS_SSHUB_CON0  ((unsigned int)(MT6358_PMIC_REG_BASE+0x1bc4))

#define PMIC_RG_BUCK_VCORE_SSHUB_SLEEP_VOSEL_EN_ADDR         MT6358_BUCK_VCORE_SSHUB_CON0
#define PMIC_RG_BUCK_VCORE_SSHUB_SLEEP_VOSEL_EN_MASK         0x1
#define PMIC_RG_BUCK_VCORE_SSHUB_SLEEP_VOSEL_EN_SHIFT        1

#define PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_SLEEP_VOSEL_EN_ADDR   MT6358_LDO_VSRAM_OTHERS_SSHUB_CON0
#define PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_SLEEP_VOSEL_EN_MASK   0x1
#define PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_SLEEP_VOSEL_EN_SHIFT  1
#endif

#ifdef CFG_TESTSUITE_SUPPORT
#define MT6358_PMIC_REG_BASE (0x0)
#define MT6358_BUCK_VCORE_SSHUB_CON1        ((unsigned int)(MT6358_PMIC_REG_BASE+0x14a6))
#define PMIC_RG_BUCK_VCORE_SSHUB_VOSEL_ADDR         MT6358_BUCK_VCORE_SSHUB_CON1
#define PMIC_RG_BUCK_VCORE_SSHUB_VOSEL_MASK         0x7f
#define PMIC_RG_BUCK_VCORE_SSHUB_VOSEL_SHIFT        0

#define MT6358_LDO_VSRAM_OTHERS_SSHUB_CON1        ((unsigned int)(MT6358_PMIC_REG_BASE+0x1bc6))
#define PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_VOSEL_ADDR         MT6358_LDO_VSRAM_OTHERS_SSHUB_CON1
#define PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_VOSEL_MASK         0x7f
#define PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_VOSEL_SHIFT        0

#define MT6358_BUCK_VCORE_SSHUB_CON0        ((unsigned int)(MT6358_PMIC_REG_BASE+0x14a4))
#define PMIC_RG_BUCK_VCORE_SSHUB_EN_ADDR         MT6358_BUCK_VCORE_SSHUB_CON0
#define PMIC_RG_BUCK_VCORE_SSHUB_EN_MASK         0x1
#define PMIC_RG_BUCK_VCORE_SSHUB_EN_SHIFT        0

#define MT6358_LDO_VSRAM_OTHERS_SSHUB_CON0  ((unsigned int)(MT6358_PMIC_REG_BASE+0x1bc4))
#define PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_EN_ADDR   MT6358_LDO_VSRAM_OTHERS_SSHUB_CON0
#define PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_EN_MASK   0x1
#define PMIC_RG_LDO_VSRAM_OTHERS_SSHUB_EN_SHIFT  0
#endif

#define REQ_ULPOSC_WAIT_TIME_US 150
#define REQ_26M_WAIT_TIME_US    700
#define REQ_INFRA_WAIT_TIME_US  150
#define REQ_DRAM_WAIT_TIME_US   500

/* Uart Clk Setting */
#define UART0_CK_SEL_MSK        0x3
#define UART0_CK_SEL_SHFT       0
#define UART0_CK_STA_MSK        0xf
#define UART0_CK_STA_SHFT       8
#define UART1_CK_SEL_MSK        0x3
#define UART1_CK_SEL_SHFT       16
#define UART1_CK_STA_MSK        0xf
#define UART1_CK_STA_SHFT       24

/* Wakeup Source ID */
typedef enum  {
    IPC0_WAKE_CLK_CTRL = 0,
    IPC1_WAKE_CLK_CTRL,
    IPC2_WAKE_CLK_CTRL,
    IPC3_WAKE_CLK_CTRL,
    SPM_WAKE_CLK_CTRL,
    CIRQ_WAKE_CLK_CTRL,
    EINT_WAKE_CLK_CTRL,
    PMIC_WAKE_CLK_CTRL,
    UART_WAKE_CLK_CTRL,
    UART1_WAKE_CLK_CTRL,
    I2C0_WAKE_CLK_CTRL,
    I2C1_WAKE_CLK_CTRL,
    I2C2_WAKE_CLK_CTRL,
    CLK_CTRL_WAKE_CLK_CTRL,
    MAD_FIFO_WAKE_CLK_CTRL,
    TIMER0_WAKE_CLK_CTRL,
    TIMER1_WAKE_CLK_CTRL,
    TIMER2_WAKE_CLK_CTRL,
    TIMER3_WAKE_CLK_CTRL,
    TIMER4_WAKE_CLK_CTRL,
    TIMER5_WAKE_CLK_CTRL,
    OS_TIMER_WAKE_CLK_CTRL,
    UART_RX_WAKE_CLK_CTRL,
    UART1_RX_WAKE_CLK_CTRL,
    DMA_WAKE_CLK_CTRL,
    AUDIO_WAKE_CLK_CTRL,
    MD1_FD216_WAKE_CLK_CTRL,
    MD1_WAKE_CLK_CTRL,
    C2K_WAKE_CLK_CTRL,
    SPI0_WAKE_CLK_CTRL,
    SPI1_WAKE_CLK_CTRL,
    SPI2_WAKE_CLK_CTRL,

    /* Wakeup Source MSB ID */
    RESERVED_0_WAKE_CLK_CTRL = 32,
    DEBUG_IRQ_WAKE_CLK_CTRL,
    CCIF_0_WAKE_CLK_CTRL,
    CCIF_1_WAKE_CLK_CTRL = 35,
    CCIF_2_WAKE_CLK_CTRL,
    WDT_WAKE_CLK_CTRL,
    USB0_WAKE_CLK_CTRL,
    USB1_WAKE_CLK_CTRL,
    TWAM_WAKE_CLK_CTRL = 40,
    INFRA_WAKE_CLK_CTRL,
    HWDVFS_HIGH_WAKE_CLK_CTRL,
    HWDVFS_LOW_WAKE_CLK_CTRL,
    CLK_CTRL_2_WAKE_CLK_CTRL,
    IPC4_WAKE_CLK_CTRL = 45,
    PERIBUS_TIMEOUT_WAKE_CLK_CTRL,
    INFRABUS_TIMEOUT_WAKE_CLK_CTRL,
    AP_WDT_WAKE_CLK_CTRL = 52,
    AP_EINT_WAKE_CLK_CTRL,
    AP_EINT_EVT_WAKE_CLK_CTRL,
    MAD_DATA_WAKE_CLK_CTRL = 55,
    FAKE0_WAKE_CLK_CTRL,
    FAKE1_WAKE_CLK_CTRL,
    FAKE2_WAKE_CLK_CTRL,
    FAKE3_WAKE_CLK_CTRL,
    FAKE4_WAKE_CLK_CTRL = 60,
    FAKE5_WAKE_CLK_CTRL,
    FAKE6_WAKE_CLK_CTRL,
    FAKE7_WAKE_CLK_CTRL,
    NR_WAKE_CLK_CTRL
} wakeup_src_id;

typedef enum  {
    CLK_OPP0 = 0,
    CLK_OPP1,
    CLK_OPP2,
    CLK_OPP3,
    CLK_OPP_NUM,
    CLK_OPP_UNINIT = 0xffff,
} clk_opp_enum;

typedef enum  {
    FREQ_82MHZ = 82,
    FREQ_110MHZ = 110,
    FREQ_125MHZ = 125,
    FREQ_130MHZ = 130,
    FREQ_165MHZ = 165,
    FREQ_250MHZ = 250,
    FREQ_260MHZ = 260,
    FREQ_273MHZ = 273,
    FREQ_328MHZ = 328,
    FREQ_330MHZ = 330,
    FREQ_416MHZ = 416,
    FREQ_UNINIT = 0xffff,
} freq_enum;

typedef struct {
    uint32_t    osc_cali;
} ulposc_con_t;

typedef enum  {
    ULPOSC_1 = 0,
    ULPOSC_2,
    ULPOSC_NUM,
} ulposc_id_enum;

enum {
    SEL_32K_CLK = 0,
    SEL_26M_CLK = 1,
    SEL_B_CLK = 2,
    SEL_M_CLK = 3,
};

enum {
    FREQ_METER_ABIST_AD_OSC_CK = 37,
    FREQ_METER_ABIST_AD_OSC_CK_2 = 38,
};

typedef enum {
    CLK_SEL_SYS     = 0x0,
    CLK_SEL_32K     = 0x1,
    CLK_SEL_ULPOSC2 = 0x2,
    CLK_SEL_ULPOSC1 = 0x3
} clk_sel_val;

enum {
    CLK_DIV_1 = 0,
    CLK_DIV_2 = 1,
    CLK_DIV_4 = 2,
    CLK_DIV_8 = 3
};

#ifdef SUPPORT_PMIC_VOW_LP_MODE
enum {
    VOW_LPMODE_OFF,
    VOW_LPMODE_ON
};

typedef struct {
    int    req_state;
    int    cur_state;
} pmic_vow_lpmode_state_t;
#endif

/***********************************************************************************
** Extern Variables
************************************************************************************/

/*****************************************
 * CLK Inform Dump APIs
 ****************************************/
extern void scp_set_opp(void);
extern freq_enum get_cur_clk(void);

/*****************************************
 * CLK_HIGH and CLK_SYS Control APIs
 ****************************************/
extern uint32_t get_clk_sys_ack(void);
extern uint32_t get_clk_sys_safe_ack(void);
extern uint32_t get_clk_sw_select(void);
extern void turn_off_clk_sys_from_isr(void);
extern uint32_t turn_on_clk_sys_from_isr(uint32_t wait_ack);
extern void request_clk_high_opp(uint32_t req_opp);
extern void request_clk_sys_opp(uint32_t req_opp);
extern void request_dfs_opp(uint32_t req_opp);
extern freq_enum convert_opp_2_clk(clk_opp_enum clk_opp);

#ifdef CFG_TESTSUITE_SUPPORT
extern int convert_opp_2_pll(clk_opp_enum clk_opp);
extern unsigned int pmic_scp_set_vcore(uint32_t req_volt);
extern unsigned int pmic_scp_set_vsram_others(uint32_t req_volt);
#endif

extern clk_opp_enum convert_clk_2_opp(freq_enum clk_freq);
extern void clk_div_select(uint32_t val);

extern void turn_on_clk_high(ulposc_id_enum ulposc);
extern void turn_off_clk_high(ulposc_id_enum ulposc);

extern void select_clk_source(clk_sel_val val);

/*****************************************
 * AP/INFRA Resource APIs
 ****************************************/
extern int32_t check_dram_ack_status(void);
extern void dvfs_enable_DRAM_resource(scp_reserve_mem_id_t dma_id);
extern void dvfs_disable_DRAM_resource(scp_reserve_mem_id_t dma_id);
extern void dvfs_enable_DRAM_resource_from_isr(scp_reserve_mem_id_t dma_id);
extern void dvfs_disable_DRAM_resource_from_isr(scp_reserve_mem_id_t dma_id);
extern void scp_wakeup_src_setup(wakeup_src_id irq, uint32_t enable);
extern int32_t check_infra_state(void);
extern void enable_clk_bus(scp_reserve_mem_id_t dma_id);
extern void disable_clk_bus(scp_reserve_mem_id_t dma_id);
extern void enable_clk_bus_from_isr(scp_reserve_mem_id_t dma_id);
extern void disable_clk_bus_from_isr(scp_reserve_mem_id_t dma_id);

extern void sshub_enable_AP_resource_nonblock(scp_reserve_mem_id_t dma_id);
extern int32_t sshub_get_AP_ack_nonblock(scp_reserve_mem_id_t dma_id);
extern unsigned int dvfs_enable_DRAM_no_wait_for_logger(scp_reserve_mem_id_t dma_id);
extern void dvfs_disable_DRAM_no_wait_for_logger(scp_reserve_mem_id_t dma_id);
void enable_dram_resource_for_dmgr(scp_reserve_mem_id_t dma_id);
int32_t get_dram_ack_from_isr_for_dmgr(void);

/*****************************************
 * DVFS Init APIs
 ****************************************/
extern void ulposc_cali_init(void);
extern void dvfs_init(void);
extern void reset_dvfs_global_variables(void);

#ifdef SUPPORT_PMIC_VOW_LP_MODE
extern int request_pmic_lp_mode_for_vow_from_isr(int enabled);
extern int request_pmic_lp_mode_for_vow(int enabled);
extern void get_vow_lpmode_state_from_isr(int *req_state, int *cur_state);
extern void get_vow_lpmode_state(int *req_state, int *cur_state);
#endif

#endif /* _VCOREDVFS_ */
