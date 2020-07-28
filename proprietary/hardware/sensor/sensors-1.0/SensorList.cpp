/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dlfcn.h>
#include <utils/Log.h>
#include <unistd.h>
#include <hardware/sensors.h>
#include <linux/hwmsensor.h>
#include "SensorList.h"
#include "hwmsen_custom.h"

#undef LOG_TAG
#define LOG_TAG "SensorList"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#ifdef CUSTOM_KERNEL_ACCELEROMETER
    #ifndef ACCELEROMETER
        #define ACCELEROMETER                           "ACCELEROMETER"
    #endif
    #ifndef ACCELEROMETER_VENDER
        #define ACCELEROMETER_VENDER                    "MTK"
    #endif
    #ifndef ACCELEROMETER_VERSION
        #define ACCELEROMETER_VERSION                   1
    #endif
    #ifndef ACCELEROMETER_RANGE
        #define ACCELEROMETER_RANGE                     39.2266f
    #endif
    #ifndef ACCELEROMETER_RESOLUTION
        #define ACCELEROMETER_RESOLUTION                0.0012
    #endif
    #ifndef ACCELEROMETER_POWER
        #define ACCELEROMETER_POWER                     0
    #endif
    #ifndef ACCELEROMETER_MINDELAY
        #define ACCELEROMETER_MINDELAY                  10000
    #endif
    #ifndef ACCELEROMETER_FIFO_MAX_COUNT
        #define ACCELEROMETER_FIFO_MAX_COUNT            0
    #endif
    #ifndef ACCELEROMETER_FIFO_RESERVE_COUNT
        #define ACCELEROMETER_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef ACCELEROMETER_MAXDELAY
        #define ACCELEROMETER_MAXDELAY                  1000000
    #endif
    #ifndef ACCELEROMETER_FLAGS
        #define ACCELEROMETER_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_MAGNETOMETER
    #ifndef MAGNETOMETER
        #define MAGNETOMETER                           "MAGNETOMETER"
    #endif
    #ifndef MAGNETOMETER_VENDER
        #define MAGNETOMETER_VENDER                    "MTK"
    #endif
    #ifndef MAGNETOMETER_VERSION
        #define MAGNETOMETER_VERSION                   1
    #endif
    #ifndef MAGNETOMETER_RANGE
        #define MAGNETOMETER_RANGE                     4912.0f
    #endif
    #ifndef MAGNETOMETER_RESOLUTION
        #define MAGNETOMETER_RESOLUTION                0.15f
    #endif
    #ifndef MAGNETOMETER_POWER
        #define MAGNETOMETER_POWER                     0
    #endif
    #ifndef MAGNETOMETER_MINDELAY
        #define MAGNETOMETER_MINDELAY                  20000
    #endif
    #ifndef MAGNETOMETER_FIFO_MAX_COUNT
        #define MAGNETOMETER_FIFO_MAX_COUNT            0
    #endif
    #ifndef MAGNETOMETER_FIFO_RESERVE_COUNT
        #define MAGNETOMETER_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef MAGNETOMETER_MAXDELAY
        #define MAGNETOMETER_MAXDELAY                  1000000
    #endif
    #ifndef MAGNETOMETER_FLAGS
        #define MAGNETOMETER_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif

    #ifndef ORIENTATION
        #define ORIENTATION                           "ORIENTATION"
    #endif
    #ifndef ORIENTATION_VENDER
        #define ORIENTATION_VENDER                    "MTK"
    #endif
    #ifndef ORIENTATION_VERSION
        #define ORIENTATION_VERSION                   1
    #endif
    #ifndef ORIENTATION_RANGE
        #define ORIENTATION_RANGE                     360.0f
    #endif
    #ifndef ORIENTATION_RESOLUTION
        #define ORIENTATION_RESOLUTION                1.0f/256.0f
    #endif
    #ifndef ORIENTATION_POWER
        #define ORIENTATION_POWER                     0
    #endif
    #ifndef ORIENTATION_MINDELAY
        #define ORIENTATION_MINDELAY                  20000
    #endif
    #ifndef ORIENTATION_FIFO_MAX_COUNT
        #define ORIENTATION_FIFO_MAX_COUNT            0
    #endif
    #ifndef ORIENTATION_FIFO_RESERVE_COUNT
        #define ORIENTATION_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef ORIENTATION_MAXDELAY
        #define ORIENTATION_MAXDELAY                  1000000
    #endif
    #ifndef ORIENTATION_FLAGS
        #define ORIENTATION_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_GYROSCOPE
    #ifndef GYROSCOPE
        #define GYROSCOPE                           "GYROSCOPE"
    #endif
    #ifndef GYROSCOPE_VENDER
        #define GYROSCOPE_VENDER                    "MTK"
    #endif
    #ifndef GYROSCOPE_VERSION
        #define GYROSCOPE_VERSION                   1
    #endif
    #ifndef GYROSCOPE_RANGE
        #define GYROSCOPE_RANGE                     34.9066f
    #endif
    #ifndef GYROSCOPE_RESOLUTION
        #define GYROSCOPE_RESOLUTION                0.0011f
    #endif
    #ifndef GYROSCOPE_POWER
        #define GYROSCOPE_POWER                     0
    #endif
    #ifndef GYROSCOPE_MINDELAY
        #define GYROSCOPE_MINDELAY                  5000
    #endif
    #ifndef GYROSCOPE_FIFO_MAX_COUNT
        #define GYROSCOPE_FIFO_MAX_COUNT            0
    #endif
    #ifndef GYROSCOPE_FIFO_RESERVE_COUNT
        #define GYROSCOPE_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef GYROSCOPE_MAXDELAY
        #define GYROSCOPE_MAXDELAY                  1000000
    #endif
    #ifndef GYROSCOPE_FLAGS
        #define GYROSCOPE_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#if defined(CUSTOM_KERNEL_ALSPS) || defined(CUSTOM_KERNEL_ALS)
    #ifndef LIGHT
        #define LIGHT                           "LIGHT"
    #endif
    #ifndef LIGHT_VENDER
        #define LIGHT_VENDER                    "MTK"
    #endif
    #ifndef LIGHT_VERSION
        #define LIGHT_VERSION                   1
    #endif
    #ifndef LIGHT_RANGE
        #define LIGHT_RANGE                     65535.0f
    #endif
    #ifndef LIGHT_RESOLUTION
        #define LIGHT_RESOLUTION                1.0f
    #endif
    #ifndef LIGHT_POWER
        #define LIGHT_POWER                     0
    #endif
    #ifndef LIGHT_MINDELAY
        #define LIGHT_MINDELAY                  0
    #endif
    #ifndef LIGHT_FIFO_MAX_COUNT
        #define LIGHT_FIFO_MAX_COUNT            0
    #endif
    #ifndef LIGHT_FIFO_RESERVE_COUNT
        #define LIGHT_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef LIGHT_MAXDELAY
        #define LIGHT_MAXDELAY                  1000000
    #endif
    #ifndef LIGHT_FLAGS
        #define LIGHT_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_BAROMETER
    #ifndef PRESSURE
        #define PRESSURE                           "PRESSURE"
    #endif
    #ifndef PRESSURE_VENDER
        #define PRESSURE_VENDER                    "MTK"
    #endif
    #ifndef PRESSURE_VERSION
        #define PRESSURE_VERSION                   1
    #endif
    #ifndef PRESSURE_RANGE
        #define PRESSURE_RANGE                     1572.86f
    #endif
    #ifndef PRESSURE_RESOLUTION
        #define PRESSURE_RESOLUTION                0.0016
    #endif
    #ifndef PRESSURE_POWER
        #define PRESSURE_POWER                     0
    #endif
    #ifndef PRESSURE_MINDELAY
        #define PRESSURE_MINDELAY                  10000
    #endif
    #ifndef PRESSURE_FIFO_MAX_COUNT
        #define PRESSURE_FIFO_MAX_COUNT            0
    #endif
    #ifndef PRESSURE_FIFO_RESERVE_COUNT
        #define PRESSURE_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef PRESSURE_MAXDELAY
        #define PRESSURE_MAXDELAY                  1000000
    #endif
    #ifndef PRESSURE_FLAGS
        #define PRESSURE_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_TEMPURATURE
    #ifndef TEMPURATURE
        #define TEMPURATURE                           "TEMPURATURE"
    #endif
    #ifndef TEMPURATURE_VENDER
        #define TEMPURATURE_VENDER                    "MTK"
    #endif
    #ifndef TEMPURATURE_VERSION
        #define TEMPURATURE_VERSION                   1
    #endif
    #ifndef TEMPURATURE_RANGE
        #define TEMPURATURE_RANGE                     85.0f
    #endif
    #ifndef TEMPURATURE_RESOLUTION
        #define TEMPURATURE_RESOLUTION                0.01f
    #endif
    #ifndef TEMPURATURE_POWER
        #define TEMPURATURE_POWER                     0
    #endif
    #ifndef TEMPURATURE_MINDELAY
        #define TEMPURATURE_MINDELAY                  0
    #endif
    #ifndef TEMPURATURE_FIFO_MAX_COUNT
        #define TEMPURATURE_FIFO_MAX_COUNT            0
    #endif
    #ifndef TEMPURATURE_FIFO_RESERVE_COUNT
        #define TEMPURATURE_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef TEMPURATURE_MAXDELAY
        #define TEMPURATURE_MAXDELAY                  1000000
    #endif
    #ifndef TEMPURATURE_FLAGS
        #define TEMPURATURE_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#if defined(CUSTOM_KERNEL_ALSPS) || defined(CUSTOM_KERNEL_PS)
    #ifndef PROXIMITY
        #define PROXIMITY                           "PROXIMITY"
    #endif
    #ifndef PROXIMITY_VENDER
        #define PROXIMITY_VENDER                    "MTK"
    #endif
    #ifndef PROXIMITY_VERSION
        #define PROXIMITY_VERSION                   1
    #endif
    #ifndef PROXIMITY_RANGE
        #define PROXIMITY_RANGE                     1.0f
    #endif
    #ifndef PROXIMITY_RESOLUTION
        #define PROXIMITY_RESOLUTION                1.0f
    #endif
    #ifndef PROXIMITY_POWER
        #define PROXIMITY_POWER                     0
    #endif
    #ifndef PROXIMITY_MINDELAY
        #define PROXIMITY_MINDELAY                  0
    #endif
    #ifndef PROXIMITY_FIFO_MAX_COUNT
        #define PROXIMITY_FIFO_MAX_COUNT            0
    #endif
    #ifndef PROXIMITY_FIFO_RESERVE_COUNT
        #define PROXIMITY_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef PROXIMITY_MAXDELAY
        #define PROXIMITY_MAXDELAY                  1000000
    #endif
    #ifndef PROXIMITY_FLAGS
        #define PROXIMITY_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_GRAVITY_SENSOR
    #ifndef GRAVITY
        #define GRAVITY                           "GRAVITY"
    #endif
    #ifndef GRAVITY_VENDER
        #define GRAVITY_VENDER                    "MTK"
    #endif
    #ifndef GRAVITY_VERSION
        #define GRAVITY_VERSION                   1
    #endif
    #ifndef GRAVITY_RANGE
        #define GRAVITY_RANGE                     39.2266f
    #endif
    #ifndef GRAVITY_RESOLUTION
        #define GRAVITY_RESOLUTION                0.0012f
    #endif
    #ifndef GRAVITY_POWER
        #define GRAVITY_POWER                     0
    #endif
    #ifndef GRAVITY_MINDELAY
        #define GRAVITY_MINDELAY                  10000
    #endif
    #ifndef GRAVITY_FIFO_MAX_COUNT
        #define GRAVITY_FIFO_MAX_COUNT            0
    #endif
    #ifndef GRAVITY_FIFO_RESERVE_COUNT
        #define GRAVITY_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef GRAVITY_MAXDELAY
        #define GRAVITY_MAXDELAY                  20000
    #endif
    #ifndef GRAVITY_FLAGS
        #define GRAVITY_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_LINEARACCEL_SENSOR
    #ifndef LINEARACCEL
        #define LINEARACCEL                           "LINEARACCEL"
    #endif
    #ifndef LINEARACCEL_VENDER
        #define LINEARACCEL_VENDER                    "MTK"
    #endif
    #ifndef LINEARACCEL_VERSION
        #define LINEARACCEL_VERSION                   1
    #endif
    #ifndef LINEARACCEL_RANGE
        #define LINEARACCEL_RANGE                     39.2266f
    #endif
    #ifndef LINEARACCEL_RESOLUTION
        #define LINEARACCEL_RESOLUTION                0.0012f
    #endif
    #ifndef LINEARACCEL_POWER
        #define LINEARACCEL_POWER                     0
    #endif
    #ifndef LINEARACCEL_MINDELAY
        #define LINEARACCEL_MINDELAY                  10000
    #endif
    #ifndef LINEARACCEL_FIFO_MAX_COUNT
        #define LINEARACCEL_FIFO_MAX_COUNT            0
    #endif
    #ifndef LINEARACCEL_FIFO_RESERVE_COUNT
        #define LINEARACCEL_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef LINEARACCEL_MAXDELAY
        #define LINEARACCEL_MAXDELAY                  20000
    #endif
    #ifndef LINEARACCEL_FLAGS
        #define LINEARACCEL_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_RV_SENSOR
    #ifndef ROTATION_VECTOR
        #define ROTATION_VECTOR                           "ROTATION_VECTOR"
    #endif
    #ifndef ROTATION_VECTOR_VENDER
        #define ROTATION_VECTOR_VENDER                    "MTK"
    #endif
    #ifndef ROTATION_VECTOR_VERSION
        #define ROTATION_VECTOR_VERSION                   1
    #endif
    #ifndef ROTATION_VECTOR_RANGE
        #define ROTATION_VECTOR_RANGE                     1.0f
    #endif
    #ifndef ROTATION_VECTOR_RESOLUTION
        #define ROTATION_VECTOR_RESOLUTION                1.0f / (1<<24)
    #endif
    #ifndef ROTATION_VECTOR_POWER
        #define ROTATION_VECTOR_POWER                     0
    #endif
    #ifndef ROTATION_VECTOR_MINDELAY
        #define ROTATION_VECTOR_MINDELAY                  20000
    #endif
    #ifndef ROTATION_VECTOR_FIFO_MAX_COUNT
        #define ROTATION_VECTOR_FIFO_MAX_COUNT            0
    #endif
    #ifndef ROTATION_VECTOR_FIFO_RESERVE_COUNT
        #define ROTATION_VECTOR_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef ROTATION_VECTOR_MAXDELAY
        #define ROTATION_VECTOR_MAXDELAY                  20000
    #endif
    #ifndef ROTATION_VECTOR_FLAGS
        #define ROTATION_VECTOR_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_HUMIDITY
    #ifndef HUMIDITY
        #define HUMIDITY                           "HUMIDITY"
    #endif
    #ifndef HUMIDITY_VENDER
        #define HUMIDITY_VENDER                    "MTK"
    #endif
    #ifndef HUMIDITY_VERSION
        #define HUMIDITY_VERSION                   1
    #endif
    #ifndef HUMIDITY_RANGE
        #define HUMIDITY_RANGE                     85.0f
    #endif
    #ifndef HUMIDITY_RESOLUTION
        #define HUMIDITY_RESOLUTION                0.1f
    #endif
    #ifndef HUMIDITY_POWER
        #define HUMIDITY_POWER                     0
    #endif
    #ifndef HUMIDITY_MINDELAY
        #define HUMIDITY_MINDELAY                  0
    #endif
    #ifndef HUMIDITY_FIFO_MAX_COUNT
        #define HUMIDITY_FIFO_MAX_COUNT            0
    #endif
    #ifndef HUMIDITY_FIFO_RESERVE_COUNT
        #define HUMIDITY_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef HUMIDITY_MAXDELAY
        #define HUMIDITY_MAXDELAY                  1000000
    #endif
    #ifndef HUMIDITY_FLAGS
        #define HUMIDITY_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_AMBIENT_TEMPERATURE
    #ifndef AMBIENT_TEMPERATURE
        #define AMBIENT_TEMPERATURE                           "AMBIENT_TEMPERATURE"
    #endif
    #ifndef AMBIENT_TEMPERATURE_VENDER
        #define AMBIENT_TEMPERATURE_VENDER                    "MTK"
    #endif
    #ifndef AMBIENT_TEMPERATURE_VERSION
        #define AMBIENT_TEMPERATURE_VERSION                   1
    #endif
    #ifndef AMBIENT_TEMPERATURE_RANGE
        #define AMBIENT_TEMPERATURE_RANGE                     85.0f
    #endif
    #ifndef AMBIENT_TEMPERATURE_RESOLUTION
        #define AMBIENT_TEMPERATURE_RESOLUTION                0.01f
    #endif
    #ifndef AMBIENT_TEMPERATURE_POWER
        #define AMBIENT_TEMPERATURE_POWER                     0
    #endif
    #ifndef AMBIENT_TEMPERATURE_MINDELAY
        #define AMBIENT_TEMPERATURE_MINDELAY                  0
    #endif
    #ifndef AMBIENT_TEMPERATURE_FIFO_MAX_COUNT
        #define AMBIENT_TEMPERATURE_FIFO_MAX_COUNT            0
    #endif
    #ifndef AMBIENT_TEMPERATURE_FIFO_RESERVE_COUNT
        #define AMBIENT_TEMPERATURE_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef AMBIENT_TEMPERATURE_MAXDELAY
        #define AMBIENT_TEMPERATURE_MAXDELAY                  1000000
    #endif
    #ifndef AMBIENT_TEMPERATURE_FLAGS
        #define AMBIENT_TEMPERATURE_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_UNCALI_MAG_SENSOR
    #ifndef UNCALI_MAG
        #define UNCALI_MAG                           "UNCALI_MAG"
    #endif
    #ifndef UNCALI_MAG_VENDER
        #define UNCALI_MAG_VENDER                    "MTK"
    #endif
    #ifndef UNCALI_MAG_VERSION
        #define UNCALI_MAG_VERSION                   1
    #endif
    #ifndef UNCALI_MAG_RANGE
        #define UNCALI_MAG_RANGE                     4912.0f
    #endif
    #ifndef UNCALI_MAG_RESOLUTION
        #define UNCALI_MAG_RESOLUTION                0.15f
    #endif
    #ifndef UNCALI_MAG_POWER
        #define UNCALI_MAG_POWER                     0
    #endif
    #ifndef UNCALI_MAG_MINDELAY
        #define UNCALI_MAG_MINDELAY                  20000
    #endif
    #ifndef UNCALI_MAG_FIFO_MAX_COUNT
        #define UNCALI_MAG_FIFO_MAX_COUNT            0
    #endif
    #ifndef UNCALI_MAG_FIFO_RESERVE_COUNT
        #define UNCALI_MAG_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef UNCALI_MAG_MAXDELAY
        #define UNCALI_MAG_MAXDELAY                  1000000
    #endif
    #ifndef UNCALI_MAG_FLAGS
        #define UNCALI_MAG_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_GRV_SENSOR
    #ifndef GAME_ROTATION_VECTOR
        #define GAME_ROTATION_VECTOR                           "GAME_ROTATION_VECTOR"
    #endif
    #ifndef GAME_ROTATION_VECTOR_VENDER
        #define GAME_ROTATION_VECTOR_VENDER                    "MTK"
    #endif
    #ifndef GAME_ROTATION_VECTOR_VERSION
        #define GAME_ROTATION_VECTOR_VERSION                   1
    #endif
    #ifndef GAME_ROTATION_VECTOR_RANGE
        #define GAME_ROTATION_VECTOR_RANGE                     1.0f
    #endif
    #ifndef GAME_ROTATION_VECTOR_RESOLUTION
        #define GAME_ROTATION_VECTOR_RESOLUTION                1.0f / (1<<24)
    #endif
    #ifndef GAME_ROTATION_VECTOR_POWER
        #define GAME_ROTATION_VECTOR_POWER                     0
    #endif
    #ifndef GAME_ROTATION_VECTOR_MINDELAY
        #define GAME_ROTATION_VECTOR_MINDELAY                  20000
    #endif
    #ifndef GAME_ROTATION_VECTOR_FIFO_MAX_COUNT
        #define GAME_ROTATION_VECTOR_FIFO_MAX_COUNT            0
    #endif
    #ifndef GAME_ROTATION_VECTOR_FIFO_RESERVE_COUNT
        #define GAME_ROTATION_VECTOR_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef GAME_ROTATION_VECTOR_MAXDELAY
        #define GAME_ROTATION_VECTOR_MAXDELAY                  20000
    #endif
    #ifndef GAME_ROTATION_VECTOR_FLAGS
        #define GAME_ROTATION_VECTOR_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_UNCALI_GYRO_SENSOR
    #ifndef UNCALI_GYRO
        #define UNCALI_GYRO                           "UNCALI_GYRO"
    #endif
    #ifndef UNCALI_GYRO_VENDER
        #define UNCALI_GYRO_VENDER                    "MTK"
    #endif
    #ifndef UNCALI_GYRO_VERSION
        #define UNCALI_GYRO_VERSION                   1
    #endif
    #ifndef UNCALI_GYRO_RANGE
        #define UNCALI_GYRO_RANGE                     34.9066f
    #endif
    #ifndef UNCALI_GYRO_RESOLUTION
        #define UNCALI_GYRO_RESOLUTION                0.0011f
    #endif
    #ifndef UNCALI_GYRO_POWER
        #define UNCALI_GYRO_POWER                     0
    #endif
    #ifndef UNCALI_GYRO_MINDELAY
        #define UNCALI_GYRO_MINDELAY                  5000
    #endif
    #ifndef UNCALI_GYRO_FIFO_MAX_COUNT
        #define UNCALI_GYRO_FIFO_MAX_COUNT            0
    #endif
    #ifndef UNCALI_GYRO_FIFO_RESERVE_COUNT
        #define UNCALI_GYRO_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef UNCALI_GYRO_MAXDELAY
        #define UNCALI_GYRO_MAXDELAY                  1000000
    #endif
    #ifndef UNCALI_GYRO_FLAGS
        #define UNCALI_GYRO_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_GYRO_TEMPERATURE_SENSOR
    #ifndef GYRO_TEMPERATURE
        #define GYRO_TEMPERATURE                           "GYRO_TEMPERATURE"
    #endif
    #ifndef GYRO_TEMPERATURE_VENDER
        #define GYRO_TEMPERATURE_VENDER                    "MTK"
    #endif
    #ifndef GYRO_TEMPERATURE_VERSION
        #define GYRO_TEMPERATURE_VERSION                   1
    #endif
    #ifndef GYRO_TEMPERATURE_RANGE
        #define GYRO_TEMPERATURE_RANGE                     65535.0f
    #endif
    #ifndef GYRO_TEMPERATURE_RESOLUTION
        #define GYRO_TEMPERATURE_RESOLUTION                1.0f
    #endif
    #ifndef GYRO_TEMPERATURE_POWER
        #define GYRO_TEMPERATURE_POWER                     0
    #endif
    #ifndef GYRO_TEMPERATURE_MINDELAY
        #define GYRO_TEMPERATURE_MINDELAY                  0
    #endif
    #ifndef GYRO_TEMPERATURE_FIFO_MAX_COUNT
        #define GYRO_TEMPERATURE_FIFO_MAX_COUNT            0
    #endif
    #ifndef GYRO_TEMPERATURE_FIFO_RESERVE_COUNT
        #define GYRO_TEMPERATURE_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef GYRO_TEMPERATURE_MAXDELAY
        #define GYRO_TEMPERATURE_MAXDELAY                  1000000
    #endif
    #ifndef GYRO_TEMPERATURE_FLAGS
        #define GYRO_TEMPERATURE_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_SIGNIFICANT_MOTION_SENSOR
    #ifndef SIGNIFICANT_MOTION
        #define SIGNIFICANT_MOTION                           "SIGNIFICANT_MOTION"
    #endif
    #ifndef SIGNIFICANT_MOTION_VENDER
        #define SIGNIFICANT_MOTION_VENDER                    "MTK"
    #endif
    #ifndef SIGNIFICANT_MOTION_VERSION
        #define SIGNIFICANT_MOTION_VERSION                   1
    #endif
    #ifndef SIGNIFICANT_MOTION_RANGE
        #define SIGNIFICANT_MOTION_RANGE                     1.0f
    #endif
    #ifndef SIGNIFICANT_MOTION_RESOLUTION
        #define SIGNIFICANT_MOTION_RESOLUTION                1.0f
    #endif
    #ifndef SIGNIFICANT_MOTION_POWER
        #define SIGNIFICANT_MOTION_POWER                     0
    #endif
    #ifndef SIGNIFICANT_MOTION_MINDELAY
        #define SIGNIFICANT_MOTION_MINDELAY                  -1
    #endif
    #ifndef SIGNIFICANT_MOTION_FIFO_MAX_COUNT
        #define SIGNIFICANT_MOTION_FIFO_MAX_COUNT            0
    #endif
    #ifndef SIGNIFICANT_MOTION_FIFO_RESERVE_COUNT
        #define SIGNIFICANT_MOTION_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef SIGNIFICANT_MOTION_MAXDELAY
        #define SIGNIFICANT_MOTION_MAXDELAY                  0
    #endif
    #ifndef SIGNIFICANT_MOTION_FLAGS
        #define SIGNIFICANT_MOTION_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_STEP_COUNTER
    #ifndef STEP_DETECTOR
        #define STEP_DETECTOR                           "STEP_DETECTOR"
    #endif
    #ifndef STEP_DETECTOR_WAKEUP
        #define STEP_DETECTOR_WAKEUP                    "STEP_DETECTOR_WAKEUP"
    #endif
    #ifndef STEP_DETECTOR_VENDER
        #define STEP_DETECTOR_VENDER                    "MTK"
    #endif
    #ifndef STEP_DETECTOR_VERSION
        #define STEP_DETECTOR_VERSION                   1
    #endif
    #ifndef STEP_DETECTOR_RANGE
        #define STEP_DETECTOR_RANGE                     1.0f
    #endif
    #ifndef STEP_DETECTOR_RESOLUTION
        #define STEP_DETECTOR_RESOLUTION                1.0f
    #endif
    #ifndef STEP_DETECTOR_POWER
        #define STEP_DETECTOR_POWER                     0
    #endif
    #ifndef STEP_DETECTOR_MINDELAY
        #define STEP_DETECTOR_MINDELAY                  0
    #endif
    #ifndef STEP_DETECTOR_FIFO_MAX_COUNT
        #define STEP_DETECTOR_FIFO_MAX_COUNT            0
    #endif
    #ifndef STEP_DETECTOR_FIFO_RESERVE_COUNT
        #define STEP_DETECTOR_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef STEP_DETECTOR_MAXDELAY
        #define STEP_DETECTOR_MAXDELAY                  0
    #endif
    #ifndef STEP_DETECTOR_FLAGS
        #define STEP_DETECTOR_FLAGS     SENSOR_FLAG_SPECIAL_REPORTING_MODE
    #endif
    #ifndef STEP_DETECTOR_WAKEUP_FLAGS
        #define STEP_DETECTOR_WAKEUP_FLAGS SENSOR_FLAG_SPECIAL_REPORTING_MODE | SENSOR_FLAG_WAKE_UP
    #endif

    #ifndef STEP_COUNTER
        #define STEP_COUNTER                           "STEP_COUNTER"
    #endif
    #ifndef STEP_COUNTER_VENDER
        #define STEP_COUNTER_VENDER                    "MTK"
    #endif
    #ifndef STEP_COUNTER_VERSION
        #define STEP_COUNTER_VERSION                   1
    #endif
    #ifndef STEP_COUNTER_RANGE
        #define STEP_COUNTER_RANGE                     2147483647.0f
    #endif
    #ifndef STEP_COUNTER_RESOLUTION
        #define STEP_COUNTER_RESOLUTION                1.0f
    #endif
    #ifndef STEP_COUNTER_POWER
        #define STEP_COUNTER_POWER                     0
    #endif
    #ifndef STEP_COUNTER_MINDELAY
        #define STEP_COUNTER_MINDELAY                  0
    #endif
    #ifndef STEP_COUNTER_FIFO_MAX_COUNT
        #define STEP_COUNTER_FIFO_MAX_COUNT            0
    #endif
    #ifndef STEP_COUNTER_FIFO_RESERVE_COUNT
        #define STEP_COUNTER_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef STEP_COUNTER_MAXDELAY
        #define STEP_COUNTER_MAXDELAY                  1000000
    #endif
    #ifndef STEP_COUNTER_FLAGS
        #define STEP_COUNTER_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif

    #ifndef FLOOR_COUNTER
        #define FLOOR_COUNTER                           "FLOOR_COUNTER"
    #endif
    #ifndef FLOOR_COUNTER_VENDER
        #define FLOOR_COUNTER_VENDER                    "MTK"
    #endif
    #ifndef FLOOR_COUNTER_VERSION
        #define FLOOR_COUNTER_VERSION                   1
    #endif
    #ifndef FLOOR_COUNTER_RANGE
        #define FLOOR_COUNTER_RANGE                     2147483647.0f
    #endif
    #ifndef FLOOR_COUNTER_RESOLUTION
        #define FLOOR_COUNTER_RESOLUTION                1.0f
    #endif
    #ifndef FLOOR_COUNTER_POWER
        #define FLOOR_COUNTER_POWER                     0
    #endif
    #ifndef FLOOR_COUNTER_MINDELAY
        #define FLOOR_COUNTER_MINDELAY                  0
    #endif
    #ifndef FLOOR_COUNTER_FIFO_MAX_COUNT
        #define FLOOR_COUNTER_FIFO_MAX_COUNT            0
    #endif
    #ifndef FLOOR_COUNTER_FIFO_RESERVE_COUNT
        #define FLOOR_COUNTER_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef FLOOR_COUNTER_MAXDELAY
        #define FLOOR_COUNTER_MAXDELAY                  1000000
    #endif
    #ifndef FLOOR_COUNTER_FLAGS
        #define FLOOR_COUNTER_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif

#endif

#ifdef CUSTOM_KERNEL_GMRV_SENSOR
    #ifndef GEOMAGNETIC_ROTATION_VECTOR
        #define GEOMAGNETIC_ROTATION_VECTOR                           "GEOMAGNETIC_ROTATION_VECTOR"
    #endif
    #ifndef GEOMAGNETIC_ROTATION_VECTOR_VENDER
        #define GEOMAGNETIC_ROTATION_VECTOR_VENDER                    "MTK"
    #endif
    #ifndef GEOMAGNETIC_ROTATION_VECTOR_VERSION
        #define GEOMAGNETIC_ROTATION_VECTOR_VERSION                   1
    #endif
    #ifndef GEOMAGNETIC_ROTATION_VECTOR_RANGE
        #define GEOMAGNETIC_ROTATION_VECTOR_RANGE                     1.0f
    #endif
    #ifndef GEOMAGNETIC_ROTATION_VECTOR_RESOLUTION
        #define GEOMAGNETIC_ROTATION_VECTOR_RESOLUTION                1.0f / (1<<24)
    #endif
    #ifndef GEOMAGNETIC_ROTATION_VECTOR_POWER
        #define GEOMAGNETIC_ROTATION_VECTOR_POWER                     0
    #endif
    #ifndef GEOMAGNETIC_ROTATION_VECTOR_MINDELAY
        #define GEOMAGNETIC_ROTATION_VECTOR_MINDELAY                  20000
    #endif
    #ifndef GEOMAGNETIC_ROTATION_VECTOR_FIFO_MAX_COUNT
        #define GEOMAGNETIC_ROTATION_VECTOR_FIFO_MAX_COUNT            0
    #endif
    #ifndef GEOMAGNETIC_ROTATION_VECTOR_FIFO_RESERVE_COUNT
        #define GEOMAGNETIC_ROTATION_VECTOR_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef GEOMAGNETIC_ROTATION_VECTOR_MAXDELAY
        #define GEOMAGNETIC_ROTATION_VECTOR_MAXDELAY                  1000000
    #endif
    #ifndef GEOMAGNETIC_ROTATION_VECTOR_FLAGS
        #define GEOMAGNETIC_ROTATION_VECTOR_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_HEART
    #ifndef HEART_RATE
        #define HEART_RATE                           "HEART_RATE"
    #endif
    #ifndef HEART_RATE_VENDER
        #define HEART_RATE_VENDER                    "MTK"
    #endif
    #ifndef HEART_RATE_VERSION
        #define HEART_RATE_VERSION                   1
    #endif
    #ifndef HEART_RATE_RANGE
        #define HEART_RATE_RANGE                     1000.0f
    #endif
    #ifndef HEART_RATE_RESOLUTION
        #define HEART_RATE_RESOLUTION                1.0f
    #endif
    #ifndef HEART_RATE_POWER
        #define HEART_RATE_POWER                     0
    #endif
    #ifndef HEART_RATE_MINDELAY
        #define HEART_RATE_MINDELAY                  0
    #endif
    #ifndef HEART_RATE_FIFO_MAX_COUNT
        #define HEART_RATE_FIFO_MAX_COUNT            0
    #endif
    #ifndef HEART_RATE_FIFO_RESERVE_COUNT
        #define HEART_RATE_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef HEART_RATE_MAXDELAY
        #define HEART_RATE_MAXDELAY                  1000000
    #endif
    #ifndef HEART_RATE_FLAGS
        #define HEART_RATE_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_TILT_DETECTOR_SENSOR
    #ifndef TILT_DETECTOR
        #define TILT_DETECTOR                           "TILT_DETECTOR"
    #endif
    #ifndef TILT_DETECTOR_VENDER
        #define TILT_DETECTOR_VENDER                    "MTK"
    #endif
    #ifndef TILT_DETECTOR_VERSION
        #define TILT_DETECTOR_VERSION                   1
    #endif
    #ifndef TILT_DETECTOR_RANGE
        #define TILT_DETECTOR_RANGE                     1.0f
    #endif
    #ifndef TILT_DETECTOR_RESOLUTION
        #define TILT_DETECTOR_RESOLUTION                1.0f
    #endif
    #ifndef TILT_DETECTOR_POWER
        #define TILT_DETECTOR_POWER                     0
    #endif
    #ifndef TILT_DETECTOR_MINDELAY
        #define TILT_DETECTOR_MINDELAY                  0
    #endif
    #ifndef TILT_DETECTOR_FIFO_MAX_COUNT
        #define TILT_DETECTOR_FIFO_MAX_COUNT            0
    #endif
    #ifndef TILT_DETECTOR_FIFO_RESERVE_COUNT
        #define TILT_DETECTOR_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef TILT_DETECTOR_MAXDELAY
        #define TILT_DETECTOR_MAXDELAY                  0
    #endif
    #ifndef TILT_DETECTOR_FLAGS
        #define TILT_DETECTOR_FLAGS     SENSOR_FLAG_SPECIAL_REPORTING_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_WAKE_GESTURE_SENSOR
    #ifndef WAKE_GESTURE
        #define WAKE_GESTURE                           "WAKE_GESTURE"
    #endif
    #ifndef WAKE_GESTURE_VENDER
        #define WAKE_GESTURE_VENDER                    "MTK"
    #endif
    #ifndef WAKE_GESTURE_VERSION
        #define WAKE_GESTURE_VERSION                   1
    #endif
    #ifndef WAKE_GESTURE_RANGE
        #define WAKE_GESTURE_RANGE                     1.0f
    #endif
    #ifndef WAKE_GESTURE_RESOLUTION
        #define WAKE_GESTURE_RESOLUTION                1.0f
    #endif
    #ifndef WAKE_GESTURE_POWER
        #define WAKE_GESTURE_POWER                     0
    #endif
    #ifndef WAKE_GESTURE_MINDELAY
        #define WAKE_GESTURE_MINDELAY                  -1
    #endif
    #ifndef WAKE_GESTURE_FIFO_MAX_COUNT
        #define WAKE_GESTURE_FIFO_MAX_COUNT            0
    #endif
    #ifndef WAKE_GESTURE_FIFO_RESERVE_COUNT
        #define WAKE_GESTURE_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef WAKE_GESTURE_MAXDELAY
        #define WAKE_GESTURE_MAXDELAY                  0
    #endif
    #ifndef WAKE_GESTURE_FLAGS
        #define WAKE_GESTURE_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_GLANCE_GESTURE_SENSOR
    #ifndef GLANCE_GESTURE
        #define GLANCE_GESTURE                           "GLANCE_GESTURE"
    #endif
    #ifndef GLANCE_GESTURE_VENDER
        #define GLANCE_GESTURE_VENDER                    "MTK"
    #endif
    #ifndef GLANCE_GESTURE_VERSION
        #define GLANCE_GESTURE_VERSION                   1
    #endif
    #ifndef GLANCE_GESTURE_RANGE
        #define GLANCE_GESTURE_RANGE                     1.0f
    #endif
    #ifndef GLANCE_GESTURE_RESOLUTION
        #define GLANCE_GESTURE_RESOLUTION                1.0f
    #endif
    #ifndef GLANCE_GESTURE_POWER
        #define GLANCE_GESTURE_POWER                     0
    #endif
    #ifndef GLANCE_GESTURE_MINDELAY
        #define GLANCE_GESTURE_MINDELAY                  -1
    #endif
    #ifndef GLANCE_GESTURE_FIFO_MAX_COUNT
        #define GLANCE_GESTURE_FIFO_MAX_COUNT            0
    #endif
    #ifndef GLANCE_GESTURE_FIFO_RESERVE_COUNT
        #define GLANCE_GESTURE_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef GLANCE_GESTURE_MAXDELAY
        #define GLANCE_GESTURE_MAXDELAY                  0
    #endif
    #ifndef GLANCE_GESTURE_FLAGS
        #define GLANCE_GESTURE_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_PICK_UP_SENSOR
    #ifndef PICK_UP
        #define PICK_UP                           "PICK_UP"
    #endif
    #ifndef PICK_UP_VENDER
        #define PICK_UP_VENDER                    "MTK"
    #endif
    #ifndef PICK_UP_VERSION
        #define PICK_UP_VERSION                   1
    #endif
    #ifndef PICK_UP_RANGE
        #define PICK_UP_RANGE                     1.0f
    #endif
    #ifndef PICK_UP_RESOLUTION
        #define PICK_UP_RESOLUTION                1.0f
    #endif
    #ifndef PICK_UP_POWER
        #define PICK_UP_POWER                     0
    #endif
    #ifndef PICK_UP_MINDELAY
        #define PICK_UP_MINDELAY                  -1
    #endif
    #ifndef PICK_UP_FIFO_MAX_COUNT
        #define PICK_UP_FIFO_MAX_COUNT            0
    #endif
    #ifndef PICK_UP_FIFO_RESERVE_COUNT
        #define PICK_UP_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef PICK_UP_MAXDELAY
        #define PICK_UP_MAXDELAY                  0
    #endif
    #ifndef PICK_UP_FLAGS
        #define PICK_UP_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_WRIST_TILT
    #ifndef WRIST_TILT
        #define WRIST_TILT                           "WRIST_TILT"
    #endif
    #ifndef WRIST_TILT_VENDER
        #define WRIST_TILT_VENDER                    "MTK"
    #endif
    #ifndef WRIST_TILT_VERSION
        #define WRIST_TILT_VERSION                   1
    #endif
    #ifndef WRIST_TILT_RANGE
        #define WRIST_TILT_RANGE                     1.0f
    #endif
    #ifndef WRIST_TILT_RESOLUTION
        #define WRIST_TILT_RESOLUTION                1.0f
    #endif
    #ifndef WRIST_TILT_POWER
        #define WRIST_TILT_POWER                     0
    #endif
    #ifndef WRIST_TILT_MINDELAY
        #define WRIST_TILT_MINDELAY                  0
    #endif
    #ifndef WRIST_TILT_FIFO_MAX_COUNT
        #define WRIST_TILT_FIFO_MAX_COUNT            0
    #endif
    #ifndef WRIST_TILT_FIFO_RESERVE_COUNT
        #define WRIST_TILT_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef WRIST_TILT_MAXDELAY
        #define WRIST_TILT_MAXDELAY                  0
    #endif
    #ifndef WRIST_TILT_FLAGS
        #define WRIST_TILT_FLAGS     SENSOR_FLAG_SPECIAL_REPORTING_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_DEVICE_ORIENTATION
    #ifndef DEVICE_ORIENTATION
        #define DEVICE_ORIENTATION                           "DEVICE_ORIENTATION"
    #endif
    #ifndef DEVICE_ORIENTATION_VENDER
        #define DEVICE_ORIENTATION_VENDER                    "MTK"
    #endif
    #ifndef DEVICE_ORIENTATION_VERSION
        #define DEVICE_ORIENTATION_VERSION                   1
    #endif
    #ifndef DEVICE_ORIENTATION_RANGE
        #define DEVICE_ORIENTATION_RANGE                     3
    #endif
    #ifndef DEVICE_ORIENTATION_RESOLUTION
        #define DEVICE_ORIENTATION_RESOLUTION                1
    #endif
    #ifndef DEVICE_ORIENTATION_POWER
        #define DEVICE_ORIENTATION_POWER                     0
    #endif
    #ifndef DEVICE_ORIENTATION_MINDELAY
        #define DEVICE_ORIENTATION_MINDELAY                  0
    #endif
    #ifndef DEVICE_ORIENTATION_FIFO_MAX_COUNT
        #define DEVICE_ORIENTATION_FIFO_MAX_COUNT            0
    #endif
    #ifndef DEVICE_ORIENTATION_FIFO_RESERVE_COUNT
        #define DEVICE_ORIENTATION_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef DEVICE_ORIENTATION_MAXDELAY
        #define DEVICE_ORIENTATION_MAXDELAY                  1000000
    #endif
    #ifndef DEVICE_ORIENTATION_FLAGS
        #define DEVICE_ORIENTATION_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_POSE_6DOF
    #ifndef POSE_6DOF
        #define POSE_6DOF                           "POSE_6DOF"
    #endif
    #ifndef POSE_6DOF_VENDER
        #define POSE_6DOF_VENDER                    "MTK"
    #endif
    #ifndef POSE_6DOF_VERSION
        #define POSE_6DOF_VERSION                   1
    #endif
    #ifndef POSE_6DOF_RANGE
        #define POSE_6DOF_RANGE                     1.0f
    #endif
    #ifndef POSE_6DOF_RESOLUTION
        #define POSE_6DOF_RESOLUTION                1.0f
    #endif
    #ifndef POSE_6DOF_POWER
        #define POSE_6DOF_POWER                     0
    #endif
    #ifndef POSE_6DOF_MINDELAY
        #define POSE_6DOF_MINDELAY                  10000
    #endif
    #ifndef POSE_6DOF_FIFO_MAX_COUNT
        #define POSE_6DOF_FIFO_MAX_COUNT            0
    #endif
    #ifndef POSE_6DOF_FIFO_RESERVE_COUNT
        #define POSE_6DOF_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef POSE_6DOF_MAXDELAY
        #define POSE_6DOF_MAXDELAY                  1000000
    #endif
    #ifndef POSE_6DOF_FLAGS
        #define POSE_6DOF_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_STATIONARY_SENSOR
    #ifndef STATIONARY_DETECT
        #define STATIONARY_DETECT                           "STATIONARY_DETECT"
    #endif
    #ifndef STATIONARY_DETECT_VENDER
        #define STATIONARY_DETECT_VENDER                    "MTK"
    #endif
    #ifndef STATIONARY_DETECT_VERSION
        #define STATIONARY_DETECT_VERSION                   1
    #endif
    #ifndef STATIONARY_DETECT_RANGE
        #define STATIONARY_DETECT_RANGE                     1.0f
    #endif
    #ifndef STATIONARY_DETECT_RESOLUTION
        #define STATIONARY_DETECT_RESOLUTION                1.0f
    #endif
    #ifndef STATIONARY_DETECT_POWER
        #define STATIONARY_DETECT_POWER                     0
    #endif
    #ifndef STATIONARY_DETECT_MINDELAY
        #define STATIONARY_DETECT_MINDELAY                  -1
    #endif
    #ifndef STATIONARY_DETECT_FIFO_MAX_COUNT
        #define STATIONARY_DETECT_FIFO_MAX_COUNT            0
    #endif
    #ifndef STATIONARY_DETECT_FIFO_RESERVE_COUNT
        #define STATIONARY_DETECT_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef STATIONARY_DETECT_MAXDELAY
        #define STATIONARY_DETECT_MAXDELAY                  0
    #endif
    #ifndef STATIONARY_DETECT_FLAGS
        #define STATIONARY_DETECT_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_MOTION_DETECT
    #ifndef MOTION_DETECT
        #define MOTION_DETECT                           "MOTION_DETECT"
    #endif
    #ifndef MOTION_DETECT_VENDER
        #define MOTION_DETECT_VENDER                    "MTK"
    #endif
    #ifndef MOTION_DETECT_VERSION
        #define MOTION_DETECT_VERSION                   1
    #endif
    #ifndef MOTION_DETECT_RANGE
        #define MOTION_DETECT_RANGE                     1.0f
    #endif
    #ifndef MOTION_DETECT_RESOLUTION
        #define MOTION_DETECT_RESOLUTION                1.0f
    #endif
    #ifndef MOTION_DETECT_POWER
        #define MOTION_DETECT_POWER                     0
    #endif
    #ifndef MOTION_DETECT_MINDELAY
        #define MOTION_DETECT_MINDELAY                  -1
    #endif
    #ifndef MOTION_DETECT_FIFO_MAX_COUNT
        #define MOTION_DETECT_FIFO_MAX_COUNT            0
    #endif
    #ifndef MOTION_DETECT_FIFO_RESERVE_COUNT
        #define MOTION_DETECT_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef MOTION_DETECT_MAXDELAY
        #define MOTION_DETECT_MAXDELAY                  0
    #endif
    #ifndef MOTION_DETECT_FLAGS
        #define MOTION_DETECT_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_HEART_BEAT
    #ifndef HEART_BEAT
        #define HEART_BEAT                           "HEART_BEAT"
    #endif
    #ifndef HEART_BEAT_VENDER
        #define HEART_BEAT_VENDER                    "MTK"
    #endif
    #ifndef HEART_BEAT_VERSION
        #define HEART_BEAT_VERSION                   1
    #endif
    #ifndef HEART_BEAT_RANGE
        #define HEART_BEAT_RANGE                     1.0f
    #endif
    #ifndef HEART_BEAT_RESOLUTION
        #define HEART_BEAT_RESOLUTION                0.01f
    #endif
    #ifndef HEART_BEAT_POWER
        #define HEART_BEAT_POWER                     0
    #endif
    #ifndef HEART_BEAT_MINDELAY
        #define HEART_BEAT_MINDELAY                  0
    #endif
    #ifndef HEART_BEAT_FIFO_MAX_COUNT
        #define HEART_BEAT_FIFO_MAX_COUNT            0
    #endif
    #ifndef HEART_BEAT_FIFO_RESERVE_COUNT
        #define HEART_BEAT_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef HEART_BEAT_MAXDELAY
        #define HEART_BEAT_MAXDELAY                  0
    #endif
    #ifndef HEART_BEAT_FLAGS
        #define HEART_BEAT_FLAGS     SENSOR_FLAG_SPECIAL_REPORTING_MODE
    #endif
#endif

/* #ifdef CUSTOM_KERNEL_SENSOR_META
    #ifndef SENSOR_META
        #define SENSOR_META                           "SENSOR_META"
    #endif
    #ifndef SENSOR_META_VENDER
        #define SENSOR_META_VENDER                    "MTK"
    #endif
    #ifndef SENSOR_META_VERSION
        #define SENSOR_META_VERSION                   1
    #endif
    #ifndef SENSOR_META_RANGE
        #define SENSOR_META_RANGE                     0.0f
    #endif
    #ifndef SENSOR_META_RESOLUTION
        #define SENSOR_META_RESOLUTION                0.0f
    #endif
    #ifndef SENSOR_META_POWER
        #define SENSOR_META_POWER                     0
    #endif
    #ifndef SENSOR_META_MINDELAY
        #define SENSOR_META_MINDELAY                  10000
    #endif
    #ifndef SENSOR_META_FIFO_MAX_COUNT
        #define SENSOR_META_FIFO_MAX_COUNT            0
    #endif
    #ifndef SENSOR_META_FIFO_RESERVE_COUNT
        #define SENSOR_META_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef SENSOR_META_MAXDELAY
        #define SENSOR_META_MAXDELAY                  1000000
    #endif
    #ifndef SENSOR_META_FLAGS
        #define SENSOR_META_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_ADDITIONAL_INFO
    #ifndef ADDITIONAL_INFO
        #define ADDITIONAL_INFO                           "ADDITIONAL_INFO"
    #endif
    #ifndef ADDITIONAL_INFO_VENDER
        #define ADDITIONAL_INFO_VENDER                    "MTK"
    #endif
    #ifndef ADDITIONAL_INFO_VERSION
        #define ADDITIONAL_INFO_VERSION                   1
    #endif
    #ifndef ADDITIONAL_INFO_RANGE
        #define ADDITIONAL_INFO_RANGE                     0.0f
    #endif
    #ifndef ADDITIONAL_INFO_RESOLUTION
        #define ADDITIONAL_INFO_RESOLUTION                0.0f
    #endif
    #ifndef ADDITIONAL_INFO_POWER
        #define ADDITIONAL_INFO_POWER                     0
    #endif
    #ifndef ADDITIONAL_INFO_MINDELAY
        #define ADDITIONAL_INFO_MINDELAY                  10000
    #endif
    #ifndef ADDITIONAL_INFO_FIFO_MAX_COUNT
        #define ADDITIONAL_INFO_FIFO_MAX_COUNT            0
    #endif
    #ifndef ADDITIONAL_INFO_FIFO_RESERVE_COUNT
        #define ADDITIONAL_INFO_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef ADDITIONAL_INFO_MAXDELAY
        #define ADDITIONAL_INFO_MAXDELAY                  1000000
    #endif
    #ifndef ADDITIONAL_INFO_FLAGS
        #define ADDITIONAL_INFO_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif */

#ifdef CUSTOM_KERNEL_PEDOMETER
    #ifndef PEDOMETER
        #define PEDOMETER                           "PEDOMETER"
    #endif
    #ifndef PEDOMETER_VENDER
        #define PEDOMETER_VENDER                    "MTK"
    #endif
    #ifndef PEDOMETER_VERSION
        #define PEDOMETER_VERSION                   1
    #endif
    #ifndef PEDOMETER_RANGE
        #define PEDOMETER_RANGE                     2147483647.0f
    #endif
    #ifndef PEDOMETER_RESOLUTION
        #define PEDOMETER_RESOLUTION                0.1f
    #endif
    #ifndef PEDOMETER_POWER
        #define PEDOMETER_POWER                     0
    #endif
    #ifndef PEDOMETER_MINDELAY
        #define PEDOMETER_MINDELAY                  0
    #endif
    #ifndef PEDOMETER_FIFO_MAX_COUNT
        #define PEDOMETER_FIFO_MAX_COUNT            0
    #endif
    #ifndef PEDOMETER_FIFO_RESERVE_COUNT
        #define PEDOMETER_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef PEDOMETER_MAXDELAY
        #define PEDOMETER_MAXDELAY                  1000000
    #endif
    #ifndef PEDOMETER_FLAGS
        #define PEDOMETER_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_IN_POCKET_SENSOR
    #ifndef IN_POCKET
        #define IN_POCKET                           "IN_POCKET"
    #endif
    #ifndef IN_POCKET_VENDER
        #define IN_POCKET_VENDER                    "MTK"
    #endif
    #ifndef IN_POCKET_VERSION
        #define IN_POCKET_VERSION                   1
    #endif
    #ifndef IN_POCKET_RANGE
        #define IN_POCKET_RANGE                     1.0f
    #endif
    #ifndef IN_POCKET_RESOLUTION
        #define IN_POCKET_RESOLUTION                1.0f
    #endif
    #ifndef IN_POCKET_POWER
        #define IN_POCKET_POWER                     0
    #endif
    #ifndef IN_POCKET_MINDELAY
        #define IN_POCKET_MINDELAY                  -1
    #endif
    #ifndef IN_POCKET_FIFO_MAX_COUNT
        #define IN_POCKET_FIFO_MAX_COUNT            0
    #endif
    #ifndef IN_POCKET_FIFO_RESERVE_COUNT
        #define IN_POCKET_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef IN_POCKET_MAXDELAY
        #define IN_POCKET_MAXDELAY                  0
    #endif
    #ifndef IN_POCKET_FLAGS
        #define IN_POCKET_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_ACTIVITY_SENSOR
    #ifndef ACTIVITY
        #define ACTIVITY                           "ACTIVITY"
    #endif
    #ifndef ACTIVITY_VENDER
        #define ACTIVITY_VENDER                    "MTK"
    #endif
    #ifndef ACTIVITY_VERSION
        #define ACTIVITY_VERSION                   1
    #endif
    #ifndef ACTIVITY_RANGE
        #define ACTIVITY_RANGE                     100.0f
    #endif
    #ifndef ACTIVITY_RESOLUTION
        #define ACTIVITY_RESOLUTION                1.0f
    #endif
    #ifndef ACTIVITY_POWER
        #define ACTIVITY_POWER                     0
    #endif
    #ifndef ACTIVITY_MINDELAY
        #define ACTIVITY_MINDELAY                  0
    #endif
    #ifndef ACTIVITY_FIFO_MAX_COUNT
        #define ACTIVITY_FIFO_MAX_COUNT            0
    #endif
    #ifndef ACTIVITY_FIFO_RESERVE_COUNT
        #define ACTIVITY_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef ACTIVITY_MAXDELAY
        #define ACTIVITY_MAXDELAY                  1000000
    #endif
    #ifndef ACTIVITY_FLAGS
        #define ACTIVITY_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_PDR_SENSOR //??need commit
    #ifndef PDR
        #define PDR                           "PDR"
    #endif
    #ifndef PDR_VENDER
        #define PDR_VENDER                    "MTK"
    #endif
    #ifndef PDR_VERSION
        #define PDR_VERSION                   1
    #endif
    #ifndef PDR_RANGE
        #define PDR_RANGE                     10240.0f
    #endif
    #ifndef PDR_RESOLUTION
        #define PDR_RESOLUTION                1.0f
    #endif
    #ifndef PDR_POWER
        #define PDR_POWER                     0
    #endif
    #ifndef PDR_MINDELAY
        #define PDR_MINDELAY                  0
    #endif
    #ifndef PDR_FIFO_MAX_COUNT
        #define PDR_FIFO_MAX_COUNT            0
    #endif
    #ifndef PDR_FIFO_RESERVE_COUNT
        #define PDR_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef PDR_MAXDELAY
        #define PDR_MAXDELAY                  1000000
    #endif
    #ifndef PDR_FLAGS
        #define PDR_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_FREEFALL
    #ifndef FREEFALL
        #define FREEFALL                           "FREEFALL"
    #endif
    #ifndef FREEFALL_VENDER
        #define FREEFALL_VENDER                    "MTK"
    #endif
    #ifndef FREEFALL_VERSION
        #define FREEFALL_VERSION                   1
    #endif
    #ifndef FREEFALL_RANGE
        #define FREEFALL_RANGE                     1.0f
    #endif
    #ifndef FREEFALL_RESOLUTION
        #define FREEFALL_RESOLUTION                1.0f
    #endif
    #ifndef FREEFALL_POWER
        #define FREEFALL_POWER                     0
    #endif
    #ifndef FREEFALL_MINDELAY
        #define FREEFALL_MINDELAY                  -1
    #endif
    #ifndef FREEFALL_FIFO_MAX_COUNT
        #define FREEFALL_FIFO_MAX_COUNT            0
    #endif
    #ifndef FREEFALL_FIFO_RESERVE_COUNT
        #define FREEFALL_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef FREEFALL_MAXDELAY
        #define FREEFALL_MAXDELAY                  0
    #endif
    #ifndef FREEFALL_FLAGS
        #define FREEFALL_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_UNCALI_ACC_SENSOR
    #ifndef UNCALI_ACC
        #define UNCALI_ACC                           "UNCALI_ACC"
    #endif
    #ifndef UNCALI_ACC_VENDER
        #define UNCALI_ACC_VENDER                    "MTK"
    #endif
    #ifndef UNCALI_ACC_VERSION
        #define UNCALI_ACC_VERSION                   1
    #endif
    #ifndef UNCALI_ACC_RANGE
        #define UNCALI_ACC_RANGE                     39.2266f
    #endif
    #ifndef UNCALI_ACC_RESOLUTION
        #define UNCALI_ACC_RESOLUTION                0.0012
    #endif
    #ifndef UNCALI_ACC_POWER
        #define UNCALI_ACC_POWER                     0
    #endif
    #ifndef UNCALI_ACC_MINDELAY
        #define UNCALI_ACC_MINDELAY                  2500
    #endif
    #ifndef UNCALI_ACC_FIFO_MAX_COUNT
        #define UNCALI_ACC_FIFO_MAX_COUNT            0
    #endif
    #ifndef UNCALI_ACC_FIFO_RESERVE_COUNT
        #define UNCALI_ACC_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef UNCALI_ACC_MAXDELAY
        #define UNCALI_ACC_MAXDELAY                  1000000
    #endif
    #ifndef UNCALI_ACC_FLAGS
        #define UNCALI_ACC_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_FACE_DOWN_SENSOR
    #ifndef FACE_DOWN
        #define FACE_DOWN                           "FACE_DOWN"
    #endif
    #ifndef FACE_DOWN_VENDER
        #define FACE_DOWN_VENDER                    "MTK"
    #endif
    #ifndef FACE_DOWN_VERSION
        #define FACE_DOWN_VERSION                   1
    #endif
    #ifndef FACE_DOWN_RANGE
        #define FACE_DOWN_RANGE                     1.0f
    #endif
    #ifndef FACE_DOWN_RESOLUTION
        #define FACE_DOWN_RESOLUTION                1.0f
    #endif
    #ifndef FACE_DOWN_POWER
        #define FACE_DOWN_POWER                     0
    #endif
    #ifndef FACE_DOWN_MINDELAY
        #define FACE_DOWN_MINDELAY                  -1
    #endif
    #ifndef FACE_DOWN_FIFO_MAX_COUNT
        #define FACE_DOWN_FIFO_MAX_COUNT            0
    #endif
    #ifndef FACE_DOWN_FIFO_RESERVE_COUNT
        #define FACE_DOWN_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef FACE_DOWN_MAXDELAY
        #define FACE_DOWN_MAXDELAY                  0
    #endif
    #ifndef FACE_DOWN_FLAGS
        #define FACE_DOWN_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_SHAKE_SENSOR
    #ifndef SHAKE
        #define SHAKE                           "SHAKE"
    #endif
    #ifndef SHAKE_VENDER
        #define SHAKE_VENDER                    "MTK"
    #endif
    #ifndef SHAKE_VERSION
        #define SHAKE_VERSION                   1
    #endif
    #ifndef SHAKE_RANGE
        #define SHAKE_RANGE                     1.0f
    #endif
    #ifndef SHAKE_RESOLUTION
        #define SHAKE_RESOLUTION                1.0f
    #endif
    #ifndef SHAKE_POWER
        #define SHAKE_POWER                     0
    #endif
    #ifndef SHAKE_MINDELAY
        #define SHAKE_MINDELAY                  -1
    #endif
    #ifndef SHAKE_FIFO_MAX_COUNT
        #define SHAKE_FIFO_MAX_COUNT            0
    #endif
    #ifndef SHAKE_FIFO_RESERVE_COUNT
        #define SHAKE_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef SHAKE_MAXDELAY
        #define SHAKE_MAXDELAY                  0
    #endif
    #ifndef SHAKE_FLAGS
        #define SHAKE_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_BRINGTOSEE_SENSOR
    #ifndef BRINGTOSEE
        #define BRINGTOSEE                           "BRINGTOSEE"
    #endif
    #ifndef BRINGTOSEE_VENDER
        #define BRINGTOSEE_VENDER                    "MTK"
    #endif
    #ifndef BRINGTOSEE_VERSION
        #define BRINGTOSEE_VERSION                   1
    #endif
    #ifndef BRINGTOSEE_RANGE
        #define BRINGTOSEE_RANGE                     1.0f
    #endif
    #ifndef BRINGTOSEE_RESOLUTION
        #define BRINGTOSEE_RESOLUTION                1.0f
    #endif
    #ifndef BRINGTOSEE_POWER
        #define BRINGTOSEE_POWER                     0
    #endif
    #ifndef BRINGTOSEE_MINDELAY
        #define BRINGTOSEE_MINDELAY                  -1
    #endif
    #ifndef BRINGTOSEE_FIFO_MAX_COUNT
        #define BRINGTOSEE_FIFO_MAX_COUNT            0
    #endif
    #ifndef BRINGTOSEE_FIFO_RESERVE_COUNT
        #define BRINGTOSEE_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef BRINGTOSEE_MAXDELAY
        #define BRINGTOSEE_MAXDELAY                  0
    #endif
    #ifndef BRINGTOSEE_FLAGS
        #define BRINGTOSEE_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_ANSWER_CALL_SENSOR
    #ifndef ANSWER_CALL
        #define ANSWER_CALL                           "ANSWER_CALL"
    #endif
    #ifndef ANSWER_CALL_VENDER
        #define ANSWER_CALL_VENDER                    "MTK"
    #endif
    #ifndef ANSWER_CALL_VERSION
        #define ANSWER_CALL_VERSION                   1
    #endif
    #ifndef ANSWER_CALL_RANGE
        #define ANSWER_CALL_RANGE                     1.0f
    #endif
    #ifndef ANSWER_CALL_RESOLUTION
        #define ANSWER_CALL_RESOLUTION                1.0f
    #endif
    #ifndef ANSWER_CALL_POWER
        #define ANSWER_CALL_POWER                     0
    #endif
    #ifndef ANSWER_CALL_MINDELAY
        #define ANSWER_CALL_MINDELAY                  -1
    #endif
    #ifndef ANSWER_CALL_FIFO_MAX_COUNT
        #define ANSWER_CALL_FIFO_MAX_COUNT            0
    #endif
    #ifndef ANSWER_CALL_FIFO_RESERVE_COUNT
        #define ANSWER_CALL_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef ANSWER_CALL_MAXDELAY
        #define ANSWER_CALL_MAXDELAY                  0
    #endif
    #ifndef ANSWER_CALL_FLAGS
        #define ANSWER_CALL_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_PDR_SENSOR
    #ifndef PDR
        #define PDR  "PDR"
        #define PDR_VENDER  "MTK"
    #endif
    #ifndef PDR_RANGE
        #define PDR_RANGE  10240.0f
    #endif
    #ifndef PDR_RESOLUTION
        #define PDR_RESOLUTION   1.0f
    #endif
    #ifndef PDR_POWER
        #define PDR_POWER   0.5f
    #endif
#endif

#ifndef EKG
    #define EKG  "EKG"
    #define EKG_VENDER  "MTK"
#endif
#ifndef EKG_RANGE
    #define EKG_RANGE  10240.0f
#endif
#ifndef EKG_RESOLUTION
    #define EKG_RESOLUTION   1.0f
#endif
#ifndef EKG_POWER
    #define EKG_POWER   0.5f
#endif
#ifndef EKG_MINDELAY
    #define EKG_MINDELAY      2000
#endif
#ifndef EKG_FIFO_RESERVE_COUNT
    #define EKG_FIFO_RESERVE_COUNT (512 * 60)
#endif
#ifndef EKG_FIFO_MAX_COUNT
    #define EKG_FIFO_MAX_COUNT ((512 + 1024 + 1024) * 60)
#endif

#ifndef PPG1
    #define PPG1  "PPG1"
    #define PPG1_VENDER  "MTK"
#endif
#ifndef PPG1_RANGE
    #define PPG1_RANGE  10240.0f
#endif
#ifndef PPG1_RESOLUTION
    #define PPG1_RESOLUTION   1.0f
#endif
#ifndef PPG1_POWER
    #define PPG1_POWER   0.5f
#endif
#ifndef PPG1_MINDELAY
    #define PPG1_MINDELAY      2000
#endif
#ifndef PPG1_FIFO_RESERVE_COUNT
    #define PPG1_FIFO_RESERVE_COUNT (1024 * 60)
#endif
#ifndef PPG1_FIFO_MAX_COUNT
    #define PPG1_FIFO_MAX_COUNT ((512 + 1024 + 1024) * 60)
#endif

#ifndef PPG2
    #define PPG2  "PPG2"
    #define PPG2_VENDER  "MTK"
#endif
#ifndef PPG2_RANGE
    #define PPG2_RANGE  10240.0f
#endif
#ifndef PPG2_RESOLUTION
    #define PPG2_RESOLUTION   1.0f
#endif
#ifndef PPG2_POWER
    #define PPG2_POWER   0.5f
#endif
#ifndef PPG2_MINDELAY
    #define PPG2_MINDELAY      2000
#endif
#ifndef PPG2_FIFO_RESERVE_COUNT
    #define PPG2_FIFO_RESERVE_COUNT (1024 * 60)
#endif
#ifndef PPG2_FIFO_MAX_COUNT
    #define PPG2_FIFO_MAX_COUNT ((512 + 1024 + 1024) * 60)
#endif

#ifdef CUSTOM_KERNEL_FLAT_SENSOR
    #ifndef FLAT
        #define FLAT                           "FLAT"
    #endif
    #ifndef FLAT_VENDER
        #define FLAT_VENDER                    "MTK"
    #endif
    #ifndef FLAT_VERSION
        #define FLAT_VERSION                   1
    #endif
    #ifndef FLAT_RANGE
        #define FLAT_RANGE                     1.0f
    #endif
    #ifndef FLAT_RESOLUTION
        #define FLAT_RESOLUTION                1.0f
    #endif
    #ifndef FLAT_POWER
        #define FLAT_POWER                     0
    #endif
    #ifndef FLAT_MINDELAY
        #define FLAT_MINDELAY                  -1
    #endif
    #ifndef FLAT_FIFO_MAX_COUNT
        #define FLAT_FIFO_MAX_COUNT            0
    #endif
    #ifndef FLAT_FIFO_RESERVE_COUNT
        #define FLAT_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef FLAT_MAXDELAY
        #define FLAT_MAXDELAY                  0
    #endif
    #ifndef FLAT_FLAGS
        #define FLAT_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

#ifdef CUSTOM_KERNEL_RGBW_SENSOR
    #ifndef RGBW
        #define RGBW                           "RGBW"
    #endif
    #ifndef RGBW_VENDER
        #define RGBW_VENDER                    "MTK"
    #endif
    #ifndef RGBW_VERSION
        #define RGBW_VERSION                   1
    #endif
    #ifndef RGBW_RANGE
        #define RGBW_RANGE                     65535.0f
    #endif
    #ifndef RGBW_RESOLUTION
        #define RGBW_RESOLUTION                1.0f
    #endif
    #ifndef RGBW_POWER
        #define RGBW_POWER                     0
    #endif
    #ifndef RGBW_MINDELAY
        #define RGBW_MINDELAY                  0
    #endif
    #ifndef RGBW_FIFO_MAX_COUNT
        #define RGBW_FIFO_MAX_COUNT            0
    #endif
    #ifndef RGBW_FIFO_RESERVE_COUNT
        #define RGBW_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef RGBW_MAXDELAY
        #define RGBW_MAXDELAY                  1000000
    #endif
    #ifndef RGBW_FLAGS
        #define RGBW_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE
    #endif
#endif

#ifdef CUSTOM_KERNEL_SAR_SENSOR
    #ifndef SAR
        #define SAR                           "SAR"
    #endif
    #ifndef SAR_VENDER
        #define SAR_VENDER                    "MTK"
    #endif
    #ifndef SAR_VERSION
        #define SAR_VERSION                   1
    #endif
    #ifndef SAR_RANGE
        #define SAR_RANGE                     1.0f
    #endif
    #ifndef SAR_RESOLUTION
        #define SAR_RESOLUTION                1.0f
    #endif
    #ifndef SAR_POWER
        #define SAR_POWER                     0
    #endif
    #ifndef SAR_MINDELAY
        #define SAR_MINDELAY                  10000
    #endif
    #ifndef SAR_FIFO_MAX_COUNT
        #define SAR_FIFO_MAX_COUNT            0
    #endif
    #ifndef SAR_FIFO_RESERVE_COUNT
        #define SAR_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef SAR_MAXDELAY
        #define SAR_MAXDELAY                  1000000
    #endif
    #ifndef SAR_FLAGS
        #define SAR_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP
    #endif
#endif

enum {
    accel,
    gyro,
    mag,
    als,
    ps,
    baro,
    sar,
    maxhandle,
};

#if 0
static inline int sensor_to_handle(int sensor)
{
    int handle = -1;

    switch (sensor) {
    case ID_ACCELEROMETER:
        handle = accel;
        break;
    case ID_GYROSCOPE:
        handle = gyro;
        break;
    case ID_MAGNETIC:
        handle = mag;
        break;
    case ID_LIGHT:
        handle = als;
        break;
    case ID_PROXIMITY:
        handle = ps;
        break;
    case ID_PRESSURE:
        handle = baro;
        break;
    case ID_SAR:
        handle = sar;
        break;
    }
    return handle;
}
#endif

static inline int handle_to_sensor(int handle)
{
    int sensor = -1;

    switch (handle) {
    case accel:
        sensor = ID_ACCELEROMETER;
        break;
    case gyro:
        sensor = ID_GYROSCOPE;
        break;
    case mag:
        sensor = ID_MAGNETIC;
        break;
    case als:
        sensor = ID_LIGHT;
        break;
    case ps:
        sensor = ID_PROXIMITY;
        break;
    case baro:
        sensor = ID_PRESSURE;
        break;
    case sar:
        sensor = ID_SAR;
        break;
    }
    return sensor;
}

struct sensorlist_info_t {
    char name[16];
};

static struct sensorlist_info_t sensorlist_info[maxhandle];

SensorList *SensorList::SensorListInstance = nullptr;
SensorList *SensorList::getInstance() {
    if (SensorListInstance == nullptr) {
        SensorList *mInterface = new SensorList;
        SensorListInstance = mInterface;
    }
    return SensorListInstance;
}

void SensorList::initSensorList(void) {
    struct sensor_t sensor;

#ifdef CUSTOM_KERNEL_ACCELEROMETER
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = ACCELEROMETER;
    sensor.vendor = ACCELEROMETER_VENDER;
    sensor.version = ACCELEROMETER_VERSION;
    sensor.handle = ID_ACCELEROMETER + ID_OFFSET;
    sensor.type = SENSOR_TYPE_ACCELEROMETER;
    sensor.maxRange = ACCELEROMETER_RANGE;
    sensor.resolution = ACCELEROMETER_RESOLUTION;
    sensor.power = ACCELEROMETER_POWER;
    sensor.minDelay = ACCELEROMETER_MINDELAY;
    sensor.fifoReservedEventCount = ACCELEROMETER_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = ACCELEROMETER_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_ACCELEROMETER;
    sensor.maxDelay = ACCELEROMETER_MAXDELAY;
    sensor.flags = ACCELEROMETER_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_MAGNETOMETER
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = MAGNETOMETER;
    sensor.vendor = MAGNETOMETER_VENDER;
    sensor.version = MAGNETOMETER_VERSION;
    sensor.handle = ID_MAGNETIC + ID_OFFSET;
    sensor.type = SENSOR_TYPE_MAGNETIC_FIELD;
    sensor.maxRange = MAGNETOMETER_RANGE;
    sensor.resolution = MAGNETOMETER_RESOLUTION;
    sensor.power = MAGNETOMETER_POWER;
    sensor.minDelay = MAGNETOMETER_MINDELAY;
    sensor.fifoReservedEventCount = MAGNETOMETER_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = MAGNETOMETER_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_MAGNETIC_FIELD;
    sensor.maxDelay = MAGNETOMETER_MAXDELAY;
    sensor.flags = MAGNETOMETER_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_ORIENTATION_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = ORIENTATION;
    sensor.vendor = ORIENTATION_VENDER;
    sensor.version = ORIENTATION_VERSION;
    sensor.handle = ID_ORIENTATION + ID_OFFSET;
    sensor.type = SENSOR_TYPE_ORIENTATION;
    sensor.maxRange = ORIENTATION_RANGE;
    sensor.resolution = ORIENTATION_RESOLUTION;
    sensor.power = ORIENTATION_POWER;
    sensor.minDelay = ORIENTATION_MINDELAY;
    sensor.fifoReservedEventCount = ORIENTATION_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = ORIENTATION_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_ORIENTATION;
    sensor.maxDelay = ORIENTATION_MAXDELAY;
    sensor.flags = ORIENTATION_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_GYROSCOPE
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = GYROSCOPE;
    sensor.vendor = GYROSCOPE_VENDER;
    sensor.version = GYROSCOPE_VERSION;
    sensor.handle = ID_GYROSCOPE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_GYROSCOPE;
    sensor.maxRange = GYROSCOPE_RANGE;
    sensor.resolution = GYROSCOPE_RESOLUTION;
    sensor.power = GYROSCOPE_POWER;
    sensor.minDelay = GYROSCOPE_MINDELAY;
    sensor.fifoReservedEventCount = GYROSCOPE_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = GYROSCOPE_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_GYROSCOPE;
    sensor.maxDelay = GYROSCOPE_MAXDELAY;
    sensor.flags = GYROSCOPE_FLAGS;
    mSensorList.push_back(sensor);
#endif

#if defined(CUSTOM_KERNEL_ALSPS) || defined(CUSTOM_KERNEL_ALS)
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = LIGHT;
    sensor.vendor = LIGHT_VENDER;
    sensor.version = LIGHT_VERSION;
    sensor.handle = ID_LIGHT + ID_OFFSET;
    sensor.type = SENSOR_TYPE_LIGHT;
    sensor.maxRange = LIGHT_RANGE;
    sensor.resolution = LIGHT_RESOLUTION;
    sensor.power = LIGHT_POWER;
    sensor.minDelay = LIGHT_MINDELAY;
    sensor.fifoReservedEventCount = LIGHT_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = LIGHT_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_LIGHT;
    sensor.maxDelay = LIGHT_MAXDELAY;
    sensor.flags = LIGHT_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_BAROMETER
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = PRESSURE;
    sensor.vendor = PRESSURE_VENDER;
    sensor.version = PRESSURE_VERSION;
    sensor.handle = ID_PRESSURE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_PRESSURE;
    sensor.maxRange = PRESSURE_RANGE;
    sensor.resolution = PRESSURE_RESOLUTION;
    sensor.power = PRESSURE_POWER;
    sensor.minDelay = PRESSURE_MINDELAY;
    sensor.fifoReservedEventCount = PRESSURE_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = PRESSURE_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_PRESSURE;
    sensor.maxDelay = PRESSURE_MAXDELAY;
    sensor.flags = PRESSURE_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_TEMPURATURE
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = TEMPURATURE;
    sensor.vendor = TEMPURATURE_VENDER;
    sensor.version = TEMPURATURE_VERSION;
    sensor.handle = ID_TEMPURATURE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_TEMPERATURE;
    sensor.maxRange = TEMPURATURE_RANGE;
    sensor.resolution = TEMPURATURE_RESOLUTION;
    sensor.power = TEMPURATURE_POWER;
    sensor.minDelay = TEMPURATURE_MINDELAY;
    sensor.fifoReservedEventCount = TEMPURATURE_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = TEMPURATURE_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_TEMPERATURE;
    sensor.maxDelay = TEMPURATURE_MAXDELAY;
    sensor.flags = TEMPURATURE_FLAGS;
    mSensorList.push_back(sensor);
#endif

#if defined(CUSTOM_KERNEL_ALSPS) || defined(CUSTOM_KERNEL_PS)
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = PROXIMITY;
    sensor.vendor = PROXIMITY_VENDER;
    sensor.version = PROXIMITY_VERSION;
    sensor.handle = ID_PROXIMITY + ID_OFFSET;
    sensor.type = SENSOR_TYPE_PROXIMITY;
    sensor.maxRange = PROXIMITY_RANGE;
    sensor.resolution = PROXIMITY_RESOLUTION;
    sensor.power = PROXIMITY_POWER;
    sensor.minDelay = PROXIMITY_MINDELAY;
    sensor.fifoReservedEventCount = PROXIMITY_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = PROXIMITY_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_PROXIMITY;
    sensor.maxDelay = PROXIMITY_MAXDELAY;
    sensor.flags = PROXIMITY_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_GRAVITY_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = GRAVITY;
    sensor.vendor = GRAVITY_VENDER;
    sensor.version = GRAVITY_VERSION;
    sensor.handle = ID_GRAVITY + ID_OFFSET;
    sensor.type = SENSOR_TYPE_GRAVITY;
    sensor.maxRange = GRAVITY_RANGE;
    sensor.resolution = GRAVITY_RESOLUTION;
    sensor.power = GRAVITY_POWER;
    sensor.minDelay = GRAVITY_MINDELAY;
    sensor.fifoReservedEventCount = GRAVITY_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = GRAVITY_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_GRAVITY;
    sensor.maxDelay = GRAVITY_MAXDELAY;
    sensor.flags = GRAVITY_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_LINEARACCEL_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = LINEARACCEL;
    sensor.vendor = LINEARACCEL_VENDER;
    sensor.version = LINEARACCEL_VERSION;
    sensor.handle = ID_LINEAR_ACCELERATION + ID_OFFSET;
    sensor.type = SENSOR_TYPE_LINEAR_ACCELERATION;
    sensor.maxRange = LINEARACCEL_RANGE;
    sensor.resolution = LINEARACCEL_RESOLUTION;
    sensor.power = LINEARACCEL_POWER;
    sensor.minDelay = LINEARACCEL_MINDELAY;
    sensor.fifoReservedEventCount = LINEARACCEL_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = LINEARACCEL_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_LINEAR_ACCELERATION;
    sensor.maxDelay = LINEARACCEL_MAXDELAY;
    sensor.flags = LINEARACCEL_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_RV_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = ROTATION_VECTOR;
    sensor.vendor = ROTATION_VECTOR_VENDER;
    sensor.version = ROTATION_VECTOR_VERSION;
    sensor.handle = ID_ROTATION_VECTOR + ID_OFFSET;
    sensor.type = SENSOR_TYPE_ROTATION_VECTOR;
    sensor.maxRange = ROTATION_VECTOR_RANGE;
    sensor.resolution = ROTATION_VECTOR_RESOLUTION;
    sensor.power = ROTATION_VECTOR_POWER;
    sensor.minDelay = ROTATION_VECTOR_MINDELAY;
    sensor.fifoReservedEventCount = ROTATION_VECTOR_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = ROTATION_VECTOR_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_ROTATION_VECTOR;
    sensor.maxDelay = ROTATION_VECTOR_MAXDELAY;
    sensor.flags = ROTATION_VECTOR_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_HUMIDITY
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = HUMIDITY;
    sensor.vendor = HUMIDITY_VENDER;
    sensor.version = HUMIDITY_VERSION;
    sensor.handle = ID_RELATIVE_HUMIDITY + ID_OFFSET;
    sensor.type = SENSOR_TYPE_RELATIVE_HUMIDITY;
    sensor.maxRange = HUMIDITY_RANGE;
    sensor.resolution = HUMIDITY_RESOLUTION;
    sensor.power = HUMIDITY_POWER;
    sensor.minDelay = HUMIDITY_MINDELAY;
    sensor.fifoReservedEventCount = HUMIDITY_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = HUMIDITY_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_RELATIVE_HUMIDITY;
    sensor.maxDelay = HUMIDITY_MAXDELAY;
    sensor.flags = HUMIDITY_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_AMBIENT_TEMPERATURE
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = AMBIENT_TEMPERATURE;
    sensor.vendor = AMBIENT_TEMPERATURE_VENDER;
    sensor.version = AMBIENT_TEMPERATURE_VERSION;
    sensor.handle = ID_AMBIENT_TEMPERATURE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
    sensor.maxRange = AMBIENT_TEMPERATURE_RANGE;
    sensor.resolution = AMBIENT_TEMPERATURE_RESOLUTION;
    sensor.power = AMBIENT_TEMPERATURE_POWER;
    sensor.minDelay = AMBIENT_TEMPERATURE_MINDELAY;
    sensor.fifoReservedEventCount = AMBIENT_TEMPERATURE_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = AMBIENT_TEMPERATURE_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_AMBIENT_TEMPERATURE;
    sensor.maxDelay = AMBIENT_TEMPERATURE_MAXDELAY;
    sensor.flags = AMBIENT_TEMPERATURE_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_UNCALI_MAG_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = UNCALI_MAG;
    sensor.vendor = UNCALI_MAG_VENDER;
    sensor.version = UNCALI_MAG_VERSION;
    sensor.handle = ID_MAGNETIC_UNCALIBRATED + ID_OFFSET;
    sensor.type = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
    sensor.maxRange = UNCALI_MAG_RANGE;
    sensor.resolution = UNCALI_MAG_RESOLUTION;
    sensor.power = UNCALI_MAG_POWER;
    sensor.minDelay = UNCALI_MAG_MINDELAY;
    sensor.fifoReservedEventCount = UNCALI_MAG_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = UNCALI_MAG_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_MAGNETIC_FIELD_UNCALIBRATED;
    sensor.maxDelay = UNCALI_MAG_MAXDELAY;
    sensor.flags = UNCALI_MAG_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_GRV_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = GAME_ROTATION_VECTOR;
    sensor.vendor = GAME_ROTATION_VECTOR_VENDER;
    sensor.version = GAME_ROTATION_VECTOR_VERSION;
    sensor.handle = ID_GAME_ROTATION_VECTOR + ID_OFFSET;
    sensor.type = SENSOR_TYPE_GAME_ROTATION_VECTOR;
    sensor.maxRange = GAME_ROTATION_VECTOR_RANGE;
    sensor.resolution = GAME_ROTATION_VECTOR_RESOLUTION;
    sensor.power = GAME_ROTATION_VECTOR_POWER;
    sensor.minDelay = GAME_ROTATION_VECTOR_MINDELAY;
    sensor.fifoReservedEventCount = GAME_ROTATION_VECTOR_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = GAME_ROTATION_VECTOR_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_GAME_ROTATION_VECTOR;
    sensor.maxDelay = GAME_ROTATION_VECTOR_MAXDELAY;
    sensor.flags = GAME_ROTATION_VECTOR_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_UNCALI_GYRO_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = UNCALI_GYRO;
    sensor.vendor = UNCALI_GYRO_VENDER;
    sensor.version = UNCALI_GYRO_VERSION;
    sensor.handle = ID_GYROSCOPE_UNCALIBRATED + ID_OFFSET;
    sensor.type = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED;
    sensor.maxRange = UNCALI_GYRO_RANGE;
    sensor.resolution = UNCALI_GYRO_RESOLUTION;
    sensor.power = UNCALI_GYRO_POWER;
    sensor.minDelay = UNCALI_GYRO_MINDELAY;
    sensor.fifoReservedEventCount = UNCALI_GYRO_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = UNCALI_GYRO_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_GYROSCOPE_UNCALIBRATED;
    sensor.maxDelay = UNCALI_GYRO_MAXDELAY;
    sensor.flags = UNCALI_GYRO_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_SIGNIFICANT_MOTION_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = SIGNIFICANT_MOTION;
    sensor.vendor = SIGNIFICANT_MOTION_VENDER;
    sensor.version = SIGNIFICANT_MOTION_VERSION;
    sensor.handle = ID_SIGNIFICANT_MOTION + ID_OFFSET;
    sensor.type = SENSOR_TYPE_SIGNIFICANT_MOTION;
    sensor.maxRange = SIGNIFICANT_MOTION_RANGE;
    sensor.resolution = SIGNIFICANT_MOTION_RESOLUTION;
    sensor.power = SIGNIFICANT_MOTION_POWER;
    sensor.minDelay = SIGNIFICANT_MOTION_MINDELAY;
    sensor.fifoReservedEventCount = SIGNIFICANT_MOTION_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = SIGNIFICANT_MOTION_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_SIGNIFICANT_MOTION;
    sensor.maxDelay = SIGNIFICANT_MOTION_MAXDELAY;
    sensor.flags = SIGNIFICANT_MOTION_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_STEP_COUNTER
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = STEP_DETECTOR;
    sensor.vendor = STEP_DETECTOR_VENDER;
    sensor.version = STEP_DETECTOR_VERSION;
    sensor.handle = ID_STEP_DETECTOR + ID_OFFSET;
    sensor.type = SENSOR_TYPE_STEP_DETECTOR;
    sensor.maxRange = STEP_DETECTOR_RANGE;
    sensor.resolution = STEP_DETECTOR_RESOLUTION;
    sensor.power = STEP_DETECTOR_POWER;
    sensor.minDelay = STEP_DETECTOR_MINDELAY;
    sensor.fifoReservedEventCount = STEP_DETECTOR_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = STEP_DETECTOR_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_STEP_DETECTOR;
    sensor.maxDelay = STEP_DETECTOR_MAXDELAY;
    sensor.flags = STEP_DETECTOR_FLAGS;
    mSensorList.push_back(sensor);

    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = STEP_DETECTOR_WAKEUP;
    sensor.vendor = STEP_DETECTOR_VENDER;
    sensor.version = STEP_DETECTOR_VERSION;
    sensor.handle = ID_STEP_DETECTOR_WAKEUP + ID_OFFSET;
    sensor.type = SENSOR_TYPE_STEP_DETECTOR;
    sensor.maxRange = STEP_DETECTOR_RANGE;
    sensor.resolution = STEP_DETECTOR_RESOLUTION;
    sensor.power = STEP_DETECTOR_POWER;
    sensor.minDelay = STEP_DETECTOR_MINDELAY;
    sensor.fifoReservedEventCount = STEP_DETECTOR_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = STEP_DETECTOR_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_STEP_DETECTOR;
    sensor.maxDelay = STEP_DETECTOR_MAXDELAY;
    sensor.flags = STEP_DETECTOR_WAKEUP_FLAGS;
    mSensorList.push_back(sensor);

    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = STEP_COUNTER;
    sensor.vendor = STEP_COUNTER_VENDER;
    sensor.version = STEP_COUNTER_VERSION;
    sensor.handle = ID_STEP_COUNTER + ID_OFFSET;
    sensor.type = SENSOR_TYPE_STEP_COUNTER;
    sensor.maxRange = STEP_COUNTER_RANGE;
    sensor.resolution = STEP_COUNTER_RESOLUTION;
    sensor.power = STEP_COUNTER_POWER;
    sensor.minDelay = STEP_COUNTER_MINDELAY;
    sensor.fifoReservedEventCount = STEP_COUNTER_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = STEP_COUNTER_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_STEP_COUNTER;
    sensor.maxDelay = STEP_COUNTER_MAXDELAY;
    sensor.flags  = STEP_COUNTER_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_FLOOR_COUNTER
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = FLOOR_COUNTER;
    sensor.vendor = FLOOR_COUNTER_VENDER;
    sensor.version = FLOOR_COUNTER_VERSION;
    sensor.handle = ID_FLOOR_COUNTER + ID_OFFSET;
    sensor.type = SENSOR_TYPE_FLOOR_COUNTER;
    sensor.maxRange = FLOOR_COUNTER_RANGE;
    sensor.resolution = FLOOR_COUNTER_RESOLUTION;
    sensor.power = FLOOR_COUNTER_POWER;
    sensor.minDelay = FLOOR_COUNTER_MINDELAY;
    sensor.fifoReservedEventCount = FLOOR_COUNTER_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = FLOOR_COUNTER_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_FLOOR_COUNTER;
    sensor.maxDelay = FLOOR_COUNTER_MAXDELAY;
    sensor.flags = FLOOR_COUNTER_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_GMRV_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = GEOMAGNETIC_ROTATION_VECTOR;
    sensor.vendor = GEOMAGNETIC_ROTATION_VECTOR_VENDER;
    sensor.version = GEOMAGNETIC_ROTATION_VECTOR_VERSION;
    sensor.handle = ID_GEOMAGNETIC_ROTATION_VECTOR + ID_OFFSET;
    sensor.type = SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR;
    sensor.maxRange = GEOMAGNETIC_ROTATION_VECTOR_RANGE;
    sensor.resolution = GEOMAGNETIC_ROTATION_VECTOR_RESOLUTION;
    sensor.power = GEOMAGNETIC_ROTATION_VECTOR_POWER;
    sensor.minDelay = GEOMAGNETIC_ROTATION_VECTOR_MINDELAY;
    sensor.fifoReservedEventCount = GEOMAGNETIC_ROTATION_VECTOR_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = GEOMAGNETIC_ROTATION_VECTOR_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_GEOMAGNETIC_ROTATION_VECTOR;
    sensor.maxDelay = GEOMAGNETIC_ROTATION_VECTOR_MAXDELAY;
    sensor.flags = GEOMAGNETIC_ROTATION_VECTOR_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_HEART
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = HEART_RATE;
    sensor.vendor = HEART_RATE_VENDER;
    sensor.version = HEART_RATE_VERSION;
    sensor.handle = ID_HEART_RATE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_HEART_RATE;
    sensor.maxRange = HEART_RATE_RANGE;
    sensor.resolution = HEART_RATE_RESOLUTION;
    sensor.power = HEART_RATE_POWER;
    sensor.minDelay = HEART_RATE_MINDELAY;
    sensor.fifoReservedEventCount = HEART_RATE_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = HEART_RATE_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_HEART_RATE;
    sensor.maxDelay = HEART_RATE_MAXDELAY;
    sensor.flags = HEART_RATE_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_TILT_DETECTOR_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = TILT_DETECTOR;
    sensor.vendor = TILT_DETECTOR_VENDER;
    sensor.version = TILT_DETECTOR_VERSION;
    sensor.handle = ID_TILT_DETECTOR + ID_OFFSET;
    sensor.type = SENSOR_TYPE_TILT_DETECTOR;
    sensor.maxRange = TILT_DETECTOR_RANGE;
    sensor.resolution = TILT_DETECTOR_RESOLUTION;
    sensor.power = TILT_DETECTOR_POWER;
    sensor.minDelay = TILT_DETECTOR_MINDELAY;
    sensor.fifoReservedEventCount = TILT_DETECTOR_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = TILT_DETECTOR_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_TILT_DETECTOR;
    sensor.maxDelay = TILT_DETECTOR_MAXDELAY;
    sensor.flags = TILT_DETECTOR_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_WAKE_GESTURE_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = WAKE_GESTURE;
    sensor.vendor = WAKE_GESTURE_VENDER;
    sensor.version = WAKE_GESTURE_VERSION;
    sensor.handle = ID_WAKE_GESTURE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_WAKE_GESTURE;
    sensor.maxRange = WAKE_GESTURE_RANGE;
    sensor.resolution = WAKE_GESTURE_RESOLUTION;
    sensor.power = WAKE_GESTURE_POWER;
    sensor.minDelay = WAKE_GESTURE_MINDELAY;
    sensor.fifoReservedEventCount = WAKE_GESTURE_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = WAKE_GESTURE_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_WAKE_GESTURE;
    sensor.maxDelay = WAKE_GESTURE_MAXDELAY;
    sensor.flags = WAKE_GESTURE_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_GLANCE_GESTURE_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = GLANCE_GESTURE;
    sensor.vendor = GLANCE_GESTURE_VENDER;
    sensor.version = GLANCE_GESTURE_VERSION;
    sensor.handle = ID_GLANCE_GESTURE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_GLANCE_GESTURE;
    sensor.maxRange = GLANCE_GESTURE_RANGE;
    sensor.resolution = GLANCE_GESTURE_RESOLUTION;
    sensor.power = GLANCE_GESTURE_POWER;
    sensor.minDelay = GLANCE_GESTURE_MINDELAY;
    sensor.fifoReservedEventCount = GLANCE_GESTURE_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = GLANCE_GESTURE_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_GLANCE_GESTURE;
    sensor.maxDelay = GLANCE_GESTURE_MAXDELAY;
    sensor.flags = GLANCE_GESTURE_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_PICK_UP_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = PICK_UP;
    sensor.vendor = PICK_UP_VENDER;
    sensor.version = PICK_UP_VERSION;
    sensor.handle = ID_PICK_UP_GESTURE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_PICK_UP_GESTURE;
    sensor.maxRange = PICK_UP_RANGE;
    sensor.resolution = PICK_UP_RESOLUTION;
    sensor.power = PICK_UP_POWER;
    sensor.minDelay = PICK_UP_MINDELAY;
    sensor.fifoReservedEventCount = PICK_UP_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = PICK_UP_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_PICK_UP_GESTURE;
    sensor.maxDelay = PICK_UP_MAXDELAY;
    sensor.flags = PICK_UP_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_WRIST_TILT
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = WRIST_TILT;
    sensor.vendor = WRIST_TILT_VENDER;
    sensor.version = WRIST_TILT_VERSION;
    sensor.handle = ID_WRIST_TILT + ID_OFFSET;
    sensor.type = SENSOR_TYPE_WRIST_TILT_GESTURE;
    sensor.maxRange = WRIST_TILT_RANGE;
    sensor.resolution = WRIST_TILT_RESOLUTION;
    sensor.power = WRIST_TILT_POWER;
    sensor.minDelay = WRIST_TILT_MINDELAY;
    sensor.fifoReservedEventCount = WRIST_TILT_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = WRIST_TILT_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_WRIST_TILT_GESTURE;
    sensor.maxDelay = WRIST_TILT_MAXDELAY;
    sensor.flags = WRIST_TILT_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_DEVICE_ORIENTATION
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = DEVICE_ORIENTATION;
    sensor.vendor = DEVICE_ORIENTATION_VENDER;
    sensor.version = DEVICE_ORIENTATION_VERSION;
    sensor.handle = ID_DEVICE_ORIENTATION + ID_OFFSET;
    sensor.type = SENSOR_TYPE_DEVICE_ORIENTATION;
    sensor.maxRange = DEVICE_ORIENTATION_RANGE;
    sensor.resolution = DEVICE_ORIENTATION_RESOLUTION;
    sensor.power = DEVICE_ORIENTATION_POWER;
    sensor.minDelay = DEVICE_ORIENTATION_MINDELAY;
    sensor.fifoReservedEventCount = DEVICE_ORIENTATION_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = DEVICE_ORIENTATION_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_DEVICE_ORIENTATION;
    sensor.maxDelay = DEVICE_ORIENTATION_MAXDELAY;
    sensor.flags = DEVICE_ORIENTATION_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_POSE_6DOF
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = POSE_6DOF;
    sensor.vendor = POSE_6DOF_VENDER;
    sensor.version = POSE_6DOF_VERSION;
    sensor.handle = ID_POSE_6DOF + ID_OFFSET;
    sensor.type = SENSOR_TYPE_POSE_6DOF;
    sensor.maxRange = POSE_6DOF_RANGE;
    sensor.resolution = POSE_6DOF_RESOLUTION;
    sensor.power = POSE_6DOF_POWER;
    sensor.minDelay = POSE_6DOF_MINDELAY;
    sensor.fifoReservedEventCount = POSE_6DOF_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = POSE_6DOF_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_POSE_6DOF;
    sensor.maxDelay = POSE_6DOF_MAXDELAY;
    sensor.flags = POSE_6DOF_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_STATIONARY_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = STATIONARY_DETECT;
    sensor.vendor = STATIONARY_DETECT_VENDER;
    sensor.version = STATIONARY_DETECT_VERSION;
    sensor.handle = ID_STATIONARY_DETECT + ID_OFFSET;
    sensor.type = SENSOR_TYPE_STATIONARY_DETECT;
    sensor.maxRange = STATIONARY_DETECT_RANGE;
    sensor.resolution = STATIONARY_DETECT_RESOLUTION;
    sensor.power = STATIONARY_DETECT_POWER;
    sensor.minDelay = STATIONARY_DETECT_MINDELAY;
    sensor.fifoReservedEventCount = STATIONARY_DETECT_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = STATIONARY_DETECT_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_STATIONARY_DETECT;
    sensor.maxDelay = STATIONARY_DETECT_MAXDELAY;
    sensor.flags = STATIONARY_DETECT_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_MOTION_DETECT
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = MOTION_DETECT;
    sensor.vendor = MOTION_DETECT_VENDER;
    sensor.version = MOTION_DETECT_VERSION;
    sensor.handle = ID_MOTION_DETECT + ID_OFFSET;
    sensor.type = SENSOR_TYPE_MOTION_DETECT;
    sensor.maxRange = MOTION_DETECT_RANGE;
    sensor.resolution = MOTION_DETECT_RESOLUTION;
    sensor.power = MOTION_DETECT_POWER;
    sensor.minDelay = MOTION_DETECT_MINDELAY;
    sensor.fifoReservedEventCount = MOTION_DETECT_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = MOTION_DETECT_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_MOTION_DETECT;
    sensor.maxDelay = MOTION_DETECT_MAXDELAY;
    sensor.flags = MOTION_DETECT_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_HEART_BEAT
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = HEART_BEAT;
    sensor.vendor = HEART_BEAT_VENDER;
    sensor.version = HEART_BEAT_VERSION;
    sensor.handle = ID_HEART_BEAT + ID_OFFSET;
    sensor.type = SENSOR_TYPE_HEART_BEAT;
    sensor.maxRange = HEART_BEAT_RANGE;
    sensor.resolution = HEART_BEAT_RESOLUTION;
    sensor.power = HEART_BEAT_POWER;
    sensor.minDelay = HEART_BEAT_MINDELAY;
    sensor.fifoReservedEventCount = HEART_BEAT_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = HEART_BEAT_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_HEART_BEAT;
    sensor.maxDelay = HEART_BEAT_MAXDELAY;
    sensor.flags = HEART_BEAT_FLAGS;
    mSensorList.push_back(sensor);
#endif

/* MTK sensor type */

#ifdef CUSTOM_KERNEL_PEDOMETER
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = PEDOMETER;
    sensor.vendor = PEDOMETER_VENDER;
    sensor.version = PEDOMETER_VERSION;
    sensor.handle = ID_PEDOMETER + ID_OFFSET;
    sensor.type = SENSOR_TYPE_PEDOMETER;
    sensor.maxRange = PEDOMETER_RANGE;
    sensor.resolution = PEDOMETER_RESOLUTION;
    sensor.power = PEDOMETER_POWER;
    sensor.minDelay = PEDOMETER_MINDELAY;
    sensor.fifoReservedEventCount = PEDOMETER_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = PEDOMETER_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_PEDOMETER;
    sensor.maxDelay = PEDOMETER_MAXDELAY;
    sensor.flags = PEDOMETER_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_IN_POCKET_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = IN_POCKET;
    sensor.vendor = IN_POCKET_VENDER;
    sensor.version = IN_POCKET_VERSION;
    sensor.handle = ID_IN_POCKET + ID_OFFSET;
    sensor.type = SENSOR_TYPE_IN_POCKET;
    sensor.maxRange = IN_POCKET_RANGE;
    sensor.resolution = IN_POCKET_RESOLUTION;
    sensor.power = IN_POCKET_POWER;
    sensor.minDelay = IN_POCKET_MINDELAY;
    sensor.fifoReservedEventCount = IN_POCKET_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = IN_POCKET_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_IN_POCKET;
    sensor.maxDelay = IN_POCKET_MAXDELAY;
    sensor.flags = IN_POCKET_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_ACTIVITY_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = ACTIVITY;
    sensor.vendor = ACTIVITY_VENDER;
    sensor.version = ACTIVITY_VERSION;
    sensor.handle = ID_ACTIVITY + ID_OFFSET;
    sensor.type = SENSOR_TYPE_ACTIVITY;
    sensor.maxRange = ACTIVITY_RANGE;
    sensor.resolution = ACTIVITY_RESOLUTION;
    sensor.power = ACTIVITY_POWER;
    sensor.minDelay = ACTIVITY_MINDELAY;
    sensor.fifoReservedEventCount = ACTIVITY_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = ACTIVITY_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_ACTIVITY;
    sensor.maxDelay = ACTIVITY_MAXDELAY;
    sensor.flags = ACTIVITY_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_PDR_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = PDR;
    sensor.vendor = PDR_VENDER;
    sensor.version = PDR_VERSION;
    sensor.handle = ID_PDR + ID_OFFSET;
    sensor.type = SENSOR_TYPE_PDR;
    sensor.maxRange = PDR_RANGE;
    sensor.resolution = PDR_RESOLUTION;
    sensor.power = PDR_POWER;
    sensor.minDelay = PDR_MINDELAY;
    sensor.fifoReservedEventCount = PDR_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = PDR_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_PDR;
    sensor.maxDelay = PDR_MAXDELAY;
    sensor.flags = PDR_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_FREEFALL
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = FREEFALL;
    sensor.vendor = FREEFALL_VENDER;
    sensor.version = FREEFALL_VERSION;
    sensor.handle = ID_FREEFALL + ID_OFFSET;
    sensor.type = SENSOR_TYPE_FREEFALL;
    sensor.maxRange = FREEFALL_RANGE;
    sensor.resolution = FREEFALL_RESOLUTION;
    sensor.power = FREEFALL_POWER;
    sensor.minDelay = FREEFALL_MINDELAY;
    sensor.fifoReservedEventCount = FREEFALL_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = FREEFALL_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_FREEFALL;
    sensor.maxDelay = FREEFALL_MAXDELAY;
    sensor.flags = FREEFALL_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_UNCALI_ACC_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = UNCALI_ACC;
    sensor.vendor = UNCALI_ACC_VENDER;
    sensor.version = UNCALI_ACC_VERSION;
    sensor.handle = ID_ACCELEROMETER_UNCALIBRATED + ID_OFFSET;
    sensor.type = SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED;
    sensor.maxRange = UNCALI_ACC_RANGE;
    sensor.resolution = UNCALI_ACC_RESOLUTION;
    sensor.power = UNCALI_ACC_POWER;
    sensor.minDelay = UNCALI_ACC_MINDELAY;
    sensor.fifoReservedEventCount = UNCALI_ACC_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = UNCALI_ACC_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_ACCELEROMETER_UNCALIBRATED;
    sensor.maxDelay = UNCALI_ACC_MAXDELAY;
    sensor.flags = UNCALI_ACC_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_FACE_DOWN_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = FACE_DOWN;
    sensor.vendor = FACE_DOWN_VENDER;
    sensor.version = FACE_DOWN_VERSION;
    sensor.handle = ID_FACE_DOWN + ID_OFFSET;
    sensor.type = SENSOR_TYPE_FACE_DOWN;
    sensor.maxRange = FACE_DOWN_RANGE;
    sensor.resolution = FACE_DOWN_RESOLUTION;
    sensor.power = FACE_DOWN_POWER;
    sensor.minDelay = FACE_DOWN_MINDELAY;
    sensor.fifoReservedEventCount = FACE_DOWN_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = FACE_DOWN_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_FACE_DOWN;
    sensor.maxDelay = FACE_DOWN_MAXDELAY;
    sensor.flags = FACE_DOWN_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_SHAKE_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = SHAKE;
    sensor.vendor = SHAKE_VENDER;
    sensor.version = SHAKE_VERSION;
    sensor.handle = ID_SHAKE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_SHAKE;
    sensor.maxRange = SHAKE_RANGE;
    sensor.resolution = SHAKE_RESOLUTION;
    sensor.power = SHAKE_POWER;
    sensor.minDelay = SHAKE_MINDELAY;
    sensor.fifoReservedEventCount = SHAKE_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = SHAKE_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_SHAKE;
    sensor.maxDelay = SHAKE_MAXDELAY;
    sensor.flags = SHAKE_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_BRINGTOSEE_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = BRINGTOSEE;
    sensor.vendor = BRINGTOSEE_VENDER;
    sensor.version = BRINGTOSEE_VERSION;
    sensor.handle = ID_BRINGTOSEE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_BRINGTOSEE;
    sensor.maxRange = BRINGTOSEE_RANGE;
    sensor.resolution = BRINGTOSEE_RESOLUTION;
    sensor.power = BRINGTOSEE_POWER;
    sensor.minDelay = BRINGTOSEE_MINDELAY;
    sensor.fifoReservedEventCount = BRINGTOSEE_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = BRINGTOSEE_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_BRINGTOSEE;
    sensor.maxDelay = BRINGTOSEE_MAXDELAY;
    sensor.flags = BRINGTOSEE_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_ANSWER_CALL_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = ANSWER_CALL;
    sensor.vendor = ANSWER_CALL_VENDER;
    sensor.version = ANSWER_CALL_VERSION;
    sensor.handle = ID_ANSWER_CALL + ID_OFFSET;
    sensor.type = SENSOR_TYPE_ANSWER_CALL;
    sensor.maxRange = ANSWER_CALL_RANGE;
    sensor.resolution = ANSWER_CALL_RESOLUTION;
    sensor.power = ANSWER_CALL_POWER;
    sensor.minDelay = ANSWER_CALL_MINDELAY;
    sensor.fifoReservedEventCount = ANSWER_CALL_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = ANSWER_CALL_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_ANSWERCALL;
    sensor.maxDelay = ANSWER_CALL_MAXDELAY;
    sensor.flags = ANSWER_CALL_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_BIOMETRIC_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = EKG;
    sensor.vendor = EKG_VENDER;
    sensor.version = 1;
    sensor.handle = ID_EKG + ID_OFFSET;
    sensor.type = SENSOR_TYPE_EKG;
    sensor.maxRange = EKG_RANGE;
    sensor.resolution = EKG_RESOLUTION;
    sensor.power = EKG_POWER;
    sensor.minDelay = EKG_MINDELAY;
    sensor.fifoReservedEventCount = EKG_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = EKG_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_EKG;
    sensor.flags = SENSOR_FLAG_CONTINUOUS_MODE;
    mSensorList.push_back(sensor);

    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = PPG1;
    sensor.vendor = PPG1_VENDER;
    sensor.version = 1;
    sensor.handle = ID_PPG1 + ID_OFFSET;
    sensor.type = SENSOR_TYPE_PPG1;
    sensor.maxRange = PPG1_RANGE;
    sensor.resolution = PPG1_RESOLUTION;
    sensor.power = PPG1_POWER;
    sensor.minDelay = PPG1_MINDELAY;
    sensor.fifoReservedEventCount = PPG1_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = PPG1_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_PPG1;
    sensor.flags = SENSOR_FLAG_CONTINUOUS_MODE;
    mSensorList.push_back(sensor);

    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = PPG2;
    sensor.vendor = PPG2_VENDER;
    sensor.version = 1;
    sensor.handle = ID_PPG2 + ID_OFFSET;
    sensor.type = SENSOR_TYPE_PPG2;
    sensor.maxRange = PPG2_RANGE;
    sensor.resolution = PPG2_RESOLUTION;
    sensor.power = PPG2_POWER;
    sensor.minDelay = PPG2_MINDELAY;
    sensor.fifoReservedEventCount = PPG2_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = PPG2_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_PPG2;
    sensor.flags = SENSOR_FLAG_CONTINUOUS_MODE;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_FLAT_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = FLAT;
    sensor.vendor = FLAT_VENDER;
    sensor.version = FLAT_VERSION;
    sensor.handle = ID_FLAT + ID_OFFSET;
    sensor.type = SENSOR_TYPE_FLAT;
    sensor.maxRange = FLAT_RANGE;
    sensor.resolution = FLAT_RESOLUTION;
    sensor.power = FLAT_POWER;
    sensor.minDelay = FLAT_MINDELAY;
    sensor.fifoReservedEventCount = FLAT_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = FLAT_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_FLAT;
    sensor.maxDelay = FLAT_MAXDELAY;
    sensor.flags = FLAT_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_RGBW_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = RGBW;
    sensor.vendor = RGBW_VENDER;
    sensor.version = RGBW_VERSION;
    sensor.handle = ID_RGBW + ID_OFFSET;
    sensor.type = SENSOR_TYPE_RGBW;
    sensor.maxRange = RGBW_RANGE;
    sensor.resolution = RGBW_RESOLUTION;
    sensor.power = RGBW_POWER;
    sensor.minDelay = RGBW_MINDELAY;
    sensor.fifoReservedEventCount = RGBW_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = RGBW_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_RGBW;
    sensor.maxDelay = RGBW_MAXDELAY;
    sensor.flags = RGBW_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_GYRO_TEMPERATURE_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = GYRO_TEMPERATURE;
    sensor.vendor = GYRO_TEMPERATURE_VENDER;
    sensor.version = GYRO_TEMPERATURE_VERSION;
    sensor.handle = ID_GYRO_TEMPERATURE + ID_OFFSET;
    sensor.type = SENSOR_TYPE_GYRO_TEMPERATURE;
    sensor.maxRange = GYRO_TEMPERATURE_RANGE;
    sensor.resolution = GYRO_TEMPERATURE_RESOLUTION;
    sensor.power = GYRO_TEMPERATURE_POWER;
    sensor.minDelay = GYRO_TEMPERATURE_MINDELAY;
    sensor.fifoReservedEventCount = GYRO_TEMPERATURE_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = GYRO_TEMPERATURE_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_GYRO_TEMPERATURE;
    sensor.maxDelay = GYRO_TEMPERATURE_MAXDELAY;
    sensor.flags = GYRO_TEMPERATURE_FLAGS;
    mSensorList.push_back(sensor);
#endif

#ifdef CUSTOM_KERNEL_SAR_SENSOR
    memset(&sensor, 0, sizeof(struct sensor_t));
    sensor.name = SAR;
    sensor.vendor = SAR_VENDER;
    sensor.version = SAR_VERSION;
    sensor.handle = ID_SAR + ID_OFFSET;
    sensor.type = SENSOR_TYPE_SAR;
    sensor.maxRange = SAR_RANGE;
    sensor.resolution = SAR_RESOLUTION;
    sensor.power = SAR_POWER;
    sensor.minDelay = SAR_MINDELAY;
    sensor.fifoReservedEventCount = SAR_FIFO_RESERVE_COUNT;
    sensor.fifoMaxEventCount = SAR_FIFO_MAX_COUNT;
    sensor.stringType = SENSOR_STRING_TYPE_SAR;
    sensor.maxDelay = SAR_MAXDELAY;
    sensor.flags = SAR_FLAGS;
    mSensorList.push_back(sensor);
#endif

    ALOGI("defaultSensorListCount=%u\n", static_cast<unsigned int>(mSensorList.size()));
}

typedef size_t (*getListFunc_t)(struct sensor_t const **);

static getListFunc_t loadSensorCustom(void **handleP) {
    void *lib_handle = dlopen("libsensor_custom.so", RTLD_GLOBAL);

    if (!lib_handle) {
        ALOGE("loadSensorCustom dlopen fail\n");
        return NULL;
    }
    getListFunc_t function =
        reinterpret_cast<getListFunc_t>(dlsym(lib_handle, "getDynamicSensorList"));

    *handleP = lib_handle;
    return function;
}

SensorList::SensorList() {
    int fd = -1, len = 0, handle = -1, count = 0, sensor = -1;
    struct sensor_t const *mDynamicSensorList;
    void *lib_handle = NULL;
    mHwGyroSupported = true;

    initSensorList();

    fd = TEMP_FAILURE_RETRY(open("/dev/sensorlist", O_RDWR));
    if (fd < 0) {
        ALOGE("open /dev/sensorlist fail we use defaultsensorlist\n");
        return;
    }
    len = TEMP_FAILURE_RETRY(read(fd, sensorlist_info, sizeof(sensorlist_info)));
    if (len < 0) {
        ALOGE("read /dev/sensorlist fail we use defaultsensorlist\n");
        close(fd);
        return;
    }
    close(fd);
    getListFunc_t getDynamicSensorListFunc = loadSensorCustom(&lib_handle);
    if (!getDynamicSensorListFunc) {
        ALOGE("loadSensorCustom fail we use defaultsensorlist\n");
        if (lib_handle)
            dlclose(lib_handle);
        return;
    }
    count = getDynamicSensorListFunc(&mDynamicSensorList);
    if (count <= 0) {
        ALOGE("getDynamicSensorList fail we use defaultsensorlist\n");
        if (lib_handle)
            dlclose(lib_handle);
        return;
    }
    for (handle = accel; handle < maxhandle; ++handle) {
        len = sizeof(sensorlist_info[handle].name) - 1;
        sensorlist_info[handle].name[len] = '\0';
        ALOGI("sensor=%d, name=%s\n", handle_to_sensor(handle),
            sensorlist_info[handle].name);
        sensor = handle_to_sensor(handle);
        if (sensor < 0)
            continue;
        /* decide whether hwgyroscope support or not */
        if (sensor == ID_GYROSCOPE) {
            if (!strcmp(sensorlist_info[handle].name, "virtual_gyro"))
                mHwGyroSupported = false;
            else if (!strcmp(sensorlist_info[handle].name, "NULL"))
                mHwGyroSupported = false;
            else
                mHwGyroSupported = true;
        }
        if (!strcmp(sensorlist_info[handle].name, "NULL")) {
            removeSensor(sensor);
        } else {
            replaceSensor(sensor,
                findDynamicSensor(sensorlist_info[handle].name,
                    count, mDynamicSensorList));
        }
    }
    if (lib_handle)
        dlclose(lib_handle);
    ALOGI("newSensorListCount=%u\n", static_cast<unsigned int>(mSensorList.size()));
}

SensorList::~SensorList() {
}

size_t SensorList::getSensorList(struct sensor_t const **list) {
    *list = mSensorList.data();
    return mSensorList.size();
}

bool SensorList::hwGyroSupported() {
    return mHwGyroSupported;
}

void SensorList::removeSensor(int sensor) {
    mSensorList.erase(std::remove_if(mSensorList.begin(),
        mSensorList.end(),
        [sensor](struct sensor_t mSensor) {
            if (mSensor.handle - ID_OFFSET == sensor)
                return true;
            else
                return false;
        }), mSensorList.end());
    ALOGI("removeSensor=%d\n", sensor);
}

void SensorList::replaceSensor(int sensor, struct sensor_t const *src) {
    if (!src)
        return;
    std::replace_if(mSensorList.begin(), mSensorList.end(),
        [sensor](struct sensor_t mSensor) {
            if (mSensor.handle - ID_OFFSET == sensor)
                return true;
            else
                return false;
        }, *src);
    ALOGI("replaceSensor=%d\n", sensor);
}

struct sensor_t const *
SensorList::findDynamicSensor(char *name,
        int count, struct sensor_t const *list) {
    for (int i = 0; i < count; ++i) {
        if (!strcmp(name, list[i].vendor))
            return &list[i];
    }
    return NULL;
}

