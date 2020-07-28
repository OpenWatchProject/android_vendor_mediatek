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

#ifndef _DVFS_SUB_COMMON_
#define _DVFS_SUB_COMMON_

/* ARMV7 RGs */
#define ITM_CONTROL             0xE0000E80
#define DWT_CONTROL             0xE0001000
#define DWT_CYCCNT              0xE0001004
#define DEMCR                   0xE000EDFC

#define CPU_RESET_CYCLECOUNTER() \
do { \
    DRV_SetReg32(DEMCR,0x01000000); \
    DRV_WriteReg32(DWT_CYCCNT,0); \
    DRV_SetReg32(DWT_CONTROL,1); \
} while (0)

enum {
    IN_DEBUG_IDLE = 0,
    ENTERING_SLEEP,
    IN_SLEEP,
    ENTERING_ACTIVE,
    IN_ACTIVE,
    DEBUG_REQ,
    DEBUG_ACK,
};

typedef enum  {
    VCORE_VOLTAGE_600 = 0,
    VCORE_VOLTAGE_650,
    VCORE_VOLTAGE_700,
    VCORE_VOLTAGE_750,
    VCORE_VOLTAGE_800,
    VCORE_VOLTAGE_850,
    VCORE_VOLTAGE_TYPE_NUM,
} voltage_enum;

typedef enum {
    PULL_HIGH,
    PULL_LOW,
} gpio_enum;

enum scp_uart_src {
    UART_26M = 0,
    UART_32K,
    UART_ULPOSC_DIV12,
};

enum bclk_src {
    BLK_SYS_DIV8 = 0,
    BLK_32K,
    BLK_ULPOSC_DIV8,
};

enum spi_src {
    SPI_SYS_DIV4 = 0,
    SPI_32K,
    SPI_ULPOSC_DIV4,
};

enum {
    BY_TIMER = 0,
    BY_COMPILER,
    BY_SEMAPHORE,
    BY_WAKELOCK,
    BY_IPI_BUSY,
    BY_SLP_DISABLED,
    BY_SLP_BUSY,
    NR_REASONS,
};

enum {
    SCP_SLEEP_OFF = 0,
    SCP_SLEEP_ON,
    SCP_SLEEP_NO_WAKEUP,
    SCP_SLEEP_NO_CONDITION
};

enum {
    WAKE_CKSW_SEL_NORMAL_CLK_HIGH = 0x0,
    WAKE_CKSW_SEL_NORMAL_CLK_HIGH_DIV = 0x1
};

/*****************************************
 * Extenal Variables
 ****************************************/
extern uint32_t ap_resource_ack_cnt;

/*****************************************
 * ADB CMD Control APIs
 ****************************************/
extern voltage_enum get_cur_volt(void);

/*****************************************
 * AP/INFRA Resource APIs
 ****************************************/
extern void init_dram_resrc_req_cnt(void);
extern void enable_infra(scp_reserve_mem_id_t dma_id, uint32_t wait_ack);
extern int32_t get_AP_ack_from_isr(scp_reserve_mem_id_t dma_id);
extern void enable_AP_resource_from_isr(scp_reserve_mem_id_t dma_id);

/*****************************************
 * Sleep Control
 ****************************************/
extern void set_CM4_CLK_AO(void);
extern void disable_CM4_CLK_AO(void);
extern void use_SPM_ctrl_sleep_mode(void);
extern void use_SCP_ctrl_sleep_mode(void);
extern void enable_SCP_sleep_mode(void);
extern void disable_SCP_sleep_mode(void);
extern void set_vreq_count(unsigned int val);
extern void set_cm4_scr(void);
extern void scp_sleep_ctrl(int id, void* data, unsigned int len);
extern void pmicw_sleep_req(uint32_t enable);
extern void scp_check_clk_src(void);
extern uint32_t scp_check_clk_sys_off(void);
extern void clkctrl_isr(void);
extern void unmask_all_int(void);
extern void mask_all_int(void);
extern void clr_cm4_scr(void);
extern void clkctrl2_isr(void);
extern void wake_cksw_sel_normal(int val);
extern void wake_cksw_sel_slow(int val);

#endif /* _DVFS_SUB_COMMON_ */
