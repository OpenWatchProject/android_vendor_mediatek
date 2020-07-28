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

#ifndef __MT_I2C_H__
#define __MT_I2C_H__

#include <stdint.h>
#include <stdbool.h>
#include <platform_mtk.h>
#include <mt_reg_base.h>

/* define error for CHRE */
#define EINVAL  1
#define ENXIO   2
#define EBUSY   3
#define ENOWR   4

#define I2CTAG                "[I2C-SCP] "

// #define I2C_DEBUG
#ifdef I2C_DEBUG
#define I2CLOG(fmt, arg...)   PRINTF_E(I2CTAG fmt, ##arg)
#define I2CDBG(fmt, arg...)   PRINTF_E(I2CTAG fmt, ##arg)
#define I2CPF(fmt, arg...)    PRINTF_E(fmt, ##arg)
#else
#define I2CLOG(fmt, arg...)   PRINTF_D(I2CTAG fmt, ##arg)
#define I2CDBG(fmt, arg...)
#define I2CPF(fmt, arg...)
#endif
#define I2CERR(fmt, arg...)   PRINTF_E(I2CTAG "%d: "fmt, __LINE__, ##arg)

// #define MACH_FPGA
#ifdef MACH_FPGA
#define CONFIG_MT_I2C_FPGA_ENABLE
#endif

#define I2C_MB()    __DMB()
#define I2C_BUG_ON(a)
#define I2C_M_RD       0x0001

#define I2C_OK                              0x0000
#define EAGAIN_I2C                          11  /* Try again */
#define EINVAL_I2C                          22  /* Invalid argument */
#define EOPNOTSUPP_I2C                      95  /* Operation not supported on transport endpoint */
#define ETIMEDOUT_I2C                       110 /* Connection timed out */
#define EREMOTEIO_I2C                       121 /* Remote I/O error */
#define ENOTSUPP_I2C                        524 /* Remote I/O error */
#define I2C_WRITE_FAIL_HS_NACKERR           0xA013
#define I2C_WRITE_FAIL_ACKERR               0xA014
#define I2C_WRITE_FAIL_TIMEOUT              0xA015

/******************************************register operation***********************************/
#define I2C_NR      1   /* Number of I2C controllers */

enum {
    I2C0 = 0,
    I2C1 = 1,
};

/*
I2C0_BASE,I2C1_BASE,CLK_CTRL_BASE defined in mt_reg_base.h
*/

#define OFFSET_CLK_CG_CTRL  (0x30)
#define OFFSET_BCLK_CK_SEL (0x48)
#define OFFSET_I3C_BCLK_SW (0x50)

#define SCP_CLK_CG_CTRL (CLK_CTRL_BASE + OFFSET_CLK_CG_CTRL)
#define SCP_I3C_BCLK_SW (CLK_CTRL_BASE + OFFSET_I3C_BCLK_SW)
#define SCP_BCLK_CK_SEL (CLK_CTRL_BASE + OFFSET_BCLK_CK_SEL)

#define I2C_MCLK_CG_BIT (1 << 3)
#define I2C_BCLK_CG_BIT (1 << 4)
#define I3C0_CG_BIT (1 << 21)
#define I3C1_CG_BIT (1 << 22)
#define DMA_CH0_CG_BIT (1 << 22)
#define DMA_CH1_CG_BIT (1 << 23)

#define SCP_BCLK_CK_SEL_ULPOSC_DIV8 (0x2)
#define SCP_I3C_BCLK_SW_ULPOSC_DIV4 (0x2)
#define SCP_I3C_BCLK_SW_CLKSYS_DIV4 (0x0)


#define SCP_BCLK_CK_STA_MASK (0xf << 8)
#define SCP_BCLK_CK_STA_ULPOSC_DIV8 (0x4 << 8)
#define SCP_I3C_BCLK_CK_STA_ULPOSC_DIV4 (0x4 << 8)
#define SCP_I3C_BCLK_CK_STA_CLKSYS_DIV4 (0x1 << 8)



#ifdef CONFIG_MT_I2C_FPGA_ENABLE
#define FPGA_CLOCK      12000   /* FPGA crystal frequency (KHz) */
#define I2C_CLK_DIV     (5*2)   /* frequency divisor */
#define I2C_CLK_RATE    (FPGA_CLOCK / I2C_CLK_DIV)  /* I2C base clock (KHz) */
#define SCP_I2C_CLK     (FPGA_CLOCK / 16)
#else
#define I2C_CLK_RATE    14000 /* TODO: Calculate from bus clock */
#define I2C_CLK_DIV     16  /* frequency divisor */
#define SCP_I2C_CLK     (26000 / 2) /* TODO: Check the correct frequency */
#endif

#define I2C_ULPOSC          250000
#define I2C_CLKSYS          26000
#define I2C_SRC_CLK_RATE    I2C_ULPOSC / 8
#ifndef CFG_VCORE_DVFS_SUPPORT
#define I3C_SRC_CLK_RATE    I2C_CLKSYS / 4
#else
#define I3C_SRC_CLK_RATE    I2C_ULPOSC / 4
#endif
#define DUTY_CYCLE          45


enum I2C_REGS_OFFSET {
    OFFSET_DATA_PORT = 0x0, /* 0x0 */
    OFFSET_SLAVE_ADDR = 0x04,   /* 0x04 */
    OFFSET_INTR_MASK = 0x08,    /* 0x08 */
    OFFSET_INTR_STAT = 0x0C,    /* 0x0C */
    OFFSET_CONTROL = 0x10,  /* 0X10 */
    OFFSET_TRANSFER_LEN = 0x14, /* 0X14 */
    OFFSET_TRANSAC_LEN = 0x18,  /* 0X18 */
    OFFSET_DELAY_LEN = 0x1C,    /* 0X1C */
    OFFSET_TIMING = 0x20,   /* 0X20 */
    OFFSET_START = 0x24,    /* 0X24 */
    OFFSET_EXT_CONF = 0x28,
    OFFSET_LTIMING = 0x2C,
    OFFSET_FIFO_STAT = 0x30,    /* 0X30 */
    OFFSET_FIFO_THRESH = 0x34,  /* 0X34 */
    OFFSET_FIFO_ADDR_CLR = 0x38,    /* 0X38 */
    OFFSET_IO_CONFIG = 0x40,    /* 0X40 */
    OFFSET_RSV_DEBUG = 0x44,    /* 0X44 */
    OFFSET_HS = 0x48,   /* 0X48 */
    OFFSET_SOFTRESET = 0x50,    /* 0X50 */
    OFFSET_DCM_EN = 0x54,   /* 0X54 */
    OFFSET_PATH_DIR = 0x60,
    OFFSET_DEBUGSTAT = 0x64,    /* 0X64 */
    OFFSET_DEBUGCTRL = 0x68,    /* 0x68 */
    OFFSET_TRANSFER_LEN_AUX = 0x6C, /* 0x6C */
    OFFSET_CLOCK_DIV = 0x70,
};

enum I2C_REGS_OFFSET_V2 {
    V2_OFFSET_DATA_PORT = 0x0,
    V2_OFFSET_SLAVE_ADDR = 0x04,
    V2_OFFSET_INTR_MASK = 0x08,
    V2_OFFSET_INTR_STAT = 0x0c,
    V2_OFFSET_CONTROL = 0x10,
    V2_OFFSET_TRANSFER_LEN = 0x14,
    V2_OFFSET_TRANSAC_LEN = 0x18,
    V2_OFFSET_DELAY_LEN = 0x1c,
    V2_OFFSET_TIMING = 0x20,
    V2_OFFSET_START = 0x24,
    V2_OFFSET_EXT_CONF = 0x28,
    V2_OFFSET_LTIMING = 0x2c,
    V2_OFFSET_FIFO_ADDR_CLR = 0x38,
    V2_OFFSET_SOFTRESET = 0x50,

    /*v2 use different offset*/
    V2_OFFSET_HS = 0x30,
    V2_OFFSET_IO_CONFIG = 0x34,
    V2_OFFSET_TRANSFER_LEN_AUX = 0x44,
    V2_OFFSET_CLOCK_DIV = 0x48,
    V2_OFFSET_DEBUGSTAT = 0xe4,
    V2_OFFSET_DEBUGCTRL = 0xe8,
    V2_OFFSET_FIFO_STAT = 0xf4,
    V2_OFFSET_FIFO_THRESH = 0xf8,
    V2_OFFSET_AED_PATCH = 0x80,
    V2_OFFSET_DCM_EN = 0xf88,

    /*v2 add*/
    V2_OFFSET_TRAFFIC = 0x54,
    V2_OFFSET_COMMAND = 0x58,
    V2_OFFSET_CRC_CODE_ = 0x5c,
    V2_OFFSET_TERNARY = 0x60,
    V2_OFFSET_IBI_TIMING = 0x64,
    V2_OFFSET_SHAPE = 0x6c,
    V2_OFFSET_HFIFO_DATA = 0x70,
    V2_OFFSET_ERROR = 0x84,
    V2_OFFSET_DELAY_STEP = 0xd4,
    V2_OFFSET_DELAY_SAMPLE = 0xd8,
    V2_OFFSET_DMA_INFO = 0xdc,
    V2_OFFSET_IRQ_INFO = 0xe0,
    V2_OFFSET_DMA_FSM_DEBUG = 0xec,
    V2_OFFSET_HFIFO_STAT = 0xfc,
    V2_OFFSET_ROLLBACK = 0xf98,

    /*not in v2*/
    V2_OFFSET_PATH_DIR = 0xfff, /*0x60*/
};

#define I2C_BUS_ERR          (0x01 << 8)
#define I2C_IBI                   (0x01 << 7)
#define I2C_HS_NACKERR            (1 << 2)
#define I2C_ACKERR                (1 << 1)
#define I2C_TRANSAC_COMP          (1 << 0)
#define I2C_INTR_ALL         (0x1FF)

#define I2C_FIFO_SIZE             8

#define MAX_ST_MODE_SPEED         100   /* khz */
#define MAX_FS_MODE_SPEED         400   /* khz */
#define MAX_HS_MODE_SPEED         3400  /* khz */

#define MAX_DMA_TRANS_SIZE        65532 /* Max(65535) aligned to 4 bytes = 65532 */
#define MAX_DMA_TRANS_NUM         256

#define MAX_SAMPLE_CNT_DIV        8
#define MAX_STEP_CNT_DIV          64
#define MAX_HS_STEP_CNT_DIV       8

#define AP_DMA_BASE         (0x11000000)
#define DMA_ADDRESS_HIGH    (0xC0000000)
#ifndef CFG_VCORE_DVFS_SUPPORT
#define CLOCK_DIV                0x0
#else
#define CLOCK_DIV                0x6/* for more calculated clk */
#endif
/* Set DMA register */
#define DMA_DIR_WR              (1 << 18)
#define DMA_WPEN                (1 << 17)
#define DMA_WPSD                (1 << 16)
#define DMA_START_EN            (1 << 15)
#define DMA_DRQ                 (1 << 4)
#define DMA_DINC                (1 << 3)
#define DMA_SINC                (1 << 2)

/******** end ***********/
#ifdef AP_DMA
enum DMA_REGS_OFFSET {
    OFFSET_INT_FLAG = 0x0,
    OFFSET_INT_EN = 0x04,
    OFFSET_EN = 0x08,
    OFFSET_RST = 0x0C,
    OFFSET_CON = 0x18,
    OFFSET_TX_MEM_ADDR = 0x1C,
    OFFSET_RX_MEM_ADDR = 0x20,
    OFFSET_TX_LEN = 0x24,
    OFFSET_RX_LEN = 0x28,
};
#else
enum DMA_REGS_OFFSET {
    OFFSET_SRC_ADDR = 0x0,
    OFFSET_DST_ADDR = 0x04,
    OFFSET_WPPT = 0x08,
    OFFSET_WPPO = 0x0C,
    OFFSET_COUNT = 0x10,
    OFFSET_CON = 0x14,
    OFFSET_DMA_START = 0x18,
    OFFSET_INTSTA = 0x1C,
    OFFSET_ACKINT = 0x20,
    OFFSET_RLCT = 0x24,
    OFFSET_LIMITER = 0x28,
};
#endif

enum i2c_trans_st_rs {
    I2C_TRANS_STOP = 0,
    I2C_TRANS_REPEATED_START,
};

typedef enum {
    ST_MODE,
    FS_MODE,
    HS_MODE,
} I2C_SPEED_MODE;

enum mt_trans_op {
    I2C_MASTER_NONE = 0,
    I2C_MASTER_WR = 1,
    I2C_MASTER_RD,
    I2C_MASTER_WRRD,
};


#define I2C_TRANSAC_START       (1 << 0)
#define I2C_FIFO_ADDR_CLR       0x0001
#define I2C_DELAY_LEN           0x000A
#define I2C_ST_START_CON        0x8001
#define I2C_FS_START_CON        0x1801
#define I2C_SOFT_RST            0x0001
#define I2C_IO_CONFIG_OPEN_DRAIN    0x0003

/* CONTROL */
#define I2C_CONTROL_RS                  (0x1 << 1)
#define I2C_CONTROL_DMA_EN              (0x1 << 2)
#define I2C_CONTROL_CLK_EXT_EN          (0x1 << 3)
#define I2C_CONTROL_DIR_CHANGE          (0x1 << 4)
#define I2C_CONTROL_ACKERR_DET_EN       (0x1 << 5)
#define I2C_CONTROL_TRANSFER_LEN_CHANGE (0x1 << 6)
#define I2C_CONTROL_DMAACK_EN           (0x1 << 8)
#define I2C_CONTROL_ASYNC_MODE          (0x1 << 9)
#define I2C_CONTROL_WRAPPER             (0x1 << 0)


/***********************************end of register operation****************************************/

/***********************************I2C Param********************************************************/
struct mt_trans_data {
    uint16_t trans_num;
    uint16_t data_size;
    uint16_t trans_len;
    uint16_t trans_auxlen;
};

typedef struct mt_i2c_t {
    /* ==========set in i2c probe============ */
    uint32_t base;      /* i2c base addr */
    uint16_t id;
    uint16_t irqnr;     /* i2c interrupt number */
    uint16_t irq_stat;  /* i2c interrupt status */
    uint32_t clk;       /* host clock speed in khz */
    uint32_t pdn;       /*clock number */
    /* ==========common data define============ */
    enum i2c_trans_st_rs st_rs;
    enum mt_trans_op op;
    uint32_t pdmabase;
    uint32_t speed;     /* The speed (khz) */
    uint16_t delay_len; /* number of half pulse between transfers in a trasaction */
    uint32_t msg_len;   /* number of half pulse between transfers in a trasaction */
    uint8_t *msg_buf;   /* pointer to msg data      */
    uint8_t addr;       /* The address of the slave device, 7bit,the value include read/write bit. */
    uint8_t master_code;    /* master code in HS mode */
    uint8_t mode;       /* ST/FS/HS mode */
    /* ==========reserved funtion============ */
    uint8_t is_push_pull_enable;    /* IO push-pull or open-drain */
    uint8_t is_clk_ext_disable; /* clk entend default enable */
    uint8_t is_dma_enabled; /* Transaction via DMA instead of 8-byte FIFO */
    uint8_t read_flag;  /* read,write,read_write */
    bool dma_en;
    bool poll_en;
    bool pushpull;      /* open drain */
    bool filter_msg;    /* filter msg error log */
    bool i2c_3dcamera_flag; /* flag for 3dcamera */

    /* ==========define reg============ */
    uint16_t timing_reg;
    uint16_t high_speed_reg;
    uint16_t control_reg;
    struct mt_trans_data trans_data;
} mt_i2c;

/* ============================================================================== */
/* I2C Exported Function */
/* ============================================================================== */
extern int32_t i2c_read(mt_i2c *i2c, uint8_t *buffer, uint32_t len);
extern int32_t i2c_write(mt_i2c *i2c, uint8_t *buffer, uint32_t len);
extern int32_t i2c_write_read(mt_i2c *i2c, uint8_t *buffer, uint32_t write_len,
                              uint32_t read_len);
extern int32_t i2c_hw_init(void);
extern int32_t i2c_init(void);

// #define I2C_EARLY_PORTING
// #define CONFIG_MT_I2C_FPGA_ENABLE
#ifdef I2C_EARLY_PORTING
extern int32_t mt_i2c_test(void);
#endif

#endif              /* __MT_I2C_H__ */
