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
#ifndef _ACCGYRO_H_
#define _ACCGYRO_H_
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>
#include <sensorFsm.h>
#include <module.h>
#include <scp_err_info.h>
#include <sensorErrInfo.h>
#ifdef CFG_MTK_CALIBRATION_V1_0
#include <common/math/macros.h>
#include <calibration/gyroscope/gyro_cal.h>
#include <calibration/over_temp/over_temp_cal.h>
#endif

#define ACC_GYRO_FIFO_INTERRUPTIBLE   1
#define GYRO_ONLY_FIFO_INTERRUPTIBLE  2
#define ACC_GYRO_FIFO_UNINTERRUPTIBLE 3
#define AXIS_X              0
#define AXIS_Y              1
#define AXIS_Z              2
#define AXES_NUM            3
#define MAX_RECV_PACKET     100
enum AccGyroState {
    CHIP_SAMPLING = 0,
    CHIP_FIFO,
    CHIP_CONVERT,
    CHIP_SAMPLING_DONE,
    CHIP_ACC_ENABLE,
    CHIP_ACC_ENABLE_DONE,
    CHIP_ACC_DISABLE,
    CHIP_ACC_DISABLE_DONE,
    CHIP_ACC_RATECHG,
    CHIP_ACC_RATECHG_DONE,
    CHIP_ACC_CALI,
    CHIP_ACC_CALI_DONE,
    CHIP_ACC_CFG,
    CHIP_ACC_CFG_DONE,
    CHIP_ACC_SELFTEST,
    CHIP_ACC_SELFTEST_DONE,
    CHIP_GYRO_ENABLE,
    CHIP_GYRO_ENABLE_DONE,
    CHIP_GYRO_DISABLE,
    CHIP_GYRO_DISABLE_DONE,
    CHIP_GYRO_RATECHG,
    CHIP_GYRO_RATECHG_DONE,
    CHIP_GYRO_CALI,
    CHIP_GYRO_CALI_DONE,
    CHIP_GYRO_CFG,
    CHIP_GYRO_CFG_DONE,
    CHIP_GYRO_SELFTEST,
    CHIP_GYRO_SELFTEST_DONE,
    CHIP_ANYMO_ENABLE,
    CHIP_ANYMO_ENABLE_DONE,
    CHIP_ANYMO_DISABLE,
    CHIP_ANYMO_DISABLE_DONE,
    CHIP_NOMO_ENABLE,
    CHIP_NOMO_ENABLE_DONE,
    CHIP_NOMO_DISABLE,
    CHIP_NOMO_DISABLE_DONE,
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    CHIP_STEP_COUNT_ENABLE,
    CHIP_STEP_COUNT_DONE,
    CHIP_STEP_COUNT_DISABLE,
    CHIP_STEP_COUNT_DISABLE_DONE,
    CHIP_STEP_COUNT_SAMPLE,
    CHIP_STEP_COUNT_SEND,
    CHIP_STEP_COUNT_SAMPLE_DONE,
#endif
    CHIP_HW_INT_STATUS_CHECK,
    CHIP_HW_INT_HANDLING,
    CHIP_HW_INT_HANDLING_DONE,
    CHIP_INIT_DONE,
    CHIP_IDLE,
    CHIP_RESET,
};
struct accGyroData {
    uint8_t sensType;
    float x, y, z;
};
struct accGyroDataHeader {
    uint8_t accOutSize;
    uint8_t gyroOutSize;
};
struct accGyroDataPacket {
    uint8_t accOutSize;
    uint8_t gyroOutSize;
    uint64_t timeStamp;
    float temperature;
    struct accGyroData outBuf[MAX_RECV_PACKET];
    uint32_t magicNum;
};
struct accGyroCntlPacket {
    uint32_t rate;
    uint32_t waterMark;
    uint64_t latency;
    uint32_t magicNum;
};
struct accGyroCaliCfgPacket {
    int32_t caliCfgData[3];
};
typedef void (*accGyroTimerCbkF)(void);
void accGyroHwIntCheckStatus(void);
void accGyroInterruptOccur(void);
void registerAccGyroInterruptMode(uint8_t mode);
void registerAccGyroDriverFsm(struct sensorFsm *mSensorFsm, uint8_t size);
void registerAccGyroFifoInfo(uint32_t accFifoDelay, uint32_t gyroFifoDelay);
void registerAccGyroTimerCbk(accGyroTimerCbkF cbk);
void accSensorRegister(void);
void gyroSensorRegister(void);
void anyMotionSensorRegister(void);
void noMotionSensorRegister(void);
void getRealSampleDelay(uint32_t *accDelay, uint32_t *gyroDelay);
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
void stepCntSensorRegister(void);
#endif
void accGyroSendCalibrationResult(uint8_t sensorType, int32_t bias[3], uint8_t status);
void accGyroSendTestResult(uint8_t sensorType, uint8_t status);
#ifdef CFG_MTK_CALIBRATION_V1_0
struct GyroCal *GetGyroCalBase(void);
struct OverTempCal *GetOverTempCalBase(void);
#endif
static inline void insertMagicNum(struct accGyroDataPacket *packet)
{
    packet->magicNum = 0xdeadbeef;
}
static inline bool detectMagicNumOverflowed(struct accGyroDataPacket *packet)
{
    if (packet->magicNum == 0xdeadbeef)
        return false;
    else
        return true;
}
/* inSize: rate and watermark
 * elemInSize: struct accGyroCntlPacket
 */
static inline int rxControlInfo(struct accGyroCntlPacket *recvPacket, void *inBuf,
    uint8_t inSize, uint8_t elemInSize, void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct accGyroCntlPacket *packet = (struct accGyroCntlPacket *)inBuf;

    if (inBuf == NULL || inSize != 1 || elemInSize != sizeof(struct accGyroCntlPacket)) {
        return -1;
    }
    recvPacket->rate = packet->rate;
    recvPacket->waterMark = packet->waterMark;
    recvPacket->latency = packet->latency;
    return 0;
}

/* inSize: caliData[3]
 * elemInSize: struct accGyroCntlPacket
 */
static inline int rxCaliCfgInfo(struct accGyroCaliCfgPacket *recvPacket, void *inBuf,
    uint8_t inSize, uint8_t elemInSize, void *outBuf, uint8_t *outSize, uint8_t *elemOutSize)
{
    struct accGyroCaliCfgPacket *packet = (struct accGyroCaliCfgPacket *)inBuf;

    if (inBuf == NULL || inSize != 1 || elemInSize != sizeof(struct accGyroCaliCfgPacket)) {
        return -1;
    }
    recvPacket->caliCfgData[0] = packet->caliCfgData[0];
    recvPacket->caliCfgData[1] = packet->caliCfgData[1];
    recvPacket->caliCfgData[2] = packet->caliCfgData[2];
    return 0;
}

/*
 * outBuf: driver genarate
 * outSize: driver genarate
 * elemOutSize: struct accGyroData
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
 * outSize: accOutSize + gyroOutSize
 */
static inline void txTransferDataInfo(struct transferDataInfo *dataInfo, uint8_t accOutSize,
    uint8_t gyroOutSize, uint64_t sampleTime, const struct accGyroData *data, float temperature)
{
    uint32_t totalNum = 0;
    struct accGyroDataPacket *packet = container_of(data, struct accGyroDataPacket, outBuf);
    struct accGyroDataPacket **pPacket = (struct accGyroDataPacket **)dataInfo->outBuf;

    if (packet == NULL || detectMagicNumOverflowed(packet))
        configASSERT(0);
    totalNum = accOutSize + gyroOutSize;
    *dataInfo->elemOutSize = sizeof(struct accGyroData);
    *dataInfo->outSize = totalNum;
    packet->accOutSize = accOutSize;
    packet->gyroOutSize = gyroOutSize;
    packet->timeStamp = sampleTime;
    packet->temperature = temperature;
    if (pPacket != NULL)
        *pPacket = packet;
    else
        configASSERT(0);
}
static inline uint64_t calcFakeInterruptTime(uint64_t swSampleTime, uint64_t hwSampleTime,
    uint64_t lastSampleTime, uint32_t accRate, bool accConfiged, uint8_t accEventSize,
    uint32_t gyroRate, bool gyroConfiged, uint8_t gyroEventSize)
{
    uint32_t maxRate = 0;
    uint32_t minDelay = 0;
    uint64_t fakeTime = 0, diff = 0;//, fakeTmp = 0, long_diff = 0;

    if (accConfiged && gyroConfiged) {
        maxRate = (accRate > gyroRate) ? accRate : gyroRate;
        minDelay = 1024000000000ULL / maxRate;
        //long_diff = ((accRate > gyroRate) ? accEventSize : gyroEventSize) * minDelay;
    } else if (accConfiged && !gyroConfiged) {
        maxRate = accRate;
        minDelay = 1024000000000ULL / maxRate;
        //long_diff = accEventSize * minDelay;
    } else if (!accConfiged && gyroConfiged) {
        maxRate = gyroRate;
        minDelay = 1024000000000ULL / maxRate;
        //long_diff = gyroEventSize * minDelay;
    } else if (!accConfiged && !gyroConfiged) {
        return hwSampleTime;
    }
    if (swSampleTime > hwSampleTime) {
        diff = swSampleTime - hwSampleTime;
        diff = (minDelay) ? ((diff / minDelay) * minDelay) : 0;
        fakeTime = hwSampleTime + diff;
    } else {
        return hwSampleTime;
    }
    /* fakeTmp = lastSampleTime + long_diff;
    if (fakeTmp > fakeTime) {
        diff = fakeTmp - fakeTime;
        diff = (minDelay) ? ((diff / minDelay) * minDelay) : 0;
        fakeTime = fakeTime + diff;
    } */
    return fakeTime;
}

static inline uint64_t calcFakeInterruptTime2(uint64_t swSampleTime, uint64_t hwSampleTime,
    uint64_t lastSampleTime, uint32_t accDelay, bool accConfiged, uint8_t accEventSize,
    uint32_t gyroDelay, bool gyroConfiged, uint8_t gyroEventSize)
{
    uint32_t minDelay = 0;
    uint64_t fakeTime = 0, diff = 0;

    if (accConfiged && gyroConfiged) {
        minDelay = (accDelay < gyroDelay) ? accDelay : gyroDelay;
    } else if (accConfiged && !gyroConfiged) {
        minDelay = accDelay;
    } else if (!accConfiged && gyroConfiged) {
        minDelay = gyroDelay;
    } else if (!accConfiged && !gyroConfiged) {
        return hwSampleTime;
    }
    if (swSampleTime > hwSampleTime) {
        diff = swSampleTime - hwSampleTime;
        diff = (minDelay) ? ((diff / minDelay) * minDelay) : 0;
        fakeTime = hwSampleTime + diff;
    } else {
        return hwSampleTime;
    }
    return fakeTime;
}
static inline void filterDataFifoUninterruptible(uint64_t nowSampleTime, uint64_t lastSampleTime,
    uint32_t accRate, bool accConfiged, uint8_t *accEventSize,
    uint32_t gyroRate, bool gyroConfiged, uint8_t *gyroEventSize)
{
    uint32_t maxRate = 0;
    uint32_t minDelay = 0;
    uint64_t diff = 0;
    uint8_t realSize = 0;
    uint8_t minEventSize = 0;

    if (nowSampleTime == lastSampleTime) {
        *accEventSize = *gyroEventSize = 0;
        return;
    }
    if (*accEventSize > 1 || *gyroEventSize > 1) {
        if (accConfiged && gyroConfiged) {
            maxRate = (accRate > gyroRate) ? accRate : gyroRate;
            minDelay = 1024000000000ULL / maxRate;
            if (*accEventSize == 0)
                minEventSize = *gyroEventSize;
            else if (*gyroEventSize == 0)
                minEventSize = *accEventSize;
            else
                minEventSize = *accEventSize > *gyroEventSize ? *gyroEventSize : *accEventSize;
            if ((nowSampleTime - minDelay * minEventSize) < lastSampleTime) {
                diff = nowSampleTime - lastSampleTime;
                realSize = diff / minDelay;
                if (*accEventSize != 0)
                    *accEventSize = realSize;
                if (*gyroEventSize != 0)
                    *gyroEventSize = realSize;
            }
        } else if (accConfiged && !gyroConfiged) {
            maxRate = accRate;
            minDelay = 1024000000000ULL / maxRate;
            minEventSize = *accEventSize;
            if ((nowSampleTime - minDelay * minEventSize) < lastSampleTime) {
                diff = nowSampleTime - lastSampleTime;
                realSize = diff / minDelay;
                *accEventSize = realSize;
            }
        } else if (!accConfiged && gyroConfiged) {
            maxRate = gyroRate;
            minDelay = 1024000000000ULL / maxRate;
            minEventSize = *gyroEventSize;
            if ((nowSampleTime - minDelay * minEventSize) < lastSampleTime) {
                diff = nowSampleTime - lastSampleTime;
                realSize = diff / minDelay;
                *gyroEventSize = realSize;
            }
        }
    }
}
#endif
