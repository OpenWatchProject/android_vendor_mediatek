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
#ifndef _BAROMETER_H_
#define _BAROMETER_H_
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

#define BARO_INTERRUPTIBLE   1
#define BARO_UNINTERRUPTIBLE 2
#define MAX_RECV_PACKET 64
enum BaroTempState {
    CHIP_SAMPLING = 0,
    CHIP_CONVERT = 1,
    CHIP_SAMPLING_DONE = 2,
    CHIP_ENABLE = 3,
    CHIP_ENABLE_DONE = 4,
    CHIP_DISABLE = 5,
    CHIP_DISABLE_DONE = 6,
    CHIP_RATECHG = 7,
    CHIP_RATECHG_DONE = 8,
    CHIP_INIT_DONE = 9,
    CHIP_IDLE = 10,
    CHIP_RESET = 11,
};
struct baroData {
    uint8_t sensType;
    float fdata;
};
struct baroDataPacket {
    struct baroData outBuf[MAX_RECV_PACKET];
};
struct baroCntlPacket {
    uint32_t rate;
    uint32_t magicNum;
};
void baroInterruptOccur(void);
void baroRegisterInterruptMode(uint8_t mode);
void registerBaroDriverFsm(struct sensorFsm *mSensorFsm, uint8_t size);
void baroSensorRegister(void);
/* inSize: rate
 * elemInSize: struct baroCntlPacket
 */
static inline int rxControlInfo(struct baroCntlPacket *recvPacket, void *inBuf,
    uint8_t inSize, uint8_t elemInSize, void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct baroCntlPacket *packet = (struct baroCntlPacket *)inBuf;

    if (inBuf == NULL || inSize != 1 || elemInSize != sizeof(struct baroCntlPacket)) {
        return -1;
    }
    recvPacket->rate = packet->rate;
    return 0;
}
/*
 * outBuf: driver genarate
 * outSize: driver genarate
 * elemOutSize: struct baroData
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
static inline void txTransferDataInfo(struct transferDataInfo *dataInfo, uint8_t outSize, const struct baroData *data)
{
    struct baroDataPacket *packet = container_of(data, struct baroDataPacket, outBuf);
    struct baroDataPacket **pPacket = (struct baroDataPacket **)dataInfo->outBuf;

    if (packet == NULL)
        configASSERT(0);
    *dataInfo->elemOutSize = sizeof(struct baroData);
    *dataInfo->outSize = outSize;
    if (pPacket != NULL)
        *pPacket = packet;
    else
        configASSERT(0);
}

#endif
