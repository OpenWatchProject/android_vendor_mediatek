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
#ifndef _MAGNETOMETER_H_
#define _MAGNETOMETER_H_
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <sensorFsm.h>
#include <module.h>
#include <scp_err_info.h>
#include <sensorErrInfo.h>
#include "cache_internal.h"

#define MAG_FIFO_INTERRUPTIBLE   1
#define MAG_FIFO_UNINTERRUPTIBLE 2
#define MAG_UNFIFO               3
#define MAX_RECV_PACKET          32
#define AXIS_X               0
#define AXIS_Y               1
#define AXIS_Z               2
#define AXES_NUM             3
enum MagState {
    CHIP_SAMPLING = 0,
    CHIP_CONVERT,
    CHIP_SAMPLING_DONE,
    CHIP_ENABLE,
    CHIP_ENABLE_DONE,
    CHIP_DISABLE,
    CHIP_DISABLE_DONE,
    CHIP_RATECHG,
    CHIP_RATECHG_DONE,
    CHIP_SELFTEST,
    CHIP_SELFTEST_DONE,
    CHIP_INIT_DONE,
    CHIP_IDLE,
    CHIP_RESET,
};
struct magData {
    float x, y, z;
};
struct magDataPacket {
    uint64_t timeStamp;
    struct magData outBuf[MAX_RECV_PACKET];
    uint32_t magicNum;
};
struct magCntlPacket {
    uint32_t rate;
    uint32_t waterMark;
    uint64_t latency;
    uint32_t magicNum;
};
struct magCaliDataOutPut {
    float x, y, z;
    float x_bias, y_bias, z_bias;
    int8_t status;
};
struct magCaliDataInPut {
    int64_t timeStamp;
    float x, y, z;
};

#define MAX_MAG_TIMER_LENGTH 4

struct magTimeStampBuffer {
    uint8_t head;
    uint8_t tail;
    uint8_t filledSize;
    uint64_t buffer[MAX_MAG_TIMER_LENGTH];
};

struct magCalibrationLibAPI {
    int (*initLib)(int hwGyroSupported);
    int (*caliApiGetOffset)(float offset[AXES_NUM]);
    int (*caliApiSetOffset)(float offset[AXES_NUM]);
    int (*caliApiGetCaliParam)(int32_t caliParameter[6]);
    int (*caliApiSetCaliParam)(int32_t caliParameter[6]);
    int (*caliApiSetGyroData)(struct magCaliDataInPut *inputData);
    int (*doCaliApi)(struct magCaliDataInPut *inputData,
        struct magCaliDataOutPut *outputData);
};
typedef void (*magTimerCbkF)(uint64_t);
void magInterruptOccur(void);
void magRegisterInterruptMode(uint8_t mode);
void registerMagDriverFsm(struct sensorFsm *mSensorFsm, uint8_t size);
void registerMagCaliAPI(struct magCalibrationLibAPI *API);
void registerMagTimerCbk(magTimerCbkF cbk);
int8_t magTimeBufferWrite(struct magTimeStampBuffer *record, uint64_t inTime);
int8_t magTimeBufferRead(struct magTimeStampBuffer *record, uint64_t *outTime);
uint8_t magTimeBufferSize(struct magTimeStampBuffer *record);
void magTimeBufferReset(struct magTimeStampBuffer *record);
uint64_t addThenRetreiveAverageMagTimeStamp(uint64_t newTime);
void resetMagTimeStamp(void);
void magSensorRegister(void);
void magSendTestResult(uint8_t status);
static inline void insertMagicNum(struct magDataPacket *packet)
{
    packet->magicNum = 0xdeadbeef;
}
static inline bool detectMagicNumOverflowed(struct magDataPacket *packet)
{
    if (packet->magicNum == 0xdeadbeef)
        return false;
    else
        return true;
}
/* inSize: rate and watermark
 * elemInSize: struct magCntlPacket
 */
static inline int rxControlInfo(struct magCntlPacket *recvPacket, void *inBuf,
    uint8_t inSize, uint8_t elemInSize, void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct magCntlPacket *packet = (struct magCntlPacket *)inBuf;

    if (inBuf == NULL || inSize != 1 || elemInSize != sizeof(struct magCntlPacket)) {
        return -1;
    }
    recvPacket->rate = packet->rate;
    recvPacket->waterMark = packet->waterMark;
    recvPacket->latency = packet->latency;
    return 0;
}
/*
 * outBuf: driver genarate
 * outSize: driver genarate
 * elemOutSize: struct magData
 */
static inline int rxTransferDataInfo(struct transferDataInfo *dataInfo, void *inBuf, uint8_t inSize,
    uint8_t elemInSize, void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    if (outBuf == NULL || outSize == NULL || elemOutSize == NULL) {
        return -1;
    }
    dataInfo->inBuf = inBuf;
    dataInfo->inSize = inSize;
    dataInfo->elemInSize = elemInSize;
    dataInfo->outBuf = outBuf;
    dataInfo->outSize = outSize;
    dataInfo->elemOutSize = elemOutSize;
    return 0;
}
/* inSize: MAX_RECV_PACKET
 */
static inline void txTransferDataInfo(struct transferDataInfo *dataInfo,
    uint8_t outSize, uint64_t sampleTime, const struct magData *data)
{
    struct magDataPacket *packet = container_of(data, struct magDataPacket, outBuf);
    struct magDataPacket **pPacket = (struct magDataPacket **)dataInfo->outBuf;

    if (packet == NULL || detectMagicNumOverflowed(packet))
        configASSERT(0);
    *dataInfo->elemOutSize = sizeof(struct magData);
    *dataInfo->outSize = outSize;
    packet->timeStamp = sampleTime;
    if (pPacket != NULL)
        *pPacket = packet;
    else
        configASSERT(0);
}
#endif
