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
#ifndef _ALSPS_H_
#define _ALSPS_H_
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

#define PS_INTERRUPT_MODE   0
#define PS_POLLING_MODE     1

enum AlsPsState {
    CHIP_SAMPLING_ALS,
    CHIP_SAMPLING_ALS_DONE,
    CHIP_SAMPLING_PS,
    CHIP_SAMPLING_PS_ONCE,
    CHIP_SAMPLING_PS_DONE,
    CHIP_ALS_ENABLE,
    CHIP_ALS_ENABLE_DONE,
    CHIP_ALS_DISABLE,
    CHIP_ALS_DISABLE_DONE,
    CHIP_ALS_RATECHG,
    CHIP_ALS_RATECHG_DONE,
    CHIP_ALS_CALI,
    CHIP_ALS_CALI_DONE,
    CHIP_ALS_CFG,
    CHIP_ALS_CFG_DONE,
    CHIP_PS_ENABLE,
    CHIP_PS_ENABLE_DONE,
    CHIP_PS_DISABLE,
    CHIP_PS_DISABLE_DONE,
    CHIP_PS_RATECHG,
    CHIP_PS_RATECHG_DONE,
    CHIP_PS_CALI,
    CHIP_PS_CALI_DONE,
    CHIP_PS_CFG,
    CHIP_PS_CFG_DONE,
    CHIP_RGBW_ENABLE,
    CHIP_RGBW_ENABLE_DONE,
    CHIP_RGBW_DISABLE,
    CHIP_RGBW_DISABLE_DONE,
    CHIP_RGBW_RATECHG,
    CHIP_RGBW_RATECHG_DONE,
    CHIP_INIT_DONE,
    CHIP_IDLE,
    CHIP_RESET,
    CHIP_ALS_RESET = CHIP_RESET,
    CHIP_PS_RESET = 150,
};

enum ProxState {
    PROX_STATE_INIT = -1,
    PROX_STATE_NEAR = 0,
    PROX_STATE_FAR = 1,
};

struct AlsPsData {
    uint8_t sensType;
    union {
        struct {
            uint32_t als_data;  // unit: lux
        };
        struct {
            uint32_t prox_data; // unit: cm
            enum ProxState prox_state;
        };
        struct {
            uint32_t r;
            uint32_t g;
            uint32_t b;
            uint32_t w;
        };
    };
};

struct alspsCaliCfgPacket {
    int32_t caliCfgData[2];
};

void registerPsInterruptMode(uint8_t mode);
void alsPsInterruptOccur(void);
void registerAlsPsDriverFsm(struct sensorFsm *mSensorFsm, uint8_t size);
void alsSensorRegister(void);
void psSensorRegister(void);
void rgbwSensorRegister(void);

void alsPsSendCalibrationResult(uint8_t sensorType, int32_t value[2]);

/* inSize: caliData[3]
 * elemInSize: struct accGyroCntlPacket
 */
static inline int rxCaliCfgInfo(struct alspsCaliCfgPacket *recvPacket, void *inBuf,
    uint8_t inSize, uint8_t elemInSize, void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct alspsCaliCfgPacket *packet = (struct alspsCaliCfgPacket *)inBuf;

    if (inBuf == NULL || inSize != 1 || elemInSize != sizeof(struct alspsCaliCfgPacket)) {
        return -1;
    }
    recvPacket->caliCfgData[0] = packet->caliCfgData[0];
    recvPacket->caliCfgData[1] = packet->caliCfgData[1];
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
                                      uint8_t outSize, struct AlsPsData *data)
{
    *(struct AlsPsData **)dataInfo->outBuf =  data;
    *dataInfo->elemOutSize = sizeof(struct AlsPsData);
    *dataInfo->outSize = outSize;
}
#endif
