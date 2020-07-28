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

#ifndef __APDS993X_H__
#define __APDS993X_H__
#include "cust_alsps.h"
#include <FreeRTOS.h>
#include <semphr.h>

#define CMD_BYTE                0x80
#define APDS993X_ENABLE_REG     0x00
#define APDS993X_ATIME_REG      0x01
#define APDS993X_PTIME_REG      0x02
#define APDS993X_WTIME_REG      0x03
#define APDS993X_AILTL_REG      0x04
#define APDS993X_AILTH_REG      0x05
#define APDS993X_AIHTL_REG      0x06
#define APDS993X_AIHTH_REG      0x07
#define APDS993X_PILTL_REG      0x08
#define APDS993X_PILTH_REG      0x09
#define APDS993X_PIHTL_REG      0x0A
#define APDS993X_PIHTH_REG      0x0B
#define APDS993X_PERS_REG       0x0C
#define APDS993X_CONFIG_REG     0x0D
#define APDS993X_PPCOUNT_REG    0x0E
#define APDS993X_CONTROL_REG    0x0F
#define APDS993X_REV_REG        0x11
#define APDS993X_ID_REG         0x12
#define APDS993X_STATUS_REG     0x13
#define APDS993X_CH0DATAL_REG   0x14
#define APDS993X_CH0DATAH_REG   0x15
#define APDS993X_CH1DATAL_REG   0x16
#define APDS993X_CH1DATAH_REG   0x17
#define APDS993X_PDATAL_REG     0x18
#define APDS993X_PDATAH_REG     0x19

/* Register Value define : ATIME */
#define APDS993X_100MS_ADC_TIME 0xDB    /* 100.64ms integration time */
#define APDS993X_50MS_ADC_TIME  0xED    /* 51.68ms integration time */
#define APDS993X_27MS_ADC_TIME  0xF6    /* 27.2ms integration time */

/* Register Value define : PRXCNFG */
#define APDS993X_ALS_REDUCE 0x04    /* ALSREDUCE - ALS Gain reduced by 4x */

/* Register Value define : PERS */
#define APDS993X_PPERS_0    0x00    /* Every proximity ADC cycle */
#define APDS993X_PPERS_1    0x10    /* 1 consecutive proximity value out of range */
#define APDS993X_PPERS_2    0x20    /* 2 consecutive proximity value out of range */
#define APDS993X_PPERS_3    0x30    /* 3 consecutive proximity value out of range */
#define APDS993X_PPERS_4    0x40    /* 4 consecutive proximity value out of range */
#define APDS993X_PPERS_5    0x50    /* 5 consecutive proximity value out of range */
#define APDS993X_PPERS_6    0x60    /* 6 consecutive proximity value out of range */
#define APDS993X_PPERS_7    0x70    /* 7 consecutive proximity value out of range */
#define APDS993X_PPERS_8    0x80    /* 8 consecutive proximity value out of range */
#define APDS993X_PPERS_9    0x90    /* 9 consecutive proximity value out of range */
#define APDS993X_PPERS_10   0xA0    /* 10 consecutive proximity value out of range */
#define APDS993X_PPERS_11   0xB0    /* 11 consecutive proximity value out of range */
#define APDS993X_PPERS_12   0xC0    /* 12 consecutive proximity value out of range */
#define APDS993X_PPERS_13   0xD0    /* 13 consecutive proximity value out of range */
#define APDS993X_PPERS_14   0xE0    /* 14 consecutive proximity value out of range */
#define APDS993X_PPERS_15   0xF0    /* 15 consecutive proximity value out of range */

#define APDS993X_APERS_0    0x00    /* Every ADC cycle */
#define APDS993X_APERS_1    0x01    /* 1 consecutive proximity value out of range */
#define APDS993X_APERS_2    0x02    /* 2 consecutive proximity value out of range */
#define APDS993X_APERS_3    0x03    /* 3 consecutive proximity value out of range */
#define APDS993X_APERS_5    0x04    /* 5 consecutive proximity value out of range */
#define APDS993X_APERS_10   0x05    /* 10 consecutive proximity value out of range */
#define APDS993X_APERS_15   0x06    /* 15 consecutive proximity value out of range */
#define APDS993X_APERS_20   0x07    /* 20 consecutive proximity value out of range */
#define APDS993X_APERS_25   0x08    /* 25 consecutive proximity value out of range */
#define APDS993X_APERS_30   0x09    /* 30 consecutive proximity value out of range */
#define APDS993X_APERS_35   0x0A    /* 35 consecutive proximity value out of range */
#define APDS993X_APERS_40   0x0B    /* 40 consecutive proximity value out of range */
#define APDS993X_APERS_45   0x0C    /* 45 consecutive proximity value out of range */
#define APDS993X_APERS_50   0x0D    /* 50 consecutive proximity value out of range */
#define APDS993X_APERS_55   0x0E    /* 55 consecutive proximity value out of range */
#define APDS993X_APERS_60   0x0F    /* 60 consecutive proximity value out of range */

/* Register Value define : CONTROL */
#define APDS993X_AGAIN_1X   0x00    /* 1X ALS GAIN */
#define APDS993X_AGAIN_8X   0x01    /* 8X ALS GAIN */
#define APDS993X_AGAIN_16X  0x02    /* 16X ALS GAIN */
#define APDS993X_AGAIN_120X 0x03    /* 120X ALS GAIN */

#define APDS993X_PRX_IR_DIOD    0x20  /* Proximity uses CH1 diode */

#define APDS993X_PGAIN_1X   0x00    /* PS GAIN 1X */
#define APDS993X_PGAIN_2X   0x04    /* PS GAIN 2X */
#define APDS993X_PGAIN_4X   0x08    /* PS GAIN 4X */
#define APDS993X_PGAIN_8X   0x0C    /* PS GAIN 8X */

#define APDS993X_PDRVIE_100MA   0x00    /* PS 100mA LED drive */
#define APDS993X_PDRVIE_50MA    0x40    /* PS 50mA LED drive */
#define APDS993X_PDRVIE_25MA    0x80    /* PS 25mA LED drive */
#define APDS993X_PDRVIE_12_5MA  0xC0    /* PS 12.5mA LED drive */

#define APDS993X_PS_PULSE_NUMBER            8
#define APDS993X_PS_DETECTION_THRESHOLD     600
#define APDS993X_PS_HSYTERESIS_THRESHOLD    500

#define APDS993X_ALS_THRESHOLD_HSYTERESIS   20  /* 20 = 20% */

#define APDS993X_GA     48  /* 0.48 without glass window */
#define APDS993X_COE_B  223 /* 2.23 without glass window */
#define APDS993X_COE_C  70  /* 0.70 without glass window */
#define APDS993X_COE_D  142 /* 1.42 without glass window */
#define APDS993X_DF     52

/* Register Value define*/
#define APDS993x_DD_PON     0x01
#define APDS993x_DD_AEN     0x02
#define APDS993x_DD_PEN     0x04
#define APDS993x_DD_AIEN    0x10
#define APDS993x_DD_PIEN    0x20

#define CMD_CLR_PS_INT      0xE5
#define CMD_CLR_ALS_INT     0xE6
#define CMD_CLR_PS_ALS_INT  0xE7

typedef enum {
    APDS993X_ALS_RES_10240 = 0, /* 27.2ms integration time */
    APDS993X_ALS_RES_19456 = 1, /* 51.68ms integration time */
    APDS993X_ALS_RES_37888 = 2, /* 100.64ms integration time */
} apds993x_als_res_e;

typedef enum {
    APDS993X_ALS_GAIN_1X = 0,   /* 1x AGAIN */
    APDS993X_ALS_GAIN_8X = 1,   /* 8x AGAIN */
    APDS993X_ALS_GAIN_16X = 2,  /* 16x AGAIN */
    APDS993X_ALS_GAIN_120X = 3, /* 120x AGAIN */
} apds993x_als_gain_e;
#endif
