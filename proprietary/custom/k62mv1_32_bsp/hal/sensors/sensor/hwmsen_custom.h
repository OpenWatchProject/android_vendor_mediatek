/*
 * Copyright (C) 2008 The Android Open Source Project
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

#ifndef __HWMSEN_CUSTOM_H__
#define __HWMSEN_CUSTOM_H__

#define MAX_NUM_SENSORS                 1
#define MSENSOR_AMI_LIB

//#define MAG_CALIBRATION_IN_SENSORHUB
#define ACC_GYRO_CALIBRATION_IN_SENSORHUB
//#define FUSION_ALGORITHM_IN_SENSORHUB
#define VIRTUAL_GYROSCOPE_ALGORITHM

#define ACCELEROMETER                           "ACCELEROMETER"
#define ACCELEROMETER_VENDER                    "MTK"
#define ACCELEROMETER_VERSION                   1
#define ACCELEROMETER_RANGE                     78.4532f
#define ACCELEROMETER_RESOLUTION                0.0012
#define ACCELEROMETER_POWER                     0
#define ACCELEROMETER_MINDELAY                  5000
#define ACCELEROMETER_FIFO_MAX_COUNT            4500
#define ACCELEROMETER_FIFO_RESERVE_COUNT        3000
#define ACCELEROMETER_MAXDELAY                  20000
#define ACCELEROMETER_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE

#define GYROSCOPE               "GYROSCOPE"
#define GYROSCOPE_VENDER            "MTK"
#define GYROSCOPE_VERSION           1
#define GYROSCOPE_RANGE             34.9066f
#define GYROSCOPE_RESOLUTION            0.0011f
#define GYROSCOPE_POWER             0
#define GYROSCOPE_MINDELAY          5000
#define GYROSCOPE_FIFO_MAX_COUNT        0
#define GYROSCOPE_FIFO_RESERVE_COUNT        0
#define GYROSCOPE_MAXDELAY          1000000
#define GYROSCOPE_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE

#define MAGNETOMETER                           "MAGNETOMETER"
#define MAGNETOMETER_VENDER                    "MTK"
#define MAGNETOMETER_VERSION                   1
#define MAGNETOMETER_RANGE                     4912.0f
#define MAGNETOMETER_RESOLUTION                0.15f
#define MAGNETOMETER_POWER                     0
#define MAGNETOMETER_MINDELAY      20000
#define MAGNETOMETER_FIFO_MAX_COUNT            4500
#define MAGNETOMETER_FIFO_RESERVE_COUNT        600
#define MAGNETOMETER_MAXDELAY                  200000
#define MAGNETOMETER_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE

#define ORIENTATION                           "ORIENTATION"
#define ORIENTATION_VENDER                    "MTK"
#define ORIENTATION_VERSION                   1
#define ORIENTATION_RANGE                     360.0f
#define ORIENTATION_RESOLUTION                1.0f/256.0f
#define ORIENTATION_POWER                     0
#define ORIENTATION_MINDELAY                  5000
#define ORIENTATION_FIFO_MAX_COUNT            0
#define ORIENTATION_FIFO_RESERVE_COUNT        0
#define ORIENTATION_MAXDELAY                  20000
#define ORIENTATION_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE

#define LIGHT                           "LIGHT"
#define LIGHT_VENDER                    "MTK"
#define LIGHT_VERSION                   1
#define LIGHT_RANGE                     65535.0f
#define LIGHT_RESOLUTION                1.0f
#define LIGHT_POWER                     0
#define LIGHT_MINDELAY                  0
#define LIGHT_FIFO_MAX_COUNT            0
#define LIGHT_FIFO_RESERVE_COUNT        0
#define LIGHT_MAXDELAY                  1000000
#define LIGHT_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE

#define PRESSURE                           "PRESSURE"
#define PRESSURE_VENDER                    "MTK"
#define PRESSURE_VERSION                   1
#define PRESSURE_RANGE                     1572.86f
#define PRESSURE_RESOLUTION                0.0016
#define PRESSURE_POWER                     0
#define PRESSURE_MINDELAY                  100000
#define PRESSURE_FIFO_MAX_COUNT            4500
#define PRESSURE_FIFO_RESERVE_COUNT        300
#define PRESSURE_MAXDELAY                  1000000
#define PRESSURE_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE

#define PROXIMITY                           "PROXIMITY"
#define PROXIMITY_VENDER                    "MTK"
#define PROXIMITY_VERSION                   1
#define PROXIMITY_RANGE                     1.0f
#define PROXIMITY_RESOLUTION                1.0f
#define PROXIMITY_POWER                     0
#define PROXIMITY_MINDELAY                  0
#define PROXIMITY_FIFO_MAX_COUNT            4500
#define PROXIMITY_FIFO_RESERVE_COUNT        100
#define PROXIMITY_MAXDELAY                  1000000
#define PROXIMITY_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP

#define UNCALI_MAG                           "UNCALI_MAG"
#define UNCALI_MAG_VENDER                    "MTK"
#define UNCALI_MAG_VERSION                   1
#define UNCALI_MAG_RANGE                     4912.0f
#define UNCALI_MAG_RESOLUTION                0.15f
#define UNCALI_MAG_POWER                     0
#define UNCALI_MAG_MINDELAY                  20000
#define UNCALI_MAG_FIFO_MAX_COUNT            4500
#define UNCALI_MAG_FIFO_RESERVE_COUNT        600
#define UNCALI_MAG_MAXDELAY                  200000
#define UNCALI_MAG_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE

#define SIGNIFICANT_MOTION                           "SIGNIFICANT_MOTION"
#define SIGNIFICANT_MOTION_VENDER                    "MTK"
#define SIGNIFICANT_MOTION_VERSION                   1
#define SIGNIFICANT_MOTION_RANGE                     1.0f
#define SIGNIFICANT_MOTION_RESOLUTION                1.0f
#define SIGNIFICANT_MOTION_POWER                     0
#define SIGNIFICANT_MOTION_MINDELAY                  -1
#define SIGNIFICANT_MOTION_FIFO_MAX_COUNT            0
#define SIGNIFICANT_MOTION_FIFO_RESERVE_COUNT        0
#define SIGNIFICANT_MOTION_MAXDELAY                  0
#define SIGNIFICANT_MOTION_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP

#define STEP_DETECTOR                           "STEP_DETECTOR"
#define STEP_DETECTOR_VENDER                    "MTK"
#define STEP_DETECTOR_VERSION                   1
#define STEP_DETECTOR_RANGE                     1.0f
#define STEP_DETECTOR_RESOLUTION                1.0f
#define STEP_DETECTOR_POWER                     0
#define STEP_DETECTOR_MINDELAY                  0
#define STEP_DETECTOR_FIFO_MAX_COUNT            4500
#define STEP_DETECTOR_FIFO_RESERVE_COUNT        100
#define STEP_DETECTOR_MAXDELAY                  0
#define STEP_DETECTOR_FLAGS     SENSOR_FLAG_SPECIAL_REPORTING_MODE

#define STEP_COUNTER                           "STEP_COUNTER"
#define STEP_COUNTER_VENDER                    "MTK"
#define STEP_COUNTER_VERSION                   1
#define STEP_COUNTER_RANGE                     2147483647.0f
#define STEP_COUNTER_RESOLUTION                1.0f
#define STEP_COUNTER_POWER                     0
#define STEP_COUNTER_MINDELAY                  0
#define STEP_COUNTER_FIFO_MAX_COUNT            0
#define STEP_COUNTER_FIFO_RESERVE_COUNT        0
#define STEP_COUNTER_MAXDELAY                  1000000
#define STEP_COUNTER_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE

#define WAKE_GESTURE                           "WAKE_GESTURE"
#define WAKE_GESTURE_VENDER                    "MTK"
#define WAKE_GESTURE_VERSION                   1
#define WAKE_GESTURE_RANGE                     1.0f
#define WAKE_GESTURE_RESOLUTION                1.0f
#define WAKE_GESTURE_POWER                     0
#define WAKE_GESTURE_MINDELAY                  -1
#define WAKE_GESTURE_FIFO_MAX_COUNT            0
#define WAKE_GESTURE_FIFO_RESERVE_COUNT        0
#define WAKE_GESTURE_MAXDELAY                  0
#define WAKE_GESTURE_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP

#define DEVICE_ORIENTATION                           "DEVICE_ORIENTATION"
#define DEVICE_ORIENTATION_VENDER                    "MTK"
#define DEVICE_ORIENTATION_VERSION                   1
#define DEVICE_ORIENTATION_RANGE                     3
#define DEVICE_ORIENTATION_RESOLUTION                1
#define DEVICE_ORIENTATION_POWER                     0
#define DEVICE_ORIENTATION_MINDELAY                  0
#define DEVICE_ORIENTATION_FIFO_MAX_COUNT            0
#define DEVICE_ORIENTATION_FIFO_RESERVE_COUNT        0
#define DEVICE_ORIENTATION_MAXDELAY                  1000000
#define DEVICE_ORIENTATION_FLAGS     SENSOR_FLAG_ON_CHANGE_MODE

#define STATIONARY_DETECT                           "STATIONARY_DETECT"
#define STATIONARY_DETECT_VENDER                    "MTK"
#define STATIONARY_DETECT_VERSION                   1
#define STATIONARY_DETECT_RANGE                     1.0f
#define STATIONARY_DETECT_RESOLUTION                1.0f
#define STATIONARY_DETECT_POWER                     0
#define STATIONARY_DETECT_MINDELAY                  -1
#define STATIONARY_DETECT_FIFO_MAX_COUNT            0
#define STATIONARY_DETECT_FIFO_RESERVE_COUNT        0
#define STATIONARY_DETECT_MAXDELAY                  0
#define STATIONARY_DETECT_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP

#define MOTION_DETECT                           "MOTION_DETECT"
#define MOTION_DETECT_VENDER                    "MTK"
#define MOTION_DETECT_VERSION                   1
#define MOTION_DETECT_RANGE                     1.0f
#define MOTION_DETECT_RESOLUTION                1.0f
#define MOTION_DETECT_POWER                     0
#define MOTION_DETECT_MINDELAY                  -1
#define MOTION_DETECT_FIFO_MAX_COUNT            0
#define MOTION_DETECT_FIFO_RESERVE_COUNT        0
#define MOTION_DETECT_MAXDELAY                  0
#define MOTION_DETECT_FLAGS     SENSOR_FLAG_ONE_SHOT_MODE | SENSOR_FLAG_WAKE_UP

__BEGIN_DECLS
size_t getDynamicSensorList(struct sensor_t const **llist);
__END_DECLS
#endif

