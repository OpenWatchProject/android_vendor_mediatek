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
#include <hardware/sensors.h>
#include <linux/hwmsensor.h>
#include "hwmsen_custom.h"

static struct sensor_t dynamicSensorList[] = {
    {
        .name       = MAGNETOMETER,
        .vendor     = "akm",
        .version    = MAGNETOMETER_VERSION,
        .handle     = ID_MAGNETIC + ID_OFFSET,
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
    {
        .name       = ACCELEROMETER,
        .vendor     = "lis2hh12",
        .version    = ACCELEROMETER_VERSION,
        .handle     = ID_ACCELEROMETER + ID_OFFSET,
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
    {
        .name       = ACCELEROMETER,
        .vendor     = "lis3dh",
        .version    = ACCELEROMETER_VERSION,
        .handle     = ID_ACCELEROMETER + ID_OFFSET,
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
    {
        .name       = GYROSCOPE,
        .vendor     = "virtual_gyro",
        .version    = GYROSCOPE_VERSION,
        .handle     = ID_GYROSCOPE + ID_OFFSET,
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
    {
        .name       = PROXIMITY,
        .vendor     = "cm36558_p",
        .version    = PROXIMITY_VERSION,
        .handle     = ID_PROXIMITY + ID_OFFSET,
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
    {
        .name       = LIGHT,
        .vendor     = "cm36558_l",
        .version    = LIGHT_VERSION,
        .handle     = ID_LIGHT + ID_OFFSET,
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
    {
        .name       = PROXIMITY,
        .vendor     = "stk3321_p",
        .version    = PROXIMITY_VERSION,
        .handle     = ID_PROXIMITY + ID_OFFSET,
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
    {
        .name       = LIGHT,
        .vendor     = "stk3321_l",
        .version    = LIGHT_VERSION,
        .handle     = ID_LIGHT + ID_OFFSET,
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
    {
        .name       = PRESSURE,
        .vendor     = "bmp280",
        .version    = PRESSURE_VERSION,
        .handle     = ID_PRESSURE + ID_OFFSET,
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
    },
};

__BEGIN_DECLS
size_t getDynamicSensorList(struct sensor_t const **llist)
{
    *llist = dynamicSensorList;
    return sizeof(dynamicSensorList) / sizeof(dynamicSensorList[0]);
}
__END_DECLS
