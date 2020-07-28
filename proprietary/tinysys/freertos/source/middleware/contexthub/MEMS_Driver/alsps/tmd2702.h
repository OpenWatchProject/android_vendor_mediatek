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
/*
 * Definitions for TMD2702 als/ps sensor chip.
 */
#ifndef __TMD2702_H__
#define __TMD2702_H__

#include "cust_alsps.h"

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

#define TMD2702_ID      0x10  // device ID


#define TMD2702_REG_ENABLE      0x80  // Enables states and functions
#define TMD2702_REG_ATIME       0x81  // ALS integration time
#define TMD2702_REG_PTIME       0x82  // Proximity sampling time
#define TMD2702_REG_WTIME       0x83  // Wait time
#define TMD2702_REG_AILT        0x84  // ALS interrupt low threshold low byte
#define TMD2702_REG_AILT_HI     0x85  // ALS interrupt low threshold high byte
#define TMD2702_REG_AIHT        0x86  // ALS interrupt high threshold low byte
#define TMD2702_REG_AIHT_HI     0x87  // ALS interrupt high threshold high byte
#define TMD2702_REG_PILT        0x88  // Proximity interrupt low threshold low byte
#define TMD2702_REG_PILT_HI     0x89  // Proximity interrupt low threshold high byte
#define TMD2702_REG_PIHT        0x8A  // Proximity interrupt high threshold low byte
#define TMD2702_REG_PIHT_HI     0x8B  // Proximity interrupt high threshold high byte
#define TMD2702_REG_INT_PERS    0x8C  // Interrupt persistence filters
#define TMD2702_REG_CFG0        0x8D  // Configuration register zero
#define TMD2702_REG_PGCFG0      0x8E  // Proximity configuration register zero
#define TMD2702_REG_PGCFG1      0x8F  // Proximity configuration register one

#define TMD2702_REG_CFG1        0x90  // Configuration register one (the bit 5 must set to 1 for low power)
#define TMD2702_REG_REVID       0x91  // Revision ID
#define TMD2702_REG_ID          0x92  // Device ID
#define TMD2702_REG_STATUS      0x93  // Device status register

#define TMD2702_REG_CH0DATA     0x94  // Photopic channel data low byte
#define TMD2702_REG_CH0DATA_HI  0x95  // Photopic channel data high byte
#define TMD2702_REG_CH1DATA     0x96  // IR channel data low byte
#define TMD2702_REG_CH1DATA_HI  0x97  // IR channel data high byte

#define TMD2702_REG_PDATA       0x9C  // Proximity channel data
#define TMD2702_REG_PDATA_HI    0x9D  // Proximity channel data
//#define TMD2702_REG_ADCDATA_L   0x9D  //

#define TMD2702_REG_AUXID       0x9E  // Auxiliary ID

#define TMD2702_REG_CFG2        0x9F  // Configuration register two

#define TMD2702_REG_CFG3        0xAB  // Configuration register three
#define TMD2702_REG_CFG4        0xAC  // Proximity offset magnitude (must be set to 0x3C)
//#define TMD2702_REG_CFG5        0xAD  // Proximity offset sign
#define TMD2702_REG_CFG6        0xAE  // Configuration register six

#define TMD2702_REG_POFFSET_L   0xC0  // Proximity offset sign low data
#define TMD2702_REG_POFFSET_H   0xC1  //Proximity offset sign high data

#define TMD2702_REG_CALIB       0xD7  // Proximity offset calibration
#define TMD2702_REG_CALIBCFG    0xD9  // Calibration configuration
#define TMD2702_REG_CALIBSTAT   0xDC  // Calibration status
#define TMD2702_REG_INTENAB     0xDD  // Interrupt enables

#define TMD2702_REG_FAC_L       0xE6  // Factory data low (lot code data)
#define TMD2702_REG_FAC_H       0xE7  // Factory data high (lot code data)

#define TMD2702_REG_TEST3       0xF2  // Test three(must be set to 0xC4)

/* enable */
#define TMD2702_PON    (1 << 0)
#define TMD2702_AEN    (1 << 1)
#define TMD2702_PEN    (1 << 2)
#define TMD2702_WEN    (1 << 3)

/* interrupt status */
#define TMD2702_ST_PGSAT_AMBIENT   (1 << 0)
#define TMD2702_ST_PGSAT_RELFLECT  (1 << 1)
#define TMD2702_ST_ZERODET         (1 << 1)
#define TMD2702_ST_CAL_IRQ         (1 << 3)
#define TMD2702_ST_ALS_IRQ         (1 << 4)
#define TMD2702_ST_PRX_IRQ         (1 << 5)
#define TMD2702_ST_PRX_SAT         (1 << 6)
#define TMD2702_ST_ALS_SAT         (1 << 7)

/* interrupt enable */
#define TMD2702_ZIEN   (1 << 2)
#define TMD2702_CIEN   (1 << 3)
#define TMD2702_AIEN   (1 << 4)
#define TMD2702_PIEN   (1 << 5)
#define TMD2702_PSIEN  (1 << 6)
#define TMD2702_ASIEN  (1 << 7)

// TMD2702_PCFG0_REG @0x8E
#define PPULSE_MASK              (0x3F << 0)
#define PPULSE_LEN_MASK          (0x03 << 6)
#define PPLEN_4US                (0x00 << 6)
#define PPLEN_8US                (0x01 << 6)
#define PPLEN_16US               (0x02 << 6)
#define PPLEN_32US               (0x03 << 6)

// TMD2702_PCFG1_REG @0x8F
#define PLDRIVE_MASK             (0x1F << 0)
#define PGAIN_MASK               (0x03 << 6)
#define PGAIN_1X                 (0x00 << 6)
#define PGAIN_2X                 (0x01 << 6)
#define PGAIN_4X                 (0x02 << 6)
#define PGAIN_8X                 (0x03 << 6)

// TMD2702_CFG1_REG @0x90
#define AGAIN_MASK               (0x03 << 0)
#define AGAIN_1X                 (0x00 << 0)
#define AGAIN_4X                 (0x01 << 0)
#define AGAIN_16X                (0x02 << 0)
#define AGAIN_64X                (0x03 << 0)
#define PLDRIVE_REDUCE           (0x01 << 5)

// Configration calculations
#define ATIME_PER_STEP_X100      280
#define ATIME_MS(ms)             (uint8_t)(((uint32_t)ms*100 + ((uint32_t)ATIME_PER_STEP_X100 >> 1)) / \
                                    (uint32_t)ATIME_PER_STEP_X100 - 1)
#define PTIME_PER_STEP           88
#define INTEGRATION_CYCLE        2780
#define PTIME_US(us)             (uint8_t)(((uint32_t)us + ((uint32_t)PTIME_PER_STEP >> 1))/PTIME_PER_STEP - 1)
#define WTIME_PER_STEP_X100      280
#define WTIME_MS(ms)             (uint8_t)(((uint32_t)ms*100 + ((uint32_t)WTIME_PER_STEP_X100 >> 1)) / \
                                    (uint32_t)WTIME_PER_STEP_X100 - 1)
#define ALS_PERSIST(p)           (uint8_t)((p & 0x0F) << 0)
#define PROX_PERSIST(p)          (uint8_t)((p & 0x0F) << 4)

/*the max num of pulse during a PS measure period, default value is 15*/
#define PPULSES(c) ({\
       u8 __reg = (c <= 64 ? (c-1) : 63);\
       __reg; \
})

//#define PLDRIVE_MA(ma)           (uint8_t)((ma+3)/6 - 1)
#define PLDRIVE_MA(p)({\
       u8 __reg = (((u8)((p)-2)/2) & 0xf);\
       /* artf24717 limit PLDRIVE to 19mA */ \
       __reg = (__reg > 0x08) ? 0x08 : __reg; \
       __reg; \
})


#define D_Factor                 200
#define B_Coef                   260
#define C_Coef                   800
#define D_Coef                   270
#define USE_MAX                  1

#define TMD2702_PRX_PERS          0x10

#define MAX_ALS_VALUE            0xFFFF
#define MIN_ALS_VALUE            3
#define GAIN_SWITCH_LEVEL        200

/* Attributes for TMD2702 Proximity data type */
#define SNS_DD_PROX_RANGE_MAX         0.05
#define SNS_DD_PROX_RES               1
#define SNS_DD_PROX_BITS              8
#define SNS_DD_PROX_FREQ              100
#define SNS_DD_PROX_PWR               1000 /* unit of uA */
#define SNS_DD_PROX_LO_PWR            1 /* unit of uA */

/* Attributes for TMD2702 ALS/Color data type */
#define SNS_DD_ALS_RANGE_MAX          32767
#define SNS_DD_ALS_RES                1 /* unit of this data type is lux */
#define SNS_DD_ALS_BITS               16
#define SNS_DD_ALS_FREQ               100
#define SNS_DD_ALS_PWR                90 /* unit of uA */
#define SNS_DD_ALS_LO_PWR             1 /* unit of uA */

static uint8_t const als_gains[] = {
    1,
    4,
    16,
    64,
    138
};


#endif



