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
#include <performance.h>

#include <contexthub_core.h>
#include <barometer.h>

enum baroSensorEvents {
    EVT_SENSOR_EVENT = EVT_APP_START + 1,
    EVT_SENSOR_TIMER,
    EVT_SENSOR_INTR,
    EVT_ERROR_EVENT,
};
struct ConfigStat {
    uint64_t latency;
    uint32_t rate;
    bool enable;
};
SRAM_REGION_BSS static struct baroTask {
    uint32_t id;
    uint32_t baroHandle;
    uint32_t baroTimerHandle;
    uint32_t rate;
    uint64_t latency;
    struct ConfigStat pendConfig;
    bool pendingConfig;
    bool pendingSample;
    bool pendingInt;
    bool baroNowOn;
    bool baroReading;
    uint8_t intrMode;
    uint8_t nowState;
    /* receive data which convert from rawdata, real output for algo and ap */
    struct baroDataPacket *packet;
    uint8_t outSize;
    uint8_t elemOutSize;
    struct fsmCurrInfo fsm;
} mTask;
static struct baroTask *baroDebugPoint;
static const uint32_t baroSupportedRates[] = {
    SENSOR_HZ(0.1f),
    SENSOR_HZ(1.0f),
    SENSOR_HZ(5.0f),
    SENSOR_HZ(10.0f),
    SENSOR_HZ(16.0f),
    SENSOR_HZ(50.0f),
    0
};

static const uint64_t rateTimerValsBaro[] = {
//should match "supported rates in length" and be the timer length for that rate in nanosecs
    10 * 1000000000ULL,
    1 * 1000000000ULL,
    1000000000ULL / 5,
    1000000000ULL / 10,
    1000000000ULL / 16,
    1000000000ULL / 50,
};

/* sensor callbacks from nanohub */

SRAM_REGION_FUNCTION static void i2cCallback(void *cookie, size_t tx, size_t rx, int err)
{
    //osLog(LOG_DEBUG, "baro: i2cCallback (%d) cookie:%d\n", err, (uint32_t)cookie);
    if (err == 0)
        osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
    else {
        osLog(LOG_ERROR, "baro: i2c error (%d)\n", err);
        osEnqueuePrivateEvt(EVT_ERROR_EVENT, (void *)ERR_SENSOR_I2C, NULL, mTask.id);
    }
}

/* sensor callbacks from nanohub */

SRAM_REGION_FUNCTION static void spiCallback(void *cookie, int err)
{
    //osLog(LOG_DEBUG, "baro: spiCallback (%d) cookie:%d\n", err, (uint32_t)cookie);
    if (err == 0)
        osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
    else {
        osLog(LOG_ERROR, "baro: spi error (%d)\n", err);
        osEnqueuePrivateEvt(EVT_ERROR_EVENT, (void *)ERR_SENSOR_SPI, NULL, mTask.id);
    }
}

SRAM_REGION_FUNCTION void baroInterruptOccur(void)
{
    osEnqueuePrivateEvt(EVT_SENSOR_INTR, NULL, NULL, mTask.id);
    extern TaskHandle_t CHRE_TaskHandle;
    if (xTaskResumeFromISR(CHRE_TaskHandle) == pdTRUE) {
        portYIELD_WITHIN_API();
    }
}

void baroRegisterInterruptMode(uint8_t mode)
{
    mTask.intrMode = mode;
}

SRAM_REGION_FUNCTION static void baroTimerCallback(uint32_t timerId, void *cookie)
{
    osEnqueuePrivateEvt(EVT_SENSOR_TIMER, cookie, NULL, mTask.id);
}
// TODO: only turn on the timer when enabled
static bool sensorPowerBaro(bool on, void *cookie)
{
    struct transferDataInfo dataInfo;

    osLog(LOG_INFO, "sensorPowerBaro:%d\n", on);
    if (!on && mTask.baroTimerHandle) {
        timTimerCancel(mTask.baroTimerHandle);
        mTask.baroTimerHandle = 0;
        mTask.baroReading = false;
    }
    if (on != mTask.baroNowOn) {
        if (mTask.nowState == CHIP_IDLE) {
            mTask.pendingConfig = false;
            mTask.baroNowOn = on;
            if (on == true) {
                mTask.nowState = CHIP_ENABLE;
                dataInfo.inBuf = NULL;
                dataInfo.inSize = 0;
                dataInfo.elemInSize = 0;
                dataInfo.outBuf = NULL;
                dataInfo.outSize = NULL;
                dataInfo.elemOutSize = NULL;
                sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ENABLE, &i2cCallback, &spiCallback);
            } else {
                mTask.nowState = CHIP_DISABLE;
                dataInfo.inBuf = NULL;
                dataInfo.inSize = 0;
                dataInfo.elemInSize = 0;
                dataInfo.outBuf = NULL;
                dataInfo.outSize = NULL;
                dataInfo.elemOutSize = NULL;
                sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_DISABLE, &i2cCallback, &spiCallback);
            }
        } else {
            osLog(LOG_INFO, "sensorPowerBaro pending by:%d\n", mTask.nowState);
            mTask.pendingConfig = true;
            mTask.pendConfig.enable = on;
        }
    }
    return true;
}

static bool sensorFirmwareBaro(void *cookie)
{
    sensorSignalInternalEvt(mTask.baroHandle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}
static void setBaroHwRate(uint32_t rate)
{
    struct transferDataInfo dataInfo;
    struct baroCntlPacket cntlPacket;

    cntlPacket.rate = rate;

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_RATECHG;
        dataInfo.inBuf = (void *)&cntlPacket;
        dataInfo.inSize = 1;
        dataInfo.elemInSize = sizeof(struct baroCntlPacket);
        dataInfo.outBuf = NULL;
        dataInfo.outSize = NULL;
        dataInfo.elemOutSize = NULL;
        sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_RATECHG, &i2cCallback, &spiCallback);
    } else {
        osLog(LOG_INFO, "sensorRateBaro pending by:%d\n", mTask.nowState);
        mTask.pendingConfig = true;
        mTask.pendConfig.enable = true;
        mTask.pendConfig.latency = mTask.latency;
        mTask.pendConfig.rate = mTask.rate;
    }
}
static bool sensorRateBaro(uint32_t rate, uint64_t latency, void *cookie)
{
    osLog(LOG_INFO, "sensorRateBaro, rate:%lu, latency:%lld\n", rate, latency);

    mTask.rate = rate;
    mTask.latency = latency;
    setBaroHwRate(rate);
    return true;
}

static bool sensorFlushBaro(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_BARO), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static bool sensorCalibrateBaro(void *cookie)
{
    return true;
}

static bool sensorCfgDataBaro(void *data, void *cookie)
{
    return true;
}

static const struct SensorInfo sensorInfoBaro = {
    .sensorName = "Pressure",
    .supportedRates = baroSupportedRates,
    .sensorType = SENS_TYPE_BARO,
    .numAxis = NUM_AXIS_EMBEDDED,
    .interrupt = NANOHUB_INT_NONWAKEUP,
    .minSamples = 300,
};


static const struct SensorOps sensorOpsBaro = {
    .sensorPower = sensorPowerBaro,
    .sensorFirmwareUpload = sensorFirmwareBaro,
    .sensorSetRate = sensorRateBaro,
    .sensorFlush = sensorFlushBaro,
    .sensorCalibrate = sensorCalibrateBaro,
    .sensorCfgData = sensorCfgDataBaro,
};


// Returns temperature in units of 0.01 degrees celsius.
static void processPendingEvt(void)
{
    if (mTask.pendingInt) {
        mTask.pendingInt = false;
        osEnqueuePrivateEvt(EVT_SENSOR_INTR, NULL, NULL, mTask.id);
        return;
    }
    if (mTask.pendingSample) {
        mTask.pendingSample = false;
        osEnqueuePrivateEvt(EVT_SENSOR_TIMER, NULL, NULL, mTask.id);
        return;
    }
    if (mTask.pendingConfig) {
        mTask.pendingConfig = false;
        if (mTask.pendConfig.enable == 0 && mTask.baroNowOn)
            sensorOpsBaro.sensorPower(false, NULL);
        else if (mTask.pendConfig.enable == 1 && !mTask.baroNowOn)
            sensorOpsBaro.sensorPower(true, NULL);
        else
            sensorOpsBaro.sensorSetRate(mTask.pendConfig.rate, mTask.pendConfig.latency, NULL);
        return;
    }
}

static void handleSensorEvent(const void *state)
{
    uint8_t i = 0;
    union EmbeddedDataPoint sample;
    enum BaroTempState mState = (enum BaroTempState)state;

    switch (mState) {
        default: {
            sensorFsmSwitchState(NULL, &mTask.fsm, &i2cCallback, &spiCallback);
            break;
        }
        case CHIP_IDLE: {
            //osLog(LOG_DEBUG, "baro: STATE_IDLE done\n");
            break;
        }

        case CHIP_INIT_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            if (mTask.baroHandle)
                sensorRegisterInitComplete(mTask.baroHandle);
            mTask.nowState = CHIP_IDLE;
            processPendingEvt();
            osLog(LOG_INFO, "baro: init done\n");
            break;
        }

        case CHIP_ENABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.baroHandle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "baro: enable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.baroHandle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "baro: disable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_RATECHG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            switch (mTask.intrMode) {
                case BARO_INTERRUPTIBLE:
                    break;
                case BARO_UNINTERRUPTIBLE:
                    if (mTask.baroTimerHandle)
                        timTimerCancel(mTask.baroTimerHandle);
                    mTask.baroTimerHandle =
                        timTimerSet(sensorTimerLookupCommon(baroSupportedRates, rateTimerValsBaro, mTask.rate),
                                    0, 50, baroTimerCallback, NULL, false);
                    break;
            }
            sensorSignalInternalEvt(mTask.baroHandle, SENSOR_INTERNAL_EVT_RATE_CHG, mTask.rate, mTask.latency);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "baro: ratechg done\n");
            processPendingEvt();
            break;
        }

        case CHIP_SAMPLING_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            if (mTask.baroNowOn && mTask.baroReading) {
                //osLog(LOG_DEBUG, "baro: %d\n", mTask.outBuf[0]);
                //if (mTask.elemOutSize == sizeof(uint32_t))
                for (i = 0; i < mTask.outSize; ++i) {
                    if (mTask.packet->outBuf[i].sensType == SENS_TYPE_BARO) {
                        sample.fdata = mTask.packet->outBuf[i].fdata;
                        osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_BARO), sample.vptr, NULL);
                    }
                }
            }
            mTask.baroReading = false;
            mTask.nowState = CHIP_IDLE;
            processPendingEvt();
            mark_timestamp(SENS_TYPE_BARO, SENS_TYPE_BARO, SAMPLE_DONE, rtcGetTime());
            break;
        }
    }
}
static void handleEvent(uint32_t evtType, const void* evtData)
{
    struct transferDataInfo dataInfo;

    switch (evtType) {
        case EVT_APP_START: {
            if (mTask.fsm.mSensorFsm != NULL) {
                osLog(LOG_INFO, "baro: app start\n");
                /* Reset chip */
                dataInfo.inBuf = NULL;
                dataInfo.inSize = 0;
                dataInfo.elemInSize = 0;
                dataInfo.outBuf = NULL;
                dataInfo.outSize = NULL;
                dataInfo.elemOutSize = NULL;
                sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_RESET, &i2cCallback, &spiCallback);
            } else
                osLog(LOG_INFO, "baro: wait for auto detect\n");
            break;
        }

        case EVT_SENSOR_EVENT: {
            handleSensorEvent(evtData);
            break;
        }

        case EVT_SENSOR_TIMER: {
            /* Start sampling for a value */
            mark_timestamp(SENS_TYPE_BARO, SENS_TYPE_BARO, SAMPLE_BEGIN, rtcGetTime());
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.baroReading) {
                    mTask.nowState = CHIP_SAMPLING;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.packet;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
                    mTask.baroReading = true;
                }
            } else {
                osLog(LOG_INFO, "baro EST pending by:%d\n", mTask.nowState);
                mTask.pendingSample = true;
            }
            break;
        }

        case EVT_SENSOR_INTR: {
            /* Start sampling for a value */
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.baroReading) {
                    mTask.nowState = CHIP_SAMPLING;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.packet;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
                    mTask.baroReading = true;
                }
            } else {
                osLog(LOG_INFO, "baro ESI pending by:%d\n", mTask.nowState);
                mTask.pendingInt = true;
            }
            break;
        }

        case EVT_ERROR_EVENT: {
            sendBusErrToAp(evtData);
            break;
        }
    }
}
void registerBaroDriverFsm(struct sensorFsm *mSensorFsm, uint8_t size)
{
    mTask.fsm.moduleName = "baro";
    mTask.fsm.mSensorFsm = mSensorFsm;
    mTask.fsm.mSensorFsmSize = size;
    osEnqueuePrivateEvt(EVT_APP_START, NULL, NULL, mTask.id);
}
void baroSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_BARO, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.baroHandle =
        sensorRegister(&sensorInfoBaro, &sensorOpsBaro, NULL, false);
}
static bool startTask(uint32_t taskId)
{

    baroDebugPoint = &mTask;

    /* Register tid first */
    mTask.id = taskId;

#ifndef CFG_OVERLAY_INIT_SUPPORT
    /* Register sensor fsm last */
    module_init(SENS_TYPE_BARO);
#else
    extern void baroOverlayRemap(void);
    baroOverlayRemap();
#endif

    return true;
}

static void endTask(void)
{

}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_BARO, 0, 0)), 0, startTask, endTask, handleEvent);
