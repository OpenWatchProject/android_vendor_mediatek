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


#include <log/log.h>

#include <hardware/sensors.h>
#include <linux/hwmsensor.h>
#include "nusensors.h"

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "Sensors_Init"
#endif

#include <hardware/sensors.h>
#include <linux/hwmsensor.h>
#include "hwmsen_custom.h"

typedef enum SENSOR_NUM_DEF
{
    SONSER_UNSUPPORTED = -1,

    #ifdef CUSTOM_KERNEL_ACCELEROMETER
        ACCELEROMETER_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_MAGNETOMETER
        MAGNETOMETER_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_ORIENTATION_SENSOR
        ORIENTATION_NUM,
    #endif

    #if defined(CUSTOM_KERNEL_ALSPS) || defined(CUSTOM_KERNEL_ALS)
        ALS_NUM,
    #endif
    #if defined(CUSTOM_KERNEL_ALSPS) || defined(CUSTOM_KERNEL_PS)
        PS_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_GYROSCOPE
        GYROSCOPE_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_BAROMETER
        PRESSURE_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_TEMPURATURE
        TEMPURATURE_NUM,
    #endif
    #ifdef CUSTOM_KERNEL_HUMIDITY
        HUMIDITY_NUM,
    #endif
    #ifdef CUSTOM_KERNEL_STEP_COUNTER
        STEP_COUNTER_NUM,
        STEP_DETECTOR_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_SIGNIFICANT_MOTION_SENSOR
        STEP_SIGNIFICANT_MOTION_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_PEDOMETER
        PEDOMETER_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_IN_POCKET_SENSOR
        IN_POCKET_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_ACTIVITY_SENSOR
        ACTIVITY_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_PICK_UP_SENSOR
        PICK_UP_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_FACE_DOWN_SENSOR
        FACE_DOWN_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_SHAKE_SENSOR
        SHAKE_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_HEART
        HEART_RATE_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_TILT_DETECTOR_SENSOR
        TILT_DETECTOR_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_WAKE_GESTURE_SENSOR
        WAKE_GESTURE_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_GLANCE_GESTURE_SENSOR
        GLANCE_GESTURE_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_GRV_SENSOR
        GAME_ROTATION_VECTOR_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_GMRV_SENSOR
        GEOMAGNETIC_ROTATION_VECTOR_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_RV_SENSOR
        ROTATION_VECTOR_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_GRAVITY_SENSOR
        GRAVITY_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_LINEARACCEL_SENSOR
        LINEARACCEL_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_BRINGTOSEE_SENSOR
        BRINGTOSEE_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_UNCALI_GYRO_SENSOR
        UNCALI_GYRO_NUM,
    #endif

    #ifdef CUSTOM_KERNEL_UNCALI_MAG_SENSOR
        UNCALI_MAG_NUM,
    #endif
#ifdef CUSTOM_KERNEL_ANSWER_CALL_SENSOR
        ANSWER_CALL_NUM,
#endif
#ifdef CUSTOM_KERNEL_STATIONARY_SENSOR
        STATIONARY_NUM,
#endif
     #ifdef CUSTOM_KERNEL_PDR_SENSOR
        PDR_NUM,
      #endif
    #ifdef CUSTOM_KERNEL_BIOMETRIC_SENSOR
        BIOMETRIC_NUM,
    #endif

    SENSORS_NUM
}SENSOR_NUM_DEF;

#define MAX_NUM_SENSOR      (SENSORS_NUM)


/*--------------------------------------------------------*/
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
        #define GRAVITY_MAXDELAY                  1000000
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
        #define LINEARACCEL_MAXDELAY                  1000000
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
        #define ROTATION_VECTOR_MAXDELAY                  1000000
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
        #define GAME_ROTATION_VECTOR_MAXDELAY                  1000000
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

#ifdef CUSTOM_KERNEL_ACCELEROMETER_UNCALIBRATED
    #ifndef ACCELEROMETER_UNCALIBRATED
        #define ACCELEROMETER_UNCALIBRATED                           "ACCELEROMETER_UNCALIBRATED"
    #endif
    #ifndef ACCELEROMETER_UNCALIBRATED_VENDER
        #define ACCELEROMETER_UNCALIBRATED_VENDER                    "MTK"
    #endif
    #ifndef ACCELEROMETER_UNCALIBRATED_VERSION
        #define ACCELEROMETER_UNCALIBRATED_VERSION                   1
    #endif
    #ifndef ACCELEROMETER_UNCALIBRATED_RANGE
        #define ACCELEROMETER_UNCALIBRATED_RANGE                     39.2266f
    #endif
    #ifndef ACCELEROMETER_UNCALIBRATED_RESOLUTION
        #define ACCELEROMETER_UNCALIBRATED_RESOLUTION                0.0012
    #endif
    #ifndef ACCELEROMETER_UNCALIBRATED_POWER
        #define ACCELEROMETER_UNCALIBRATED_POWER                     0
    #endif
    #ifndef ACCELEROMETER_UNCALIBRATED_MINDELAY
        #define ACCELEROMETER_UNCALIBRATED_MINDELAY                  10000
    #endif
    #ifndef ACCELEROMETER_UNCALIBRATED_FIFO_MAX_COUNT
        #define ACCELEROMETER_UNCALIBRATED_FIFO_MAX_COUNT            0
    #endif
    #ifndef ACCELEROMETER_UNCALIBRATED_FIFO_RESERVE_COUNT
        #define ACCELEROMETER_UNCALIBRATED_FIFO_RESERVE_COUNT        0
    #endif
    #ifndef ACCELEROMETER_UNCALIBRATED_MAXDELAY
        #define ACCELEROMETER_UNCALIBRATED_MAXDELAY                  1000000
    #endif
    #ifndef ACCELEROMETER_UNCALIBRATED_FLAGS
        #define ACCELEROMETER_UNCALIBRATED_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE
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
/*--------------------------------------------------------*/

struct sensor_t sSensorList[] =
{
#ifdef CUSTOM_KERNEL_ACCELEROMETER
    {
        .name       = ACCELEROMETER,
        .vendor     = ACCELEROMETER_VENDER,
        .version    = ACCELEROMETER_VERSION,
        .handle     = ID_ACCELEROMETER+ID_OFFSET,
        .type       = SENSOR_TYPE_ACCELEROMETER,
        .maxRange   = ACCELEROMETER_RANGE,
        .resolution = ACCELEROMETER_RESOLUTION,
        .power      = ACCELEROMETER_POWER,
        .minDelay   = ACCELEROMETER_MINDELAY,
        .fifoReservedEventCount = ACCELEROMETER_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = ACCELEROMETER_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_ACCELEROMETER,
        .maxDelay   = ACCELEROMETER_MAXDELAY,
        .flags      = ACCELEROMETER_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_MAGNETOMETER
    {
        .name       = MAGNETOMETER,
        .vendor     = MAGNETOMETER_VENDER,
        .version    = MAGNETOMETER_VERSION,
        .handle     = ID_MAGNETIC+ID_OFFSET,
        .type       = SENSOR_TYPE_MAGNETIC_FIELD,
        .maxRange   = MAGNETOMETER_RANGE,
        .resolution = MAGNETOMETER_RESOLUTION,
        .power      = MAGNETOMETER_POWER,
        .minDelay   = MAGNETOMETER_MINDELAY,
        .fifoReservedEventCount = MAGNETOMETER_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = MAGNETOMETER_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_MAGNETIC_FIELD,
        .maxDelay   = MAGNETOMETER_MAXDELAY,
        .flags      = MAGNETOMETER_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_ORIENTATION_SENSOR
    {
        .name       = ORIENTATION,
        .vendor     = ORIENTATION_VENDER,
        .version    = ORIENTATION_VERSION,
        .handle     = ID_ORIENTATION+ID_OFFSET,
        .type       = SENSOR_TYPE_ORIENTATION,
        .maxRange   = ORIENTATION_RANGE,
        .resolution = ORIENTATION_RESOLUTION,
        .power      = ORIENTATION_POWER,
        .minDelay   = ORIENTATION_MINDELAY,
        .fifoReservedEventCount = ORIENTATION_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = ORIENTATION_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_ORIENTATION,
        .maxDelay   = ORIENTATION_MAXDELAY,
        .flags      = ORIENTATION_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_GYROSCOPE
    {
        .name       = GYROSCOPE,
        .vendor     = GYROSCOPE_VENDER,
        .version    = GYROSCOPE_VERSION,
        .handle     = ID_GYROSCOPE+ID_OFFSET,
        .type       = SENSOR_TYPE_GYROSCOPE,
        .maxRange   = GYROSCOPE_RANGE,
        .resolution = GYROSCOPE_RESOLUTION,
        .power      = GYROSCOPE_POWER,
        .minDelay   = GYROSCOPE_MINDELAY,
        .fifoReservedEventCount = GYROSCOPE_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = GYROSCOPE_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_GYROSCOPE,
        .maxDelay   = GYROSCOPE_MAXDELAY,
        .flags      = GYROSCOPE_FLAGS,
        .reserved   = {}
    },
#endif

#if defined(CUSTOM_KERNEL_ALSPS) || defined(CUSTOM_KERNEL_ALS)
    {
        .name       = LIGHT,
        .vendor     = LIGHT_VENDER,
        .version    = LIGHT_VERSION,
        .handle     = ID_LIGHT+ID_OFFSET,
        .type       = SENSOR_TYPE_LIGHT,
        .maxRange   = LIGHT_RANGE,
        .resolution = LIGHT_RESOLUTION,
        .power      = LIGHT_POWER,
        .minDelay   = LIGHT_MINDELAY,
        .fifoReservedEventCount = LIGHT_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = LIGHT_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_LIGHT,
        .maxDelay   = LIGHT_MAXDELAY,
        .flags      = LIGHT_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_BAROMETER
    {
        .name       = PRESSURE,
        .vendor     = PRESSURE_VENDER,
        .version    = PRESSURE_VERSION,
        .handle     = ID_PRESSURE+ID_OFFSET,
        .type       = SENSOR_TYPE_PRESSURE,
        .maxRange   = PRESSURE_RANGE,
        .resolution = PRESSURE_RESOLUTION,
        .power      = PRESSURE_POWER,
        .minDelay   = PRESSURE_MINDELAY,
        .fifoReservedEventCount = PRESSURE_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = PRESSURE_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_PRESSURE,
        .maxDelay   = PRESSURE_MAXDELAY,
        .flags      = PRESSURE_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_TEMPURATURE
    {
        .name       = TEMPURATURE,
        .vendor     = TEMPURATURE_VENDER,
        .version    = TEMPURATURE_VERSION,
        .handle     = ID_TEMPURATURE+ID_OFFSET,
        .type       = SENSOR_TYPE_TEMPERATURE,
        .maxRange   = TEMPURATURE_RANGE,
        .resolution = TEMPURATURE_RESOLUTION,
        .power      = TEMPURATURE_POWER,
        .minDelay   = TEMPURATURE_MINDELAY,
        .fifoReservedEventCount = TEMPURATURE_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = TEMPURATURE_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_TEMPERATURE,
        .maxDelay   = TEMPURATURE_MAXDELAY,
        .flags      = TEMPURATURE_FLAGS,
        .reserved   = {}
    },
#endif

#if defined(CUSTOM_KERNEL_ALSPS) || defined(CUSTOM_KERNEL_PS)
    {
        .name       = PROXIMITY,
        .vendor     = PROXIMITY_VENDER,
        .version    = PROXIMITY_VERSION,
        .handle     = ID_PROXIMITY+ID_OFFSET,
        .type       = SENSOR_TYPE_PROXIMITY,
        .maxRange   = PROXIMITY_RANGE,
        .resolution = PROXIMITY_RESOLUTION,
        .power      = PROXIMITY_POWER,
        .minDelay   = PROXIMITY_MINDELAY,
        .fifoReservedEventCount = PROXIMITY_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = PROXIMITY_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_PROXIMITY,
        .maxDelay   = PROXIMITY_MAXDELAY,
        .flags      = PROXIMITY_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_GRAVITY_SENSOR
    {
        .name       = GRAVITY,
        .vendor     = GRAVITY_VENDER,
        .version    = GRAVITY_VERSION,
        .handle     = ID_GRAVITY+ID_OFFSET,
        .type       = SENSOR_TYPE_GRAVITY,
        .maxRange   = GRAVITY_RANGE,
        .resolution = GRAVITY_RESOLUTION,
        .power      = GRAVITY_POWER,
        .minDelay   = GRAVITY_MINDELAY,
        .fifoReservedEventCount = GRAVITY_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = GRAVITY_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_GRAVITY,
        .maxDelay   = GRAVITY_MAXDELAY,
        .flags      = GRAVITY_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_LINEARACCEL_SENSOR
    {
        .name       = LINEARACCEL,
        .vendor     = LINEARACCEL_VENDER,
        .version    = LINEARACCEL_VERSION,
        .handle     = ID_LINEAR_ACCELERATION+ID_OFFSET,
        .type       = SENSOR_TYPE_LINEAR_ACCELERATION,
        .maxRange   = LINEARACCEL_RANGE,
        .resolution = LINEARACCEL_RESOLUTION,
        .power      = LINEARACCEL_POWER,
        .minDelay   = LINEARACCEL_MINDELAY,
        .fifoReservedEventCount = LINEARACCEL_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = LINEARACCEL_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_LINEAR_ACCELERATION,
        .maxDelay   = LINEARACCEL_MAXDELAY,
        .flags      = LINEARACCEL_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_RV_SENSOR
    {
        .name       = ROTATION_VECTOR,
        .vendor     = ROTATION_VECTOR_VENDER,
        .version    = ROTATION_VECTOR_VERSION,
        .handle     = ID_ROTATION_VECTOR+ID_OFFSET,
        .type       = SENSOR_TYPE_ROTATION_VECTOR,
        .maxRange   = ROTATION_VECTOR_RANGE,
        .resolution = ROTATION_VECTOR_RESOLUTION,
        .power      = ROTATION_VECTOR_POWER,
        .minDelay   = ROTATION_VECTOR_MINDELAY,
        .fifoReservedEventCount = ROTATION_VECTOR_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = ROTATION_VECTOR_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_ROTATION_VECTOR,
        .maxDelay   = ROTATION_VECTOR_MAXDELAY,
        .flags      = ROTATION_VECTOR_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_HUMIDITY
    {
        .name       = HUMIDITY,
        .vendor     = HUMIDITY_VENDER,
        .version    = HUMIDITY_VERSION,
        .handle     = ID_RELATIVE_HUMIDITY+ID_OFFSET,
        .type       = SENSOR_TYPE_RELATIVE_HUMIDITY,
        .maxRange   = HUMIDITY_RANGE,
        .resolution = HUMIDITY_RESOLUTION,
        .power      = HUMIDITY_POWER,
        .minDelay   = HUMIDITY_MINDELAY,
        .fifoReservedEventCount = HUMIDITY_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = HUMIDITY_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_RELATIVE_HUMIDITY,
        .maxDelay   = HUMIDITY_MAXDELAY,
        .flags      = HUMIDITY_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_AMBIENT_TEMPERATURE
    {
        .name       = AMBIENT_TEMPERATURE,
        .vendor     = AMBIENT_TEMPERATURE_VENDER,
        .version    = AMBIENT_TEMPERATURE_VERSION,
        .handle     = ID_AMBIENT_TEMPERATURE+ID_OFFSET,
        .type       = SENSOR_TYPE_AMBIENT_TEMPERATURE,
        .maxRange   = AMBIENT_TEMPERATURE_RANGE,
        .resolution = AMBIENT_TEMPERATURE_RESOLUTION,
        .power      = AMBIENT_TEMPERATURE_POWER,
        .minDelay   = AMBIENT_TEMPERATURE_MINDELAY,
        .fifoReservedEventCount = AMBIENT_TEMPERATURE_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = AMBIENT_TEMPERATURE_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_AMBIENT_TEMPERATURE,
        .maxDelay   = AMBIENT_TEMPERATURE_MAXDELAY,
        .flags      = AMBIENT_TEMPERATURE_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_UNCALI_MAG_SENSOR
    {
        .name       = UNCALI_MAG,
        .vendor     = UNCALI_MAG_VENDER,
        .version    = UNCALI_MAG_VERSION,
        .handle     = ID_MAGNETIC_UNCALIBRATED+ID_OFFSET,
        .type       = SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED,
        .maxRange   = UNCALI_MAG_RANGE,
        .resolution = UNCALI_MAG_RESOLUTION,
        .power      = UNCALI_MAG_POWER,
        .minDelay   = UNCALI_MAG_MINDELAY,
        .fifoReservedEventCount = UNCALI_MAG_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = UNCALI_MAG_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_MAGNETIC_FIELD_UNCALIBRATED,
        .maxDelay   = UNCALI_MAG_MAXDELAY,
        .flags      = UNCALI_MAG_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_GRV_SENSOR
    {
        .name       = GAME_ROTATION_VECTOR,
        .vendor     = GAME_ROTATION_VECTOR_VENDER,
        .version    = GAME_ROTATION_VECTOR_VERSION,
        .handle     = ID_GAME_ROTATION_VECTOR+ID_OFFSET,
        .type       = SENSOR_TYPE_GAME_ROTATION_VECTOR,
        .maxRange   = GAME_ROTATION_VECTOR_RANGE,
        .resolution = GAME_ROTATION_VECTOR_RESOLUTION,
        .power      = GAME_ROTATION_VECTOR_POWER,
        .minDelay   = GAME_ROTATION_VECTOR_MINDELAY,
        .fifoReservedEventCount = GAME_ROTATION_VECTOR_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = GAME_ROTATION_VECTOR_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_GAME_ROTATION_VECTOR,
        .maxDelay   = GAME_ROTATION_VECTOR_MAXDELAY,
        .flags      = GAME_ROTATION_VECTOR_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_UNCALI_GYRO_SENSOR
    {
        .name       = UNCALI_GYRO,
        .vendor     = UNCALI_GYRO_VENDER,
        .version    = UNCALI_GYRO_VERSION,
        .handle     = ID_GYROSCOPE_UNCALIBRATED+ID_OFFSET,
        .type       = SENSOR_TYPE_GYROSCOPE_UNCALIBRATED,
        .maxRange   = UNCALI_GYRO_RANGE,
        .resolution = UNCALI_GYRO_RESOLUTION,
        .power      = UNCALI_GYRO_POWER,
        .minDelay   = UNCALI_GYRO_MINDELAY,
        .fifoReservedEventCount = UNCALI_GYRO_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = UNCALI_GYRO_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_GYROSCOPE_UNCALIBRATED,
        .maxDelay   = UNCALI_GYRO_MAXDELAY,
        .flags      = UNCALI_GYRO_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_SIGNIFICANT_MOTION_SENSOR
    {
        .name       = SIGNIFICANT_MOTION,
        .vendor     = SIGNIFICANT_MOTION_VENDER,
        .version    = SIGNIFICANT_MOTION_VERSION,
        .handle     = ID_SIGNIFICANT_MOTION+ID_OFFSET,
        .type       = SENSOR_TYPE_SIGNIFICANT_MOTION,
        .maxRange   = SIGNIFICANT_MOTION_RANGE,
        .resolution = SIGNIFICANT_MOTION_RESOLUTION,
        .power      = SIGNIFICANT_MOTION_POWER,
        .minDelay   = SIGNIFICANT_MOTION_MINDELAY,
        .fifoReservedEventCount = SIGNIFICANT_MOTION_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = SIGNIFICANT_MOTION_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_SIGNIFICANT_MOTION,
        .maxDelay   = SIGNIFICANT_MOTION_MAXDELAY,
        .flags      = SIGNIFICANT_MOTION_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_STEP_COUNTER
    {
        .name       = STEP_DETECTOR,
        .vendor     = STEP_DETECTOR_VENDER,
        .version    = STEP_DETECTOR_VERSION,
        .handle     = ID_STEP_DETECTOR+ID_OFFSET,
        .type       = SENSOR_TYPE_STEP_DETECTOR,
        .maxRange   = STEP_DETECTOR_RANGE,
        .resolution = STEP_DETECTOR_RESOLUTION,
        .power      = STEP_DETECTOR_POWER,
        .minDelay   = STEP_DETECTOR_MINDELAY,
        .fifoReservedEventCount = STEP_DETECTOR_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = STEP_DETECTOR_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_STEP_DETECTOR,
        .maxDelay   = STEP_DETECTOR_MAXDELAY,
        .flags      = STEP_DETECTOR_FLAGS,
        .reserved   = {}
    },

    {
        .name       = STEP_COUNTER,
        .vendor     = STEP_COUNTER_VENDER,
        .version    = STEP_COUNTER_VERSION,
        .handle     = ID_STEP_COUNTER+ID_OFFSET,
        .type       = SENSOR_TYPE_STEP_COUNTER,
        .maxRange   = STEP_COUNTER_RANGE,
        .resolution = STEP_COUNTER_RESOLUTION,
        .power      = STEP_COUNTER_POWER,
        .minDelay   = STEP_COUNTER_MINDELAY,
        .fifoReservedEventCount = STEP_COUNTER_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = STEP_COUNTER_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_STEP_COUNTER,
        .maxDelay   = STEP_COUNTER_MAXDELAY,
        .flags      = STEP_COUNTER_FLAGS,
        .reserved   = {}
    },

    {
        .name       = FLOOR_COUNTER,
        .vendor     = FLOOR_COUNTER_VENDER,
        .version    = FLOOR_COUNTER_VERSION,
        .handle     = ID_FLOOR_COUNTER+ID_OFFSET,
        .type       = SENSOR_TYPE_FLOOR_COUNTER,
        .maxRange   = FLOOR_COUNTER_RANGE,
        .resolution = FLOOR_COUNTER_RESOLUTION,
        .power      = FLOOR_COUNTER_POWER,
        .minDelay   = FLOOR_COUNTER_MINDELAY,
        .fifoReservedEventCount = FLOOR_COUNTER_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = FLOOR_COUNTER_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_FLOOR_COUNTER,
        .maxDelay   = FLOOR_COUNTER_MAXDELAY,
        .flags      = FLOOR_COUNTER_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_GMRV_SENSOR
    {
        .name       = GEOMAGNETIC_ROTATION_VECTOR,
        .vendor     = GEOMAGNETIC_ROTATION_VECTOR_VENDER,
        .version    = GEOMAGNETIC_ROTATION_VECTOR_VERSION,
        .handle     = ID_GEOMAGNETIC_ROTATION_VECTOR+ID_OFFSET,
        .type       = SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR,
        .maxRange   = GEOMAGNETIC_ROTATION_VECTOR_RANGE,
        .resolution = GEOMAGNETIC_ROTATION_VECTOR_RESOLUTION,
        .power      = GEOMAGNETIC_ROTATION_VECTOR_POWER,
        .minDelay   = GEOMAGNETIC_ROTATION_VECTOR_MINDELAY,
        .fifoReservedEventCount = GEOMAGNETIC_ROTATION_VECTOR_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = GEOMAGNETIC_ROTATION_VECTOR_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_GEOMAGNETIC_ROTATION_VECTOR,
        .maxDelay   = GEOMAGNETIC_ROTATION_VECTOR_MAXDELAY,
        .flags      = GEOMAGNETIC_ROTATION_VECTOR_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_HEART
    {
        .name       = HEART_RATE,
        .vendor     = HEART_RATE_VENDER,
        .version    = HEART_RATE_VERSION,
        .handle     = ID_HEART_RATE+ID_OFFSET,
        .type       = SENSOR_TYPE_HEART_RATE,
        .maxRange   = HEART_RATE_RANGE,
        .resolution = HEART_RATE_RESOLUTION,
        .power      = HEART_RATE_POWER,
        .minDelay   = HEART_RATE_MINDELAY,
        .fifoReservedEventCount = HEART_RATE_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = HEART_RATE_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_HEART_RATE,
        .maxDelay   = HEART_RATE_MAXDELAY,
        .flags      = HEART_RATE_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_TILT_DETECTOR_SENSOR
    {
        .name       = TILT_DETECTOR,
        .vendor     = TILT_DETECTOR_VENDER,
        .version    = TILT_DETECTOR_VERSION,
        .handle     = ID_TILT_DETECTOR+ID_OFFSET,
        .type       = SENSOR_TYPE_TILT_DETECTOR,
        .maxRange   = TILT_DETECTOR_RANGE,
        .resolution = TILT_DETECTOR_RESOLUTION,
        .power      = TILT_DETECTOR_POWER,
        .minDelay   = TILT_DETECTOR_MINDELAY,
        .fifoReservedEventCount = TILT_DETECTOR_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = TILT_DETECTOR_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_TILT_DETECTOR,
        .maxDelay   = TILT_DETECTOR_MAXDELAY,
        .flags      = TILT_DETECTOR_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_WAKE_GESTURE_SENSOR
    {
        .name       = WAKE_GESTURE,
        .vendor     = WAKE_GESTURE_VENDER,
        .version    = WAKE_GESTURE_VERSION,
        .handle     = ID_WAKE_GESTURE+ID_OFFSET,
        .type       = SENSOR_TYPE_WAKE_GESTURE,
        .maxRange   = WAKE_GESTURE_RANGE,
        .resolution = WAKE_GESTURE_RESOLUTION,
        .power      = WAKE_GESTURE_POWER,
        .minDelay   = WAKE_GESTURE_MINDELAY,
        .fifoReservedEventCount = WAKE_GESTURE_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = WAKE_GESTURE_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_WAKE_GESTURE,
        .maxDelay   = WAKE_GESTURE_MAXDELAY,
        .flags      = WAKE_GESTURE_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_GLANCE_GESTURE_SENSOR
    {
        .name       = GLANCE_GESTURE,
        .vendor     = GLANCE_GESTURE_VENDER,
        .version    = GLANCE_GESTURE_VERSION,
        .handle     = ID_GLANCE_GESTURE+ID_OFFSET,
        .type       = SENSOR_TYPE_GLANCE_GESTURE,
        .maxRange   = GLANCE_GESTURE_RANGE,
        .resolution = GLANCE_GESTURE_RESOLUTION,
        .power      = GLANCE_GESTURE_POWER,
        .minDelay   = GLANCE_GESTURE_MINDELAY,
        .fifoReservedEventCount = GLANCE_GESTURE_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = GLANCE_GESTURE_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_GLANCE_GESTURE,
        .maxDelay   = GLANCE_GESTURE_MAXDELAY,
        .flags      = GLANCE_GESTURE_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_PICK_UP_SENSOR
    {
        .name       = PICK_UP,
        .vendor     = PICK_UP_VENDER,
        .version    = PICK_UP_VERSION,
        .handle     = ID_PICK_UP_GESTURE+ID_OFFSET,
        .type       = SENSOR_TYPE_PICK_UP_GESTURE,
        .maxRange   = PICK_UP_RANGE,
        .resolution = PICK_UP_RESOLUTION,
        .power      = PICK_UP_POWER,
        .minDelay   = PICK_UP_MINDELAY,
        .fifoReservedEventCount = PICK_UP_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = PICK_UP_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_PICK_UP_GESTURE,
        .maxDelay   = PICK_UP_MAXDELAY,
        .flags      = PICK_UP_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_WRIST_TILT
    {
        .name       = WRIST_TILT,
        .vendor     = WRIST_TILT_VENDER,
        .version    = WRIST_TILT_VERSION,
        .handle     = ID_WRIST_TILT+ID_OFFSET,
        .type       = SENSOR_TYPE_WRIST_TILT_GESTURE,
        .maxRange   = WRIST_TILT_RANGE,
        .resolution = WRIST_TILT_RESOLUTION,
        .power      = WRIST_TILT_POWER,
        .minDelay   = WRIST_TILT_MINDELAY,
        .fifoReservedEventCount = WRIST_TILT_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = WRIST_TILT_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_WRIST_TILT_GESTURE,
        .maxDelay   = WRIST_TILT_MAXDELAY,
        .flags      = WRIST_TILT_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_DEVICE_ORIENTATION
    {
        .name       = DEVICE_ORIENTATION,
        .vendor     = DEVICE_ORIENTATION_VENDER,
        .version    = DEVICE_ORIENTATION_VERSION,
        .handle     = ID_DEVICE_ORIENTATION+ID_OFFSET,
        .type       = SENSOR_TYPE_DEVICE_ORIENTATION,
        .maxRange   = DEVICE_ORIENTATION_RANGE,
        .resolution = DEVICE_ORIENTATION_RESOLUTION,
        .power      = DEVICE_ORIENTATION_POWER,
        .minDelay   = DEVICE_ORIENTATION_MINDELAY,
        .fifoReservedEventCount = DEVICE_ORIENTATION_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = DEVICE_ORIENTATION_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_DEVICE_ORIENTATION,
        .maxDelay   = DEVICE_ORIENTATION_MAXDELAY,
        .flags      = DEVICE_ORIENTATION_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_POSE_6DOF
    {
        .name       = POSE_6DOF,
        .vendor     = POSE_6DOF_VENDER,
        .version    = POSE_6DOF_VERSION,
        .handle     = ID_POSE_6DOF+ID_OFFSET,
        .type       = SENSOR_TYPE_POSE_6DOF,
        .maxRange   = POSE_6DOF_RANGE,
        .resolution = POSE_6DOF_RESOLUTION,
        .power      = POSE_6DOF_POWER,
        .minDelay   = POSE_6DOF_MINDELAY,
        .fifoReservedEventCount = POSE_6DOF_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = POSE_6DOF_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_POSE_6DOF,
        .maxDelay   = POSE_6DOF_MAXDELAY,
        .flags      = POSE_6DOF_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_STATIONARY_SENSOR
    {
        .name       = STATIONARY_DETECT,
        .vendor     = STATIONARY_DETECT_VENDER,
        .version    = STATIONARY_DETECT_VERSION,
        .handle     = ID_STATIONARY_DETECT+ID_OFFSET,
        .type       = SENSOR_TYPE_STATIONARY_DETECT,
        .maxRange   = STATIONARY_DETECT_RANGE,
        .resolution = STATIONARY_DETECT_RESOLUTION,
        .power      = STATIONARY_DETECT_POWER,
        .minDelay   = STATIONARY_DETECT_MINDELAY,
        .fifoReservedEventCount = STATIONARY_DETECT_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = STATIONARY_DETECT_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_STATIONARY_DETECT,
        .maxDelay   = STATIONARY_DETECT_MAXDELAY,
        .flags      = STATIONARY_DETECT_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_MOTION_DETECT
    {
        .name       = MOTION_DETECT,
        .vendor     = MOTION_DETECT_VENDER,
        .version    = MOTION_DETECT_VERSION,
        .handle     = ID_MOTION_DETECT+ID_OFFSET,
        .type       = SENSOR_TYPE_MOTION_DETECT,
        .maxRange   = MOTION_DETECT_RANGE,
        .resolution = MOTION_DETECT_RESOLUTION,
        .power      = MOTION_DETECT_POWER,
        .minDelay   = MOTION_DETECT_MINDELAY,
        .fifoReservedEventCount = MOTION_DETECT_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = MOTION_DETECT_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_MOTION_DETECT,
        .maxDelay   = MOTION_DETECT_MAXDELAY,
        .flags      = MOTION_DETECT_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_HEART_BEAT
    {
        .name       = HEART_BEAT,
        .vendor     = HEART_BEAT_VENDER,
        .version    = HEART_BEAT_VERSION,
        .handle     = ID_HEART_BEAT+ID_OFFSET,
        .type       = SENSOR_TYPE_HEART_BEAT,
        .maxRange   = HEART_BEAT_RANGE,
        .resolution = HEART_BEAT_RESOLUTION,
        .power      = HEART_BEAT_POWER,
        .minDelay   = HEART_BEAT_MINDELAY,
        .fifoReservedEventCount = HEART_BEAT_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = HEART_BEAT_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_HEART_BEAT,
        .maxDelay   = HEART_BEAT_MAXDELAY,
        .flags      = HEART_BEAT_FLAGS,
        .reserved   = {}
    },
#endif

/* MTK sensor type */

#ifdef CUSTOM_KERNEL_PEDOMETER
    {
        .name       = PEDOMETER,
        .vendor     = PEDOMETER_VENDER,
        .version    = PEDOMETER_VERSION,
        .handle     = ID_PEDOMETER+ID_OFFSET,
        .type       = SENSOR_TYPE_PEDOMETER,
        .maxRange   = PEDOMETER_RANGE,
        .resolution = PEDOMETER_RESOLUTION,
        .power      = PEDOMETER_POWER,
        .minDelay   = PEDOMETER_MINDELAY,
        .fifoReservedEventCount = PEDOMETER_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = PEDOMETER_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_PEDOMETER,
        .maxDelay   = PEDOMETER_MAXDELAY,
        .flags      = PEDOMETER_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_IN_POCKET_SENSOR
    {
        .name       = IN_POCKET,
        .vendor     = IN_POCKET_VENDER,
        .version    = IN_POCKET_VERSION,
        .handle     = ID_IN_POCKET+ID_OFFSET,
        .type       = SENSOR_TYPE_IN_POCKET,
        .maxRange   = IN_POCKET_RANGE,
        .resolution = IN_POCKET_RESOLUTION,
        .power      = IN_POCKET_POWER,
        .minDelay   = IN_POCKET_MINDELAY,
        .fifoReservedEventCount = IN_POCKET_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = IN_POCKET_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_IN_POCKET,
        .maxDelay   = IN_POCKET_MAXDELAY,
        .flags      = IN_POCKET_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_ACTIVITY_SENSOR
    {
        .name       = ACTIVITY,
        .vendor     = ACTIVITY_VENDER,
        .version    = ACTIVITY_VERSION,
        .handle     = ID_ACTIVITY+ID_OFFSET,
        .type       = SENSOR_TYPE_ACTIVITY,
        .maxRange   = ACTIVITY_RANGE,
        .resolution = ACTIVITY_RESOLUTION,
        .power      = ACTIVITY_POWER,
        .minDelay   = ACTIVITY_MINDELAY,
        .fifoReservedEventCount = ACTIVITY_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = ACTIVITY_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_ACTIVITY,
        .maxDelay   = ACTIVITY_MAXDELAY,
        .flags      = ACTIVITY_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_PDR_SENSOR //??need commit
    {
        .name       = PDR,
        .vendor     = PDR_VENDER,
        .version    = PDR_VERSION,
        .handle     = ID_PDR+ID_OFFSET,
        .type       = SENSOR_TYPE_PDR,
        .maxRange   = PDR_RANGE,
        .resolution = PDR_RESOLUTION,
        .power      = PDR_POWER,
        .minDelay   = PDR_MINDELAY,
        .fifoReservedEventCount = PDR_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = PDR_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_PDR,
        .maxDelay   = PDR_MAXDELAY,
        .flags      = PDR_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_FREEFALL
    {
        .name       = FREEFALL,
        .vendor     = FREEFALL_VENDER,
        .version    = FREEFALL_VERSION,
        .handle     = ID_FREEFALL+ID_OFFSET,
        .type       = SENSOR_TYPE_FREEFALL,
        .maxRange   = FREEFALL_RANGE,
        .resolution = FREEFALL_RESOLUTION,
        .power      = FREEFALL_POWER,
        .minDelay   = FREEFALL_MINDELAY,
        .fifoReservedEventCount = FREEFALL_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = FREEFALL_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_FREEFALL,
        .maxDelay   = FREEFALL_MAXDELAY,
        .flags      = FREEFALL_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_ACCELEROMETER_UNCALIBRATED
    {
        .name       = ACCELEROMETER_UNCALIBRATED,
        .vendor     = ACCELEROMETER_UNCALIBRATED_VENDER,
        .version    = ACCELEROMETER_UNCALIBRATED_VERSION,
        .handle     = ID_ACCELEROMETER_UNCALIBRATED+ID_OFFSET,
        .type       = SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED,
        .maxRange   = ACCELEROMETER_UNCALIBRATED_RANGE,
        .resolution = ACCELEROMETER_UNCALIBRATED_RESOLUTION,
        .power      = ACCELEROMETER_UNCALIBRATED_POWER,
        .minDelay   = ACCELEROMETER_UNCALIBRATED_MINDELAY,
        .fifoReservedEventCount = ACCELEROMETER_UNCALIBRATED_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = ACCELEROMETER_UNCALIBRATED_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_ACCELEROMETER_UNCALIBRATED,
        .maxDelay   = ACCELEROMETER_UNCALIBRATED_MAXDELAY,
        .flags      = ACCELEROMETER_UNCALIBRATED_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_FACE_DOWN_SENSOR
    {
        .name       = FACE_DOWN,
        .vendor     = FACE_DOWN_VENDER,
        .version    = FACE_DOWN_VERSION,
        .handle     = ID_FACE_DOWN+ID_OFFSET,
        .type       = SENSOR_TYPE_FACE_DOWN,
        .maxRange   = FACE_DOWN_RANGE,
        .resolution = FACE_DOWN_RESOLUTION,
        .power      = FACE_DOWN_POWER,
        .minDelay   = FACE_DOWN_MINDELAY,
        .fifoReservedEventCount = FACE_DOWN_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = FACE_DOWN_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_FACE_DOWN,
        .maxDelay   = FACE_DOWN_MAXDELAY,
        .flags      = FACE_DOWN_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_SHAKE_SENSOR
    {
        .name       = SHAKE,
        .vendor     = SHAKE_VENDER,
        .version    = SHAKE_VERSION,
        .handle     = ID_SHAKE+ID_OFFSET,
        .type       = SENSOR_TYPE_SHAKE,
        .maxRange   = SHAKE_RANGE,
        .resolution = SHAKE_RESOLUTION,
        .power      = SHAKE_POWER,
        .minDelay   = SHAKE_MINDELAY,
        .fifoReservedEventCount = SHAKE_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = SHAKE_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_SHAKE,
        .maxDelay   = SHAKE_MAXDELAY,
        .flags      = SHAKE_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_BRINGTOSEE_SENSOR
    {
        .name       = BRINGTOSEE,
        .vendor     = BRINGTOSEE_VENDER,
        .version    = BRINGTOSEE_VERSION,
        .handle     = ID_BRINGTOSEE+ID_OFFSET,
        .type       = SENSOR_TYPE_BRINGTOSEE,
        .maxRange   = BRINGTOSEE_RANGE,
        .resolution = BRINGTOSEE_RESOLUTION,
        .power      = BRINGTOSEE_POWER,
        .minDelay   = BRINGTOSEE_MINDELAY,
        .fifoReservedEventCount = BRINGTOSEE_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = BRINGTOSEE_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_BRINGTOSEE,
        .maxDelay   = BRINGTOSEE_MAXDELAY,
        .flags      = BRINGTOSEE_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_ANSWER_CALL_SENSOR
    {
        .name       = ANSWER_CALL,
        .vendor     = ANSWER_CALL_VENDER,
        .version    = ANSWER_CALL_VERSION,
        .handle     = ID_ANSWER_CALL+ID_OFFSET,
        .type       = SENSOR_TYPE_ANSWER_CALL,
        .maxRange   = ANSWER_CALL_RANGE,
        .resolution = ANSWER_CALL_RESOLUTION,
        .power      = ANSWER_CALL_POWER,
        .minDelay   = ANSWER_CALL_MINDELAY,
        .fifoReservedEventCount = ANSWER_CALL_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = ANSWER_CALL_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_ANSWERCALL,
        .maxDelay   = ANSWER_CALL_MAXDELAY,
        .flags      = ANSWER_CALL_FLAGS,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_BIOMETRIC_SENSOR
    {
        .name       = EKG,
        .vendor     = EKG_VENDER,
        .version    = 1,
        .handle     = ID_EKG+ID_OFFSET,
        .type       = SENSOR_TYPE_EKG,
        .maxRange   = EKG_RANGE,
        .resolution = EKG_RESOLUTION,
        .power      = EKG_POWER,
        .minDelay   = EKG_MINDELAY,
        .fifoReservedEventCount = EKG_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = EKG_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_EKG,
        .flags      = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved   = {}
    },
    {
        .name       = PPG1,
        .vendor     = PPG1_VENDER,
        .version    = 1,
        .handle     = ID_PPG1+ID_OFFSET,
        .type       = SENSOR_TYPE_PPG1,
        .maxRange   = PPG1_RANGE,
        .resolution = PPG1_RESOLUTION,
        .power      = PPG1_POWER,
        .minDelay   = PPG1_MINDELAY,
        .fifoReservedEventCount = PPG1_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = PPG1_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_PPG1,
        .flags      = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved   = {}
    },
    {
        .name       = PPG2,
        .vendor     = PPG2_VENDER,
        .version    = 1,
        .handle     = ID_PPG2+ID_OFFSET,
        .type       = SENSOR_TYPE_PPG2,
        .maxRange   = PPG2_RANGE,
        .resolution = PPG2_RESOLUTION,
        .power      = PPG2_POWER,
        .minDelay   = PPG2_MINDELAY,
        .fifoReservedEventCount = PPG2_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = PPG2_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_PPG2,
        .flags      = SENSOR_FLAG_CONTINUOUS_MODE,
        .reserved   = {}
    },
#endif

#ifdef CUSTOM_KERNEL_RGBW_SENSOR
    {
        .name       = RGBW,
        .vendor     = RGBW_VENDER,
        .version    = RGBW_VERSION,
        .handle     = ID_RGBW + ID_OFFSET,
        .type       = SENSOR_TYPE_RGBW,
        .maxRange   = RGBW_RANGE,
        .resolution = RGBW_RESOLUTION,
        .power      = RGBW_POWER,
        .minDelay   = RGBW_MINDELAY,
        .fifoReservedEventCount = RGBW_FIFO_RESERVE_COUNT,
        .fifoMaxEventCount = RGBW_FIFO_MAX_COUNT,
        .stringType = SENSOR_STRING_TYPE_RGBW,
        .maxDelay   = RGBW_MAXDELAY,
        .flags      = RGBW_FLAGS,
        .reserved   = {}
    },
#endif
};

/*****************************************************************************/

/*
 * The SENSORS Module
 */

/*
 * the AK8973 has a 8-bit ADC but the firmware seems to average 16 samples,
 * or at least makes its calibration on 12-bits values. This increases the
 * resolution by 4 bits.
 */

//extern  struct sensor_t sSensorList[MAX_NUM_SENSOR];


static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static int sensors__get_sensors_list(struct sensors_module_t* module,
        struct sensor_t const** list)
{
    *list = sSensorList;
    return ARRAY_SIZE(sSensorList);
}


static struct hw_module_methods_t sensors_module_methods = {
    .open = open_sensors
};

struct sensors_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = SENSORS_HARDWARE_MODULE_ID,
        .name = "MTK SENSORS Module",
        .author = "Mediatek",
        .methods = &sensors_module_methods,
    },
    .get_sensors_list = sensors__get_sensors_list,
};

/*****************************************************************************/

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
   ALOGI("%s: name: %s! fwq debug\r\n", __func__, name);

   return init_nusensors(module, device);
}
