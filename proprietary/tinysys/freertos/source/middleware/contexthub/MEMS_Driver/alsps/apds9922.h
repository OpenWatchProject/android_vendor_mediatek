
#ifndef __APDS9922_H__
#define __APDS9922_H__
#include "cust_alsps.h"
#include <FreeRTOS.h>
#include <semphr.h>

#define APDS9922_DD_MAIN_CTRL_ADDR 0x00
#define APDS9922_DD_PRX_LED_ADDR 0x01
#define APDS9922_DD_PRX_PULSES_ADDR 0x02
#define APDS9922_DD_PRX_MEAS_RATE_ADDR 0x03
#define APDS9922_DD_ALS_MEAS_RATE_ADDR 0x04
#define APDS9922_DD_ALS_GAIN_ADDR 0x05
#define APDS9922_DD_PART_ID_ADDR 0x06
#define APDS9922_DD_MAIN_STATUS_ADDR 0x07
#define APDS9922_DD_PRX_DATA_ADDR 0x08
#define APDS9922_DD_PRX_DATA_0_ADDR 0x08
#define APDS9922_DD_PRX_DATA_1_ADDR 0x09
#define APDS9922_DD_ALS_DATA_ADDR 0x0D
#define APDS9922_DD_ALS_DATA_0_ADDR 0x0D
#define APDS9922_DD_ALS_DATA_1_ADDR 0x0E
#define APDS9922_DD_ALS_DATA_2_ADDR 0x0F
#define APDS9922_DD_INT_CFG_ADDR 0x19
#define APDS9922_DD_INT_PERSISTENCE_ADDR 0x1A
#define APDS9922_DD_PRX_THRES_UP_ADDR 0x1B
#define APDS9922_DD_PRX_THRES_UP_0_ADDR 0x1B
#define APDS9922_DD_PRX_THRES_UP_1_ADDR 0x1C
#define APDS9922_DD_PRX_THRES_LOW_ADDR 0x1D
#define APDS9922_DD_PRX_THRES_LOW_0_ADDR 0x1D
#define APDS9922_DD_PRX_THRES_LOW_1_ADDR 0x1E
#define APDS9922_DD_ALS_THRES_UP_ADDR 0x21
#define APDS9922_DD_ALS_THRES_UP_0_ADDR 0x21
#define APDS9922_DD_ALS_THRES_UP_1_ADDR 0x22
#define APDS9922_DD_ALS_THRES_UP_2_ADDR 0x23
#define APDS9922_DD_ALS_THRES_LOW_ADDR 0x24
#define APDS9922_DD_ALS_THRES_LOW_0_ADDR 0x24
#define APDS9922_DD_ALS_THRES_LOW_1_ADDR 0x25
#define APDS9922_DD_ALS_THRES_LOW_2_ADDR 0x26

/* Register Value define : MAIN_CTRL */
#define APDS9922_DD_PRX_EN 0x01
#define APDS9922_DD_ALS_EN 0x02
#define APDS9922_DD_SW_RESET 0x10

/* Register Value define : PS_LED */
#define APDS9922_DD_LED_CURRENT_2_5_MA 0x00 /* 2.5 mA */
#define APDS9922_DD_LED_CURRENT_5_MA 0x01 /* 5 mA */
#define APDS9922_DD_LED_CURRENT_10_MA 0x02 /* 10 mA */
#define APDS9922_DD_LED_CURRENT_25_MA 0x03 /* 25 mA */
#define APDS9922_DD_LED_CURRENT_50_MA 0x04 /* 50 mA */
#define APDS9922_DD_LED_CURRENT_75_MA 0x05 /* 75 mA */
#define APDS9922_DD_LED_CURRENT_100_MA 0x06 /* 100 mA */
#define APDS9922_DD_LED_CURRENT_125_MA 0x07 /* 125 mA */

#define APDS9922_DD_LED_FREQ_60_KHZ 0x30 /* LED Pulse frequency = 60KHz */
#define APDS9922_DD_LED_FREQ_70_KHZ 0x40 /* LED Pulse frequency = 70KHz */
#define APDS9922_DD_LED_FREQ_80_KHZ 0x50 /* LED Pulse frequency = 80KHz */
#define APDS9922_DD_LED_FREQ_90_KHZ 0x60 /* LED Pulse frequency = 90KHz */
#define APDS9922_DD_LED_FREQ_100_KHZ 0x70 /* LED Pulse frequency = 100KHz */

/* Register Value define : PS_MEAS_RATE */
#define APDS9922_DD_PRX_MEAS_RATE_6_25_MS 0x01 /* PS Measurement rate = 6.25 ms */
#define APDS9922_DD_PRX_MEAS_RATE_12_5_MS 0x02 /* PS Measurement rate = 12.5 ms */
#define APDS9922_DD_PRX_MEAS_RATE_25_MS 0x03 /* PS Measurement rate = 25 ms */
#define APDS9922_DD_PRX_MEAS_RATE_50_MS 0x04 /* PS Measurement rate = 50 ms */
#define APDS9922_DD_PRX_MEAS_RATE_100_MS 0x05 /* PS Measurement rate = 100 ms */
#define APDS9922_DD_PRX_MEAS_RATE_200_MS 0x06 /* PS Measurement rate = 200 ms */
#define APDS9922_DD_PRX_MEAS_RATE_400_MS 0x07 /* PS Measurement rate = 400 ms */

#define APDS9922_DD_PRX_MEAS_RES_8_BIT 0x00 /* PS resolution 8 bit (full range : 0 ~ 255) */
#define APDS9922_DD_PRX_MEAS_RES_9_BIT 0x08 /* PS resolution 9 bit (full range : 0 ~ 511) */
#define APDS9922_DD_PRX_MEAS_RES_10_BIT 0x10 /* PS resolution 10 bit (full range : 0 ~ 1023) */
#define APDS9922_DD_PRX_MEAS_RES_11_BIT 0x18 /* PS resolution 11 bit (full range : 0 ~ 2047) */

/* Register Value define : ALS_MEAS_RATE */
#define APDS9922_DD_ALS_MEAS_RATE_25_MS 0x00 /* ALS Measurement rate = 25 ms */
#define APDS9922_DD_ALS_MEAS_RATE_50_MS 0x01 /* ALS Measurement rate = 50 ms */
#define APDS9922_DD_ALS_MEAS_RATE_100_MS 0x02 /* ALS Measurement rate = 100 ms */
#define APDS9922_DD_ALS_MEAS_RATE_200_MS 0x03 /* ALS Measurement rate = 200 ms */
#define APDS9922_DD_ALS_MEAS_RATE_500_MS 0x04 /* ALS Measurement rate = 500 ms */
#define APDS9922_DD_ALS_MEAS_RATE_1000_MS 0x05 /* ALS Measurement rate = 1000 ms */
#define APDS9922_DD_ALS_MEAS_RATE_2000_MS 0x06 /* ALS Measurement rate = 2000 ms */

#define APDS9922_DD_ALS_MEAS_RES_20_BIT 0x00
#define APDS9922_DD_ALS_MEAS_RES_19_BIT 0x10
#define APDS9922_DD_ALS_MEAS_RES_18_BIT 0x20
#define APDS9922_DD_ALS_MEAS_RES_17_BIT 0x30
#define APDS9922_DD_ALS_MEAS_RES_16_BIT 0x40

/* Register Value define : ALS_GAIN */
#define APDS9922_DD_ALS_GAIN_1 0x00 /* ALS Gain 1 */
#define APDS9922_DD_ALS_GAIN_3 0x01 /* ALS Gain 3 */
#define APDS9922_DD_ALS_GAIN_6 0x02 /* ALS Gain 6 */
#define APDS9922_DD_ALS_GAIN_9 0x03 /* ALS Gain 9 */
#define APDS9922_DD_ALS_GAIN_18 0x04 /* ALS Gain 18 */

/* Register Value define : MAIN_STATUS */
#define APDS9922_DD_PRX_DATA_STATUS 0x01 /* 1: New data, not read yet (cleared after read) */
#define APDS9922_DD_PRX_INT_STATUS 0x02 /* 1: Interrupt condition fulfilled (cleared after read) */
#define APDS9922_DD_PRX_LOGICAL_STATUS 0x04 /* 1: object is close */
#define APDS9922_DD_ALS_DATA_STATUS 0x08 /* 1: New data, not read yet (cleared after read) */
#define APDS9922_DD_ALS_INT_STATUS 0x10 /* 1: Interrupt condition fulfilled (cleared after read) */
#define APDS9922_DD_POWER_ON_STATUS 0x20 /* 1: Power on cycle */

/* Register Value define : INT_CFG */
#define APDS9922_DD_PRX_INT_EN 0x01 /* 1: PS Interrupt enabled */
#define APDS9922_DD_PRX_LOGIC_MODE 0x02 /* 1: PS Logic Output Mode */
#define APDS9922_DD_ALS_INT_EN 0x04 /* 1: ALS Interrupt enabled */
#define APDS9922_DD_ALS_VAR_MODE 0x08 /* 1: ALS variation interrupt mode */
#define APDS9922_DD_ALS_INT_SEL_ALS 0x10 /* ALS channel selected for interrupt */
#define APDS9922_DD_ALS_INT_SEL_CLEAR 0x00 /* Clear channel selected for interrupt */

/* Register Value define : INT_PERSISTENCE */
#define APDS9922_DD_PRX_PERS_1 0x00 /* Every PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_2 0x01 /* 2 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_3 0x02 /* 3 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_4 0x03 /* 4 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_5 0x04 /* 5 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_6 0x05 /* 6 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_7 0x06 /* 7 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_8 0x07 /* 8 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_9 0x08 /* 9 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_10 0x09 /* 10 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_11 0x0A /* 11 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_12 0x0B /* 12 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_13 0x0C /* 13 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_14 0x0D /* 14 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_15 0x0E /* 15 consecutive PS value out of threshold range */
#define APDS9922_DD_PRX_PERS_16 0x0F /* 16 consecutive PS value out of threshold range */

#define APDS9922_DD_ALS_PERS_1 0x00 /* Every ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_2 0x10 /* 2 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_3 0x20 /* 3 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_4 0x30 /* 4 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_5 0x40 /* 5 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_6 0x50 /* 6 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_7 0x60 /* 7 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_8 0x70 /* 8 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_9 0x80 /* 9 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_10 0x90 /* 10 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_11 0xA0 /* 11 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_12 0xB0 /* 12 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_13 0xC0 /* 13 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_14 0xD0 /* 14 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_15 0xE0 /* 15 consecutive ALS value out of threshold range */
#define APDS9922_DD_ALS_PERS_16 0xF0 /* 16 consecutive ALS value out of threshold range */


#define APDS9922_DD_LUX_FACTOR 315 /* for glass transmittance of 10%, use 30 for open air */
#define APDS9922_DD_ALS_DEFAULT_RES APDS9922_DD_ALS_MEAS_RES_16_BIT
#define APDS9922_DD_ALS_DEFAULT_MEAS_RATE APDS9922_DD_ALS_MEAS_RATE_25_MS
#define APDS9922_DD_ALS_DEFAULT_GAIN APDS9922_DD_ALS_GAIN_18

#define APDS9922_PS_DETECTION_THRESHOLD 400
#define APDS9922_PS_HSYTERESIS_THRESHOLD 350

#define APDS9922_DD_PRX_DEFAULT_PULSE 16
#define APDS9922_DD_PRX_DEFAULT_LED_CURRENT APDS9922_DD_LED_CURRENT_125_MA
#define APDS9922_DD_PRX_DEFAULT_LED_FREQ APDS9922_DD_LED_FREQ_60_KHZ
#define APDS9922_DD_PRX_DEFAULT_RES APDS9922_DD_PRX_MEAS_RES_11_BIT
#define APDS9922_DD_PRX_DEFAULT_MEAS_RATE APDS9922_DD_PRX_MEAS_RATE_50_MS

#endif
