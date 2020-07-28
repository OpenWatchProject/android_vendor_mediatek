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

#ifndef __MT_REG_BASE_H__
#define __MT_REG_BASE_H__

#define CFGREG_BASE                  0x405C0000

/* CFGREG BASE */
#define AP_RESOURCE                  (CFGREG_BASE + 0x04)
#define BUS_RESOURCE                 (CFGREG_BASE + 0x08)

#define TCM_LOCK_CFG                 (CFGREG_BASE + 0x10)

//#define SCP_TO_HOST_INT              (CFGREG_BASE + 0x14)
//#define SCP_TO_HOST_INT_2            (CFGREG_BASE + 0x18)

#define SCP_A_TO_HOST                (CFGREG_BASE + 0x1C)
#define SCP_A_TO_HOST_REG            (*(volatile unsigned int *)(CFGREG_BASE + 0x1C))
    #define IPC_SCP2HOST_SSHUB       (0xff << 16)
    #define WDT_INT                  (0x1 << 8)
    #define IPC_SCP2HOST             (0xff << 0)
    #define IPC_SCP2HOST_BIT         (0x1 << 0)

#define SCP_TO_SPM_INT               (CFGREG_BASE + 0x20)
#define SCP_TO_SPM_REG               (*(volatile unsigned int *)SCP_TO_SPM_INT)

#define SCP_TO_SPM_INT_2             (CFGREG_BASE + 0x24)
#define SCP_GIPC_IN                  (CFGREG_BASE + 0x28)
#define GIPC_TO_SCP_REG              (*(volatile unsigned int *)(CFGREG_BASE + 0x28))

#define SCP_TO_CONN_INT              (CFGREG_BASE + 0x2C)
#define SCP_TO_CONN_REG              (*(volatile unsigned int *)SCP_TO_CONN_INT)

#define SCP_GPR_GPR0                 (CFGREG_BASE + 0x50)
#define SCP_GPR_GPR1                 (CFGREG_BASE + 0x54)
#define SCP_GPR_GPR2                 (CFGREG_BASE + 0x58)
#define SCP_GPR_GPR3                 (CFGREG_BASE + 0x5c)
#define EXPECTED_FREQ_REG        SCP_GPR_GPR3
#define SCP_GPR_GPR4                 (CFGREG_BASE + 0x60)
#define CURRENT_FREQ_REG         SCP_GPR_GPR4

#define SCP_GPR_GPR5                 (CFGREG_BASE + 0x64)
#define SCP_GPR_CM4_A_REBOOT         (CFGREG_BASE + 0x64)
    #define CM4_A_READY_TO_REBOOT    0x34
    #define CM4_A_REBOOT_OK          0x1
#define SCP_GPR_GPR6                 (CFGREG_BASE + 0x68)
#define SCP_GPR_CM4_B_REBOOT         (CFGREG_BASE + 0x68)
    #define CM4_B_READY_TO_REBOOT    0x35
#define SCP_GPR_GPR7                 (CFGREG_BASE + 0x6c)

#define WDT_REG_BASE                 (CFGREG_BASE + 0x84)       /*SCP A WDT*/
    #define WDT_EN_VALUE             (1 << 31)
    #define WDT_VALUE_MASK           0x7FFFF

#define SEMAPHORE                    (CFGREG_BASE + 0x90)
#define SEM_REG                      (*(volatile unsigned int *)SEMAPHORE)

#define SPM2SCP_GPR                  (CFGREG_BASE + 0x9C)

#define CM4_CONTROL                  (CFGREG_BASE + 0xA0)
#define JTAG_DBG_REQ_BIT         3
#define SCP_FPU_FLAGS                (CFGREG_BASE + 0xA4)
#define REG_SP                       (CFGREG_BASE + 0xA8)
#define REG_LR                       (CFGREG_BASE + 0xAC)
#define REG_PSP                      (CFGREG_BASE + 0xB0)
#define REG_PC                       (CFGREG_BASE + 0xB4)
#define SCP_SLP_PROTECT_CFG          (CFGREG_BASE + 0xC8)
#define P_CACHE_SLP_PROT_EN          (1 << 3)
#define D_CACHE_SLP_PROT_EN          (1 << 4)

#define SCP_ONE_TIME_LOCK            (CFGREG_BASE + 0xDC)
#define SCP_SECURE_CRTL              (CFGREG_BASE + 0xE0)
#define SCP_REMAP_DISABLE          (1<<31)
#define SCP_JTAG_DISABLE           (1<<21)
#define SCP_AP_RW_TCM_DISABLE      (1<<20)
#define SCP_ENABLE_SPM_MASK_VREQ_BIT   28

#define SYS_CTRL                     (CFGREG_BASE + 0xE4)
#define AHB_SLICE_POST_WT   1
#define AUTO_DDREN_BIT      18
#define DDREN_FIX_VALUE_BIT 28
#define BYPASS_P_CACHE      30
#define BYPASS_D_CACHE      31

#define BUS_QOS                      (CFGREG_BASE + 0xE8)
#define AXI_AWULTRA    0
#define AXI_ARULTRA    6

#define REMAP_CFG1                   (CFGREG_BASE + 0x120)
#define REMAP_CFG2                   (CFGREG_BASE + 0x124)
#define REMAP_CFG3                   (CFGREG_BASE + 0x128)
#define L1C_REMAP_CFG0               (CFGREG_BASE + 0x12C)
    #define L1C_SHIFT_MASK           0xfff00000
    #define L1C_ALIGN                0x00100000
    #define L1C_EXT_ADDR_SHIFT       20
#define L1C_REMAP_CFG1               (CFGREG_BASE + 0x130)
#define L1C_REMAP_CFG2               (CFGREG_BASE + 0x134)
#define L1C_REMAP_CFG3               (CFGREG_BASE + 0x138)
#define L1C_REMAP_OTHER              (CFGREG_BASE + 0x13C)



#define MDAIF_BASE                 (CFGREG_BASE + 0x1000)

/* INTC Registers */
#define IRQ_STATUS                   (CFGREG_BASE + 0x2000)  /* SCP Interrupt Status */
#define SCP_INTC_IRQ_STATUS          (CFGREG_BASE + 0x2000)

#define SCP_INTC_IRQ_ENABLE          (CFGREG_BASE + 0x2004)
#define IRQ_ENABLE                   (CFGREG_BASE + 0x2004)  /* SCP IRQ Enable */
    #define CLK_CTRL_IRQ_EN_BIT        13
    #define CLK_CTRL_IRQ_EN            (1<<13)
    #define IPC3_IRQ_EN                (1<<3)
    #define IPC2_IRQ_EN                (1<<2)
    #define IPC1_IRQ_EN                (1<<1)
    #define IPC0_IRQ_EN                (1<<0)

#define SCP_INTC_IRQ_OUT             (CFGREG_BASE + 0x2008)  /* Final Interrupt output (IRQ_STATUS & IRQ_EN) */
#define SCP_INTC_IRQ_SLEEP_EN        (CFGREG_BASE + 0x200C)
#define IRQ_SLEEP_EN                 (CFGREG_BASE + 0x200C)  /* IRQ WAKE UP  Enable */
    #define CLK_CTRL_SLP_IRQ_EN        (1<<13)
    #define I2C2_SLP_IRQ_EN            (1<<12)
    #define I2C1_SLP_IRQ_EN            (1<<11)
    #define I2C0_SLP_IRQ_EN            (1<<10)
    #define UART1_SLP_IRQ_EN           (1<<9)
    #define UART_SLP_IRQ_EN            (1<<8)
    #define PMIC_SLP_IRQ_EN            (1<<7)
    #define EINT_SLP_IRQ_EN            (1<<6)
    #define CIRQ_SLP_IRQ_EN            (1<<5)
    #define SPM_SLP_IRQ_EN             (1<<4)
    #define IPC3_SLP_IRQ_EN            (1<<3)
    #define IPC2_SLP_IRQ_EN            (1<<2)
    #define IPC1_SLP_IRQ_EN            (1<<1)
    #define IPC0_SLP_IRQ_EN            (1<<0)

#define SCP_INTC_NMI_IRQ_SEL         (CFGREG_BASE + 0x2010)
#define SCP_INTC_SPM_WAKEUP          (CFGREG_BASE + 0x2014)
#define SCP_INTC_SPM_WAKEUP_MSB      (CFGREG_BASE + 0x2018)
#define SCP_INTC_UART_RX_IRQ         (CFGREG_BASE + 0x201C)

#define SCP_INTC_IRQ_STATUS_MSB      (CFGREG_BASE + 0x2080)  /* SCP Interrupt Status */
#define SCP_INTC_IRQ_ENABLE_MSB      (CFGREG_BASE + 0x2084)
#define IRQ_ENABLE_MSB               (CFGREG_BASE + 0x2084)  /* SCP IRQ Enable */
    #define WDT_IRQ_EN                 (1<<5)
    #define DBG_IRQ_EN                 (1<<1)

#define IRQ_OUT_MSB                  (CFGREG_BASE + 0x2088)  /* Final Interrupt output (IRQ_STATUS & IRQ_EN) */
#define IRQ_SLEEP_EN_MSB             (CFGREG_BASE + 0x208C)  /* IRQ WAKE UP  Enable */

#define SCP_TIMTER_START_ADDRESS     (CFGREG_BASE + 0x3000)
#define SCP_TIMER0_EN                (CFGREG_BASE + 0x3000)
#define TIMER_BASE                   (CFGREG_BASE + 0x3000)

#define SCP_TIMER0_RST_VAL           (CFGREG_BASE + 0x3004)
#define SCP_TIMER0_CUR_VAL           (CFGREG_BASE + 0x3008)
#define SCP_TIMER0_IRQ_CTRL          (CFGREG_BASE + 0x300C)

#define TIMER4_EN                    (CFGREG_BASE + 0x3040)
    #define TIMER4_EN_SHF       0
    #define TIMER4_CLK_SRC_SHF  4
    #define TIMER4_CLK_SRC_MSK  0x3
#define TIMER4_RST_VAL               (CFGREG_BASE + 0x3044)
#define TIMER4_CUR_VAL               (CFGREG_BASE + 0x3048)
#define TIMER4_IRQ_CTRL              (CFGREG_BASE + 0x304C)

#define TIMER5_EN                    (CFGREG_BASE + 0x3050)
    #define TIMER5_EN_SHF       0
    #define TIMER5_CLK_SRC_SHF  4
    #define TIMER5_CLK_SRC_MSK  0x3
#define TIMER5_RST_VAL               (CFGREG_BASE + 0x3054)
#define TIMER5_CUR_VAL               (CFGREG_BASE + 0x3058)
#define TIMER5_IRQ_CTRL              (CFGREG_BASE + 0x305C)

#define OSTIMER_CON                  (CFGREG_BASE + 0x3080)
#define OSTIMER_INIT_L               (CFGREG_BASE + 0x3084)
#define OSTIMER_INIT_H               (CFGREG_BASE + 0x3088)
#define OSTIMER_CUR_L                (CFGREG_BASE + 0x308C)
#define OSTIMER_CUR_H                (CFGREG_BASE + 0x3090)
#define OSTIMER_TVAL                 (CFGREG_BASE + 0x3094)
#define OSTIMER_IRQ_ACK              (CFGREG_BASE + 0x3098)
#define OS_TIMER_LATCH_CTRL          (CFGREG_BASE + 0x30A0)
    #define OS_TIMER_LATCH_EN_0        (1 << 5)
    #define OS_TIMER_LATCH_SEL_0_SHIFT 0
    #define OS_TIMER_LATCH_SEL_0_MASK  (0x1f << OS_TIMER_LATCH_SEL_0_SHIFT)
    #define OS_TIMER_LATCH_EN_1        (1 << 13)
    #define OS_TIMER_LATCH_SEL_1_SHIFT 8
    #define OS_TIMER_LATCH_SEL_1_MASK  (0x1f << OS_TIMER_LATCH_SEL_1_SHIFT)
    #define OS_TIMER_LATCH_EN_2        (1 << 21)
    #define OS_TIMER_LATCH_SEL_2_SHIFT 16
    #define OS_TIMER_LATCH_SEL_2_MASK  (0x1f << OS_TIMER_LATCH_SEL_2_SHIFT)

    #define OS_TIMER_LATCH_VALUE_0     (CFGREG_BASE + 0x30A4)
    #define OS_TIMER_LATCH_VALUE_0_MSB (CFGREG_BASE + 0x30A8)
    #define OS_TIMER_LATCH_VALUE_1     (CFGREG_BASE + 0x30AC)
    #define OS_TIMER_LATCH_VALUE_1_MSB (CFGREG_BASE + 0x30B0)
    #define OS_TIMER_LATCH_VALUE_2     (CFGREG_BASE + 0x30B4)
    #define OS_TIMER_LATCH_VALUE_2_MSB (CFGREG_BASE + 0x30B8)

/* CLK_CTRL Registers */
#define CLK_CTRL_BASE           (CFGREG_BASE + 0x4000)
#define CLK_SW_SEL              (CFGREG_BASE + 0x4000)  /* Clock Source Select */
    #define CKSW_SEL_O_BIT      8
    #define CKSW_SEL_O_MASK     0xf

#define CLK_ENABLE              (CFGREG_BASE + 0x4004)  /* Clock Source Enable */
    #define CLK_SYS_EN_BIT      0
    #define CLK_HIGH_EN_BIT     1
    #define CLK_HIGH_CG_BIT     2
    #define CLK_SYS_IRQ_EN_BIT  16
    #define CLK_HIGH_IRQ_EN_BIT 17

#define CLK_SAFE_ACK            (CFGREG_BASE + 0x4008)  /* Clock Safe Ack */
    #define CLK_SYS_SAFE_ACK_BIT    0
    #define CLK_HIGH_SAFE_ACK_BIT   1

#define CLK_ACK                 (CFGREG_BASE + 0x400C)  /* Clock Ack */
    #define CLK_SYS_ACK_BIT     0
    #define CLK_HIGH_ACK_BIT    1

#define CLK_IRQ_ACK             (CFGREG_BASE + 0x4010)  /* Clock Interrupt Acknowledge */
#define SCP_CLKCTRL_IRQ_ACK     (CFGREG_BASE + 0x4010)
#define SCP_CLK_SYS_VAL         (CFGREG_BASE + 0x4014)
#define CLK_SYS_VAL             (CFGREG_BASE + 0x4014)  /* System Clock Counter Value */
    #define CLK_SYS_VAL_BIT     0
    #define CLK_SYS_VAL_MASK    0x3ff

#define SCP_CLK_HIGH_VAL        (CFGREG_BASE + 0x4018)
#define CLK_HIGH_VAL            (CFGREG_BASE + 0x4018)  /* ULPOSC Clock Counter Value */
    #define CLK_HIGH_VAL_BIT    0
    #define CLK_HIGH_VAL_MASK   0x3ff

#define CLK_SEL_SLOW            (CFGREG_BASE + 0x401C)
    #define CLK_SW_SEL_SLOW_BIT     0
    #define CLK_SW_SEL_SLOW_MASK    0x3
    #define CLK_DIVSW_SEL_SLOW_BIT  4
    #define CLK_DIVSW_SEL_SLOW_MASK 0x3

#define SLEEP_CTRL              (CFGREG_BASE + 0x4020)  /* Sleep mode control */
#define SCP_CLKCTRL_SLEEP_CTRL  (CFGREG_BASE + 0x4020)
    #define SLP_CTRL_EN_BIT         0
    #define VREQ_COUNT_BIT          1
    #define VREQ_COUNT_MASK         0x7F
    #define SPM_SLP_MODE_BIT        8
    #define SPM_SLP_MODE_CLK_AO_BIT 9

#define CLK_DIV_SEL             (CFGREG_BASE + 0x4024)  /* Clock Divider Select */
#define SLEEP_DEBUG             (CFGREG_BASE + 0x4028)  /* Sleep mode debug signals */
#define SCP_CLKCTRL_SLEEP_DEBUG (CFGREG_BASE + 0x4028)
#define SRAM_PDN                (CFGREG_BASE + 0x402C)  /* Sram Power Down */
#define SCP_CLKCTRL_SRAM_PDN    (CFGREG_BASE + 0x402C)
#define SET_CLK_CG              (CFGREG_BASE + 0x4030)  /* Clock Gating Control */
#define SCP_SET_CLK_CG          (CFGREG_BASE + 0x4030)
/* Clock Gating ID */
    #define TMR_M_CLK_CG      0
    #define TMR_B_CLK_CG      1
    #define MAD_MCLK_CG       2
    #define I2C_MCLK_CG       3
    #define I2C_BCLK_CG       4
    #define GPIO_MCLK_CG      5
    #define AP2P_MCLK_CG      6
    #define UART_MCLK_CG      7
    #define UART_BCLK_CG      8
    #define UART_RST_N        9
    #define UART1_MCLK_CG     10
    #define UART1_BCLK_CG     11
    #define UART1_RST_N       12
    #define SPI0_CG           13
    #define SPI1_CG           14
    #define SPI2_CG           15
    #define DMA_CH0_CG        16
    #define DMA_CH1_CG        17
    #define DMA_CH2_CG        18
    #define DMA_CH3_CG        19
    #define TWAM_CG           20
    #define CACHE_I_CTRL_CG   23
    #define CACHE_D_CTRL_CG   24

#define PMICW_CTRL              (CFGREG_BASE + 0x4034)  /* PMIC Wrapper Control */
    #define PMICW_SLEEP_REQ_BIT 0
    #define PMICW_SLEEP_ACK_BIT 4
    #define PMICW_CLK_MUX_BIT   8
    #define PMICW_DCM_BIT       9

#define SLEEP_WAKE_DEBUG        (CFGREG_BASE + 0x4038)  /* Sleep state machine wakeup registe */
#define DCM_EN                  (CFGREG_BASE + 0x403C)  /* AHB bus DCM control */
#define WAKE_CKSW_SEL           (CFGREG_BASE + 0x4040)
    #define WAKE_CKSW_SEL_NORMAL_BIT    0
    #define WAKE_CKSW_SEL_NORMAL_MASK   0x3
    #define WAKE_CKSW_SEL_SLOW_BIT      4
    #define WAKE_CKSW_SEL_SLOW_MASK     0x3

#define UART_CK_SEL                  (CFGREG_BASE + 0x4044)  /* UART_CK_SEL */
#define SCP_UART_CK_SEL              (CFGREG_BASE + 0x4044)  /*Sleep Control Flow*/
#define BCK_CK_SEL                   (CFGREG_BASE + 0x4048)  /* BCLK_CK_SEL */
#define SPI_BCK_CK_SEL               (CFGREG_BASE + 0x404C)  /* SPI_BCLK_CK_SEL */
#define CLK_DIV_CNT                  (CFGREG_BASE + 0x4050)  /* PERI clock divider */
#define CPU_VREQ_CTRL                (CFGREG_BASE + 0x4054)  /* CPU_A_VREQ_CTRL */
#define CLR_CLK_CG                   (CFGREG_BASE + 0x4058)  /* PERI Clock Gating Clear */
#define CLK_HIGH_CORE                (CFGREG_BASE + 0x405C)  /* ulposc2 core control CLK_HIGH_CORE_CG */
    #define HIGH_CORE_CG_BIT   1

#define SLP_IRQ2                     (CFGREG_BASE + 0x4064)
    #define SLP_IRQ2_BIT        0
    #define SLP_IRQ2_MASK       1

#define CLK_ON_CTRL                  (CFGREG_BASE + 0x406C)
    #define HIGH_AO_BIT             0
    #define HIGH_CG_AO_BIT          2
    #define HIGH_CORE_AO_BIT        4
    #define HIGH_CORE_CG_AO_BIT     6
    #define HIGH_CORE_DIS_SUB_BIT   5
    #define HIGH_FINAL_VAL_BIT      8
    #define HIGH_FINAL_VAL_MASK     0x1f

#define CLK_CTRL_L1_SRAM_PD          (CFGREG_BASE + 0x4080)
#define CLK_CTRL_TCM_TAIL_SRAM_PD    (CFGREG_BASE + 0x4094)
#define CLK_CTRL_SLP_CTRL            (CFGREG_BASE + 0x40A0)
    #define SLOW_CLK_WAKE_DIS_BIT    0
    #define SLOW_CLK_WAKE_DIS_MASK   0x1

#define FAST_WAKE_CNT_END            (CFGREG_BASE + 0x40A4)
    #define FAST_WAKE_CNT_END_BIT   0
    #define FAST_WAKE_CNT_END_MASK  0xfff

#define I2C0_BASE                    (CFGREG_BASE + 0x5000)
#define I2C1_BASE                    (CFGREG_BASE + 0x6000)
#define I2C2_BASE                    (CFGREG_BASE + 0x7000)
#define GPIO_BASE                    (CFGREG_BASE + 0x8000)
#define UART0_BASE                   (CFGREG_BASE + 0x9000)
#define EINT_BASE                    (CFGREG_BASE + 0xA000)
#define PMICWP2P_BASE                (CFGREG_BASE + 0xB000)
#define PMIC_WRAP_WACS_CMD           (CFGREG_BASE + 0xB200)
#define PMIC_WRAP_WACS_RDATA         (CFGREG_BASE + 0xB204)
#define PMIC_WRAP_WACS_VLDCLR        (CFGREG_BASE + 0xB208)

#define SPMP2P_BASE                  (CFGREG_BASE + 0xC000)

#define DMA_BASE                     (CFGREG_BASE + 0xD000)

#define SCP_DMA_ACKINT_CHX           (CFGREG_BASE + 0xD020)

#define UART1_BASE                   (CFGREG_BASE + 0xE000)
#define SPI0_BASE                    (CFGREG_BASE + 0xF000)
#define SPI1_BASE                    (CFGREG_BASE + 0x10000)
#define SPI2_BASE                    (CFGREG_BASE + 0x11000)


#ifndef BUILD_SLT
#define MT_LOG_BUF_LEN  4096
#else
#define MT_LOG_BUF_LEN  512
#endif
/* scp semaphore definition
 * need to sync with AP side
 */
enum SEMAPHORE_FLAG{
  SEMAPHORE_CLK_CFG_5 = 0,
  SEMAPHORE_PTP,
  SEMAPHORE_I2C0,
  SEMAPHORE_I2C1,
  SEMAPHORE_TOUCH,
  SEMAPHORE_APDMA,
  SEMAPHORE_SENSOR,
  SEMAPHORE_SCP_A_AWAKE,
  SEMAPHORE_SCP_B_AWAKE,
  NR_FLAG = 9,
};

/* scp semaphore 3 way definition
 * need to sync with AP side
 */
enum SEMAPHORE_3WAY_FLAG {
    SEMA_3WAY_APDMA = 0,
    SEMA_3WAY_TOTAL = 8,
};


/*
 * scp ID definition
 */
enum SCP_CORE_ID {
    SCP_A_ID = 0,
    SCP_B_ID,
    SCP_CORE_TOTAL = 2,
};

/*
 * revise for diff scp core
 */
#define SCP_CORE                 SCP_A_ID


/* AP site RG Base */
#define AP_BASE                 0xa0000000
#define TOPCK_BASE              (AP_BASE + 0x0)

/* PLL CG control */
#define SCP_SW_CG_0_SET         (AP_BASE + 0x80)
#define SCP_SW_CG_0_CLR         (AP_BASE + 0x84)
#define SCP_SW_CG_0_STA         (AP_BASE + 0x90)
#define CLK_INFRA_SCP           4

/* AP CG CTRL */
#define CLK_AP_CG_CTRL_SET      (AP_BASE + 0x1080)
#define CLK_AP_CG_CTRL_CLR      (AP_BASE + 0x1084)
#define CLK_AP_CG_CTRL_STA      (AP_BASE + 0x1090)
#define CLK_AP_UART1_CLK        (1 << 22)

/* SCP System Reset */
#define MODULE_RESET_SET         (AP_BASE + 0x1140)
#define MODULE_RESET_CLR         (AP_BASE + 0x1144)
    #define SCP_RESET           (1 << 3)
    #define SCP_SEC_RESET       (1 << 10)


/* INFRA_IRQ (always on register) */
#define INFRA_IRQ_SET         (AP_BASE + 0x1B14)
#define INFRA_IRQ_CLR         (AP_BASE + 0x1B18)
    #define AP_AWAKE_LOCK        (0)
    #define AP_AWAKE_UNLOCK      (1)
    #define CONNSYS_AWAKE_LOCK   (2)
    #define CONNSYS_AWAKE_UNLOCK (3)

/* PLL_ULPOSC_CON0/PLL_ULPOSC_CON2 */
#define PLL_ULPOSC_CON0         (AP_BASE + 0xC700)
#define PLL_ULPOSC_CON2         (AP_BASE + 0xC708)
    #define RG_OSC_CALI_MSK         0x3f
    #define RG_OSC_CALI_SHFT        0
    #define RG_OSC_IBAND_MSK        0x7f
    #define RG_OSC_IBAND_SHFT       6
    #define RG_OSC_FBAND_MSK        0xf
    #define RG_OSC_FBAND_SHFT       13
    #define RG_OSC_DIV_MSK          0x3f
    #define RG_OSC_DIV_SHFT         17
    #define RG_OSC_CP_EN_MSK        0x1
    #define RG_OSC_CP_EN_SHFT       23
    #define RG_OSC_RSV_MSK          0xff
    #define RG_OSC_RSV_SHFT         24

/* PLL_ULPOSC_CON1/PLL_ULPOSC_CON3 */
#define PLL_ULPOSC_CON1         (AP_BASE + 0xC704)
#define PLL_ULPOSC_CON3         (AP_BASE + 0xC70C)
    #define RG_OSC_MOD_MSK          0x3
    #define RG_OSC_MOD_SHFT         0
    #define RG_OSC_DIV2_EN_MSK      0x1
    #define RG_OSC_DIV2_EN_SHFT     2

/* PERISYS_UART0 */
#define AP_UART0_BASE           (AP_BASE + 0x1002000)


/* SCP PLL MUX RG */
#define CLK_CFG_8               (TOPCK_BASE + 0x00C0)
#define CLK_CFG_8_SET           (TOPCK_BASE + 0x00C4)
#define CLK_CFG_8_CLR           (TOPCK_BASE + 0x00C8)
    #define CLK_TOP_SCP_SEL_MSK     0x7
    #define CLK_TOP_SCP_SEL_SHFT    8

/* FREQUENCY METER CONTROL */
#define CLK_MISC_CFG_0          (TOPCK_BASE + 0x0104)
#define CLK_DBG_CFG             (TOPCK_BASE + 0x010C)
#define CLK_SCP_CFG_0           (TOPCK_BASE + 0x0220)
#define CLK_SCP_CFG_1           (TOPCK_BASE + 0x0224)

/* ARMV7 System Control Register */
#define ARMV7_SYS_CTRL          0xE000ED10
#define SLEEPONEXIT_BIT         1
#define SLEEPDEEP_BIT           2
#define SEVONPEND_BIT           4
#define CM4_MOIFICATION_EMABLE      0xE00FE000
#define CM4_DCM_FEATURE             0xE00FE004
#endif
