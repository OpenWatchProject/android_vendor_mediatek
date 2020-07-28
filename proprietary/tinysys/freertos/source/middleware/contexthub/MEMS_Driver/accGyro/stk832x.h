/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __STK832X_H__
#define __STK832X_H__

/* stk832x register */
#define STK832X_REG_CHIPID          0x00
#define STK832X_REG_XOUT1           0x02
#define STK832X_REG_XOUT2           0x03
#define STK832X_REG_YOUT1           0x04
#define STK832X_REG_YOUT2           0x05
#define STK832X_REG_ZOUT1           0x06
#define STK832X_REG_ZOUT2           0x07
#define STK832X_REG_INTSTS1         0x09
#define STK832X_REG_INTSTS2         0x0A
#define STK832X_REG_EVENTINFO1      0x0B
#define STK832X_REG_FIFOSTS         0x0C
#define STK832X_REG_STEPOUT1        0x0D
#define STK832X_REG_STEPOUT2        0x0E
#define STK832X_REG_RANGESEL        0x0F
#define STK832X_REG_BWSEL           0x10
#define STK832X_REG_POWMODE         0x11
#define STK832X_REG_SWRST           0x14
#define STK832X_REG_INTEN1          0x16
#define STK832X_REG_INTEN2          0x17
#define STK832X_REG_INTMAP1         0x19
#define STK832X_REG_INTMAP2         0x1A
#define STK832X_REG_INTCFG1         0x20
#define STK832X_REG_INTCFG2         0x21
#define STK832X_REG_SLOPEDLY        0x27
#define STK832X_REG_SLOPETHD        0x28
#define STK832X_REG_SIGMOT1         0x29
#define STK832X_REG_SIGMOT2         0x2A
#define STK832X_REG_SIGMOT3         0x2B
#define STK832X_REG_STEPCNT1        0x2C
#define STK832X_REG_STEPCNT2        0x2D
#define STK832X_REG_STEPTHD         0x2E
#define STK832X_REG_STEPDEB         0x2F
#define STK832X_REG_STEPMAXTW       0x31
#define STK832X_REG_INTFCFG         0x34
#define STK832X_REG_OFSTCOMP1       0x36
#define STK832X_REG_OFSTX           0x38
#define STK832X_REG_OFSTY           0x39
#define STK832X_REG_OFSTZ           0x3A
#define STK832X_REG_CFG1            0x3D
#define STK832X_REG_CFG2            0x3E
#define STK832X_REG_FIFOOUT         0x3F

/* STK832X_REG_CHIPID */
#define STK8323_ID                      0x23 /* include for STK8321 */
#define STK8325_ID                      0x25

/* STK832X_REG_INTSTS1 */
#define STK832X_INTSTS1_SIG_MOT_STS     0x1
#define STK832X_INTSTS1_ANY_MOT_STS     0x4

/* STK832X_REG_INTSTS2 */
#define STK832X_INTSTS2_FWM_STS_MASK    0x40

/* STK832X_REG_FIFOSTS */
#define STK832X_FIFOSTS_FIFO_FRAME_CNT_MASK     0x7F
#define STK832X_FIFOSTS_FIFO_OVER               0x80

/* STK832X_REG_RANGESEL */
#define STK832X_RANGESEL_2G             0x3
#define STK832X_RANGESEL_4G             0x5
#define STK832X_RANGESEL_8G             0x8
#define STK832X_RANGESEL_BW_MASK        0xF
#define STK832X_RANGESEL_DEF            STK832X_RANGESEL_2G
typedef enum
{
    STK_2G = STK832X_RANGESEL_2G,
    STK_4G = STK832X_RANGESEL_4G,
    STK_8G = STK832X_RANGESEL_8G
} stk_rangesel;

/* STK832X_REG_BWSEL */
#define STK832X_BWSEL_BW_7_81           0x08    /* ODR = BW x 2 = 15.62Hz */
#define STK832X_BWSEL_BW_15_63          0x09    /* ODR = BW x 2 = 31.26Hz */
#define STK832X_BWSEL_BW_31_25          0x0A    /* ODR = BW x 2 = 62.5Hz */
#define STK832X_BWSEL_BW_62_5           0x0B    /* ODR = BW x 2 = 125Hz */
#define STK832X_BWSEL_BW_125            0x0C    /* ODR = BW x 2 = 250Hz */
#define STK832X_BWSEL_BW_250            0x0D    /* ODR = BW x 2 = 500Hz */
// add IDLHLYE-201 by tao.lan 20180704 start
#define STK832X_BWSEL_BW_500            0x0E    /* ODR = BW x 2 = 1000Hz */
// add IDLHLYE-201 by tao.lan 20180704 end	

/* STK832X_REG_POWMODE */
#define STK832X_PWMD_SUSPEND            0x80
#define STK832X_PWMD_LOWPOWER           0x40
#define STK832X_PWMD_NORMAL             0x00
#define STK832X_PWMD_SLP_MASK           0x3E

/* STK832X_REG_SWRST */
#define STK832X_SWRST_VAL               0xB6

/* STK832X_REG_INTEN1 */
#define STK832X_INTEN1_SLP_EN_XYZ       0x07

/* STK832X_REG_INTEN2 */
#define STK832X_INTEN2_DATA_EN          0x10
#define STK832X_INTEN2_FWM_EN           0x40

/* STK832X_REG_INTMAP1 */
#define STK832X_INTMAP1_SIGMOT2INT1         0x01
#define STK832X_INTMAP1_ANYMOT2INT1         0x04

/* STK832X_REG_INTMAP2 */
#define STK832X_INTMAP2_DATA2INT1           0x01
#define STK832X_INTMAP2_FWM2INT1            0x02
#define STK832X_INTMAP2_FWM2INT2            0x40

/* STK832X_REG_INTCFG1 */
#define STK832X_INTCFG1_INT1_ACTIVE_H       0x01
#define STK832X_INTCFG1_INT1_OD_PUSHPULL    0x00
#define STK832X_INTCFG1_INT2_ACTIVE_H       0x04
#define STK832X_INTCFG1_INT2_OD_PUSHPULL    0x00

/* STK832X_REG_INTCFG2 */
#define STK832X_INTCFG2_NOLATCHED           0x00
#define STK832X_INTCFG2_LATCHED             0x0F
#define STK832X_INTCFG2_INT_RST             0x80

/* STK832X_REG_SLOPETHD */
#define STK832X_SLOPETHD_DEF                0x14

/* STK832X_REG_SIGMOT1 */
#define STK832X_SIGMOT1_SKIP_TIME_3SEC      0x96    /* default value */

/* STK832X_REG_SIGMOT2 */
#define STK832X_SIGMOT2_SIG_MOT_EN          0x02
#define STK832X_SIGMOT2_ANY_MOT_EN          0x04

/* STK832X_REG_SIGMOT3 */
#define STK832X_SIGMOT3_PROOF_TIME_1SEC     0x32    /* default value */

/* STK832X_REG_INTFCFG */
#define STK832X_INTFCFG_I2C_WDT_EN          0x04

/* STK832X_REG_STEPCNT2 */
#define STK832X_STEPCNT2_RST_CNT            0x04
#define STK832X_STEPCNT2_STEP_CNT_EN        0x08

/* STK832X_REG_OFSTCOMP1 */
#define STK832X_OFSTCOMP1_OFST_RST          0x80

/* STK832X_REG_CFG1 */
/* the maximum space for FIFO is 32*3 bytes */
//#define STK832X_CFG1_XYZ_FRAME_MAX      32
#define STK832X_CFG1_XYZ_FRAME_MAX          25

/* STK832X_REG_CFG2 */
#define STK832X_CFG2_FIFO_MODE_BYPASS       0x0
#define STK832X_CFG2_FIFO_MODE_FIFO         0x1
#define STK832X_CFG2_FIFO_MODE_STREAM       0x6
#define STK832X_CFG2_FIFO_MODE_SHIFT        5
#define STK832X_CFG2_FIFO_DATA_SEL_XYZ      0x0
#define STK832X_CFG2_FIFO_DATA_SEL_X        0x1
#define STK832X_CFG2_FIFO_DATA_SEL_Y        0x2
#define STK832X_CFG2_FIFO_DATA_SEL_Z        0x3
#define STK832X_CFG2_FIFO_DATA_SEL_MASK     0x3

/* STK832X_REG_OFSTx */
#define STK832X_OFST_LSB                    128     /* 8 bits for +-1G */
#endif /* ifndef __STK832X_H__ */
