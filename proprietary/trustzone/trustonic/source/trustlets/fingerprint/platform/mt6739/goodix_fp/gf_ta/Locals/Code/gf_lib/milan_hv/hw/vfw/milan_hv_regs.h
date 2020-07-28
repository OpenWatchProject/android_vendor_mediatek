/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __MILAN_HV_REGS_H__
#define __MILAN_HV_REGS_H__

#include "gf_type_define.h"

#define MILAN_HV_REG_MAX_SIZE    24

#define MILAN_HV_REG_CHIP_ID_SIZE      4
#define MILAN_HV_REG_FDT_THRES_SIZE    24
#define MILAN_HV_REG_FDT_AVG_SIZE      24

/* Currently not used */
#define MILAN_HV_REG_ADC_CTRL_SIZE     10
#define MILAN_HV_REG_MANUAL_CRC_SIZE   4
#define MILAN_HV_REG_AREA_CMP_NUM_SIZE 12

typedef enum {
    MILAN_HV_REG_CHIP_ID = 0x0000, /* RO, 4 byte   chip id*/
    MILAN_HV_REG_VENDOR_ID = 0x0006, /* RO, 2 byte   vendor id*/
    MILAN_HV_REG_GPIO_CTRL = 0x0008, /* RO, 2 byte   gpio ctrl*/
    MILAN_HV_REG_CHIP_STATE = 0x000e, /* RO, 2 byte   chip state*/
    /* RW, 2 byte   [0]:clk_1m_enable [4]:clk_sys_main_enable*/
    MILAN_HV_REG_CLOCK_GATING_CTRL = 0x0010,
    /* RW, 2 byte   [0:3]:clock for digital diveder [8:12]:clock for bg chop divider*/
    MILAN_HV_REG_CLOCK_DIV_CTRL = 0x0012,
    MILAN_HV_REG_CLOCK_RST_CTRL = 0x0014, /* WO, 2 byte   [0:4]:11111*/
    MILAN_HV_REG_MODE_CTRL0 = 0x0020, /* RW, 2 byte   osc and timer wakeup*/
    /* RW, 2 byte   [0]:time enable [4]:timer manual mode enable
     * [8:15]:timer count divider=32k/(2*pscale)*/
    MILAN_HV_REG_MODE_CTRL1 = 0x0022,
    MILAN_HV_REG_MODE_CTRL2 = 0x0024, /* RW, 2 byte   [0:15]:time int data, default-0xFFFF*/
    MILAN_HV_REG_MODE_CTRL3 = 0x0026, /* RW, 2 byte   [0:7]:timer counter [8:13]:ldo18_dly*/
    MILAN_HV_REG_ADC_CTRL = 0x0030, /* RW, 10 byte */
    /* RW, 2 byte   [0:7]:set rate clock div [8:13]:dac chp clock div*/
    MILAN_HV_REG_PIXEL_CTRL0 = 0x0050,
    MILAN_HV_REG_PIXEL_CTRL1 = 0x0052, /* RW, 2 byte   [2]:Manual mode [8:14]:line num*/
    MILAN_HV_REG_PIXEL_CTRL2 = 0x0054, /* RW, 2 byte   [0]:line scan */
    MILAN_HV_REG_PIXEL_CTRL3 = 0x0056, /* RW, 2 byte   [0:7]:start block0 [7:15]:start block1*/
    MILAN_HV_REG_PIXEL_CTRL4 = 0x0058, /* RW, 2 byte   [0:7]:start block2 [8]:block length*/
    /* RW, 2 byte   [0:3]:pxl_timing_t1 [4:7]:pxl_timing_t2 [8:15]:pxl_timing_t5*/
    MILAN_HV_REG_PIXEL_CTRL5 = 0x005a,
    MILAN_HV_REG_PIXEL_CTRL6 = 0x005c, /* RW, 2 byte   [0:12]:tx clock number*/
    MILAN_HV_REG_PIXEL_CTRL7 = 0x005e, /* RW, 2 byte   [0:7]:pxl_timing_t3*/
    MILAN_HV_REG_FIFO_USED = 0x0060, /* RO, 2 byte   [0:8]:how many data in fifo*/
    MILAN_HV_REG_PIXEL_CTRL8 = 0x0062, /* RW, 2 byte*/
    MILAN_HV_REG_PIXEL_CTRL9 = 0x0064, /* RW, 2 byte*/
    MILAN_HV_REG_PIXEL_CTRL10 = 0x0066, /* RW, 2 byte*/
    /* RW, 2 byte   [0]:encryption module enable [1]:adc data-12bit/low 8bit*/
    MILAN_HV_REG_ENCRYPT_ENABLE = 0x0070,
    MILAN_HV_REG_ENCRYPT_CTRL = 0x0072, /* RW, 4 byte   initial key word*/
    MILAN_HV_REG_MANUAL_CRC = 0x0076, /* RW, 4 byte   initial word*/
    MILAN_HV_REG_CRC_CLR = 0x007a, /* RW, 2 byte   [0]:clear crc*/
    MILAN_HV_REG_ENCRYPT_CTRL3 = 0x007c,
    MILAN_HV_REG_FDT0 = 0x0080, /* RW, 2 bytes  FDT enable and Up/Down*/
    MILAN_HV_REG_FDT_DELTA = 0x0082, /* RW, 2 byte   compare an  average threshold delta*/
    MILAN_HV_REG_FDT_THRES = 0x0084, /* RW, 20 byte  compare base and  average base*/
    MILAN_HV_REG_FDT_CMP_FLAG = 0x009e, /* RO, 2 byte   area compare  flag*/
    MILAN_HV_REG_FDT_AVG_FLAG0 = 0x00a0, /* RO, 2 byte   area average flag 0*/
    MILAN_HV_REG_FDT_AVG = 0x00a2, /* RO, 20 byte  area average data*/
    MILAN_HV_REG_CMP_NUM_CTRL = 0x00ba, /* RW, 2 byte   compare number threshold*/
    MILAN_HV_REG_AREA_CMP_NUM = 0x00bc, /* RO, 12 byte  compare valid pixel number*/
    MILAN_HV_REG_TOUCH_FLAG0 = 0x00c8, /* RO, 2 byte   which area is touch 0*/
    MILAN_HV_REG_FDT_AREA_NUM = 0x00ca, /* RW, 2 byte   [0:3]:area number touch threshold*/
    MILAN_HV_REG_FDT_AVG_FLAG1 = 0x00cc, /* RO, 2 byte   area average flag 1*/
    MILAN_HV_REG_TOUCH_FLAG1 = 0x00ce, /* RO, 2 byte   which area is touched 1*/
    MILAN_HV_REG_FDT1 = 0x00d0, /* RW, 2 byte   abnormal touch down number in wait for finger*/
    MILAN_HV_REG_FDT_thr12 = 0x00d2,
    MILAN_HV_REG_FDT_thr13 = 0x00d4,
    MILAN_HV_REG_FDT_thr14 = 0x00d6,
    MILAN_HV_REG_FDT_thr15 = 0x00d8,
    MILAN_HV_REG_AREA1213_CMP_NUM = 0x00da,
    MILAN_HV_REG_AREA1415_CMP_NUM = 0x00dc,
    MILAN_HV_REG_FDT_AVG12 = 0x00de,
    MILAN_HV_REG_FDT_AVG13 = 0x00e0,
    MILAN_HV_REG_FDT_AVG14 = 0x00e2,
    MILAN_HV_REG_FDT_AVG15 = 0x00e4,
    MILAN_HV_REG_OTP_CTRL0 = 0x0100, /* RW, 2 byte   [8]:otp auto read when power up*/
    MILAN_HV_REG_OTP_CTRL1 = 0x0102, /* RW, 2 byte   [5]:otp_ptm*/
    MILAN_HV_REG_OTP_CTRL2 = 0x0104, /* RW, 2 byte   [0:7]:otp_pdin [8:9]:otp_pa*/
    MILAN_HV_REG_OTP_CTRL3 = 0x0106, /* RW, 2 byte   [0:7]:otp_pdop*/
    /* RW, 2 byte   [0:7]:Interrupt enable bit [8:15]:int pules width 256us*/
    MILAN_HV_REG_IRQ_CTRL0 = 0x0120,
    MILAN_HV_REG_IRQ_CTRL1 = 0x0122, /* RW, 2 byte   Interrupt ctrl [0:7]:int regen time 20ms*/
    MILAN_HV_REG_IRQ_CTRL2 = 0x0124, /* RW, 2 byte   [0:10]:Interrupt clear*/
    MILAN_HV_REG_IRQ_CTRL3 = 0x0126, /* RO, 2 byte   [0:10]:Interrupt flags*/
    MILAN_HV_REG_IRQ_CTRL4 = 0x0128, /* WO, 2 byte   [0:10]:host irq set*/
    MILAN_HV_REG_IRQ_CTRL5 = 0x012a, /* WO, 2 byte   [0:9]:interrupt enable*/
    MILAN_HV_REG_IO_CTRL0 = 0x0130, /* miso enforce PDA*/
    MILAN_HV_REG_SPI_CTRL0 = 0x0140, /* WO, 2 byte   [0]: by pass spi deglitch bypass logic*/
    MILAN_HV_REG_OSC_CTRL = 0x0200, /* RW, 2 byte  */
    MILAN_HV_REG_PMU_CTRL = 0x0202, /* RW, 2 byte */
    MILAN_HV_REG_ESD_DETECT = 0x0204, /* RW, 2 byte  */
    MILAN_HV_REG_RG_PIXEL_CTRL0 = 0x0220, /* RW, 2 byte  */
    MILAN_HV_REG_RG_PIXEL_CTRL5 = 0x022c, /* RW, 2 byte  */
    MILAN_HV_REG_RG_PIXEL_DAC_P2 = 0x0236, /* RW, 2 byte  */
    MILAN_HV_REG_RG_PIXEL_DAC_P3 = 0x0238, /* RW, 2 byte  */
    MILAN_HV_REG_RG_PIXEL_DAC_P4 = 0x023a, /* RW, 2 byte  */
    MILAN_HV_REG_DUMMY = 0x0234, /* RW, 2 byte  */
    MILAN_HV_REG_DATA_READ = 0xaaaa, /* RW, 2 byte  */
} milan_hv_reg_t;

#endif  // __MILAN_HV_REGS_H__
