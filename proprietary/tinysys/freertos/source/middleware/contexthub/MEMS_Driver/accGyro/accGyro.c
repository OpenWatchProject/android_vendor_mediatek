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

#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <eventnums.h>
#include <heap.h>
#include <hostIntf.h>
#include <i2c.h>
#include <spi.h>
#include <nanohubPacket.h>
#include <sensors.h>
#include <seos.h>
#include <timer.h>
#include <util.h>
#include <slab.h>
#include <plat/inc/rtc.h>

#include <contexthub_core.h>
#include <accGyro.h>
#ifndef CFG_MTK_CALIBRATION_V1_0
#include <API_sensor_calibration.h>
#endif
#include <API_timestamp_calibration.h>
#include <performance.h>
#include <wakelock.h>

#define MAX_NUM_COMMS_EVENT_SAMPLES 15
#define GYRO_FIFO_TIMER_DELAY 20000000
#define ACC_FIFO_TIMER_DELAY 60000000

#define GYRO_TEMPERATURE_CONVERT 10000000

enum sensorHandleState {
    ACC = 0,
    GYRO,
    ANY_MOTION,
    NO_MOTION,
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    STEP_CNT,
#endif
    MAX_HANDLE,
};
enum accGyroSensorEvents {
    EVT_SENSOR_EVENT = EVT_APP_START + 1,
    EVT_SENSOR_TIMER,
    EVT_SENSOR_INTR,
    EVT_HW_INT_STATUS_CHECK,
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    EVT_HW_STEPCNT_SAMPLE,
#endif
    EVT_ERROR_EVENT,
};
struct ConfigStat {
    uint64_t latency;
    uint32_t rate;
    bool enable;
};
struct sensPerUnit {
    uint32_t handle;
    uint32_t rate;
    uint64_t latency;
    bool nowOn;
    bool configed;
    bool fifoEnabled;
    //bool isReading;
    uint32_t timerDelay;
    uint32_t fifoDelay;
    uint64_t fifoStartTime;
    uint64_t prev_rtc_time;
    struct ConfigStat pendConfig;
    int32_t pendCaliCfg[12];
    /* receive data which convert from rawdata, real output for algo and ap */
    struct TripleAxisDataEvent *data_evt;
    uint8_t flush;
    wakelock_t wakeLock;
};
static struct accGyroTask {
    uint32_t id;
    uint32_t timerHandle;
    bool isReading;
    bool pendingInt;
    bool pendingSample;
    bool pendingConfig[MAX_HANDLE];
    bool pendingHwIntStatusCheck;
    bool pendingCali[MAX_HANDLE];
    bool pendingCaliCfg[MAX_HANDLE];
    bool pendingSelfTest[MAX_HANDLE];
    bool pendingFlushFifo;
    struct sensPerUnit mSensorPerUnit[MAX_HANDLE];
    struct accGyroDataPacket *packet;
    uint8_t outSize;
    uint8_t elemOutSize;
    struct SlabAllocator *mDataSlab;
    struct fsmCurrInfo fsm;
    accGyroTimerCbkF timerInterruptCbk;
    uint8_t intrMode;
    uint8_t nowState;
#ifndef CFG_MTK_CALIBRATION_V1_0
    float bias[3];
    float slope[3];
    float intercept[3];
#else
    struct GyroCal gyro_cal;
    struct OverTempCal over_temp_gyro_cal;
#endif
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    uint32_t stepCntSamplingTimerHandle;
#endif
} mTask;
static struct accGyroTask *accGyroDebugPoint;
static const uint32_t accSupportedRates[] = {
    SENSOR_HZ(5.0f),
    SENSOR_HZ(10.0f),
    SENSOR_HZ(16.0f),
    SENSOR_HZ(50.0f),
    SENSOR_HZ(100.0f),
    SENSOR_HZ(200.0f),
    SENSOR_HZ(400.0f),
    0
};

static const uint64_t rateTimerValsAcc[] = {
//should match "supported rates in length" and be the timer length for that rate in nanosecs
    1000000000ULL / 5,
    1000000000ULL / 10,
    1000000000ULL / 16,
    1000000000ULL / 50,
    1000000000ULL / 100,
    1000000000ULL / 200,
    1000000000ULL / 400,
};
static const uint32_t gyroSupportedRates[] = {
    SENSOR_HZ(5.0f),
    SENSOR_HZ(10.0f),
    SENSOR_HZ(16.0f),
    SENSOR_HZ(50.0f),
    SENSOR_HZ(100.0f),
    SENSOR_HZ(200.0f),
    SENSOR_HZ(400.0f),
    0
};

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
static uint32_t StepCntRates[] = {
    SENSOR_HZ(1.0f/300.0f),
    SENSOR_HZ(1.0f/240.0f),
    SENSOR_HZ(1.0f/180.0f),
    SENSOR_HZ(1.0f/120.0f),
    SENSOR_HZ(1.0f/90.0f),
    SENSOR_HZ(1.0f/60.0f),
    SENSOR_HZ(1.0f/45.0f),
    SENSOR_HZ(1.0f/30.0f),
    SENSOR_HZ(1.0f/15.0f),
    SENSOR_HZ(1.0f/10.0f),
    SENSOR_HZ(1.0f/5.0f),
    SENSOR_RATE_ONCHANGE,
    0
};
// should match StepCntRates and be the timer length for that rate in nanosecs
static const uint64_t stepCntRateTimerVals[] = {
    300 * 1000000000ULL,
    240 * 1000000000ULL,
    180 * 1000000000ULL,
    120 * 1000000000ULL,
    90 * 1000000000ULL,
    60 * 1000000000ULL,
    45 * 1000000000ULL,
    30 * 1000000000ULL,
    15 * 1000000000ULL,
    10 * 1000000000ULL,
    5 * 1000000000ULL,
};
#endif

static const uint64_t rateTimerValsGyro[] = {
//should match "supported rates in length" and be the timer length for that rate in nanosecs
    1000000000ULL / 5,
    1000000000ULL / 10,
    1000000000ULL / 16,
    1000000000ULL / 50,
    1000000000ULL / 100,
    1000000000ULL / 200,
    1000000000ULL / 400,
};
static uint64_t last_acc_time;
static uint64_t last_gyro_time;

/* sensor callbacks from nanohub */

static void i2cCallback(void *cookie, size_t tx, size_t rx, int err)
{
    //osLog(LOG_DEBUG, "accGyro: i2cCallback (%d) cookie:%d\n", err, (uint32_t)cookie);
    if (err == 0)
        osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
    else {
        osLog(LOG_ERROR, "accGyro: i2c error (%d)\n", err);
        osEnqueuePrivateEvt(EVT_ERROR_EVENT, (void *)ERR_SENSOR_I2C, NULL, mTask.id);
    }
}

/* sensor callbacks from nanohub */

static void spiCallback(void *cookie, int err)
{
    //osLog(LOG_DEBUG, "accGyro: spiCallback (%d) cookie:%d\n", err, (uint32_t)cookie);
    if (err == 0)
        osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
    else {
        osLog(LOG_ERROR, "accGyro: spi error (%d)\n", err);
        osEnqueuePrivateEvt(EVT_ERROR_EVENT, (void *)ERR_SENSOR_SPI, NULL, mTask.id);
    }
}
void accGyroInterruptOccur(void)
{
    osEnqueuePrivateEvt(EVT_SENSOR_INTR, NULL, NULL, mTask.id);
    extern TaskHandle_t CHRE_TaskHandle;
    if (xTaskResumeFromISR(CHRE_TaskHandle) == pdTRUE) {
        portYIELD_WITHIN_API();
    }
}
void accGyroHwIntCheckStatus(void)
{
    osEnqueuePrivateEvt(EVT_HW_INT_STATUS_CHECK, NULL, NULL, mTask.id);
    extern TaskHandle_t CHRE_TaskHandle;
    if (xTaskResumeFromISR(CHRE_TaskHandle) == pdTRUE) {
        portYIELD_WITHIN_API();
    }
}
static void accGyroTimerCallback(uint32_t timerId, void *cookie)
{
    //osLog(LOG_DEBUG, "accGyroTimerCallback:cookie:%d\n", (uint32_t)cookie);
    mTask.timerInterruptCbk();
    osEnqueuePrivateEvt(EVT_SENSOR_TIMER, cookie, NULL, mTask.id);
}
static uint32_t calcuWaterMarkFifoUninterruptible(void)
{
    int handle;
    uint64_t min_latency = 0XFFFFFFFFFFFFFFFFULL;
    uint8_t min_watermark = 1;
    uint8_t final_watermark = 0;
    uint8_t max_watermark = MAX_RECV_PACKET;
    uint8_t watermark = 0;
    uint32_t temp_cnt = 0;
    uint32_t temp_delay = 0;

    for (handle = ACC; handle <= GYRO; handle++) {
        if (mTask.mSensorPerUnit[handle].nowOn && mTask.mSensorPerUnit[handle].latency != SENSOR_LATENCY_NODATA) {
            min_latency =
                mTask.mSensorPerUnit[handle].latency < min_latency ? mTask.mSensorPerUnit[handle].latency : min_latency;
        }
    }

    for (handle = ACC; handle <= GYRO; handle++) {
        if (mTask.mSensorPerUnit[handle].nowOn && mTask.mSensorPerUnit[handle].latency != SENSOR_LATENCY_NODATA) {
            temp_delay = (1000000000ULL / mTask.mSensorPerUnit[handle].rate) << 10;
            temp_cnt = min_latency / temp_delay;
            min_watermark = mTask.mSensorPerUnit[handle].rate / SENSOR_HZ(400.0f);
            temp_cnt = temp_cnt > min_watermark ? temp_cnt : min_watermark;
            temp_cnt = temp_cnt > 0 ? temp_cnt : 1;
            if (final_watermark == 0)
                final_watermark = temp_cnt;
            else
                final_watermark = temp_cnt < final_watermark ? temp_cnt : final_watermark;
            osLog(LOG_INFO, "accGyro, delay=%lu, latency:%lld, min_wm=%d, final_watermark=%d\n",
                temp_delay, min_latency, min_watermark, final_watermark);
        }
    }
    watermark = final_watermark;
    watermark = watermark < min_watermark ? min_watermark : watermark;
    watermark = watermark > max_watermark ? max_watermark : watermark;
    osLog(LOG_INFO, "accGyro, watermark:%d\n", watermark);
    return watermark;
}
static uint32_t calcuWaterMark(void)
{
    int handle;
    uint64_t min_latency = 0XFFFFFFFFFFFFFFFFULL;
    uint8_t min_watermark = 1;
    uint8_t max_watermark = MAX_RECV_PACKET;
    uint8_t watermark = 0;
    uint32_t temp_cnt, total_cnt = 0;
    uint32_t temp_delay = 0;

    for (handle = ACC; handle <= GYRO; handle++) {
        if (mTask.mSensorPerUnit[handle].nowOn && mTask.mSensorPerUnit[handle].latency != SENSOR_LATENCY_NODATA) {
            min_latency =
                mTask.mSensorPerUnit[handle].latency < min_latency ? mTask.mSensorPerUnit[handle].latency : min_latency;
        }
    }

    for (handle = ACC; handle <= GYRO; handle++) {
        if (mTask.mSensorPerUnit[handle].nowOn && mTask.mSensorPerUnit[handle].latency != SENSOR_LATENCY_NODATA) {
            temp_delay = (1000000000ULL / mTask.mSensorPerUnit[handle].rate) << 10;
            temp_cnt = min_latency / temp_delay;
            min_watermark = mTask.mSensorPerUnit[handle].rate / SENSOR_HZ(400.0f);
            total_cnt += temp_cnt > min_watermark ? temp_cnt : min_watermark;
            osLog(LOG_INFO, "accGyro, delay=%lu, latency:%lld, min_wm=%d, total_cnt=%lu\n",
                temp_delay, min_latency, min_watermark, total_cnt);
        }
    }
    watermark = total_cnt;
    watermark = watermark < min_watermark ? min_watermark : watermark;
    watermark = watermark > max_watermark ? max_watermark : watermark;
    //osLog(LOG_INFO, "accGyro, watermark:%d\n", watermark);
    if (watermark < 1) {
        osLog(LOG_INFO, "accGyro, watermark:%d < 1\n", watermark);
        watermark = 1;
    }

    return watermark;
}
static inline bool anyFifoEnabled(void)
{
    return (mTask.mSensorPerUnit[ACC].fifoEnabled || mTask.mSensorPerUnit[GYRO].fifoEnabled);
}

static void flushFifo(void)
{
    struct transferDataInfo dataInfo;
    bool any_fifo_enabled_prev = anyFifoEnabled();

    if (mTask.mSensorPerUnit[ACC].configed && mTask.mSensorPerUnit[ACC].latency != SENSOR_LATENCY_NODATA)
        mTask.mSensorPerUnit[ACC].fifoEnabled = true;
    else
        mTask.mSensorPerUnit[ACC].fifoEnabled = false;

    if (mTask.mSensorPerUnit[GYRO].configed && mTask.mSensorPerUnit[GYRO].latency != SENSOR_LATENCY_NODATA)
        mTask.mSensorPerUnit[GYRO].fifoEnabled = true;
    else
        mTask.mSensorPerUnit[GYRO].fifoEnabled = false;

    // if this is not the first fifo enabled or last fifo disabled, flush all fifo data;
    if (any_fifo_enabled_prev && anyFifoEnabled()) {
        if (!mTask.isReading && (mTask.mSensorPerUnit[ACC].nowOn || mTask.mSensorPerUnit[GYRO].nowOn)) {
            mTask.nowState = CHIP_SAMPLING;
            dataInfo.inBuf = NULL;
            dataInfo.inSize = 0;
            dataInfo.elemInSize = 0;
            dataInfo.outBuf = &mTask.packet;
            dataInfo.outSize = &mTask.outSize;
            dataInfo.elemOutSize = &mTask.elemOutSize;
            sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
            mTask.isReading = true;
            mTask.pendingFlushFifo = true;
        }
    }
}
static void setAccHwEnable(bool on, uint32_t waterMark)
{
    struct transferDataInfo dataInfo;
    struct accGyroCntlPacket cntlPacket;

    mTask.pendingFlushFifo = false;
    if (on == true) {
        mTask.nowState = CHIP_ACC_ENABLE;
        dataInfo.inBuf = NULL;
        dataInfo.inSize = 0;
        dataInfo.elemInSize = 0;
        dataInfo.outBuf = NULL;
        dataInfo.outSize = NULL;
        dataInfo.elemOutSize = NULL;
        sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ACC_ENABLE, &i2cCallback, &spiCallback);
    } else {
        wake_unlock(&mTask.mSensorPerUnit[ACC].wakeLock);
        cntlPacket.waterMark = waterMark;
        mTask.nowState = CHIP_ACC_DISABLE;
        dataInfo.inBuf = (void *)&cntlPacket;
        dataInfo.inSize = 1;
        dataInfo.elemInSize = sizeof(struct accGyroCntlPacket);
        dataInfo.outBuf = NULL;
        dataInfo.outSize = NULL;
        dataInfo.elemOutSize = NULL;
        sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ACC_DISABLE, &i2cCallback, &spiCallback);
    }
}
static void accPowerCalculate(bool on)
{
    uint32_t timerDelay = 0, waterMark = 0;

    if (mTask.nowState == CHIP_IDLE) {
        if (!on)
            mTask.mSensorPerUnit[ACC].timerDelay = 0XFFFFFFFF;
        mTask.mSensorPerUnit[ACC].nowOn = on;
        switch (mTask.intrMode) {
            case ACC_GYRO_FIFO_UNINTERRUPTIBLE: {
                if (!on && !mTask.mSensorPerUnit[GYRO].nowOn) {
                    timTimerCancel(mTask.timerHandle);
                    mTask.timerHandle = 0;
                } else if (!on && mTask.mSensorPerUnit[GYRO].nowOn) {
                    waterMark = calcuWaterMarkFifoUninterruptible();
                    mTask.mSensorPerUnit[GYRO].timerDelay =
                        waterMark * (1024000000000ULL / mTask.mSensorPerUnit[GYRO].rate);
                    timerDelay = (mTask.mSensorPerUnit[GYRO].timerDelay > mTask.mSensorPerUnit[ACC].timerDelay) ?
                            mTask.mSensorPerUnit[ACC].timerDelay : mTask.mSensorPerUnit[GYRO].timerDelay;
                    timTimerCancel(mTask.timerHandle);
                    mTask.timerHandle =
                        timTimerSet(timerDelay, 0, 50, accGyroTimerCallback, NULL, false);
               }
                break;
            }
            case GYRO_ONLY_FIFO_INTERRUPTIBLE: {
                if (!on) {
                    timTimerCancel(mTask.timerHandle);
                    mTask.timerHandle = 0;
                }
                break;
            }
            case ACC_GYRO_FIFO_INTERRUPTIBLE: {
                waterMark = calcuWaterMark();
                break;
            }
        }
        setAccHwEnable(on, waterMark);
        mTask.pendingConfig[ACC] = false;
    } else {
        osLog(LOG_INFO, "accPowerCalculate pending by:%d\n", mTask.nowState);
        mTask.pendingConfig[ACC] = true;
        mTask.mSensorPerUnit[ACC].pendConfig.enable = on;
    }
}
static void setAccHwRate(uint32_t rate, uint32_t waterMark, uint64_t latency)
{
    struct accGyroCntlPacket cntlPacket;
    struct transferDataInfo dataInfo;

    if ((rate >= SENSOR_HZ(200.0f)) && waterMark <= 4)
        wake_lock(&mTask.mSensorPerUnit[ACC].wakeLock);
    else
        wake_unlock(&mTask.mSensorPerUnit[ACC].wakeLock);
    mTask.pendingFlushFifo = false;
    cntlPacket.rate = rate;
    cntlPacket.waterMark = waterMark;
    cntlPacket.latency= latency;

    mTask.nowState = CHIP_ACC_RATECHG;
    dataInfo.inBuf = (void *)&cntlPacket;
    dataInfo.inSize = 1;
    dataInfo.elemInSize = sizeof(struct accGyroCntlPacket);
    dataInfo.outBuf = NULL;
    dataInfo.outSize = NULL;
    dataInfo.elemOutSize = NULL;
    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ACC_RATECHG, &i2cCallback, &spiCallback);
}
static void accRateCalculate(uint32_t rate, uint64_t latency)
{
    uint32_t waterMark = 0;

    if (mTask.nowState == CHIP_IDLE) {
        mTask.mSensorPerUnit[ACC].rate = rate;
        mTask.mSensorPerUnit[ACC].latency = latency;
        switch (mTask.intrMode) {
            case ACC_GYRO_FIFO_UNINTERRUPTIBLE: {
                waterMark = calcuWaterMarkFifoUninterruptible();
                mTask.mSensorPerUnit[ACC].timerDelay = waterMark * (1024000000000ULL / rate);
                setAccHwRate(rate, waterMark, latency);
                break;
            }
            case GYRO_ONLY_FIFO_INTERRUPTIBLE: {
                mTask.mSensorPerUnit[ACC].timerDelay =
                    sensorTimerLookupCommon(accSupportedRates, rateTimerValsAcc, rate);
                setAccHwRate(rate, 0, latency);
                break;
            }
            case ACC_GYRO_FIFO_INTERRUPTIBLE: {
                setAccHwRate(rate, calcuWaterMark(), latency);
                break;
            }
        }
    } else {
        osLog(LOG_INFO, "accRateCalculate pending by:%d\n", mTask.nowState);
        mTask.pendingConfig[ACC] = true;
        mTask.mSensorPerUnit[ACC].pendConfig.enable = true;
        mTask.mSensorPerUnit[ACC].pendConfig.latency = latency;
        mTask.mSensorPerUnit[ACC].pendConfig.rate = rate;
    }
}
static bool sensorPowerAcc(bool on, void *cookie)
{
    osLog(LOG_INFO, "sensorPowerAcc on:%d, nowOn:%d\n", on, mTask.mSensorPerUnit[ACC].nowOn);
    if (on != mTask.mSensorPerUnit[ACC].nowOn) {
        if (mTask.nowState == CHIP_IDLE) {
            if (!on && !mTask.pendingFlushFifo) {
                mTask.mSensorPerUnit[ACC].configed = false;
                flushFifo();
            }
            accPowerCalculate(on);
        } else {
            osLog(LOG_INFO, "sensorPowerAcc pending by:%d\n", mTask.nowState);
            mTask.pendingConfig[ACC] = true;
            mTask.mSensorPerUnit[ACC].pendConfig.enable = on;
        }
    } else
        osLog(LOG_ERROR, "sensorPowerAcc on != nowOn\n");
    return true;
}

static bool sensorFirmwareAcc(void *cookie)
{
    osLog(LOG_INFO, "sensorFirmwareAcc\n");

    sensorSignalInternalEvt(mTask.mSensorPerUnit[ACC].handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}
static bool sensorRateAcc(uint32_t rate, uint64_t latency, void *cookie)
{
    osLog(LOG_INFO, "sensorRateAcc rate:%lu, latency:%lld\n", rate, latency);

    if (mTask.nowState == CHIP_IDLE) {
        mTask.mSensorPerUnit[ACC].configed = true;
        if (!mTask.pendingFlushFifo)
            flushFifo();
    #ifdef CFG_ACC_NO_FIFO
        accRateCalculate(rate, sensorTimerLookupCommon(accSupportedRates, rateTimerValsAcc, rate));
    #else
        accRateCalculate(rate, latency);
    #endif
    } else {
        osLog(LOG_INFO, "sensorRateAcc pending by:%d\n", mTask.nowState);
        mTask.pendingConfig[ACC] = true;
        mTask.mSensorPerUnit[ACC].pendConfig.enable = true;
        mTask.mSensorPerUnit[ACC].pendConfig.latency = latency;
        mTask.mSensorPerUnit[ACC].pendConfig.rate = rate;
    }
    return true;
}

static bool sensorFlushAcc(void *cookie)
{
    struct transferDataInfo dataInfo;

    osLog(LOG_INFO, "sensorFlushAcc\n");
    mTask.mSensorPerUnit[ACC].flush++;
    if (mTask.nowState == CHIP_IDLE) {
        if (!mTask.isReading && (mTask.mSensorPerUnit[ACC].nowOn || mTask.mSensorPerUnit[GYRO].nowOn)) {
            mTask.nowState = CHIP_SAMPLING;
            dataInfo.inBuf = NULL;
            dataInfo.inSize = 0;
            dataInfo.elemInSize = 0;
            dataInfo.outBuf = &mTask.packet;
            dataInfo.outSize = &mTask.outSize;
            dataInfo.elemOutSize = &mTask.elemOutSize;
            sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
            mTask.isReading = true;
        }
    } else {
        osLog(LOG_INFO, "sensorFlushAcc pending by:%d\n", mTask.nowState);
        mTask.pendingSample = true;
    }
    return 0;
}

static bool sensorCaliAcc(void *cookie)
{
    struct transferDataInfo dataInfo;
    osLog(LOG_INFO, "sensorCaliAcc\n");
    /*if (mTask.mSensorPerUnit[ACC].nowOn) {
        osLog(LOG_ERROR, "sensor is on now\n");
        return false;
    }*/
    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_ACC_CALI;
        dataInfo.inBuf = NULL;
        dataInfo.inSize = 0;
        dataInfo.elemInSize = 0;
        dataInfo.outBuf = NULL;
        dataInfo.outSize = 0;
        dataInfo.elemOutSize = 0;
        if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ACC_CALI, &i2cCallback, &spiCallback) < 0)
            mTask.nowState = CHIP_IDLE;
    } else {
        osLog(LOG_INFO, "sensorCaliAcc pending by:%d\n", mTask.nowState);
        mTask.pendingCali[ACC] = true;
    }
    return true;
}

static void setAccHwCaliCfg(int32_t *data)
{
    struct accGyroCaliCfgPacket caliCfgPacket;
    struct transferDataInfo dataInfo;

    //osLog(LOG_ERROR, "setAccHwCaliCfg\n");

    caliCfgPacket.caliCfgData[0] = data[0];
    caliCfgPacket.caliCfgData[1] = data[1];
    caliCfgPacket.caliCfgData[2] = data[2];

    mTask.nowState = CHIP_ACC_CFG;
    dataInfo.inBuf = (void *)&caliCfgPacket;
    dataInfo.inSize = 1;
    dataInfo.elemInSize = sizeof(struct accGyroCaliCfgPacket);
    dataInfo.outBuf = NULL;
    dataInfo.outSize = 0;
    dataInfo.elemOutSize = 0;
    if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ACC_CFG, &i2cCallback, &spiCallback) < 0)
        mTask.nowState = CHIP_IDLE;
}

static bool sensorCfgAcc(void *data, void *cookie)
{
#ifndef CFG_MTK_CALIBRATION_V1_0
    float bias[3];
#endif
    int32_t *values = data;
    uint8_t i = 0;

    osLog(LOG_INFO, "sensorCfgAcc:\n");
    osLog(LOG_INFO, "bias: %f, %f, %f\n",
            (double)((float)values[0] / ACCELEROMETER_INCREASE_NUM_AP),
            (double)((float)values[1] / ACCELEROMETER_INCREASE_NUM_AP),
            (double)((float)values[2] / ACCELEROMETER_INCREASE_NUM_AP));
    osLog(LOG_INFO, "cali: %ld, %ld, %ld\n", values[3], values[4], values[5]);

    if (mTask.nowState == CHIP_IDLE) {
#ifndef CFG_MTK_CALIBRATION_V1_0
        bias[0] = ((float)values[0] / ACCELEROMETER_INCREASE_NUM_AP);
        bias[1] = ((float)values[1] / ACCELEROMETER_INCREASE_NUM_AP);
        bias[2] = ((float)values[2] / ACCELEROMETER_INCREASE_NUM_AP);
        Acc_init_calibration(bias);
        Acc_update_accurancy((int *)&values[3]);
#endif
        setAccHwCaliCfg(&values[3]);
    } else {
        osLog(LOG_INFO, "sensorCfgAcc pending by:%d\n", mTask.nowState);
        mTask.pendingCaliCfg[ACC] = true;
        for (i = 0; i < 6; ++i) {
            mTask.mSensorPerUnit[ACC].pendCaliCfg[i] = values[i];
        }
    }

    return true;
}

static bool sensorSelfTestAcc(void *data)
{
    struct transferDataInfo dataInfo;

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_ACC_SELFTEST;
        dataInfo.inBuf = NULL;
        dataInfo.inSize = 0;
        dataInfo.elemInSize = 0;
        dataInfo.outBuf = NULL;
        dataInfo.outSize = 0;
        dataInfo.elemOutSize = 0;
        if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ACC_SELFTEST, &i2cCallback, &spiCallback) < 0)
            mTask.nowState = CHIP_IDLE;
    } else {
        osLog(LOG_INFO, "accSelfTest pending by:%d\n", mTask.nowState);
        mTask.pendingSelfTest[ACC] = true;
    }

    return true;
}

static void setGyroHwEnable(bool on, uint32_t waterMark)
{
    struct transferDataInfo dataInfo;
    struct accGyroCntlPacket cntlPacket;

    mTask.pendingFlushFifo = false;
    if (on == true) {
        mTask.nowState = CHIP_GYRO_ENABLE;
        dataInfo.inBuf = NULL;
        dataInfo.inSize = 0;
        dataInfo.elemInSize = 0;
        dataInfo.outBuf = NULL;
        dataInfo.outSize = NULL;
        dataInfo.elemOutSize = NULL;
        sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_GYRO_ENABLE, &i2cCallback, &spiCallback);
    } else {
        wake_unlock(&mTask.mSensorPerUnit[GYRO].wakeLock);
        cntlPacket.waterMark = waterMark;
        mTask.nowState = CHIP_GYRO_DISABLE;
        dataInfo.inBuf = (void *)&cntlPacket;
        dataInfo.inSize = 1;
        dataInfo.elemInSize = sizeof(struct accGyroCntlPacket);
        dataInfo.outBuf = NULL;
        dataInfo.outSize = NULL;
        dataInfo.elemOutSize = NULL;
        sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_GYRO_DISABLE, &i2cCallback, &spiCallback);
    }
}
static void gyroPowerCalculate(bool on)
{
    uint32_t waterMark = 0, timerDelay = 0;

    if (mTask.nowState == CHIP_IDLE) {
        if (!on)
            mTask.mSensorPerUnit[GYRO].timerDelay = 0XFFFFFFFF;
        mTask.mSensorPerUnit[GYRO].nowOn = on;
        switch (mTask.intrMode) {
            case ACC_GYRO_FIFO_UNINTERRUPTIBLE: {
                if (!on && !mTask.mSensorPerUnit[ACC].nowOn) {
                    timTimerCancel(mTask.timerHandle);
                    mTask.timerHandle = 0;
                } else if (!on && mTask.mSensorPerUnit[ACC].nowOn) {
                    waterMark = calcuWaterMarkFifoUninterruptible();
                    mTask.mSensorPerUnit[ACC].timerDelay =
                        waterMark * (1024000000000ULL / mTask.mSensorPerUnit[ACC].rate);
                    timerDelay = (mTask.mSensorPerUnit[GYRO].timerDelay > mTask.mSensorPerUnit[ACC].timerDelay) ?
                            mTask.mSensorPerUnit[ACC].timerDelay : mTask.mSensorPerUnit[GYRO].timerDelay;
                    timTimerCancel(mTask.timerHandle);
                    mTask.timerHandle =
                        timTimerSet(timerDelay, 0, 50, accGyroTimerCallback, NULL, false);
                }
                break;
            }
            case GYRO_ONLY_FIFO_INTERRUPTIBLE: {
                waterMark = calcuWaterMark();
                break;
            }
            case ACC_GYRO_FIFO_INTERRUPTIBLE: {
                waterMark = calcuWaterMark();
                break;
            }
        }
        setGyroHwEnable(on, waterMark);
        mTask.pendingConfig[GYRO] = false;
    } else {
        osLog(LOG_INFO, "gyroPowerCalculate pending by:%d\n", mTask.nowState);
        mTask.pendingConfig[GYRO] = true;
        mTask.mSensorPerUnit[GYRO].pendConfig.enable = on;
    }
}
static void setGyroHwRate(uint32_t rate, uint32_t waterMark, uint64_t latency)
{
    struct accGyroCntlPacket cntlPacket;
    struct transferDataInfo dataInfo;

    if ((rate >= SENSOR_HZ(200.0f)) && waterMark <= 4)
        wake_lock(&mTask.mSensorPerUnit[GYRO].wakeLock);
    else
        wake_unlock(&mTask.mSensorPerUnit[GYRO].wakeLock);
    mTask.pendingFlushFifo = false;
    cntlPacket.rate = rate;
    cntlPacket.waterMark = waterMark;
    cntlPacket.latency = latency;

    mTask.nowState = CHIP_GYRO_RATECHG;
    dataInfo.inBuf = (void *)&cntlPacket;
    dataInfo.inSize = 1;
    dataInfo.elemInSize = sizeof(struct accGyroCntlPacket);
    dataInfo.outBuf = NULL;
    dataInfo.outSize = NULL;
    dataInfo.elemOutSize = NULL;
    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_GYRO_RATECHG, &i2cCallback, &spiCallback);
}
static void gyroRateCalculate(uint32_t rate, uint64_t latency)
{
    uint32_t waterMark = 0;

    if (mTask.nowState == CHIP_IDLE) {
        mTask.mSensorPerUnit[GYRO].rate = rate;
        mTask.mSensorPerUnit[GYRO].latency = latency;
        switch (mTask.intrMode) {
            case ACC_GYRO_FIFO_UNINTERRUPTIBLE: {
                waterMark = calcuWaterMarkFifoUninterruptible();
                mTask.mSensorPerUnit[GYRO].timerDelay = waterMark * (1024000000000ULL / rate);
                setGyroHwRate(rate, waterMark, latency);
                break;
            }
            case GYRO_ONLY_FIFO_INTERRUPTIBLE: {
                setGyroHwRate(rate, calcuWaterMark(), latency);
                break;
            }
            case ACC_GYRO_FIFO_INTERRUPTIBLE: {
                setGyroHwRate(rate, calcuWaterMark(), latency);
                break;
            }
        }
    } else {
        osLog(LOG_INFO, "gyroRateCalculate pending by:%d\n", mTask.nowState);
        mTask.pendingConfig[GYRO] = true;
        mTask.mSensorPerUnit[GYRO].pendConfig.enable = true;
        mTask.mSensorPerUnit[GYRO].pendConfig.latency = latency;
        mTask.mSensorPerUnit[GYRO].pendConfig.rate = rate;
    }
}
static bool sensorPowerGyro(bool on, void *cookie)
{
    osLog(LOG_INFO, "sensorPowerGyro on:%d, nowOn:%d\n", on, mTask.mSensorPerUnit[GYRO].nowOn);
    if (on != mTask.mSensorPerUnit[GYRO].nowOn) {
        if (mTask.nowState == CHIP_IDLE) {
            if (!on && !mTask.pendingFlushFifo) {
                mTask.mSensorPerUnit[GYRO].configed = false;
                flushFifo();
            }
            gyroPowerCalculate(on);
        } else {
            osLog(LOG_INFO, "sensorPowerGyro pending by:%d\n", mTask.nowState);
            mTask.pendingConfig[GYRO] = true;
            mTask.mSensorPerUnit[GYRO].pendConfig.enable = on;
        }
    } else
        osLog(LOG_ERROR, "sensorPowerGyro on != nowOn\n");

    return true;
}

static bool sensorFirmwareGyro(void *cookie)
{
    osLog(LOG_INFO, "sensorFirmwareGyro\n");

    sensorSignalInternalEvt(mTask.mSensorPerUnit[GYRO].handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}
static bool sensorRateGyro(uint32_t rate, uint64_t latency, void *cookie)
{
    osLog(LOG_INFO, "sensorRateGyro rate:%lu, latency:%lld\n", rate, latency);

    if (mTask.nowState == CHIP_IDLE) {
        mTask.mSensorPerUnit[GYRO].configed = true;
        if (!mTask.pendingFlushFifo)
            flushFifo();
        gyroRateCalculate(rate, latency);
    } else {
        osLog(LOG_INFO, "sensorRateGyro pending by:%d\n", mTask.nowState);
        mTask.pendingConfig[GYRO] = true;
        mTask.mSensorPerUnit[GYRO].pendConfig.enable = true;
        mTask.mSensorPerUnit[GYRO].pendConfig.latency = latency;
        mTask.mSensorPerUnit[GYRO].pendConfig.rate = rate;
    }
    return true;
}

static bool sensorFlushGyro(void *cookie)
{
    struct transferDataInfo dataInfo;

    osLog(LOG_INFO, "sensorFlushGyro\n");
    mTask.mSensorPerUnit[GYRO].flush++;
    if (mTask.nowState == CHIP_IDLE) {
        if (!mTask.isReading && (mTask.mSensorPerUnit[ACC].nowOn || mTask.mSensorPerUnit[GYRO].nowOn)) {
            mTask.nowState = CHIP_SAMPLING;
            dataInfo.inBuf = NULL;
            dataInfo.inSize = 0;
            dataInfo.elemInSize = 0;
            dataInfo.outBuf = &mTask.packet;
            dataInfo.outSize = &mTask.outSize;
            dataInfo.elemOutSize = &mTask.elemOutSize;
            sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
            mTask.isReading = true;
        }
    } else {
        osLog(LOG_INFO, "sensorFlushGyro pending by:%d\n", mTask.nowState);
        mTask.pendingSample = true;
    }
    return 0;
}

static bool sensorCaliGyro(void *cookie)
{
    struct transferDataInfo dataInfo;

    /*osLog(LOG_INFO, "accGyro sensorCaliGyro\n");
    if (mTask.mSensorPerUnit[GYRO].nowOn) {
        osLog(LOG_ERROR, "sensor is on now\n");
        return false;
    }*/

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_GYRO_CALI;
        dataInfo.inBuf = NULL;
        dataInfo.inSize = 0;
        dataInfo.elemInSize = 0;
        dataInfo.outBuf = NULL;
        dataInfo.outSize = 0;
        dataInfo.elemOutSize = 0;
        if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_GYRO_CALI, &i2cCallback, &spiCallback) < 0)
            mTask.nowState = CHIP_IDLE;
    } else {
        osLog(LOG_INFO, "sensorCaliGyro pending by:%d\n", mTask.nowState);
        mTask.pendingCali[GYRO] = true;
    }
    return true;
}

static void setGyroHwCaliCfg(int32_t *data)
{
    struct accGyroCaliCfgPacket caliCfgPacket;
    struct transferDataInfo dataInfo;

    //osLog(LOG_ERROR, "setGyroHwCaliCfg\n");

    caliCfgPacket.caliCfgData[0] = data[0];
    caliCfgPacket.caliCfgData[1] = data[1];
    caliCfgPacket.caliCfgData[2] = data[2];

    mTask.nowState = CHIP_GYRO_CFG;
    dataInfo.inBuf = (void *)&caliCfgPacket;
    dataInfo.inSize = 1;
    dataInfo.elemInSize = sizeof(struct accGyroCaliCfgPacket);
    dataInfo.outBuf = NULL;
    dataInfo.outSize = 0;
    dataInfo.elemOutSize = 0;
    if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_GYRO_CFG, &i2cCallback, &spiCallback) < 0)
        mTask.nowState = CHIP_IDLE;
}

static bool sensorCfgGyro(void *data, void *cookie)
{
    int32_t *values = data;
    uint8_t i = 0;

    osLog(LOG_INFO, "sensorCfgGyro:\n");
    osLog(LOG_INFO, "bias: %f, %f, %f\n",
            (double)((float)values[0] / RADIRAN_TO_DEGREE_SCALAR / GYROSCOPE_INCREASE_NUM_AP),
            (double)((float)values[1] / RADIRAN_TO_DEGREE_SCALAR / GYROSCOPE_INCREASE_NUM_AP),
            (double)((float)values[2] / RADIRAN_TO_DEGREE_SCALAR / GYROSCOPE_INCREASE_NUM_AP));
    osLog(LOG_INFO, "cali: %ld, %ld, %ld\n", values[3], values[4], values[5]);
    osLog(LOG_INFO, "temperature cali: %ld, %ld, %ld, %ld, %ld, %ld\n",
        values[6], values[7], values[8], values[9], values[10], values[11]);

    if (mTask.nowState == CHIP_IDLE) {
#ifndef CFG_MTK_CALIBRATION_V1_0
        mTask.bias[0] = ((float)values[0] / RADIRAN_TO_DEGREE_SCALAR / GYROSCOPE_INCREASE_NUM_AP);
        mTask.bias[1] = ((float)values[1] / RADIRAN_TO_DEGREE_SCALAR / GYROSCOPE_INCREASE_NUM_AP);
        mTask.bias[2] = ((float)values[2] / RADIRAN_TO_DEGREE_SCALAR / GYROSCOPE_INCREASE_NUM_AP);

        mTask.slope[0] = ((float)values[6] / GYRO_TEMPERATURE_CONVERT);
        mTask.slope[1] = ((float)values[7] / GYRO_TEMPERATURE_CONVERT);
        mTask.slope[2] = ((float)values[8] / GYRO_TEMPERATURE_CONVERT);

        mTask.intercept[0] = ((float)values[9] / GYRO_TEMPERATURE_CONVERT);
        mTask.intercept[1] = ((float)values[10] / GYRO_TEMPERATURE_CONVERT);
        mTask.intercept[2] = ((float)values[11] / GYRO_TEMPERATURE_CONVERT);
        Gyro_init_calibration(mTask.slope, mTask.intercept);
#else
        const float dummy_temperature_celsius = 25.0f;
        float bias[3] = {0}, lastOffset[3] = {0}, sensitivity[3] = {0}, intercept[3] = {0};
        float lastTemperature = 0;
        bias[0] = ((float)values[0] / RADIRAN_TO_DEGREE_SCALAR / GYROSCOPE_INCREASE_NUM_AP);
        bias[1] = ((float)values[1] / RADIRAN_TO_DEGREE_SCALAR / GYROSCOPE_INCREASE_NUM_AP);
        bias[2] = ((float)values[2] / RADIRAN_TO_DEGREE_SCALAR / GYROSCOPE_INCREASE_NUM_AP);
        gyroCalSetBias(&mTask.gyro_cal, bias[0], bias[1],
            bias[2], dummy_temperature_celsius, rtcGetTime());
        sensitivity[0] = ((float)values[6] / GYRO_TEMPERATURE_CONVERT);
        sensitivity[1] = ((float)values[7] / GYRO_TEMPERATURE_CONVERT);
        sensitivity[2] = ((float)values[8] / GYRO_TEMPERATURE_CONVERT);
        intercept[0] = ((float)values[9] / GYRO_TEMPERATURE_CONVERT);
        intercept[1] = ((float)values[10] / GYRO_TEMPERATURE_CONVERT);
        intercept[2] = ((float)values[11] / GYRO_TEMPERATURE_CONVERT);
        overTempCalSetModel(&mTask.over_temp_gyro_cal, lastOffset, lastTemperature,
            rtcGetTime(), sensitivity, intercept, false /*not jumpstart*/);
#endif
        setGyroHwCaliCfg(&values[3]);
    } else {
        osLog(LOG_INFO, "sensorCfgGyro pending by:%d\n", mTask.nowState);
        mTask.pendingCaliCfg[GYRO] = true;
        for (i = 0; i < 12; ++i) {
            mTask.mSensorPerUnit[GYRO].pendCaliCfg[i] = values[i];
        }
    }
    return true;
}

static bool sensorSelfTestGyro(void *data)
{
    struct transferDataInfo dataInfo;

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_GYRO_SELFTEST;
        dataInfo.inBuf = NULL;
        dataInfo.inSize = 0;
        dataInfo.elemInSize = 0;
        dataInfo.outBuf = NULL;
        dataInfo.outSize = 0;
        dataInfo.elemOutSize = 0;
        if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_GYRO_SELFTEST, &i2cCallback, &spiCallback) < 0)
            mTask.nowState = CHIP_IDLE;
    } else {
        osLog(LOG_INFO, "gyroSelfTest pending by:%d\n", mTask.nowState);
        mTask.pendingSelfTest[GYRO] = true;
    }
    return true;
}

static bool anyMotionPower(bool on, void *cookie)
{
    struct transferDataInfo dataInfo;

    osLog(LOG_INFO, "anyMotionPower, on:%d, nowOn:%d\n", on, mTask.mSensorPerUnit[ANY_MOTION].nowOn);

    if (on) {
        if (mTask.nowState == CHIP_IDLE) {
            mTask.pendingConfig[ANY_MOTION] = false;
            mTask.mSensorPerUnit[ANY_MOTION].nowOn = on;
            mTask.nowState = CHIP_ANYMO_ENABLE;
            dataInfo.inBuf = NULL;
            dataInfo.inSize = 0;
            dataInfo.elemInSize = 0;
            dataInfo.outBuf = NULL;
            dataInfo.outSize = NULL;
            dataInfo.elemOutSize = NULL;
            sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ANYMO_ENABLE, &i2cCallback, &spiCallback);
        } else {
            osLog(LOG_INFO, "accGyro CAE pending by:%d\n", mTask.nowState);
            mTask.pendingConfig[ANY_MOTION] = true;
            mTask.mSensorPerUnit[ANY_MOTION].pendConfig.enable = on;
        }
    } else {
        if (mTask.nowState == CHIP_IDLE) {
            mTask.pendingConfig[ANY_MOTION] = false;
            mTask.mSensorPerUnit[ANY_MOTION].nowOn = on;
            mTask.nowState = CHIP_ANYMO_DISABLE;
            dataInfo.inBuf = NULL;
            dataInfo.inSize = 0;
            dataInfo.elemInSize = 0;
            dataInfo.outBuf = NULL;
            dataInfo.outSize = NULL;
            dataInfo.elemOutSize = NULL;
            sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ANYMO_DISABLE, &i2cCallback, &spiCallback);
        } else {
            osLog(LOG_INFO, "accGyro CAD pending by:%d\n", mTask.nowState);
            mTask.pendingConfig[ANY_MOTION] = true;
            mTask.mSensorPerUnit[ANY_MOTION].pendConfig.enable = on;
        }
    }

    return true;
}

static bool anyMotionSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    mTask.mSensorPerUnit[ANY_MOTION].rate = rate;
    mTask.mSensorPerUnit[ANY_MOTION].latency = latency;

    sensorSignalInternalEvt(mTask.mSensorPerUnit[ANY_MOTION].handle,
            SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool anyMotionFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask.mSensorPerUnit[ANY_MOTION].handle,
                            SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool anyMotionFlush(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_ANY_MOTION), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static bool noMotionPower(bool on, void *cookie)
{
    struct transferDataInfo dataInfo;

    osLog(LOG_INFO, "noMotionPower, on:%d, nowOn:%d\n", on, mTask.mSensorPerUnit[NO_MOTION].nowOn);

    if (on) {
        if (mTask.nowState == CHIP_IDLE) {
            mTask.pendingConfig[NO_MOTION] = false;
            mTask.mSensorPerUnit[NO_MOTION].nowOn = on;
            mTask.nowState = CHIP_NOMO_ENABLE;
            dataInfo.inBuf = NULL;
            dataInfo.inSize = 0;
            dataInfo.elemInSize = 0;
            dataInfo.outBuf = NULL;
            dataInfo.outSize = NULL;
            dataInfo.elemOutSize = NULL;
            sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_NOMO_ENABLE, &i2cCallback, &spiCallback);
        } else {
            osLog(LOG_INFO, "accGyro CNE pending by:%d\n", mTask.nowState);
            mTask.pendingConfig[NO_MOTION] = true;
            mTask.mSensorPerUnit[NO_MOTION].pendConfig.enable = on;
        }
    } else {
        if (mTask.nowState == CHIP_IDLE) {
            mTask.pendingConfig[NO_MOTION] = false;
            mTask.mSensorPerUnit[NO_MOTION].nowOn = on;
            mTask.nowState = CHIP_NOMO_DISABLE;
            dataInfo.inBuf = NULL;
            dataInfo.inSize = 0;
            dataInfo.elemInSize = 0;
            dataInfo.outBuf = NULL;
            dataInfo.outSize = NULL;
            dataInfo.elemOutSize = NULL;
            sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_NOMO_DISABLE, &i2cCallback, &spiCallback);
        } else {
            osLog(LOG_INFO, "accGyro CND pending by:%d\n", mTask.nowState);
            mTask.pendingConfig[NO_MOTION] = true;
            mTask.mSensorPerUnit[NO_MOTION].pendConfig.enable = on;
        }
    }

    return true;
}

static bool noMotionFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask.mSensorPerUnit[NO_MOTION].handle,
                            SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool noMotionSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    mTask.mSensorPerUnit[NO_MOTION].rate = rate;
    mTask.mSensorPerUnit[NO_MOTION].latency = latency;

    sensorSignalInternalEvt(mTask.mSensorPerUnit[NO_MOTION].handle,
            SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool noMotionFlush(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_NO_MOTION), SENSOR_DATA_EVENT_FLUSH, NULL);
}

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
static void stepCntSamplingCallback(uint32_t timerId, void *data)
{
    osEnqueuePrivateEvt(EVT_HW_STEPCNT_SAMPLE, NULL, NULL, mTask.id);
}

static bool stepCountPower(bool on, void *cookie)
{
    struct transferDataInfo dataInfo;

    osLog(LOG_INFO, "stepCountPower, on:%d, nowOn:%d\n", on, mTask.mSensorPerUnit[STEP_CNT].nowOn);
    if (on) {
        if (mTask.nowState == CHIP_IDLE) {
            mTask.pendingConfig[STEP_CNT] = false;
            mTask.mSensorPerUnit[STEP_CNT].nowOn = on;
            mTask.nowState = CHIP_STEP_COUNT_ENABLE;
            dataInfo.inBuf = NULL;
            dataInfo.inSize = 0;
            dataInfo.elemInSize = 0;
            dataInfo.outBuf = NULL;
            dataInfo.outSize = NULL;
            dataInfo.elemOutSize = NULL;
            sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_STEP_COUNT_ENABLE, &i2cCallback, &spiCallback);
        } else {
            osLog(LOG_INFO, "accGyro CSCE pending by:%d\n", mTask.nowState);
            mTask.pendingConfig[STEP_CNT] = true;
            mTask.mSensorPerUnit[STEP_CNT].pendConfig.enable = on;
        }
    } else {
        if (mTask.nowState == CHIP_IDLE) {
            if (mTask.stepCntSamplingTimerHandle) {
                timTimerCancel(mTask.stepCntSamplingTimerHandle);
                mTask.stepCntSamplingTimerHandle = 0;
            }
            mTask.pendingConfig[STEP_CNT] = false;
            mTask.mSensorPerUnit[STEP_CNT].nowOn = on;
            mTask.nowState = CHIP_STEP_COUNT_DISABLE;
            dataInfo.inBuf = NULL;
            dataInfo.inSize = 0;
            dataInfo.elemInSize = 0;
            dataInfo.outBuf = NULL;
            dataInfo.outSize = NULL;
            dataInfo.elemOutSize = NULL;
            sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_STEP_COUNT_DISABLE, &i2cCallback, &spiCallback);
        } else {
            osLog(LOG_INFO, "accGyro CSCD pending by:%d\n", mTask.nowState);
            mTask.pendingConfig[STEP_CNT] = true;
            mTask.mSensorPerUnit[STEP_CNT].pendConfig.enable = on;
        }
    }

    return true;
}

static bool stepCountFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask.mSensorPerUnit[STEP_CNT].handle,
                            SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool stepCountSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    uint64_t report_rate;
    mTask.mSensorPerUnit[STEP_CNT].rate = rate;
    mTask.mSensorPerUnit[STEP_CNT].latency = latency;

    osLog(LOG_INFO, "stepCountSetRate: rate=%d\n", rate);
    /* set 2s to report data */
    report_rate = 2000000000;
    if (mTask.stepCntSamplingTimerHandle) {
        timTimerCancel(mTask.stepCntSamplingTimerHandle);
        mTask.stepCntSamplingTimerHandle = 0;
    }
    mTask.stepCntSamplingTimerHandle = timTimerSet(report_rate , 0, 50, stepCntSamplingCallback, NULL, false);
    sensorSignalInternalEvt(mTask.mSensorPerUnit[STEP_CNT].handle,
            SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool stepCountFlush(void *cookie)
{
   return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_STEP_COUNT), SENSOR_DATA_EVENT_FLUSH, NULL);
}
#endif

#define DEC_INFO(name, type, axis, inter, samples) \
    .sensorName = name, \
    .sensorType = type, \
    .numAxis = axis, \
    .interrupt = inter, \
    .minSamples = samples

#define DEC_INFO_RATE(name, rates, type, axis, inter, samples) \
    DEC_INFO(name, type, axis, inter, samples), \
    .supportedRates = rates

#define DEC_INFO_RATE_RAW(name, rates, type, axis, inter, samples, raw, scale) \
    DEC_INFO(name, type, axis, inter, samples), \
    .supportedRates = rates, \
    .flags1 = SENSOR_INFO_FLAGS1_RAW, \
    .rawType = raw, \
    .rawScale = scale

#define DEC_INFO_RATE_BIAS(name, rates, type, axis, inter, samples, bias) \
    DEC_INFO(name, type, axis, inter, samples), \
    .supportedRates = rates, \
    .flags1 = SENSOR_INFO_FLAGS1_BIAS, \
    .biasType = bias

#define DEC_OPS(power, firmware, rate, flush) \
    .sensorPower = power, \
    .sensorFirmwareUpload = firmware, \
    .sensorSetRate = rate, \
    .sensorFlush = flush

#define DEC_OPS_SEND(power, firmware, rate, flush, send) \
    DEC_OPS(power, firmware, rate, flush), \
    .sensorSendOneDirectEvt = send

#define DEC_OPS_CAL_CFG(power, firmware, rate, flush, cal, cfg) \
    DEC_OPS(power, firmware, rate, flush), \
    .sensorCalibrate = cal, \
    .sensorCfgData = cfg

#define DEC_OPS_CFG(power, firmware, rate, flush, cfg) \
    DEC_OPS(power, firmware, rate, flush), \
    .sensorCfgData = cfg

#define DEC_OPS_ALL(power, firmware, rate, flush, cal, cfg, selftest) \
    DEC_OPS(power, firmware, rate, flush), \
    .sensorCalibrate = cal, \
    .sensorCfgData = cfg, \
    .sensorSelfTest = selftest

static const struct SensorInfo mSensorInfo[MAX_HANDLE] = {
    { DEC_INFO_RATE_BIAS("Accelerometer", accSupportedRates, SENS_TYPE_ACCEL, NUM_AXIS_THREE, NANOHUB_INT_NONWAKEUP, 3000, SENS_TYPE_ACC_BIAS) },
    { DEC_INFO_RATE_BIAS("Gyroscope", gyroSupportedRates, SENS_TYPE_GYRO, NUM_AXIS_THREE, NANOHUB_INT_NONWAKEUP, 20, SENS_TYPE_GYRO_BIAS) },
    { DEC_INFO("Any Motion", SENS_TYPE_ANY_MOTION, NUM_AXIS_EMBEDDED, NANOHUB_INT_NONWAKEUP, 20) },
    { DEC_INFO("No Motion", SENS_TYPE_NO_MOTION, NUM_AXIS_EMBEDDED, NANOHUB_INT_NONWAKEUP, 20) },
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    { DEC_INFO_RATE("Step Counter", StepCntRates, SENS_TYPE_STEP_COUNT, NUM_AXIS_EMBEDDED, NANOHUB_INT_NONWAKEUP, 20) },
#endif
};

static const struct SensorOps mSensorOps[MAX_HANDLE] = {
    { DEC_OPS_ALL(sensorPowerAcc, sensorFirmwareAcc, sensorRateAcc, sensorFlushAcc, sensorCaliAcc, sensorCfgAcc, sensorSelfTestAcc) },
    { DEC_OPS_ALL(sensorPowerGyro, sensorFirmwareGyro, sensorRateGyro, sensorFlushGyro, sensorCaliGyro, sensorCfgGyro, sensorSelfTestGyro) },
    { DEC_OPS(anyMotionPower, anyMotionFirmwareUpload, anyMotionSetRate, anyMotionFlush) },
    { DEC_OPS(noMotionPower, noMotionFirmwareUpload, noMotionSetRate, noMotionFlush) },
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
    { DEC_OPS(stepCountPower, stepCountFirmwareUpload, stepCountSetRate, stepCountFlush) },
#endif
};

static void sendFlushEvt(void)
{
    while (mTask.mSensorPerUnit[ACC].flush > 0) {
        osLog(LOG_INFO, "acc sendFlushEvt\n");
        osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_ACCEL), SENSOR_DATA_EVENT_FLUSH, NULL);
        mTask.mSensorPerUnit[ACC].flush--;
    }
    while (mTask.mSensorPerUnit[GYRO].flush > 0) {
        osLog(LOG_INFO, "gyro sendFlushEvt\n");
        osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_GYRO), SENSOR_DATA_EVENT_FLUSH, NULL);
        mTask.mSensorPerUnit[GYRO].flush--;
    }
}

static bool allocateDataEvt(struct sensPerUnit *sensorTask, uint64_t timeStamp)
{
    sensorTask->data_evt = slabAllocatorAlloc(mTask.mDataSlab);

    if (sensorTask->data_evt == NULL) {
        osLog(LOG_ERROR, "accGyro: Slab allocation failed\n");
        return false;
    }
    memset(&sensorTask->data_evt->samples[0].firstSample, 0x00, sizeof(struct SensorFirstSample));
    sensorTask->data_evt->referenceTime = timeStamp;
    sensorTask->prev_rtc_time = timeStamp;

    return true;
}

static void dataEvtFree(void *ptr)
{
    struct TripleAxisDataEvent *ev = (struct TripleAxisDataEvent *)ptr;
    slabAllocatorFree(mTask.mDataSlab, ev);
}

// Returns temperature in units of 0.01 degrees celsius.

static void flushRawData(void)
{
    enum sensorHandleState handle;

    for (handle = ACC; handle < MAX_HANDLE; handle++) {
        if (mTask.mSensorPerUnit[handle].data_evt) {
             if (!osEnqueueEvt(EVENT_TYPE_BIT_DISCARDABLE | sensorGetMyEventType(mSensorInfo[handle].sensorType),
                     mTask.mSensorPerUnit[handle].data_evt, dataEvtFree)) {
                 osLog(LOG_INFO, "accGyro: handle %d flush sensor data when disable\n",
                    mTask.mSensorPerUnit[handle].data_evt->samples[0].firstSample.numSamples);
                 dataEvtFree(mTask.mSensorPerUnit[handle].data_evt);
             }
             mTask.mSensorPerUnit[handle].data_evt = NULL;
         }
    }
}
static void flushBiasData(struct sensPerUnit *sensorTask, uint32_t eventId)
{
    if (sensorTask->data_evt) {
        if (!osEnqueueEvt(eventId, sensorTask->data_evt, dataEvtFree)) {
            // don't log error since queue is already full. silently drop
            // this data event.
            dataEvtFree(sensorTask->data_evt);
        }
        sensorTask->data_evt = NULL;
    }
}

void accGyroSendCalibrationResult(uint8_t sensorType,
        int32_t bias[3], uint8_t status) {
    struct CalibrationData *data = heapAlloc(sizeof(struct CalibrationData));
    if (!data) {
        osLog(LOG_ERROR, "Couldn't alloc data cali result pkt");
        return;
    }

    data->header.appId = APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ACCEL, 0, 0));
    data->header.dataLen = (sizeof(struct CalibrationData) - sizeof(struct HostHubRawPacket));
    data->data_header.msgId = SENSOR_APP_MSG_ID_CAL_RESULT;
    data->data_header.sensorType = sensorType;
    data->data_header.status = status;

    data->values[0] = bias[0];
    data->values[1] = bias[1];
    data->values[2] = bias[2];

#ifndef CFG_MTK_CALIBRATION_V1_0
    if (sensorType == SENS_TYPE_ACCEL)
        Acc_update_accurancy((int *)bias);
#endif

    if (!osEnqueueEvtOrFree(EVT_APP_TO_HOST, data, heapFree))
        osLog(LOG_ERROR, "Couldn't send data cali result evt");
}

static void accGyroSendTemperatureCaliResult(uint8_t sensorType,
        int32_t bias[6]) {
    struct CalibrationData *data = heapAlloc(sizeof(struct CalibrationData));
    if (!data) {
        osLog(LOG_ERROR, "Couldn't alloc temp cali result pkt");
        return;
    }

    data->header.appId = APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ACCEL, 0, 0));
    data->header.dataLen = (sizeof(struct CalibrationData) - sizeof(struct HostHubRawPacket));
    data->data_header.msgId = SENSOR_APP_MSG_ID_TEMP_RESULT;
    data->data_header.sensorType = sensorType;
    data->data_header.status = 0;

    data->values[0] = bias[0];
    data->values[1] = bias[1];
    data->values[2] = bias[2];
    data->values[3] = bias[3];
    data->values[4] = bias[4];
    data->values[5] = bias[5];

    if (!osEnqueueEvtOrFree(EVT_APP_TO_HOST, data, heapFree))
        osLog(LOG_ERROR, "Couldn't send temp cali result evt");
}

void accGyroSendTestResult(uint8_t sensorType, uint8_t status) {
    struct TestResultData *data = heapAlloc(sizeof(struct TestResultData));
    if (!data) {
        osLog(LOG_WARN, "Couldn't alloc test result packet");
        return;
    }

    data->header.appId = APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ACCEL, 0, 0));
    data->header.dataLen = (sizeof(struct TestResultData) - sizeof(struct HostHubRawPacket));
    data->data_header.msgId = SENSOR_APP_MSG_ID_TEST_RESULT;
    data->data_header.sensorType = sensorType;
    data->data_header.status = status;

    if (!osEnqueueEvtOrFree(EVT_APP_TO_HOST, data, heapFree))
        osLog(LOG_WARN, "Couldn't send test result packet");
}

extern float gyroBias[3];
extern float accTemp;
extern int temp_status;

static void parseRawData(struct sensPerUnit *sensorTask,
    const struct accGyroData *outBuf, uint64_t timeStamp, float temperature_c)
{
    struct TripleAxisDataPoint *sample;
    int32_t delta_time = 0;
    float raw_data_input[3];
    float calibrated_data_output[3];
    int32_t accAccurancy = 0, gyroAccurancy = 0;
    bool newGyroBias = false, biasUpdate = false, newAccBias = false;
    static uint8_t acc_last_accurancy = 0;
    float cali_bias[3] = {0.0f, 0.0f, 0.0f};
#ifndef CFG_MTK_CALIBRATION_V1_0
    static uint8_t gyro_last_accurancy = 0;
    float bias[3], slope[3], intercept[3];
#endif

    memset(calibrated_data_output, 0, sizeof(calibrated_data_output));

    if (sensorTask->data_evt == NULL) {
        if (!allocateDataEvt(sensorTask, timeStamp))
            return;
    }
    if (sensorTask->data_evt->samples[0].firstSample.numSamples >= MAX_NUM_COMMS_EVENT_SAMPLES) {
       osLog(LOG_ERROR, "sensType %d parseRawData BAD INDEX\n", outBuf->sensType);
       return;
    }
    raw_data_input[0] = outBuf->x;
    raw_data_input[1] = outBuf->y;
    raw_data_input[2] = outBuf->z;
    if (outBuf->sensType == SENS_TYPE_ACCEL) {
#ifndef CFG_MTK_CALIBRATION_V1_0
        Acc_run_calibration(delta_time, raw_data_input, calibrated_data_output, (int *)&accAccurancy);
#else
        calibrated_data_output[0] = raw_data_input[0];
        calibrated_data_output[1] = raw_data_input[1];
        calibrated_data_output[2] = raw_data_input[2];
        accAccurancy = 3;
        gyroCalUpdateAccel(&mTask.gyro_cal, timeStamp,
            calibrated_data_output[0], calibrated_data_output[1], calibrated_data_output[2]);
#endif
        if (accAccurancy != acc_last_accurancy)
            newAccBias = true;
        else
            newAccBias = false;
        acc_last_accurancy = accAccurancy;
        if (newAccBias) {
            if (sensorTask->data_evt->samples[0].firstSample.numSamples > 0) {
                // flush existing samples so the bias appears after them
                flushRawData();
                if (!allocateDataEvt(sensorTask, timeStamp))
                    return;
            }
            if (accAccurancy >= 2)
                sensorTask->data_evt->samples[0].firstSample.biasCurrent = 1;
            else
                sensorTask->data_evt->samples[0].firstSample.biasCurrent = 0;
            sensorTask->data_evt->samples[0].firstSample.biasPresent = 1;
            sensorTask->data_evt->samples[0].firstSample.biasSample =
                     sensorTask->data_evt->samples[0].firstSample.numSamples;
            sample = &sensorTask->data_evt->samples[sensorTask->data_evt->samples[0].firstSample.numSamples++];
            sample->x = raw_data_input[0] - calibrated_data_output[0];
            sample->y = raw_data_input[1] - calibrated_data_output[1];
            sample->z = raw_data_input[2] - calibrated_data_output[2];
            flushBiasData(sensorTask, sensorGetMyEventType(SENS_TYPE_ACC_BIAS));
            // bias is non-discardable, if we fail to enqueue, don't clear new_mag_bias
            if (!allocateDataEvt(sensorTask, timeStamp))
                return;
        }
    } else if (outBuf->sensType == SENS_TYPE_GYRO) {
#ifndef CFG_MTK_CALIBRATION_V1_0
        //Gyro_run_calibration(delta_time, 1, &raw_data_input, &calibrated_data_output, (int *)&gyroAccurancy);
        Gyro_run_calibration(delta_time, raw_data_input, calibrated_data_output, (int *)&gyroAccurancy, temperature_c);
        Gyro_get_calibration_parameter(bias, slope, intercept);
        /*too many if statement, should move the judgement into libk in the future*/
        if ((slope[0] != 0) && (slope[1] != 0) && (slope[2] != 0)) {
            temp_status = 1;
        } else {
            temp_status = 0;
        }
        accTemp = temperature_c;
        cali_bias[0] = raw_data_input[0] - calibrated_data_output[0];
        cali_bias[1] = raw_data_input[1] - calibrated_data_output[1];
        cali_bias[2] = raw_data_input[2] - calibrated_data_output[2];
        gyroBias[0] = cali_bias[0];
        gyroBias[1] = cali_bias[1];
        gyroBias[2] = cali_bias[2];
        if (gyroAccurancy != gyro_last_accurancy)
            newGyroBias = true;
        else
            newGyroBias = false;
        gyro_last_accurancy = gyroAccurancy;
        biasUpdate = newGyroBias;
#else
        float gyro_offset_temperature_celsius = 0.0f;
        uint64_t calibration_time_nanos = 0;
        gyroCalUpdateGyro(&mTask.gyro_cal, timeStamp,
            raw_data_input[0], raw_data_input[1], raw_data_input[2],
            temperature_c);
        if (mTask.over_temp_gyro_cal.over_temp_enable) {
            overTempCalSetTemperature(&mTask.over_temp_gyro_cal, timeStamp,
                temperature_c);
            overTempCalRemoveOffset(&mTask.over_temp_gyro_cal, timeStamp,
                raw_data_input[0], raw_data_input[1], raw_data_input[2],
                &calibrated_data_output[0], &calibrated_data_output[1],
                &calibrated_data_output[2]);
            static size_t print_flag = 0;
            // This flag keeps GyroCal and OverTempCal from printing back-to-back.
            // If they do, then sometimes important print log data gets dropped.
            if (print_flag > 0) {
                // Gyro Cal -- Read out Debug data.
#ifdef GYRO_CAL_DBG_ENABLED
                gyroCalDebugPrint(&mTask.gyro_cal, timeStamp);
#endif
                print_flag = 0;
            } else {
                // Over-Temp Gyro Cal -- Read out Debug data.
#ifdef OVERTEMPCAL_DBG_ENABLED
                overTempCalDebugPrint(&mTask.over_temp_gyro_cal, timeStamp);
#endif
                print_flag = 1;
            }
            newGyroBias = gyroCalNewBiasAvailable(&mTask.gyro_cal);
            if (newGyroBias) {
                gyroCalGetBias(&mTask.gyro_cal, &cali_bias[0], &cali_bias[1],
                    &cali_bias[2], &gyro_offset_temperature_celsius,
                    &calibration_time_nanos);
                overTempCalUpdateSensorEstimate(&mTask.over_temp_gyro_cal, timeStamp,
                    cali_bias,
                    gyro_offset_temperature_celsius);
            }
            bool new_otc_offset_update =
                overTempCalNewOffsetAvailable(&mTask.over_temp_gyro_cal);
            overTempCalGetOffset(&mTask.over_temp_gyro_cal,
                &gyro_offset_temperature_celsius, cali_bias);
            bool new_otc_model_update =
                overTempCalNewModelUpdateAvailable(&mTask.over_temp_gyro_cal);
            if (new_otc_model_update) {
                uint64_t timestamp = 0;
                float lastTemperature = 0, lastOffset[3] = {0}, sensitivity[3] = {0}, intercept[3] = {0};
                int32_t gyroTempCali[6] = {0};
                overTempCalGetModel(&mTask.over_temp_gyro_cal, lastOffset, &lastTemperature,
                    &timestamp, sensitivity, intercept);
                gyroTempCali[0] = sensitivity[0] * GYRO_TEMPERATURE_CONVERT;
                gyroTempCali[1] = sensitivity[1] * GYRO_TEMPERATURE_CONVERT;
                gyroTempCali[2] = sensitivity[2] * GYRO_TEMPERATURE_CONVERT;
                gyroTempCali[3] = intercept[0] * GYRO_TEMPERATURE_CONVERT;
                gyroTempCali[4] = intercept[1] * GYRO_TEMPERATURE_CONVERT;
                gyroTempCali[5] = intercept[2] * GYRO_TEMPERATURE_CONVERT;
                accGyroSendTemperatureCaliResult(SENS_TYPE_GYRO, gyroTempCali);
            }
            if (newGyroBias && new_otc_offset_update) {
                biasUpdate = true;
                gyroAccurancy = 3;
            }
            if (new_otc_model_update)
                 temp_status = 1;
            accTemp = temperature_c;
            gyroBias[0] = cali_bias[0];
            gyroBias[1] = cali_bias[1];
            gyroBias[2] = cali_bias[2];
        } else {
            gyroCalRemoveBias(&mTask.gyro_cal,
                raw_data_input[0], raw_data_input[1], raw_data_input[2],
                &calibrated_data_output[0], &calibrated_data_output[1],
                &calibrated_data_output[2]);
#ifdef GYRO_CAL_DBG_ENABLED
            gyroCalDebugPrint(&mTask.gyro_cal, timeStamp);
#endif
            newGyroBias = gyroCalNewBiasAvailable(&mTask.gyro_cal);
            gyroCalGetBias(&mTask.gyro_cal, &cali_bias[0], &cali_bias[1],
                &cali_bias[2], &gyro_offset_temperature_celsius,
                    &calibration_time_nanos);
            biasUpdate = newGyroBias;
            temp_status = 0;
            accTemp = temperature_c;
            gyroBias[0] = cali_bias[0];
            gyroBias[1] = cali_bias[1];
            gyroBias[2] = cali_bias[2];
        }
#endif
        if (biasUpdate) {
            if (sensorTask->data_evt->samples[0].firstSample.numSamples > 0) {
                // flush existing samples so the bias appears after them
                flushRawData();
                if (!allocateDataEvt(sensorTask, timeStamp))
                    return;
            }
            if (gyroAccurancy >= 2)
                sensorTask->data_evt->samples[0].firstSample.biasCurrent = 1;
            else
                sensorTask->data_evt->samples[0].firstSample.biasCurrent = 0;
            sensorTask->data_evt->samples[0].firstSample.biasPresent = 1;
            sensorTask->data_evt->samples[0].firstSample.biasSample =
                     sensorTask->data_evt->samples[0].firstSample.numSamples;
            sample = &sensorTask->data_evt->samples[sensorTask->data_evt->samples[0].firstSample.numSamples++];
            sample->x = cali_bias[0];
            sample->y = cali_bias[1];
            sample->z = cali_bias[2];
            flushBiasData(sensorTask, sensorGetMyEventType(SENS_TYPE_GYRO_BIAS));
            // bias is non-discardable, if we fail to enqueue, don't clear new_mag_bias
            if (!allocateDataEvt(sensorTask, timeStamp))
                return;
        }
    }
    sample = &sensorTask->data_evt->samples[sensorTask->data_evt->samples[0].firstSample.numSamples++];
    /* osLog(LOG_ERROR, "sensType:%d numSamples:%d, rtc_time:%lld, prev_rtc_time:%lld\n", outBuf->sensType,
        sensorTask->data_evt->samples[0].firstSample.numSamples,
        timeStamp, sensorTask->prev_rtc_time); */
    if (sensorTask->data_evt->samples[0].firstSample.numSamples > 1) {
        delta_time = timeStamp - sensorTask->prev_rtc_time;
        if (UNLIKELY(timeStamp < sensorTask->prev_rtc_time)) {
            osLog(LOG_ERROR, "sensType:%d delta_time(%ld) < 0, numSamples:%d, timestamp:%lld, pretimestamp:%lld\n",
               outBuf->sensType, delta_time,
               sensorTask->data_evt->samples[0].firstSample.numSamples, timeStamp, sensorTask->prev_rtc_time);
            configASSERT(0);
        } else if (UNLIKELY((timeStamp - sensorTask->prev_rtc_time) > 1000000000)) {
            osLog(LOG_ERROR, "sensType:%d delta_time(%ld) > 1000000000, numSamples:%d, timestamp:%lld, pretimestamp:%lld\n",
               outBuf->sensType, delta_time,
               sensorTask->data_evt->samples[0].firstSample.numSamples, timeStamp, sensorTask->prev_rtc_time);
            configASSERT(0);
        }
        delta_time = delta_time < 0 ? 0 : delta_time;
        sample->deltaTime = delta_time;
        sensorTask->prev_rtc_time = timeStamp;
    }
    sample->x = calibrated_data_output[0];
    sample->y = calibrated_data_output[1];
    sample->z = calibrated_data_output[2];
    /* osLog(LOG_DEBUG, "sensType:%d time:%lld, x: %f, y: %f, z: %f\n", outBuf->sensType, timeStamp,
       (double)sample->x, (double)sample->y, (double)sample->z); */

    if (sensorTask->data_evt->samples[0].firstSample.numSamples == MAX_NUM_COMMS_EVENT_SAMPLES) {
        if (!osEnqueueEvt(EVENT_TYPE_BIT_DISCARDABLE | sensorGetMyEventType(outBuf->sensType),
                sensorTask->data_evt, dataEvtFree)) {
            dataEvtFree(sensorTask->data_evt);
        }
        sensorTask->data_evt = NULL;
        //osLog(LOG_DEBUG, "sensType:%d time:%lld, nowtime: %lld\n", outBuf->sensType, timeStamp, rtcGetTime());
    }
}

static void configEvent(struct sensPerUnit *mSensor, struct ConfigStat *ConfigData)
{
    enum sensorHandleState handle;

    for (handle = ACC; &mTask.mSensorPerUnit[handle] != mSensor; handle++) ;

    if (ConfigData->enable == 0 && mSensor->nowOn)
        mSensorOps[handle].sensorPower(false, (void *)handle);
    else if (ConfigData->enable == 1 && !mSensor->nowOn)
        mSensorOps[handle].sensorPower(true, (void *)handle);
    else
        mSensorOps[handle].sensorSetRate(ConfigData->rate, ConfigData->latency, (void *)handle);
}

static void processPendingEvt(void)
{
    enum sensorHandleState handle;
    /* pending event int first */
    if (mTask.pendingInt) {
        mTask.pendingInt = false;
        osEnqueuePrivateEvt(EVT_SENSOR_INTR, NULL, NULL, mTask.id);
        return;
    }
    /* pending event sample second */
    if (mTask.pendingSample) {
        mTask.pendingSample = false;
        osEnqueuePrivateEvt(EVT_SENSOR_TIMER, NULL, NULL, mTask.id);
        return;
    }
    /* pending event control third */
    for (handle = ACC; handle < MAX_HANDLE; handle++) {
        if (mTask.pendingConfig[handle]) {
            mTask.pendingConfig[handle] = false;
            configEvent(&mTask.mSensorPerUnit[handle], &mTask.mSensorPerUnit[handle].pendConfig);
            return;
        }
    }
    /* pending event HwIntStatusCheck sixth */
    if (mTask.pendingHwIntStatusCheck) {
        mTask.pendingHwIntStatusCheck = false;
        osEnqueuePrivateEvt(EVT_HW_INT_STATUS_CHECK, NULL, NULL, mTask.id);
        return;
    }
    /* pending cali seventh */
    for (handle = ACC; handle < MAX_HANDLE; handle++) {
        if (mTask.pendingCali[handle]) {
            mTask.pendingCali[handle] = false;
            mSensorOps[handle].sensorCalibrate((void *)handle);
            return;
        }
    }
    /* pending cali eighth */
    for (handle = ACC; handle < MAX_HANDLE; handle++) {
        if (mTask.pendingCaliCfg[handle]) {
            //osLog(LOG_INFO, "processPendingEvt handle :%d\n", handle);
            mTask.pendingCaliCfg[handle] = false;
            mSensorOps[handle].sensorCfgData(mTask.mSensorPerUnit[handle].pendCaliCfg,
                (void *)handle);
            return;
        }
    }
    /* pending selftest */
    for (handle = ACC; handle < MAX_HANDLE; handle++) {
        if (mTask.pendingSelfTest[handle]) {
            mTask.pendingSelfTest[handle] = false;
            mSensorOps[handle].sensorSelfTest((void *)handle);
            return;
        }
    }
}

static void handleSensorEvent(const void *state)
{
    uint8_t i = 0;
    uint64_t timeStamp = 0, final_sample_time_acc = 0, final_sample_time_gyro = 0;
    uint64_t eventTimeStamp = 0, nowTime = 0, destTime = 0;
    enum AccGyroState mState = (enum AccGyroState)state;
    uint32_t timerDelay = 0;
    uint8_t accOutSize = 0;
    uint8_t gyroOutSize = 0;
#ifndef CFG_MTK_CALIBRATION_V1_0
    int32_t gyroTempCali[6] = {0};
#endif

    //osLog(LOG_DEBUG, "handleSensorEvent: %d\n", mState);
    switch (mState) {
        default: {
            sensorFsmSwitchState(NULL, &mTask.fsm, &i2cCallback, &spiCallback);
            break;
        }
        case CHIP_IDLE: {
            //osLog(LOG_DEBUG, "accGyro: STATE_IDLE done\n");
            break;
        }

        case CHIP_INIT_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            if (mTask.mSensorPerUnit[ACC].handle)
                sensorRegisterInitComplete(mTask.mSensorPerUnit[ACC].handle);
            if (mTask.mSensorPerUnit[GYRO].handle)
                sensorRegisterInitComplete(mTask.mSensorPerUnit[GYRO].handle);
            if (mTask.mSensorPerUnit[ANY_MOTION].handle)
                sensorRegisterInitComplete(mTask.mSensorPerUnit[ANY_MOTION].handle);
            if (mTask.mSensorPerUnit[NO_MOTION].handle)
                sensorRegisterInitComplete(mTask.mSensorPerUnit[NO_MOTION].handle);
#ifdef CFG_HW_STEP_COUNTER_SUPPORT
            if (mTask.mSensorPerUnit[STEP_CNT].handle)
                sensorRegisterInitComplete(mTask.mSensorPerUnit[STEP_CNT].handle);
#endif
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "accGyro: init done\n");
            processPendingEvt();
            break;
        }

        case CHIP_HW_INT_HANDLING_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            /* osLog(LOG_DEBUG, "accGyro HW_INT_HANDLING done\n"); */
            processPendingEvt();
            break;
        }

        case CHIP_ANYMO_ENABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[ANY_MOTION].handle,
                SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "anymo: enable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_ANYMO_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[ANY_MOTION].handle,
                SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "anymo: disable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_NOMO_ENABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[NO_MOTION].handle,
                SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "nomo: enable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_NOMO_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[NO_MOTION].handle,
                SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "nomo: disable done\n");
            processPendingEvt();
            break;
        }

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
        case CHIP_STEP_COUNT_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[STEP_CNT].handle,
                SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "stepCNT: enable done\n");
            processPendingEvt();
            break;
        }
        case CHIP_STEP_COUNT_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[STEP_CNT].handle,
                SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "stepCNT: disable done\n");
            processPendingEvt();
            break;
        }
        case CHIP_STEP_COUNT_SAMPLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            /*osLog(LOG_DEBUG, "stepCNT: sample done\n");*/
            processPendingEvt();
            break;
        }
#endif

        case CHIP_ACC_ENABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[ACC].handle,
                SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "acc: enable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_ACC_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[ACC].handle,
                SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "acc: disable done\n");
            processPendingEvt();
            /* when disable sensor we should flush the sensor data remain in data_evt pointer buffer */
            flushRawData();
            break;
        }

        case CHIP_ACC_CALI_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "acc: cali done\n");
            processPendingEvt();
            break;
        }

        case CHIP_ACC_CFG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "acc: cfg done\n");
            processPendingEvt();
            break;
        }

        case CHIP_ACC_SELFTEST_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "acc: selftest done\n");
            processPendingEvt();
            break;
        }

        case CHIP_ACC_RATECHG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            switch (mTask.intrMode) {
                case ACC_GYRO_FIFO_UNINTERRUPTIBLE: {
                    if (mTask.timerHandle)
                        timTimerCancel(mTask.timerHandle);
                    if (mTask.mSensorPerUnit[GYRO].nowOn) {
                        timerDelay =
                            (mTask.mSensorPerUnit[GYRO].timerDelay > mTask.mSensorPerUnit[ACC].timerDelay) ?
                            mTask.mSensorPerUnit[ACC].timerDelay : mTask.mSensorPerUnit[GYRO].timerDelay;
                    } else {
                        timerDelay = mTask.mSensorPerUnit[ACC].timerDelay;
                    }
                    mTask.timerHandle =
                        timTimerSet(timerDelay, 0, 50, accGyroTimerCallback, NULL, false);
                    break;
                }
                case GYRO_ONLY_FIFO_INTERRUPTIBLE: {
                    if (mTask.timerHandle)
                        timTimerCancel(mTask.timerHandle);
                    timerDelay = mTask.mSensorPerUnit[ACC].timerDelay;
                    mTask.timerHandle =
                        timTimerSet(timerDelay, 0, 50, accGyroTimerCallback, NULL, false);
                    break;
                }
                case ACC_GYRO_FIFO_INTERRUPTIBLE: {
                    break;
                }
            }

            sensorSignalInternalEvt(mTask.mSensorPerUnit[ACC].handle,
                SENSOR_INTERNAL_EVT_RATE_CHG, mTask.mSensorPerUnit[ACC].rate, mTask.mSensorPerUnit[ACC].latency);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "acc: ratechg done\n");
            processPendingEvt();
            break;
        }
        case CHIP_GYRO_ENABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[GYRO].handle,
                SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "gyro: enable done\n");
#ifndef CFG_MTK_CALIBRATION_V1_0
            Gyro_init_calibration(mTask.slope, mTask.intercept);
#endif
            processPendingEvt();
            break;
        }

        case CHIP_GYRO_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[GYRO].handle,
                SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "gyro: disable done\n");
#ifndef CFG_MTK_CALIBRATION_V1_0
            Gyro_get_calibration_parameter(mTask.bias, mTask.slope, mTask.intercept);

            gyroTempCali[0] = mTask.slope[0] * GYRO_TEMPERATURE_CONVERT;
            gyroTempCali[1] = mTask.slope[1] * GYRO_TEMPERATURE_CONVERT;
            gyroTempCali[2] = mTask.slope[2] * GYRO_TEMPERATURE_CONVERT;

            gyroTempCali[3] = mTask.intercept[0] * GYRO_TEMPERATURE_CONVERT;
            gyroTempCali[4] = mTask.intercept[1] * GYRO_TEMPERATURE_CONVERT;
            gyroTempCali[5] = mTask.intercept[2] * GYRO_TEMPERATURE_CONVERT;
            accGyroSendTemperatureCaliResult(SENS_TYPE_GYRO, gyroTempCali);
#endif
            processPendingEvt();
            /* when disable sensor we should flush the sensor data remain in data_evt pointer buffer */
            flushRawData();
            break;
        }

        case CHIP_GYRO_CALI_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "gyro: cali done\n");
            processPendingEvt();
            break;
        }

        case CHIP_GYRO_CFG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "gyro: cfg done\n");
            processPendingEvt();
            break;
        }

        case CHIP_GYRO_SELFTEST_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "gyro: selftest done\n");
            processPendingEvt();
            break;
        }

        case CHIP_GYRO_RATECHG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            switch (mTask.intrMode) {
                case ACC_GYRO_FIFO_UNINTERRUPTIBLE: {
                    if (mTask.timerHandle)
                        timTimerCancel(mTask.timerHandle);
                    if (mTask.mSensorPerUnit[ACC].nowOn) {
                        timerDelay =
                            (mTask.mSensorPerUnit[GYRO].timerDelay > mTask.mSensorPerUnit[ACC].timerDelay) ?
                            mTask.mSensorPerUnit[ACC].timerDelay : mTask.mSensorPerUnit[GYRO].timerDelay;
                    } else {
                        timerDelay = mTask.mSensorPerUnit[GYRO].timerDelay;
                    }
                    mTask.timerHandle =
                        timTimerSet(timerDelay, 0, 50, accGyroTimerCallback, NULL, false);
                    break;
                }
                case GYRO_ONLY_FIFO_INTERRUPTIBLE: {
                    break;
                }
                case ACC_GYRO_FIFO_INTERRUPTIBLE: {
                    break;
                }
            }
            sensorSignalInternalEvt(mTask.mSensorPerUnit[GYRO].handle,
                SENSOR_INTERNAL_EVT_RATE_CHG, mTask.mSensorPerUnit[GYRO].rate, mTask.mSensorPerUnit[GYRO].latency);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "gyro: ratechg done\n");
            processPendingEvt();
            break;
        }
        case CHIP_SAMPLING_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            if ((mTask.mSensorPerUnit[ACC].nowOn || mTask.mSensorPerUnit[GYRO].nowOn) && mTask.isReading) {
                timeStamp = mTask.packet->timeStamp;
                accOutSize = mTask.packet->accOutSize;
                gyroOutSize = mTask.packet->gyroOutSize;
                nowTime = rtcGetTime();
                //osLog(LOG_DEBUG, "interrupt time:%lld, outSize:%d\n", timeStamp, mTask.outSize);
                //osLog(LOG_DEBUG, "temperature=%f\n", (double)mTask.packet->temperature);
                if (mTask.mSensorPerUnit[ACC].nowOn && accOutSize != 0) {
                    AccSensor_calibrate_fifo_timestamp(timeStamp, accOutSize,
                    (unsigned long long *)&final_sample_time_acc, (unsigned long long *)&mTask.mSensorPerUnit[ACC].fifoDelay);
                    //osLog(LOG_DEBUG, "acc_final %lld nowtime %lld Delay %d cn %d\n", final_sample_time_acc,
                        //timeStamp, mTask.mSensorPerUnit[ACC].fifoDelay, accOutSize);
                }
                if (mTask.mSensorPerUnit[GYRO].nowOn && gyroOutSize != 0) {
                    GyroSensor_calibrate_fifo_timestamp(timeStamp, gyroOutSize,
                    (unsigned long long *)&final_sample_time_gyro, (unsigned long long *)&mTask.mSensorPerUnit[GYRO].fifoDelay);
                    //osLog(LOG_DEBUG, "gyro_final %lld nowtime %lld Delay %d cn %d\n", final_sample_time_gyro,
                        //timeStamp, mTask.mSensorPerUnit[GYRO].fifoDelay, gyroOutSize);
                }
                for (i = 0; i < mTask.outSize; ++i) {
                    if (mTask.packet->outBuf[i].sensType == SENS_TYPE_ACCEL && mTask.mSensorPerUnit[ACC].nowOn && accOutSize > 0) {
                        if (mTask.intrMode != GYRO_ONLY_FIFO_INTERRUPTIBLE) {
                            destTime = (uint64_t)mTask.mSensorPerUnit[ACC].fifoDelay * (--accOutSize);
                            eventTimeStamp = timeStamp - destTime;
                        }
                        if (eventTimeStamp <= last_acc_time) {
                            osLog(LOG_INFO, "acc time reverse nowtime=%lld, lasttime=%lld\n",
                                eventTimeStamp, last_acc_time);
                            eventTimeStamp = last_acc_time + 10000;
                        }
                        last_acc_time = eventTimeStamp;
                        if (eventTimeStamp >= nowTime)
                            osLog(LOG_INFO, "acc eventTimeStamp - nowtime = %lld\n", eventTimeStamp - nowTime);
                        //osLog(LOG_DEBUG, "acc cali-time %d %lld %d\n", i, eventTimeStamp, accOutSize);
                        parseRawData(&mTask.mSensorPerUnit[ACC], &mTask.packet->outBuf[i], eventTimeStamp, mTask.packet->temperature);
#ifdef DEBUG_PERFORMANCE_HW_TICK
                        if (i == (mTask.outSize - 1)) {
                            osLog(LOG_INFO, "[Performance Debug] Last data: HW int timeStamp:%llu, Chinlung timeStamp:%llu\n", mTask.packet->timeStamp, timeStamp);
                            osLog(LOG_INFO, "[Performance Debug]Last data: sample_done, Stanley timeTick:%llu\n", timer_get_global_timer_tick());
                        }
#endif
                    } else if (mTask.packet->outBuf[i].sensType == SENS_TYPE_GYRO && mTask.mSensorPerUnit[GYRO].nowOn && gyroOutSize > 0) {
                        destTime = (uint64_t)mTask.mSensorPerUnit[GYRO].fifoDelay * (--gyroOutSize);
                        eventTimeStamp = timeStamp - destTime;
                        if (eventTimeStamp <= last_gyro_time) {
                            osLog(LOG_ERROR, "gyro time reverse nowtime=%lld, lasttime=%lld\n",
                                eventTimeStamp, last_gyro_time);
                            eventTimeStamp = last_gyro_time + 10000;
                        }
                        last_gyro_time = eventTimeStamp;
                        if (eventTimeStamp >= nowTime)
                            osLog(LOG_ERROR, "gyro eventTimeStamp - nowtime = %lld\n", eventTimeStamp - nowTime);
                        //osLog(LOG_DEBUG, "gyro cali-time %d %lld %d\n", i, eventTimeStamp, gyroOutSize);
                        parseRawData(&mTask.mSensorPerUnit[GYRO], &mTask.packet->outBuf[i], eventTimeStamp, mTask.packet->temperature);
                    }
                }
            }
            mTask.isReading = false;
            flushRawData();
            sendFlushEvt();
            mTask.nowState = CHIP_IDLE;
            processPendingEvt();
            mark_timestamp(SENS_TYPE_ACCEL, SENS_TYPE_ACCEL, SAMPLE_DONE, rtcGetTime());
            mark_timestamp(SENS_TYPE_GYRO, SENS_TYPE_GYRO, SAMPLE_DONE, rtcGetTime());
            //osLog(LOG_DEBUG, "mTask.isReading:%d\n", mTask.isReading);
            break;
        }
    }
}
static void handleEvent(uint32_t evtType, const void* evtData)
{
    struct transferDataInfo dataInfo;
    const struct sensorFsm *cmd;

    switch (evtType) {
        case EVT_APP_START: {
            if (mTask.fsm.mSensorFsm != NULL) {
                osLog(LOG_INFO, "accGyro: app start\n");
                /* Reset chip */
                dataInfo.inBuf = NULL;
                dataInfo.inSize = 0;
                dataInfo.elemInSize = 0;
                dataInfo.outBuf = NULL;
                dataInfo.outSize = NULL;
                dataInfo.elemOutSize = NULL;
                sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_RESET, &i2cCallback, &spiCallback);
            } else
               osLog(LOG_INFO, "accGyro: wait for auto detect\n");
            break;
        }

        case EVT_SENSOR_EVENT: {
            handleSensorEvent(evtData);
            break;
        }

        case EVT_SENSOR_TIMER: {
            /* Start sampling for a value */
            mark_timestamp(SENS_TYPE_ACCEL, SENS_TYPE_ACCEL, SAMPLE_BEGIN, rtcGetTime());
            mark_timestamp(SENS_TYPE_GYRO, SENS_TYPE_GYRO, SAMPLE_BEGIN, rtcGetTime());
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.isReading) {
                    mTask.nowState = CHIP_SAMPLING;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.packet;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
                    mTask.isReading = true;
                }
            } else {
                osLog(LOG_INFO, "accGyro EST pending by:%d\n", mTask.nowState);
                mTask.pendingSample = true;
            }
            break;
        }

        case EVT_SENSOR_INTR: {
            /* Start sampling for a value */
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.isReading) {
                    mTask.nowState = CHIP_SAMPLING;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.packet;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
                    mTask.isReading = true;
                } else {
                    osLog(LOG_ERROR, "accGyro: Warning!! will not unmask A+G Eint\n");
                }
            } else {
                //osLog(LOG_INFO, "accGyro EVT_SENSOR_INTR pending by nowState:%d\n", mTask.nowState);
                mTask.pendingInt = true;
            }
            break;
        }

        case EVT_HW_INT_STATUS_CHECK: {
            mark_timestamp(SENS_TYPE_ACCEL, SENS_TYPE_ACCEL, INT_CHECK, rtcGetTime());
            mark_timestamp(SENS_TYPE_GYRO, SENS_TYPE_GYRO, INT_CHECK, rtcGetTime());
            cmd = sensorFsmFindCmd(mTask.fsm.mSensorFsm, mTask.fsm.mSensorFsmSize, (const void *)CHIP_HW_INT_STATUS_CHECK);
            if (cmd == NULL) {
                osLog(LOG_INFO, "accGyro FsmFindCmd CHISC NULL\n");
                /* If not define fsm state CHIP_HW_INT_STATUS_CHECK */
                accGyroInterruptOccur();
            } else {
                if (mTask.nowState == CHIP_IDLE) {
                    mTask.nowState = CHIP_HW_INT_STATUS_CHECK;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = NULL;
                    dataInfo.outSize = NULL;
                    dataInfo.elemOutSize = NULL;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_HW_INT_STATUS_CHECK, &i2cCallback, &spiCallback);
                } else {
                    osLog(LOG_INFO, "accGyro CHISC pending by:%d\n", mTask.nowState);
                    mTask.pendingHwIntStatusCheck = true;
                }
            }
            break;
        }

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
        case EVT_HW_STEPCNT_SAMPLE: {
            /* Start sampling for a value */
            /*osLog(LOG_ERROR, "EVT_HW_STEPCNT_SAMPLE\n");*/
            if (mTask.nowState == CHIP_IDLE) {
                    mTask.nowState = CHIP_STEP_COUNT_SAMPLE;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = NULL;
                    dataInfo.outSize = NULL;
                    dataInfo.elemOutSize = NULL;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_STEP_COUNT_SAMPLE,
                        &i2cCallback, &spiCallback);
            }
            break;
        }
#endif
        case EVT_ERROR_EVENT: {
            sendBusErrToAp(evtData);
            break;
        }
    }
}

void getRealSampleDelay(uint32_t *accDelay, uint32_t *gyroDelay)
{
	*accDelay = mTask.mSensorPerUnit[ACC].fifoDelay;
	*gyroDelay = mTask.mSensorPerUnit[GYRO].fifoDelay;
}

void registerAccGyroInterruptMode(uint8_t mode)
{
    mTask.intrMode = mode;
}
void registerAccGyroFifoInfo(uint32_t accFifoDelay, uint32_t gyroFifoDelay)
{
    uint64_t gyroFifoStartTime, accFifoStartTime;
    gyroFifoStartTime = accFifoStartTime = rtcGetTime();
    if (mTask.mSensorPerUnit[ACC].fifoStartTime <= accFifoStartTime)
        mTask.mSensorPerUnit[ACC].fifoStartTime = accFifoStartTime;
    if (mTask.mSensorPerUnit[GYRO].fifoStartTime <= gyroFifoStartTime)
        mTask.mSensorPerUnit[GYRO].fifoStartTime = gyroFifoStartTime;
    mTask.mSensorPerUnit[ACC].fifoDelay = accFifoDelay;
    mTask.mSensorPerUnit[GYRO].fifoDelay = gyroFifoDelay;
    AccSensor_set_init_timestamp((long long)mTask.mSensorPerUnit[ACC].fifoStartTime, (int)accFifoDelay);
    GyroSensor_set_init_timestamp((long long)mTask.mSensorPerUnit[GYRO].fifoStartTime, (int)gyroFifoDelay);
    if (accFifoDelay)
        registerHwSampleInfo(SENS_TYPE_ACCEL, accFifoDelay);
    if (gyroFifoDelay)
        registerHwSampleInfo(SENS_TYPE_GYRO, gyroFifoDelay);
}
void registerAccGyroDriverFsm(struct sensorFsm *mSensorFsm, uint8_t size)
{
    mTask.fsm.moduleName = "accGyro";
    mTask.fsm.mSensorFsm = mSensorFsm;
    mTask.fsm.mSensorFsmSize = size;
    osEnqueuePrivateEvt(EVT_APP_START, NULL, NULL, mTask.id);
}

void registerAccGyroTimerCbk(accGyroTimerCbkF cbk)
{
    mTask.timerInterruptCbk = cbk;
}

void accSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_ACCEL, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.mSensorPerUnit[ACC].handle =
        sensorRegister(&mSensorInfo[ACC], &mSensorOps[ACC], NULL, false);
}

void gyroSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_GYRO, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.mSensorPerUnit[GYRO].handle =
        sensorRegister(&mSensorInfo[GYRO], &mSensorOps[GYRO], NULL, false);
}

void anyMotionSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_ANY_MOTION, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.mSensorPerUnit[ANY_MOTION].handle =
        sensorRegister(&mSensorInfo[ANY_MOTION], &mSensorOps[ANY_MOTION], NULL, false);
}

void noMotionSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_NO_MOTION, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.mSensorPerUnit[NO_MOTION].handle =
        sensorRegister(&mSensorInfo[NO_MOTION], &mSensorOps[NO_MOTION], NULL, false);
}

#ifdef CFG_HW_STEP_COUNTER_SUPPORT
void stepCntSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_STEP_COUNT, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.mSensorPerUnit[STEP_CNT].handle =
        sensorRegister(&mSensorInfo[STEP_CNT], &mSensorOps[STEP_CNT], NULL, false);
}
#endif

#ifdef CFG_MTK_CALIBRATION_V1_0
struct GyroCal *GetGyroCalBase(void)
{
    return &mTask.gyro_cal;
}

struct OverTempCal *GetOverTempCalBase(void)
{
    return &mTask.over_temp_gyro_cal;
}
#endif
static bool startTask(uint32_t taskId)
{
    size_t slabSize;

    accGyroDebugPoint = &mTask;


    wake_lock_init(&mTask.mSensorPerUnit[ACC].wakeLock, "acc");
    wake_lock_init(&mTask.mSensorPerUnit[GYRO].wakeLock, "gyro");

    slabSize = sizeof(struct TripleAxisDataEvent) +
        MAX_NUM_COMMS_EVENT_SAMPLES * sizeof(struct TripleAxisDataPoint);

    mTask.mDataSlab = slabAllocatorNew(slabSize, 4, 15);
    if (!mTask.mDataSlab) {
        osLog(LOG_ERROR, "Slab allocation failed\n");
        return false;
    }
    mTask.mSensorPerUnit[ACC].data_evt = NULL;
    mTask.mSensorPerUnit[GYRO].data_evt = NULL;

    /* Register tid first */
    mTask.id = taskId;

#ifndef CFG_MTK_CALIBRATION_V1_0
    mTask.bias[0] = 0;
    mTask.bias[1] = 0;
    mTask.bias[2] = 0;
    mTask.slope[0] = 0;
    mTask.slope[1] = 0;
    mTask.slope[2] = 0;
    mTask.intercept[0] = 0;
    mTask.intercept[2] = 0;
    mTask.intercept[2] = 0;
    Acc_init_calibration(mTask.bias);
    Gyro_init_calibration(mTask.slope, mTask.intercept);
#endif

#ifndef CFG_OVERLAY_INIT_SUPPORT
    /* Register sensor fsm last */
    module_init(SENS_TYPE_ACCEL);
#else
    extern void accGyroOverlayRemap(void);
    accGyroOverlayRemap();
#endif

    return true;
}

static void endTask(void)
{
}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ACCEL, 0, 0)), 0, startTask, endTask, handleEvent);
