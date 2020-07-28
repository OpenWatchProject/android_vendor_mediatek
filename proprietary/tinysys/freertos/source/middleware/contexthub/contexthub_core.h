
/*
 * Copyright (c) 2008 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __CONTEXTHUB_CORE_H__
#define __CONTEXTHUB_CORE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <contexthub_fw.h>
#include "hwsen.h"

#define AXIS_X             0
#define AXIS_Y             1
#define AXIS_Z             2
#define AXES_NUM           3

struct sensorCoreInfo {
    uint8_t sensType;
    uint32_t gain;
    float sensitivity;
    struct sensorDriverConvert   cvt;
    void (*setCalibration)(int32_t *cali, int32_t size);
    void (*getCalibration)(int32_t *cali, int32_t size);
    void (*setThreshold)(uint32_t threshold_high, uint32_t threshold_low);
    void (*getThreshold)(uint32_t *threshold_high, uint32_t *threshold_low);
    void (*getData)(void *sample);
    void (*setDebugTrace)(int32_t trace);
    void (*getSensorInfo)(struct sensorInfo_t *data);
};
void sensorCoreInit(void);
int sensorCoreRegister(struct sensorCoreInfo *mInfo);
int sensorCoreResetCalibration(uint8_t sensType);
int sensorCoreWriteCalibration(uint8_t sensType, int32_t *dat);
int sensorCoreSetThreshold(uint8_t sensType, int32_t *data);
int sensorCoreGetData(uint8_t sensType, struct data_unit_t *data);
int sensorCoreSetDebugTrace(uint8_t sensType, int32_t trace);
int sensorCoreGetSensorInfo(uint8_t sensType, struct sensorInfo_t *data);

#endif

