#ifndef MT_REG_BASE_H
#define MT_REG_BASE_H

#ifndef __ASSEMBLER__
enum SCP_CORE_ID {
    SCP_A_ID = 0,
    SCP_B_ID,
    SCP_CORE_TOTAL = 2,
};

enum HIFI3_CORE_ID {
    HIFI3_A_ID = 0,
    HIFI3_CORE_TOTAL = 1,
};

#define SCP_CORE                 HIFI3_A_ID
#endif  /* __ASSEMBLER__ */

#define ADSP_CFGREG_BASE                  0x30000000

#define ADSP_CFGREG_SW_RSTN              (ADSP_CFGREG_BASE + 0x00)
#define ADSP_HIFI3_IO_CONFIG             (ADSP_CFGREG_BASE + 0x08)

#define ADSP_HIFI3_INTR                  (ADSP_CFGREG_BASE + 0x14)
#define ADSP_SW_INT_SET                  (*(volatile unsigned int *)(ADSP_CFGREG_BASE + 0x1C)) //change
#define ADSP_SW_INT_CLR                  (*(volatile unsigned int *)(ADSP_CFGREG_BASE + 0x20)) //change

#define ADSP_GENERAL_IRQ_SET             (*(volatile unsigned int *)(ADSP_CFGREG_BASE + 0x34)) // 0: sysc_irq 4:adsp2conn
#define ADSP_GENERAL_IRQ_CLR             (*(volatile unsigned int *)(ADSP_CFGREG_BASE + 0x38))

#define ADSP_DVFSRC_STATE                (ADSP_CFGREG_BASE + 0x3C)
#define ADSP_DVFSRC_REQ                  (ADSP_CFGREG_BASE + 0x40)
#define ADSP_DDREN_REQ                   (ADSP_CFGREG_BASE + 0x44) //check with FK
#define ADSP_DDR_ENABLE                  (0x1 << 0)
#define ADSP_SSPM_REQ                    (ADSP_CFGREG_BASE + 0x48)
#define ADSP_SPM_SRC_BITS                (0xF << 0)
#define ADSP_SSPM_ACK                    (ADSP_CFGREG_BASE + 0x4C)

#define ADSP_IRQ_EN                      (ADSP_CFGREG_BASE + 0x50)
#define ADSP_SEMAPHORE                   (ADSP_CFGREG_BASE + 0x58)
#define ADSP_SPM_WAKEUPSRC               (*(volatile unsigned int *)(ADSP_CFGREG_BASE + 0x5C))


#define ADSP_CFGREG_WDT                  (ADSP_CFGREG_BASE + 0x7C)
#define ADSP_WDT_INIT_VALUE              (ADSP_CFGREG_BASE + 0x80)
#define ADSP_WDT_CNT                     (ADSP_CFGREG_BASE + 0x84)

#define ADSP_CFGREG_RSV_RW_REG0          (ADSP_CFGREG_BASE + 0x8C) //Reserved R/W register
#define ADSP_CFGREG_RSV_RW_REG1          (ADSP_CFGREG_BASE + 0x90) //Reserved R/W register
#define ADSP_CFGREG_RSV_RW_REG2          (ADSP_CFGREG_BASE + 0x94) //Reserved R/W register

#define ADSP_WAKEUPSRC_MASK              (ADSP_CFGREG_BASE + 0xA8)
#define ADSP_WAKEUPSRC_IRQ               (ADSP_CFGREG_BASE + 0xAC)

#define ADSP_PC_E                        (ADSP_CFGREG_BASE + 0xC4)
#define DBG_EXCCAUSE                     (ADSP_CFGREG_BASE + 0xD8)
#define DBG_EXCVADDR                     (ADSP_CFGREG_BASE + 0xDC)

#define ADSP_LATMON_DVFS_MODE            (ADSP_CFGREG_BASE + 0x100)
#define ADSP_LATMON_CON1                 (ADSP_CFGREG_BASE + 0x104)
#define ADSP_LATMON_CON2                 (ADSP_CFGREG_BASE + 0x108)
#define ADSP_LATMON_MARGIN               (ADSP_CFGREG_BASE + 0x10C)
#define ADSP_LATMON_THRESHOLD            (ADSP_CFGREG_BASE + 0x110)
#define ADSP_LATMON_STATE                (ADSP_CFGREG_BASE + 0x114)
#define ADSP_LATMON_ACCCNT               (ADSP_CFGREG_BASE + 0x118)
#define ADSP_LATMON_CONT0                (ADSP_CFGREG_BASE + 0x12C)
#define ADSP_LATMON_CONT1                (ADSP_CFGREG_BASE + 0x130)
#define ADSP_LATMON_CONT2                (ADSP_CFGREG_BASE + 0x134)

#define ADSP_HIFI3_SLEEP                 (ADSP_CFGREG_BASE + 0x158)
#define ADSP_DBG_PEND_CNT                (ADSP_CFGREG_BASE + 0x15C)
// WDT latch
#define ADSP_DBG_PC_LATCH                (ADSP_CFGREG_BASE + 0x170)
#define ADSP_DBG_SP_LATCH                (ADSP_CFGREG_BASE + 0x174)
#define ADSP_DBG_EXCVADDR_LATCH          (ADSP_CFGREG_BASE + 0x178)
#define ADSP_DBG_EXCCAUSE_LATCH          (ADSP_CFGREG_BASE + 0x17C)

#define ADSP_CK_CTRL_BASE                (ADSP_CFGREG_BASE + 0x1000)
#define ADSP_CK_EN                       (ADSP_CK_CTRL_BASE + 0x00)
#define ADSP_MCLK_DIV_REG                (ADSP_CK_CTRL_BASE + 0x04)
#define ADSP_DCM_CTRL                    (ADSP_CK_CTRL_BASE + 0x08)

#define ADSP_DMA_BASE                    (ADSP_CFGREG_BASE + 0x2000)
#define ADSP_TIMER_BASE                  (ADSP_CFGREG_BASE + 0x3000)

#define ADSP_OS_TIMER_CNT_L              (ADSP_CFGREG_BASE + 0x308C)
#define ADSP_OS_TIMER_CNT_H              (ADSP_CFGREG_BASE + 0x3090)

#define ADSP_UART_BASE                   (ADSP_CFGREG_BASE + 0x4000)
#define ADSP_BUS_MON_BASE                (ADSP_CFGREG_BASE + 0x5000)

#define BUS_DBG_CON                      (ADSP_CFGREG_BASE + 0x5000)
#define BUS_DBG_TIMER_CON0               (ADSP_CFGREG_BASE + 0x5004)
#define BUS_DBG_TIMER_CON1               (ADSP_CFGREG_BASE + 0x5008)
#define BUS_DBG_TIMER0                   (ADSP_CFGREG_BASE + 0x500C)
#define BUS_DBG_TIMER1                   (ADSP_CFGREG_BASE + 0x5010)
#define BUS_DBG_WP                       (ADSP_CFGREG_BASE + 0x5014)
#define BUS_DBG_WP_MASK                  (ADSP_CFGREG_BASE + 0x5018)
#define BUS_DBG_AR_TRACK0_L              (ADSP_CFGREG_BASE + 0x5100)
#define BUS_DBG_AR_TRACK1_L              (ADSP_CFGREG_BASE + 0x5108)
#define BUS_DBG_AR_TRACK2_L              (ADSP_CFGREG_BASE + 0x5110)
#define BUS_DBG_AR_TRACK3_L              (ADSP_CFGREG_BASE + 0x5118)
#define BUS_DBG_AR_TRACK4_L              (ADSP_CFGREG_BASE + 0x5120)
#define BUS_DBG_AR_TRACK5_L              (ADSP_CFGREG_BASE + 0x5128)
#define BUS_DBG_AR_TRACK6_L              (ADSP_CFGREG_BASE + 0x5130)
#define BUS_DBG_AR_TRACK7_L              (ADSP_CFGREG_BASE + 0x5138)
#define BUS_DBG_AW_TRACK0_L              (ADSP_CFGREG_BASE + 0x5200)
#define BUS_DBG_AW_TRACK1_L              (ADSP_CFGREG_BASE + 0x5208)
#define BUS_DBG_AW_TRACK2_L              (ADSP_CFGREG_BASE + 0x5210)
#define BUS_DBG_AW_TRACK3_L              (ADSP_CFGREG_BASE + 0x5218)
#define BUS_DBG_AW_TRACK4_L              (ADSP_CFGREG_BASE + 0x5220)
#define BUS_DBG_AW_TRACK5_L              (ADSP_CFGREG_BASE + 0x5228)
#define BUS_DBG_AW_TRACK6_L              (ADSP_CFGREG_BASE + 0x5230)
#define BUS_DBG_AW_TRACK7_L              (ADSP_CFGREG_BASE + 0x5238)

#define ADSP_DTCM_SHARE_BASE             (CFG_HIFI3_DRAM_ADDRESS + CFG_HIFI3_DRAM_SIZE)
#define ADSP_MPU_INFO_BASE               (ADSP_DTCM_SHARE_BASE - 0x0020)
#define ADSP_TIMESYC_BASE                (ADSP_DTCM_SHARE_BASE - 0x0040)
#define ADSP_IPC_BUF_BASE                (ADSP_DTCM_SHARE_BASE - 0x0280)
#define ADSP_AUDIO_IPI_BASE              (ADSP_DTCM_SHARE_BASE - 0x0680)
#define ADSP_AP_WAKELOCK_BASE            (ADSP_DTCM_SHARE_BASE - 0x0684)
#define ADSP_SYS_STATUS                  (ADSP_DTCM_SHARE_BASE - 0x0688)
#define ADSP_BUS_MON_BACKUP_BASE         (ADSP_DTCM_SHARE_BASE - 0x0744)
#define ADSP_INFRA_BUS_DUMP_BASE         (ADSP_DTCM_SHARE_BASE - 0x07E4)
#define ADSP_LATMON_BASE                 (ADSP_DTCM_SHARE_BASE - 0x0800)

#define INFRA_BUS_DUMP_SLOT_01           (ADSP_CK_EN)               /* ADSP_INFRA_BUS_DUMP_BASE +   0 */
#define INFRA_BUS_DUMP_SLOT_02           (ADSP_CFGREG_RSV_RW_REG0)  /* ADSP_INFRA_BUS_DUMP_BASE +   4 */    /* counter */
#define INFRA_BUS_DUMP_SLOT_03           (ADSP_CFGREG_RSV_RW_REG0)  /* ADSP_INFRA_BUS_DUMP_BASE +   8 */    /* save a1 */
#define INFRA_BUS_DUMP_SLOT_04           (ADSP_CFGREG_RSV_RW_REG0)  /* ADSP_INFRA_BUS_DUMP_BASE +  12 */    /* save a2 */
#define INFRA_BUS_DUMP_SLOT_05           (ADSP_HIFI3_SLEEP)         /* ADSP_INFRA_BUS_DUMP_BASE +  16 */
#define INFRA_BUS_DUMP_SLOT_06           (ADSP_SYS_STATUS)          /* ADSP_INFRA_BUS_DUMP_BASE +  20 */
#define INFRA_BUS_DUMP_SLOT_07           (ADSP_PC_E)                /* ADSP_INFRA_BUS_DUMP_BASE +  24 */
#define INFRA_BUS_DUMP_SLOT_08           (DBG_EXCCAUSE)             /* ADSP_INFRA_BUS_DUMP_BASE +  28 */
#define INFRA_BUS_DUMP_SLOT_09           (DBG_EXCVADDR)             /* ADSP_INFRA_BUS_DUMP_BASE +  32 */
#define INFRA_BUS_DUMP_SLOT_10           (ADSP_OS_TIMER_CNT_L)      /* ADSP_INFRA_BUS_DUMP_BASE +  36 */
#define INFRA_BUS_DUMP_SLOT_11           (ADSP_OS_TIMER_CNT_H)      /* ADSP_INFRA_BUS_DUMP_BASE +  40 */
#define INFRA_BUS_DUMP_SLOT_12           (ADSP_HIFI3_INTR)          /* ADSP_INFRA_BUS_DUMP_BASE +  44 */
#define INFRA_BUS_DUMP_SLOT_13           (ADSP_DBG_PEND_CNT)        /* ADSP_INFRA_BUS_DUMP_BASE +  48 */
#define INFRA_BUS_DUMP_SLOT_14           (BUS_DBG_CON)              /* ADSP_INFRA_BUS_DUMP_BASE +  52 */
#define INFRA_BUS_DUMP_SLOT_15           (BUS_DBG_TIMER_CON0)       /* ADSP_INFRA_BUS_DUMP_BASE +  56 */
#define INFRA_BUS_DUMP_SLOT_16           (BUS_DBG_TIMER_CON1)       /* ADSP_INFRA_BUS_DUMP_BASE +  60 */
#define INFRA_BUS_DUMP_SLOT_17           (BUS_DBG_AR_TRACK0_L)      /* ADSP_INFRA_BUS_DUMP_BASE +  64 */
#define INFRA_BUS_DUMP_SLOT_18           (BUS_DBG_AR_TRACK1_L)      /* ADSP_INFRA_BUS_DUMP_BASE +  68 */
#define INFRA_BUS_DUMP_SLOT_19           (BUS_DBG_AR_TRACK2_L)      /* ADSP_INFRA_BUS_DUMP_BASE +  72 */
#define INFRA_BUS_DUMP_SLOT_20           (BUS_DBG_AR_TRACK3_L)      /* ADSP_INFRA_BUS_DUMP_BASE +  76 */
#define INFRA_BUS_DUMP_SLOT_21           (BUS_DBG_AR_TRACK4_L)      /* ADSP_INFRA_BUS_DUMP_BASE +  80 */
#define INFRA_BUS_DUMP_SLOT_22           (BUS_DBG_AR_TRACK5_L)      /* ADSP_INFRA_BUS_DUMP_BASE +  84 */
#define INFRA_BUS_DUMP_SLOT_23           (BUS_DBG_AR_TRACK6_L)      /* ADSP_INFRA_BUS_DUMP_BASE +  88 */
#define INFRA_BUS_DUMP_SLOT_24           (BUS_DBG_AR_TRACK7_L)      /* ADSP_INFRA_BUS_DUMP_BASE +  92 */
#define INFRA_BUS_DUMP_SLOT_25           (BUS_DBG_AW_TRACK0_L)      /* ADSP_INFRA_BUS_DUMP_BASE +  96 */
#define INFRA_BUS_DUMP_SLOT_26           (BUS_DBG_AW_TRACK1_L)      /* ADSP_INFRA_BUS_DUMP_BASE + 100 */
#define INFRA_BUS_DUMP_SLOT_27           (BUS_DBG_AW_TRACK2_L)      /* ADSP_INFRA_BUS_DUMP_BASE + 104 */
#define INFRA_BUS_DUMP_SLOT_28           (BUS_DBG_AW_TRACK3_L)      /* ADSP_INFRA_BUS_DUMP_BASE + 108 */
#define INFRA_BUS_DUMP_SLOT_29           (BUS_DBG_AW_TRACK4_L)      /* ADSP_INFRA_BUS_DUMP_BASE + 112 */
#define INFRA_BUS_DUMP_SLOT_30           (BUS_DBG_AW_TRACK5_L)      /* ADSP_INFRA_BUS_DUMP_BASE + 116 */
#define INFRA_BUS_DUMP_SLOT_31           (BUS_DBG_AW_TRACK6_L)      /* ADSP_INFRA_BUS_DUMP_BASE + 120 */
#define INFRA_BUS_DUMP_SLOT_32           (BUS_DBG_AW_TRACK7_L)      /* ADSP_INFRA_BUS_DUMP_BASE + 124 */
#define INFRA_BUS_DUMP_SLOT_33           (BUS_DBG_WP)               /* ADSP_INFRA_BUS_DUMP_BASE + 128 */
#define INFRA_BUS_DUMP_SLOT_34           (BUS_DBG_WP_MASK)          /* ADSP_INFRA_BUS_DUMP_BASE + 132 */
#define INFRA_BUS_DUMP_SLOT_35           (ADSP_CFGREG_RSV_RW_REG0)  /* ADSP_INFRA_BUS_DUMP_BASE + 136 */
#define INFRA_BUS_DUMP_SLOT_36           (ADSP_CFGREG_RSV_RW_REG0)  /* ADSP_INFRA_BUS_DUMP_BASE + 140 */
#define INFRA_BUS_DUMP_SLOT_37           (ADSP_CFGREG_RSV_RW_REG0)  /* ADSP_INFRA_BUS_DUMP_BASE + 144 */
#define INFRA_BUS_DUMP_SLOT_38           (ADSP_CFGREG_RSV_RW_REG0)  /* ADSP_INFRA_BUS_DUMP_BASE + 148 */
#define INFRA_BUS_DUMP_SLOT_39           (ADSP_CFGREG_RSV_RW_REG0)  /* ADSP_INFRA_BUS_DUMP_BASE + 152 */
#define INFRA_BUS_DUMP_SLOT_40           (ADSP_CFGREG_RSV_RW_REG0)  /* ADSP_INFRA_BUS_DUMP_BASE + 156 */
#define NUM_OF_INFRA_BUS_HANG_REGISTERS  (40)

#define TOPCKGEN_BASE                    0x10000000
#define CLK_CFG_11_STA                   (TOPCKGEN_BASE + 0x00D0)
#define CLK_CFG_11_SET                   (TOPCKGEN_BASE + 0x00D4)
#define CLK_CFG_11_CLR                   (TOPCKGEN_BASE + 0x00D8)
#define CLK_CFG_UPDATE1                  (TOPCKGEN_BASE + 0x0008)

#define ADSP_CK_UPDATE                   (0x1 << 16)
#define ADSP_SEL_BASE                    (24)

#define PLL_BASE                         0x1000C000
#define ADSPPLL_CON0                     (PLL_BASE + 0x02B0)
#define ADSPPLL_CON1                     (PLL_BASE + 0x02B4)
#define ADSPPLL_PWR_CON0                 (PLL_BASE + 0x02BC)

#endif /* MT_REG_BASE_H */
