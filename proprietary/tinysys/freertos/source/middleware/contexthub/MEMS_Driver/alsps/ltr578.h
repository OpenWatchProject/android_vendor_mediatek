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
 * Definitions for APDS9921 als/ps sensor chip.
 */
#ifndef __GN_LTR578_CALI_H__
#define __GN_LTR578_CALI_H__

#include "cust_alsps.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define APDS9921_DD_MAIN_CTRL_ADDR 0x00
#define APDS9921_DD_PRX_LED_ADDR 0x01
#define APDS9921_DD_PRX_PULSES_ADDR 0x02
#define APDS9921_DD_PRX_MEAS_RATE_ADDR 0x03
#define APDS9921_DD_ALS_MEAS_RATE_ADDR 0x04
#define APDS9921_DD_ALS_GAIN_ADDR 0x05
#define APDS9921_DD_PART_ID_ADDR 0x06
#define APDS9921_DD_MAIN_STATUS_ADDR 0x07
#define APDS9921_DD_PRX_DATA_ADDR 0x08
#define APDS9921_DD_PRX_DATA_0_ADDR 0x08
#define APDS9921_DD_PRX_DATA_1_ADDR 0x09
#define APDS9921_DD_CLEAR_DATA_ADDR 0x0A
#define APDS9921_DD_CLEAR_DATA_0_ADDR 0x0A
#define APDS9921_DD_CLEAR_DATA_1_ADDR 0x0B
#define APDS9921_DD_CLEAR_DATA_2_ADDR 0x0C
#define APDS9921_DD_ALS_DATA_ADDR 0x0D
#define APDS9921_DD_ALS_DATA_0_ADDR 0x0D
#define APDS9921_DD_ALS_DATA_1_ADDR 0x0E
#define APDS9921_DD_ALS_DATA_2_ADDR 0x0F
#define APDS9921_DD_COMP_DATA_ADDR 0x16
#define APDS9921_DD_COMP_DATA_0_ADDR 0x16
#define APDS9921_DD_COMP_DATA_1_ADDR 0x17
#define APDS9921_DD_COMP_DATA_2_ADDR 0x18
#define APDS9921_DD_INT_CFG_ADDR 0x19
#define APDS9921_DD_INT_PERSISTENCE_ADDR 0x1A
#define APDS9921_DD_PRX_THRES_UP_ADDR 0x1B
#define APDS9921_DD_PRX_THRES_UP_0_ADDR 0x1B
#define APDS9921_DD_PRX_THRES_UP_1_ADDR 0x1C
#define APDS9921_DD_PRX_THRES_LOW_ADDR 0x1D
#define APDS9921_DD_PRX_THRES_LOW_0_ADDR 0x1D
#define APDS9921_DD_PRX_THRES_LOW_1_ADDR 0x1E
#define APDS9921_DD_PRX_CAN_ADDR 0x1F
#define APDS9921_DD_PRX_CAN_0_ADDR 0x1F
#define APDS9921_DD_PRX_CAN_1_ADDR 0x20
#define APDS9921_DD_ALS_THRES_UP_ADDR 0x21
#define APDS9921_DD_ALS_THRES_UP_0_ADDR 0x21
#define APDS9921_DD_ALS_THRES_UP_1_ADDR 0x22
#define APDS9921_DD_ALS_THRES_UP_2_ADDR 0x23
#define APDS9921_DD_ALS_THRES_LOW_ADDR 0x24
#define APDS9921_DD_ALS_THRES_LOW_0_ADDR 0x24
#define APDS9921_DD_ALS_THRES_LOW_1_ADDR 0x25
#define APDS9921_DD_ALS_THRES_LOW_2_ADDR 0x26
#define APDS9921_DD_ALS_THRES_VAR_ADDR 0x27
#define APDS9921_DD_DEVICE_CONFIG_ADDR 0x2F

/* Register Value define : MAIN_CTRL */
#define APDS9921_DD_PRX_EN 0x01
#define APDS9921_DD_ALS_EN 0x02
#define APDS9921_DD_SW_RESET 0x10

/* Register Value define : PS_LED */
#define APDS9921_DD_LED_CURRENT_2_5_MA 0x00 /* 2.5 mA */
#define APDS9921_DD_LED_CURRENT_5_MA 0x01 /* 5 mA */
#define APDS9921_DD_LED_CURRENT_10_MA 0x02 /* 10 mA */
#define APDS9921_DD_LED_CURRENT_25_MA 0x03 /* 25 mA */
#define APDS9921_DD_LED_CURRENT_50_MA 0x04 /* 50 mA */
#define APDS9921_DD_LED_CURRENT_75_MA 0x05 /* 75 mA */
#define APDS9921_DD_LED_CURRENT_100_MA 0x06 /* 100 mA */
#define APDS9921_DD_LED_CURRENT_125_MA 0x07 /* 125 mA */

#define APDS9921_DD_LED_CURRENT_PEAK_ON 0x08

#define APDS9921_DD_LED_FREQ_60_KHZ 0x30 /* LED Pulse frequency = 60KHz */
#define APDS9921_DD_LED_FREQ_70_KHZ 0x40 /* LED Pulse frequency = 70KHz */
#define APDS9921_DD_LED_FREQ_80_KHZ 0x50 /* LED Pulse frequency = 80KHz */
#define APDS9921_DD_LED_FREQ_90_KHZ 0x60 /* LED Pulse frequency = 90KHz */
#define APDS9921_DD_LED_FREQ_100_KHZ 0x70 /* LED Pulse frequency = 100KHz */

/* Register Value define : PS_MEAS_RATE */
#define APDS9921_DD_PRX_MEAS_RATE_6_25_MS   0x01 /* PS Measurement rate = 6.25 ms */
#define APDS9921_DD_PRX_MEAS_RATE_12_5_MS   0x02 /* PS Measurement rate = 12.5 ms */
#define APDS9921_DD_PRX_MEAS_RATE_25_MS 0x03 /* PS Measurement rate = 25 ms */
#define APDS9921_DD_PRX_MEAS_RATE_50_MS 0x04 /* PS Measurement rate = 50 ms */
#define APDS9921_DD_PRX_MEAS_RATE_100_MS 0x05 /* PS Measurement rate = 100 ms */
#define APDS9921_DD_PRX_MEAS_RATE_200_MS 0x06 /* PS Measurement rate = 200 ms */
#define APDS9921_DD_PRX_MEAS_RATE_400_MS 0x07 /* PS Measurement rate = 400 ms */

#define APDS9921_DD_PRX_MEAS_RES_8_BIT 0x00 /* PS resolution 8 bit (full range : 0 ~ 255) */
#define APDS9921_DD_PRX_MEAS_RES_9_BIT 0x08 /* PS resolution 9 bit (full range : 0 ~ 511) */
#define APDS9921_DD_PRX_MEAS_RES_10_BIT 0x10 /* PS resolution 10 bit (full range : 0 ~ 1023) */
#define APDS9921_DD_PRX_MEAS_RES_11_BIT 0x18 /* PS resolution 11 bit (full range : 0 ~ 2047) */

/* Register Value define : ALS_MEAS_RATE */
#define APDS9921_DD_ALS_MEAS_RATE_25_MS 0x00 /* ALS Measurement rate = 25 ms */
#define APDS9921_DD_ALS_MEAS_RATE_50_MS 0x01 /* ALS Measurement rate = 50 ms */
#define APDS9921_DD_ALS_MEAS_RATE_100_MS 0x02 /* ALS Measurement rate = 100 ms */
#define APDS9921_DD_ALS_MEAS_RATE_200_MS 0x03 /* ALS Measurement rate = 200 ms */
#define APDS9921_DD_ALS_MEAS_RATE_500_MS 0x04 /* ALS Measurement rate = 500 ms */
#define APDS9921_DD_ALS_MEAS_RATE_1000_MS   0x05 /* ALS Measurement rate = 1000 ms */
#define APDS9921_DD_ALS_MEAS_RATE_2000_MS   0x06 /* ALS Measurement rate = 2000 ms */

#define APDS9921_DD_ALS_MEAS_RES_20_BIT 0x00 /* ALS resolution 20 bit (full range : 0 ~ 1048575) [ADC conversion time = 400ms] */
#define APDS9921_DD_ALS_MEAS_RES_19_BIT 0x10 /* ALS resolution 19 bit (full range : 0 ~ 524287) [ADC conversion time = 200ms]  */
#define APDS9921_DD_ALS_MEAS_RES_18_BIT 0x20 /* ALS resolution 18 bit (full range : 0 ~ 262143) [ADC conversion time = 100ms]  */
#define APDS9921_DD_ALS_MEAS_RES_17_BIT 0x30 /* ALS resolution 17 bit (full range : 0 ~ 131071) [ADC conversion time = 50ms]  */
#define APDS9921_DD_ALS_MEAS_RES_16_BIT 0x40 /* ALS resolution 16 bit (full range : 0 ~ 65535) [ADC conversion time = 25ms]  */

/* Register Value define : ALS_GAIN */
#define APDS9921_DD_ALS_GAIN_1 0x00 /* ALS Gain 1 */
#define APDS9921_DD_ALS_GAIN_3 0x01 /* ALS Gain 3 */
#define APDS9921_DD_ALS_GAIN_6 0x02 /* ALS Gain 6 */
#define APDS9921_DD_ALS_GAIN_9 0x03 /* ALS Gain 9 */
#define APDS9921_DD_ALS_GAIN_18 0x04 /* ALS Gain 18 */

/* Register Value define : MAIN_STATUS */
#define APDS9921_DD_PRX_DATA_STATUS 0x01 /* 1: New data, not read yet (cleared after read) */
#define APDS9921_DD_PRX_INT_STATUS 0x02 /* 1: Interrupt condition fulfilled (cleared after read) */
#define APDS9921_DD_PRX_LOGICAL_STATUS 0x04 /* 1: object is close */
#define APDS9921_DD_ALS_DATA_STATUS 0x08 /* 1: New data, not read yet (cleared after read) */
#define APDS9921_DD_ALS_INT_STATUS 0x10 /* 1: Interrupt condition fulfilled (cleared after read) */
#define APDS9921_DD_POWER_ON_STATUS 0x20 /* 1: Power on cycle */

/* Register Value define : INT_CFG */
#define APDS9921_DD_PRX_INT_EN 0x01 /* 1: PS Interrupt enabled */
#define APDS9921_DD_PRX_LOGIC_MODE 0x02 /* 1: PS Logic Output Mode: INT pad is updated after every measurement and maintains output state between measurements */
#define APDS9921_DD_ALS_INT_EN 0x04 /* 1: ALS Interrupt enabled */
#define APDS9921_DD_ALS_VAR_MODE 0x08 /* 1: ALS variation interrupt mode */
#define APDS9921_DD_ALS_INT_SEL_ALS 0x10 /* ALS channel selected for interrupt */
#define APDS9921_DD_ALS_INT_SEL_CLEAR 0x00 /* Clear channel selected for interrupt */

/* Register Value define : INT_PERSISTENCE */
#define APDS9921_DD_PRX_PERS_1 0x00 /* Every PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_2 0x01 /* 2 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_3 0x02 /* 3 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_4 0x03 /* 4 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_5 0x04 /* 5 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_6 0x05 /* 6 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_7 0x06 /* 7 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_8 0x07 /* 8 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_9 0x08 /* 9 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_10 0x09 /* 10 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_11 0x0A /* 11 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_12 0x0B /* 12 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_13 0x0C /* 13 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_14 0x0D /* 14 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_15 0x0E /* 15 consecutive PS value out of threshold range */
#define APDS9921_DD_PRX_PERS_16 0x0F /* 16 consecutive PS value out of threshold range */

#define APDS9921_DD_ALS_PERS_1 0x00 /* Every ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_2 0x10 /* 2 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_3 0x20 /* 3 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_4 0x30 /* 4 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_5 0x40 /* 5 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_6 0x50 /* 6 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_7 0x60 /* 7 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_8 0x70 /* 8 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_9 0x80 /* 9 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_10 0x90 /* 10 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_11 0xA0 /* 11 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_12 0xB0 /* 12 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_13 0xC0 /* 13 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_14 0xD0 /* 14 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_15 0xE0 /* 15 consecutive ALS value out of threshold range */
#define APDS9921_DD_ALS_PERS_16 0xF0 /* 16 consecutive ALS value out of threshold range */

/* Register Value define : ALS_THRES_VAR */
#define APDS9921_DD_ALS_VAR_8_COUNT 0x00 /* ALS result varies by 8 counts compared to previous result */
#define APDS9921_DD_ALS_VAR_16_COUNT 0x01 /* ALS result varies by 16 counts compared to previous result */
#define APDS9921_DD_ALS_VAR_32_COUNT 0x02 /* ALS result varies by 32 counts compared to previous result */
#define APDS9921_DD_ALS_VAR_64_COUNT 0x03 /* ALS result varies by 64 counts compared to previous result */
#define APDS9921_DD_ALS_VAR_128_COUNT 0x04 /* ALS result varies by 128 counts compared to previous result */
#define APDS9921_DD_ALS_VAR_256_COUNT 0x05 /* ALS result varies by 256 counts compared to previous result */
#define APDS9921_DD_ALS_VAR_512_COUNT 0x06 /* ALS result varies by 512 counts compared to previous result */
#define APDS9921_DD_ALS_VAR_1024_COUNT 0x07 /* ALS result varies by 1024 counts compared to previous result */

#define APDS9921_SUCCESS 0
#define APDS9921_ERR_I2C -1
#define APDS9921_ERR_STATUS -3
#define APDS9921_ERR_SETUP_FAILURE -4
#define APDS9921_ERR_GETGSENSORDATA -5
#define APDS9921_ERR_IDENTIFICATION -6

//#define APDS9921_DD_LUX_FACTOR 315 // for glass transmittance of 10%, use 30 for open air
#define APDS9921_DD_LUX_FACTOR 120 // for glass transmittance of 10%, use 30 for open air

#define APDS9921_DD_ALS_DEFAULT_RES APDS9921_DD_ALS_MEAS_RES_16_BIT
#define APDS9921_DD_ALS_DEFAULT_MEAS_RATE APDS9921_DD_ALS_MEAS_RATE_25_MS
#define APDS9921_DD_ALS_DEFAULT_GAIN APDS9921_DD_ALS_GAIN_18

#define APDS9921_PS_DETECTION_THRESHOLD 200
#define APDS9921_PS_HSYTERESIS_THRESHOLD 150

/*Gionee <GN_BSP_SENSOR> <duyonghong> <20170323> add for ID#68283 start*/
#define APDS9921_DD_PRX_DEFAULT_PULSE 28    // drop to 16 if crosstalk is too high
/*Gionee <GN_BSP_SENSOR> <duyonghong> <20170323> add for ID#68283 end*/
#define APDS9921_DD_PRX_DEFAULT_LED_CURRENT APDS9921_DD_LED_CURRENT_100_MA
#define APDS9921_DD_PRX_DEFAULT_LED_FREQ APDS9921_DD_LED_FREQ_60_KHZ
#define APDS9921_DD_PRX_DEFAULT_RES APDS9921_DD_PRX_MEAS_RES_11_BIT
#define APDS9921_DD_PRX_DEFAULT_MEAS_RATE APDS9921_DD_PRX_MEAS_RATE_12_5_MS
#define  NOISE_HIGH   150
#define  NOISE_LOW    145
#define  COUNT        5
#define  NOISE_MAX  800
#define PS_MAX_VALUE     1023

#define GN_PS_CALI

/////int set_ps_thrd(u16 ps_hig,u16 ps_low);
/////int read_ps_raw_data(u16 *rawdata);

#endif/* __GN_LTR578_CALI_H__*/

