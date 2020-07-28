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
#define ACC_GYRO_CALIBRATION_SUPPORT


/*  set sensor .mindelay, ms */
#define MAGNETOMETER_MINDELAY      20000
#define ORIENTATION_MINDELAY       20000
#define ACCELEROMETER_MINDELAY     10000
#define GYROSCOPE_MINDELAY         10000

#define UNCALI_PRESSURE                         "UNCALI_PRESSURE"
#define UNCALI_PRESSURE_VENDER                    "Bosch"
#define UNCALI_PRESSURE_VERSION                   1
#define UNCALI_PRESSURE_RANGE                     1572.86f
#define UNCALI_PRESSURE_RESOLUTION                0.0016
#define UNCALI_PRESSURE_POWER                     0
#define UNCALI_PRESSURE_MINDELAY                  20000
#define UNCALI_PRESSURE_FIFO_MAX_COUNT            0
#define UNCALI_PRESSURE_FIFO_RESERVE_COUNT        0
#define UNCALI_PRESSURE_MAXDELAY                  1000000
#define UNCALI_PRESSURE_FLAGS     SENSOR_FLAG_CONTINUOUS_MODE

#endif

