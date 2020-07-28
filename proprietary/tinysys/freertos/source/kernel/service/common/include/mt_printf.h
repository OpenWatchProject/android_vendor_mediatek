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

#ifndef MT_PRINTF_H
#define MT_PRINTF_H

#include <stdio.h>


/******************************************************************************
* Log's module and level fileds share the same 32-bit integer.
******************************************************************************/
#define LOG_MODULE_MASK     0xFFFFFFF8
#define LOG_LEVEL_MASK      0x00000007


typedef enum log_module {
    MOD_OS          = 0x80000000,  /* B31: operating system and platform */
    MOD_UART        = 0x40000000,  /* B30: UART */
    MOD_I2C         = 0x20000000,  /* B29: I2C */
    MOD_SPI         = 0x10000000,  /* B28: SPI */
    MOD_WDT         = 0x08000000,  /* B27: WDT */
    MOD_TIMER       = 0x04000000,  /* B26: TIMER */
    MOD_PMIC        = 0x02000000,  /* B25: PMIC */
    MOD_DVFS        = 0x01000000,  /* B24: DVFS */
    MOD_CHRE        = 0x00800000,  /* B23: CHRE */
    MOD_AUDIO       = 0x00400000,  /* B22: AUDIO */
    MOD_VOW         = 0x00200000,  /* B21: VoW (Voice Wakeup) */
    MOD_RSRV_B20    = 0x00100000,  /* B20: reserved */
    MOD_RSRV_B19    = 0x00080000,  /* B19: reserved */
    MOD_RSRV_B18    = 0x00040000,  /* B18: reserved */
    MOD_RSRV_B17    = 0x00020000,  /* B17: reserved */
    MOD_RSRV_B16    = 0x00010000,  /* B16: reserved */
    MOD_ACCGYRO     = 0x00008000,  /* B15: sensor: AccGyro */
    MOD_MAG         = 0x00004000,  /* B14: sensor: Mag */
    MOD_ALSPS       = 0x00002000,  /* B13: sensor: AlsPs */
    MOD_BARO        = 0x00001000,  /* B12: sensor: Baro */
    MOD_SAR         = 0x00000800,  /* B11: sensor: Sar */
    MOD_FUSION      = 0x00000400,  /* B10: sensor: Fusion */
    MOD_GESTURE     = 0x00000200,  /* B09: sensor: Gesture */
    MOD_RSRV_B8     = 0x00000100,  /* B08: reserved for sensor */
    MOD_RSRV_B7     = 0x00000080,  /* B07: reserved for sensor */
    MOD_RSRV_B6     = 0x00000040,  /* B06: reserved for sensor */
    MOD_RSRV_B5     = 0x00000020,  /* B05: reserved for sensor */
    MOD_RSRV_B4     = 0x00000010,  /* B04: reserved for sensor */
    MOD_COMPATIBLE  = 0x00000008,  /* B03: compatible w/ old print mechanism */
} log_module_t;


#ifdef CFG_LOG_FILTER
/******************************************************************************
* The lower the value the more important it is.
******************************************************************************/
typedef enum log_level {
    LEVEL_EMERG     = 0,  /* emergency and system is unusable */
    LEVEL_ALERT     = 1,  /* action must be taken immediately */
    LEVEL_CRIT      = 2,  /* critical condition */
    LEVEL_ERROR     = 3,  /* error condition */
    LEVEL_WARN      = 4,  /* warning condition */
    LEVEL_NOTE      = 5,  /* normal but significant */
    LEVEL_INFO      = 6,  /* information only */
    LEVEL_DEBUG     = 7   /* debug messages */
} log_level_t;
#else
/******************************************************************************
* When CFG_LOG_FILTER is turned off, the following defintions makes osLog()
* compatible with previous definition.  The osLog() is also re-written so
* that code size does not increase when CFG_LOG_FILTER is turned off.
******************************************************************************/
typedef enum log_level {
    LEVEL_EMERG     = 0,  /* emergency and system is unusable */
    LEVEL_ALERT     = 0,  /* action must be taken immediately */
    LEVEL_CRIT      = 0,  /* critical condition */
    LEVEL_ERROR     = 0,  /* error condition */
    LEVEL_WARN      = 1,  /* warning condition */
    LEVEL_NOTE      = 1,  /* normal but significant */
    LEVEL_INFO      = 2,  /* information only */
    LEVEL_DEBUG     = 3   /* debug messages */
} log_level_t;
#endif  // CFG_LOG_FILTER


/******************************************************************************
* Module and level specifiers are defined below.
******************************************************************************/
#define LOG_OS_EMERG        (MOD_OS | LEVEL_EMERG)
#define LOG_OS_ALERT        (MOD_OS | LEVEL_ALERT)
#define LOG_OS_CRIT         (MOD_OS | LEVEL_CRIT)
#define LOG_OS_ERROR        (MOD_OS | LEVEL_ERROR)
#define LOG_OS_WARN         (MOD_OS | LEVEL_WARN)
#define LOG_OS_NOTE         (MOD_OS | LEVEL_NOTE)
#define LOG_OS_INFO         (MOD_OS | LEVEL_INFO)
#define LOG_OS_DEBUG        (MOD_OS | LEVEL_DEBUG)

#define LOG_UART_EMERG      (MOD_UART | LEVEL_EMERG)
#define LOG_UART_ALERT      (MOD_UART | LEVEL_ALERT)
#define LOG_UART_CRIT       (MOD_UART | LEVEL_CRIT)
#define LOG_UART_ERROR      (MOD_UART | LEVEL_ERROR)
#define LOG_UART_WARN       (MOD_UART | LEVEL_WARN)
#define LOG_UART_NOTE       (MOD_UART | LEVEL_NOTE)
#define LOG_UART_INFO       (MOD_UART | LEVEL_INFO)
#define LOG_UART_DEBUG      (MOD_UART | LEVEL_DEBUG)

#define LOG_I2C_EMERG       (MOD_I2C | LEVEL_EMERG)
#define LOG_I2C_ALERT       (MOD_I2C | LEVEL_ALERT)
#define LOG_I2C_CRIT        (MOD_I2C | LEVEL_CRIT)
#define LOG_I2C_ERROR       (MOD_I2C | LEVEL_ERROR)
#define LOG_I2C_WARN        (MOD_I2C | LEVEL_WARN)
#define LOG_I2C_NOTE        (MOD_I2C | LEVEL_NOTE)
#define LOG_I2C_INFO        (MOD_I2C | LEVEL_INFO)
#define LOG_I2C_DEBUG       (MOD_I2C | LEVEL_DEBUG)

#define LOG_SPI_EMERG       (MOD_SPI | LEVEL_EMERG)
#define LOG_SPI_ALERT       (MOD_SPI | LEVEL_ALERT)
#define LOG_SPI_CRIT        (MOD_SPI | LEVEL_CRIT)
#define LOG_SPI_ERROR       (MOD_SPI | LEVEL_ERROR)
#define LOG_SPI_WARN        (MOD_SPI | LEVEL_WARN)
#define LOG_SPI_NOTE        (MOD_SPI | LEVEL_NOTE)
#define LOG_SPI_INFO        (MOD_SPI | LEVEL_INFO)
#define LOG_SPI_DEBUG       (MOD_SPI | LEVEL_DEBUG)

#define LOG_WDT_EMERG       (MOD_WDT | LEVEL_EMERG)
#define LOG_WDT_ALERT       (MOD_WDT | LEVEL_ALERT)
#define LOG_WDT_CRIT        (MOD_WDT | LEVEL_CRIT)
#define LOG_WDT_ERROR       (MOD_WDT | LEVEL_ERROR)
#define LOG_WDT_WARN        (MOD_WDT | LEVEL_WARN)
#define LOG_WDT_NOTE        (MOD_WDT | LEVEL_NOTE)
#define LOG_WDT_INFO        (MOD_WDT | LEVEL_INFO)
#define LOG_WDT_DEBUG       (MOD_WDT | LEVEL_DEBUG)

#define LOG_TIMER_EMERG     (MOD_TIMER | LEVEL_EMERG)
#define LOG_TIMER_ALERT     (MOD_TIMER | LEVEL_ALERT)
#define LOG_TIMER_CRIT      (MOD_TIMER | LEVEL_CRIT)
#define LOG_TIMER_ERROR     (MOD_TIMER | LEVEL_ERROR)
#define LOG_TIMER_WARN      (MOD_TIMER | LEVEL_WARN)
#define LOG_TIMER_NOTE      (MOD_TIMER | LEVEL_NOTE)
#define LOG_TIMER_INFO      (MOD_TIMER | LEVEL_INFO)
#define LOG_TIMER_DEBUG     (MOD_TIMER | LEVEL_DEBUG)

#define LOG_PMIC_EMERG      (MOD_PMIC | LEVEL_EMERG)
#define LOG_PMIC_ALERT      (MOD_PMIC | LEVEL_ALERT)
#define LOG_PMIC_CRIT       (MOD_PMIC | LEVEL_CRIT)
#define LOG_PMIC_ERROR      (MOD_PMIC | LEVEL_ERROR)
#define LOG_PMIC_WARN       (MOD_PMIC | LEVEL_WARN)
#define LOG_PMIC_NOTE       (MOD_PMIC | LEVEL_NOTE)
#define LOG_PMIC_INFO       (MOD_PMIC | LEVEL_INFO)
#define LOG_PMIC_DEBUG      (MOD_PMIC | LEVEL_DEBUG)

#define LOG_DVFS_EMERG      (MOD_DVFS | LEVEL_EMERG)
#define LOG_DVFS_ALERT      (MOD_DVFS | LEVEL_ALERT)
#define LOG_DVFS_CRIT       (MOD_DVFS | LEVEL_CRIT)
#define LOG_DVFS_ERROR      (MOD_DVFS | LEVEL_ERROR)
#define LOG_DVFS_WARN       (MOD_DVFS | LEVEL_WARN)
#define LOG_DVFS_NOTE       (MOD_DVFS | LEVEL_NOTE)
#define LOG_DVFS_INFO       (MOD_DVFS | LEVEL_INFO)
#define LOG_DVFS_DEBUG      (MOD_DVFS | LEVEL_DEBUG)

#define LOG_CHRE_EMERG      (MOD_CHRE | LEVEL_EMERG)
#define LOG_CHRE_ALERT      (MOD_CHRE | LEVEL_ALERT)
#define LOG_CHRE_CRIT       (MOD_CHRE | LEVEL_CRIT)
#define LOG_CHRE_ERROR      (MOD_CHRE | LEVEL_ERROR)
#define LOG_CHRE_WARN       (MOD_CHRE | LEVEL_WARN)
#define LOG_CHRE_NOTE       (MOD_CHRE | LEVEL_NOTE)
#define LOG_CHRE_INFO       (MOD_CHRE | LEVEL_INFO)
#define LOG_CHRE_DEBUG      (MOD_CHRE | LEVEL_DEBUG)

#define LOG_AUDIO_EMERG     (MOD_AUDIO | LEVEL_EMERG)
#define LOG_AUDIO_ALERT     (MOD_AUDIO | LEVEL_ALERT)
#define LOG_AUDIO_CRIT      (MOD_AUDIO | LEVEL_CRIT)
#define LOG_AUDIO_ERROR     (MOD_AUDIO | LEVEL_ERROR)
#define LOG_AUDIO_WARN      (MOD_AUDIO | LEVEL_WARN)
#define LOG_AUDIO_NOTE      (MOD_AUDIO | LEVEL_NOTE)
#define LOG_AUDIO_INFO      (MOD_AUDIO | LEVEL_INFO)
#define LOG_AUDIO_DEBUG     (MOD_AUDIO | LEVEL_DEBUG)

#define LOG_VOW_EMERG       (MOD_VOW | LEVEL_EMERG)
#define LOG_VOW_ALERT       (MOD_VOW | LEVEL_ALERT)
#define LOG_VOW_CRIT        (MOD_VOW | LEVEL_CRIT)
#define LOG_VOW_ERROR       (MOD_VOW | LEVEL_ERROR)
#define LOG_VOW_WARN        (MOD_VOW | LEVEL_WARN)
#define LOG_VOW_NOTE        (MOD_VOW | LEVEL_NOTE)
#define LOG_VOW_INFO        (MOD_VOW | LEVEL_INFO)
#define LOG_VOW_DEBUG       (MOD_VOW | LEVEL_DEBUG)

#define LOG_RSRV_B20_EMERG  (MOD_RSRV_B20 | LEVEL_EMERG)
#define LOG_RSRV_B20_ALERT  (MOD_RSRV_B20 | LEVEL_ALERT)
#define LOG_RSRV_B20_CRIT   (MOD_RSRV_B20 | LEVEL_CRIT)
#define LOG_RSRV_B20_ERROR  (MOD_RSRV_B20 | LEVEL_ERROR)
#define LOG_RSRV_B20_WARN   (MOD_RSRV_B20 | LEVEL_WARN)
#define LOG_RSRV_B20_NOTE   (MOD_RSRV_B20 | LEVEL_NOTE)
#define LOG_RSRV_B20_INFO   (MOD_RSRV_B20 | LEVEL_INFO)
#define LOG_RSRV_B20_DEBUG  (MOD_RSRV_B20 | LEVEL_DEBUG)

#define LOG_RSRV_B19_EMERG  (MOD_RSRV_B19 | LEVEL_EMERG)
#define LOG_RSRV_B19_ALERT  (MOD_RSRV_B19 | LEVEL_ALERT)
#define LOG_RSRV_B19_CRIT   (MOD_RSRV_B19 | LEVEL_CRIT)
#define LOG_RSRV_B19_ERROR  (MOD_RSRV_B19 | LEVEL_ERROR)
#define LOG_RSRV_B19_WARN   (MOD_RSRV_B19 | LEVEL_WARN)
#define LOG_RSRV_B19_NOTE   (MOD_RSRV_B19 | LEVEL_NOTE)
#define LOG_RSRV_B19_INFO   (MOD_RSRV_B19 | LEVEL_INFO)
#define LOG_RSRV_B19_DEBUG  (MOD_RSRV_B19 | LEVEL_DEBUG)

#define LOG_RSRV_B18_EMERG  (MOD_RSRV_B18 | LEVEL_EMERG)
#define LOG_RSRV_B18_ALERT  (MOD_RSRV_B18 | LEVEL_ALERT)
#define LOG_RSRV_B18_CRIT   (MOD_RSRV_B18 | LEVEL_CRIT)
#define LOG_RSRV_B18_ERROR  (MOD_RSRV_B18 | LEVEL_ERROR)
#define LOG_RSRV_B18_WARN   (MOD_RSRV_B18 | LEVEL_WARN)
#define LOG_RSRV_B18_NOTE   (MOD_RSRV_B18 | LEVEL_NOTE)
#define LOG_RSRV_B18_INFO   (MOD_RSRV_B18 | LEVEL_INFO)
#define LOG_RSRV_B18_DEBUG  (MOD_RSRV_B18 | LEVEL_DEBUG)

#define LOG_RSRV_B17_EMERG  (MOD_RSRV_B17 | LEVEL_EMERG)
#define LOG_RSRV_B17_ALERT  (MOD_RSRV_B17 | LEVEL_ALERT)
#define LOG_RSRV_B17_CRIT   (MOD_RSRV_B17 | LEVEL_CRIT)
#define LOG_RSRV_B17_ERROR  (MOD_RSRV_B17 | LEVEL_ERROR)
#define LOG_RSRV_B17_WARN   (MOD_RSRV_B17 | LEVEL_WARN)
#define LOG_RSRV_B17_NOTE   (MOD_RSRV_B17 | LEVEL_NOTE)
#define LOG_RSRV_B17_INFO   (MOD_RSRV_B17 | LEVEL_INFO)
#define LOG_RSRV_B17_DEBUG  (MOD_RSRV_B17 | LEVEL_DEBUG)

#define LOG_RSRV_B16_EMERG  (MOD_RSRV_B16 | LEVEL_EMERG)
#define LOG_RSRV_B16_ALERT  (MOD_RSRV_B16 | LEVEL_ALERT)
#define LOG_RSRV_B16_CRIT   (MOD_RSRV_B16 | LEVEL_CRIT)
#define LOG_RSRV_B16_ERROR  (MOD_RSRV_B16 | LEVEL_ERROR)
#define LOG_RSRV_B16_WARN   (MOD_RSRV_B16 | LEVEL_WARN)
#define LOG_RSRV_B16_NOTE   (MOD_RSRV_B16 | LEVEL_NOTE)
#define LOG_RSRV_B16_INFO   (MOD_RSRV_B16 | LEVEL_INFO)
#define LOG_RSRV_B16_DEBUG  (MOD_RSRV_B16 | LEVEL_DEBUG)

#define LOG_ACCGYRO_EMERG   (MOD_ACCGYRO | LEVEL_EMERG)
#define LOG_ACCGYRO_ALERT   (MOD_ACCGYRO | LEVEL_ALERT)
#define LOG_ACCGYRO_CRIT    (MOD_ACCGYRO | LEVEL_CRIT)
#define LOG_ACCGYRO_ERROR   (MOD_ACCGYRO | LEVEL_ERROR)
#define LOG_ACCGYRO_WARN    (MOD_ACCGYRO | LEVEL_WARN)
#define LOG_ACCGYRO_NOTE    (MOD_ACCGYRO | LEVEL_NOTE)
#define LOG_ACCGYRO_INFO    (MOD_ACCGYRO | LEVEL_INFO)
#define LOG_ACCGYRO_DEBUG   (MOD_ACCGYRO | LEVEL_DEBUG)

#define LOG_MAG_EMERG       (MOD_MAG | LEVEL_EMERG)
#define LOG_MAG_ALERT       (MOD_MAG | LEVEL_ALERT)
#define LOG_MAG_CRIT        (MOD_MAG | LEVEL_CRIT)
#define LOG_MAG_ERROR       (MOD_MAG | LEVEL_ERROR)
#define LOG_MAG_WARN        (MOD_MAG | LEVEL_WARN)
#define LOG_MAG_NOTE        (MOD_MAG | LEVEL_NOTE)
#define LOG_MAG_INFO        (MOD_MAG | LEVEL_INFO)
#define LOG_MAG_DEBUG       (MOD_MAG | LEVEL_DEBUG)

#define LOG_ALSPS_EMERG     (MOD_ALSPS | LEVEL_EMERG)
#define LOG_ALSPS_ALERT     (MOD_ALSPS | LEVEL_ALERT)
#define LOG_ALSPS_CRIT      (MOD_ALSPS | LEVEL_CRIT)
#define LOG_ALSPS_ERROR     (MOD_ALSPS | LEVEL_ERROR)
#define LOG_ALSPS_WARN      (MOD_ALSPS | LEVEL_WARN)
#define LOG_ALSPS_NOTE      (MOD_ALSPS | LEVEL_NOTE)
#define LOG_ALSPS_INFO      (MOD_ALSPS | LEVEL_INFO)
#define LOG_ALSPS_DEBUG     (MOD_ALSPS | LEVEL_DEBUG)

#define LOG_BARO_EMERG      (MOD_BARO | LEVEL_EMERG)
#define LOG_BARO_ALERT      (MOD_BARO | LEVEL_ALERT)
#define LOG_BARO_CRIT       (MOD_BARO | LEVEL_CRIT)
#define LOG_BARO_ERROR      (MOD_BARO | LEVEL_ERROR)
#define LOG_BARO_WARN       (MOD_BARO | LEVEL_WARN)
#define LOG_BARO_NOTE       (MOD_BARO | LEVEL_NOTE)
#define LOG_BARO_INFO       (MOD_BARO | LEVEL_INFO)
#define LOG_BARO_DEBUG      (MOD_BARO | LEVEL_DEBUG)

#define LOG_SAR_EMERG       (MOD_SAR | LEVEL_EMERG)
#define LOG_SAR_ALERT       (MOD_SAR | LEVEL_ALERT)
#define LOG_SAR_CRIT        (MOD_SAR | LEVEL_CRIT)
#define LOG_SAR_ERROR       (MOD_SAR | LEVEL_ERROR)
#define LOG_SAR_WARN        (MOD_SAR | LEVEL_WARN)
#define LOG_SAR_NOTE        (MOD_SAR | LEVEL_NOTE)
#define LOG_SAR_INFO        (MOD_SAR | LEVEL_INFO)
#define LOG_SAR_DEBUG       (MOD_SAR | LEVEL_DEBUG)

#define LOG_FUSION_EMERG    (MOD_FUSION | LEVEL_EMERG)
#define LOG_FUSION_ALERT    (MOD_FUSION | LEVEL_ALERT)
#define LOG_FUSION_CRIT     (MOD_FUSION | LEVEL_CRIT)
#define LOG_FUSION_ERROR    (MOD_FUSION | LEVEL_ERROR)
#define LOG_FUSION_WARN     (MOD_FUSION | LEVEL_WARN)
#define LOG_FUSION_NOTE     (MOD_FUSION | LEVEL_NOTE)
#define LOG_FUSION_INFO     (MOD_FUSION | LEVEL_INFO)
#define LOG_FUSION_DEBUG    (MOD_FUSION | LEVEL_DEBUG)

#define LOG_GESTURE_EMERG   (MOD_GESTURE | LEVEL_EMERG)
#define LOG_GESTURE_ALERT   (MOD_GESTURE | LEVEL_ALERT)
#define LOG_GESTURE_CRIT    (MOD_GESTURE | LEVEL_CRIT)
#define LOG_GESTURE_ERROR   (MOD_GESTURE | LEVEL_ERROR)
#define LOG_GESTURE_WARN    (MOD_GESTURE | LEVEL_WARN)
#define LOG_GESTURE_NOTE    (MOD_GESTURE | LEVEL_NOTE)
#define LOG_GESTURE_INFO    (MOD_GESTURE | LEVEL_INFO)
#define LOG_GESTURE_DEBUG   (MOD_GESTURE | LEVEL_DEBUG)

#define LOG_RSRV_B8_EMERG   (MOD_RSRV_B8 | LEVEL_EMERG)
#define LOG_RSRV_B8_ALERT   (MOD_RSRV_B8 | LEVEL_ALERT)
#define LOG_RSRV_B8_CRIT    (MOD_RSRV_B8 | LEVEL_CRIT)
#define LOG_RSRV_B8_ERROR   (MOD_RSRV_B8 | LEVEL_ERROR)
#define LOG_RSRV_B8_WARN    (MOD_RSRV_B8 | LEVEL_WARN)
#define LOG_RSRV_B8_NOTE    (MOD_RSRV_B8 | LEVEL_NOTE)
#define LOG_RSRV_B8_INFO    (MOD_RSRV_B8 | LEVEL_INFO)
#define LOG_RSRV_B8_DEBUG   (MOD_RSRV_B8 | LEVEL_DEBUG)

#define LOG_RSRV_B7_EMERG   (MOD_RSRV_B7 | LEVEL_EMERG)
#define LOG_RSRV_B7_ALERT   (MOD_RSRV_B7 | LEVEL_ALERT)
#define LOG_RSRV_B7_CRIT    (MOD_RSRV_B7 | LEVEL_CRIT)
#define LOG_RSRV_B7_ERROR   (MOD_RSRV_B7 | LEVEL_ERROR)
#define LOG_RSRV_B7_WARN    (MOD_RSRV_B7 | LEVEL_WARN)
#define LOG_RSRV_B7_NOTE    (MOD_RSRV_B7 | LEVEL_NOTE)
#define LOG_RSRV_B7_INFO    (MOD_RSRV_B7 | LEVEL_INFO)
#define LOG_RSRV_B7_DEBUG   (MOD_RSRV_B7 | LEVEL_DEBUG)

#define LOG_RSRV_B6_EMERG   (MOD_RSRV_B6 | LEVEL_EMERG)
#define LOG_RSRV_B6_ALERT   (MOD_RSRV_B6 | LEVEL_ALERT)
#define LOG_RSRV_B6_CRIT    (MOD_RSRV_B6 | LEVEL_CRIT)
#define LOG_RSRV_B6_ERROR   (MOD_RSRV_B6 | LEVEL_ERROR)
#define LOG_RSRV_B6_WARN    (MOD_RSRV_B6 | LEVEL_WARN)
#define LOG_RSRV_B6_NOTE    (MOD_RSRV_B6 | LEVEL_NOTE)
#define LOG_RSRV_B6_INFO    (MOD_RSRV_B6 | LEVEL_INFO)
#define LOG_RSRV_B6_DEBUG   (MOD_RSRV_B6 | LEVEL_DEBUG)

#define LOG_RSRV_B5_EMERG   (MOD_RSRV_B5 | LEVEL_EMERG)
#define LOG_RSRV_B5_ALERT   (MOD_RSRV_B5 | LEVEL_ALERT)
#define LOG_RSRV_B5_CRIT    (MOD_RSRV_B5 | LEVEL_CRIT)
#define LOG_RSRV_B5_ERROR   (MOD_RSRV_B5 | LEVEL_ERROR)
#define LOG_RSRV_B5_WARN    (MOD_RSRV_B5 | LEVEL_WARN)
#define LOG_RSRV_B5_NOTE    (MOD_RSRV_B5 | LEVEL_NOTE)
#define LOG_RSRV_B5_INFO    (MOD_RSRV_B5 | LEVEL_INFO)
#define LOG_RSRV_B5_DEBUG   (MOD_RSRV_B5 | LEVEL_DEBUG)

#define LOG_RSRV_B4_EMERG   (MOD_RSRV_B4 | LEVEL_EMERG)
#define LOG_RSRV_B4_ALERT   (MOD_RSRV_B4 | LEVEL_ALERT)
#define LOG_RSRV_B4_CRIT    (MOD_RSRV_B4 | LEVEL_CRIT)
#define LOG_RSRV_B4_ERROR   (MOD_RSRV_B4 | LEVEL_ERROR)
#define LOG_RSRV_B4_WARN    (MOD_RSRV_B4 | LEVEL_WARN)
#define LOG_RSRV_B4_NOTE    (MOD_RSRV_B4 | LEVEL_NOTE)
#define LOG_RSRV_B4_INFO    (MOD_RSRV_B4 | LEVEL_INFO)
#define LOG_RSRV_B4_DEBUG   (MOD_RSRV_B4 | LEVEL_DEBUG)

#define LOG_COMPAT_EMERG    (MOD_COMPATIBLE | LEVEL_EMERG)
#define LOG_COMPAT_ALERT    (MOD_COMPATIBLE | LEVEL_ALERT)
#define LOG_COMPAT_CRIT     (MOD_COMPATIBLE | LEVEL_CRIT)
#define LOG_COMPAT_ERROR    (MOD_COMPATIBLE | LEVEL_ERROR)
#define LOG_COMPAT_WARN     (MOD_COMPATIBLE | LEVEL_WARN)
#define LOG_COMPAT_NOTE     (MOD_COMPATIBLE | LEVEL_NOTE)
#define LOG_COMPAT_INFO     (MOD_COMPATIBLE | LEVEL_INFO)
#define LOG_COMPAT_DEBUG    (MOD_COMPATIBLE | LEVEL_DEBUG)


/******************************************************************************
* Function prototypes
******************************************************************************/
void set_log_filter(int id, void *data, unsigned int len);


/******************************************************************************
******************************************************************************/
#ifndef CFG_LOG_FILTER
/* DEBUG LEVEL definition*/
#define LOG_DEBUG       3
#define LOG_INFO        2
#define LOG_WARN        1
#define LOG_ERROR       0
#define PRINTF(x...)    PRINTF_D(x)

#define PRINTF_D(x...)            \
({                                \
    if (LOG_DEBUG <= DEBUGLEVEL)  \
        printf(x);                \
})

#define PRINTF_I(x...)            \
({                                \
    if (LOG_INFO <= DEBUGLEVEL)   \
        printf(x);                \
})

#define PRINTF_W(x...)            \
({                                \
    if (LOG_WARN <= DEBUGLEVEL)   \
        printf(x);                \
})


#define PRINTF_E(x...)            \
({                                \
    if (LOG_ERROR <= DEBUGLEVEL)  \
        printf(x);                \
})

#define PRINTF_ONCE(x...)                                       \
({                                                              \
    static char __print_once;                                   \
                                                                \
    if (!__print_once) {                                        \
        __print_once = 1;                                       \
        printf(x);                                              \
    }                                                           \
})

/******************************************************************************
* mlprintf: printf with module and level specifiers
* This macro is defined to be compatible on previous platforms when the common
* code makes use of the mlprintf() macro.
******************************************************************************/
#define mlprintf(MOD_LEVEL, fmt, ...)                                   \
    do {                                                                \
        if ((MOD_LEVEL & LOG_LEVEL_MASK) < LEVEL_WARN) {                \
            printf(fmt, ##__VA_ARGS__);                                 \
        }                                                               \
    } while (0)


#else


/******************************************************************************
* This global variable should not be used directly by any module.
* It should be used only in mlprintf(), and set_log_filter().
******************************************************************************/
extern uint32_t log_filter;


/******************************************************************************
* ML: Module Level
* IS_MOD_SET: Is any module bit set?
* IS_LEVEL_PRINTABLE: If the level is less than or equal to the level in the
*                     filter, the log should be printed.
******************************************************************************/
#define IS_MOD_SET(ML)          (ML & LOG_MODULE_MASK & log_filter)
#define GET_LEVEL(ML)           (ML & LOG_LEVEL_MASK)
#define FILTER_LEVEL            (log_filter & LOG_LEVEL_MASK)
#define IS_LEVEL_PRINTABLE(ML)  (GET_LEVEL(ML) <= FILTER_LEVEL)


/******************************************************************************
* mlprintf: printf with module and level specifiers
******************************************************************************/
#define mlprintf(MOD_LEVEL, fmt, ...)                                   \
    do {                                                                \
        if (IS_MOD_SET(MOD_LEVEL) && IS_LEVEL_PRINTABLE(MOD_LEVEL)) {   \
            printf(fmt, ##__VA_ARGS__);                                 \
        }                                                               \
    } while (0)


/******************************************************************************
* The following macros are deprecated. Do not use them in new drivers.
* They are defined below simply for compatibility.
******************************************************************************/
#define LOG_DEBUG       3
#define LOG_INFO        2
#define LOG_WARN        1
#define LOG_ERROR       0

#define PRINTF          PRINTF_D

#define PRINTF_D(fmt, ...)  mlprintf(LOG_COMPAT_INFO, fmt, ##__VA_ARGS__)
#define PRINTF_I(fmt, ...)  mlprintf(LOG_COMPAT_INFO, fmt, ##__VA_ARGS__)
#define PRINTF_W(fmt, ...)  mlprintf(LOG_COMPAT_WARN, fmt, ##__VA_ARGS__)
#define PRINTF_E(fmt, ...)  mlprintf(LOG_COMPAT_ERROR, fmt, ##__VA_ARGS__)

#define PRINTF_ONCE(fmt, ...)                                   \
    do {                                                        \
        static char __print_once;                               \
        if (!__print_once) {                                    \
            __print_once = 1;                                   \
            printf(fmt, ##__VA_ARGS__);                         \
        }                                                       \
    } while (0)
/******************************************************************************
* The macros above are deprecated. Do not use them in new drivers.
* They are defined above simply for compatibility.
******************************************************************************/
#endif  // CFG_LOG_FILTER


#endif  /* MT_PRINTF_H */

