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
#include <alsps.h>

#define MAX_NUM_COMMS_EVENT_SAMPLES 15
#define PS_DEFAULT_RATE SENSOR_HZ(5)
#define PS_DEBOUNCE_TIME 200000000 /*for Interrupt PS once sample delay*/
enum sensorHandleState {
    ALS = 0,
    PS,
    RGBW,
    MAX_HANDLE,
};
enum AlsPsSensorEvents {
    EVT_SENSOR_EVENT = EVT_APP_START + 1,
    EVT_SENSOR_TIMER,
    EVT_SENSOR_INTR,
    EVT_ERROR_EVENT,
    EVT_PS_SAMPLE_ONCE,
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

    bool isReading;
    uint32_t timerDelay;
    uint32_t timerHandle;
    struct AlsPsData last_sample;
    struct ConfigStat pendConfig;
    int32_t pendCaliCfg[2];
};
SRAM_REGION_BSS static struct alsPsTask {
    uint32_t id;
    //bool isReading;
    bool pendingConfig[MAX_HANDLE];
    bool pendingCali[MAX_HANDLE];
    bool pendingCaliCfg[MAX_HANDLE];
    bool pendingSample;
    bool pendingInt;
    bool pendingSampleOnce;
    struct sensPerUnit mSensorPerUnit[MAX_HANDLE];
    struct AlsPsData *current_sample;
    uint8_t outSize;
    uint8_t elemOutSize;
    struct fsmCurrInfo fsm;
    uint8_t intrMode;
    uint8_t nowState;
    uint64_t prev_rtc_time;
    struct SlabAllocator *mDataSlab;
    struct TripleAxisDataEvent *data_evt;
} mTask;

static struct alsPsTask *alsPsDebugPoint;

static const uint32_t alsSupportedRates[] = {
    SENSOR_HZ(5),
    SENSOR_HZ(15),
    SENSOR_HZ(50),
    SENSOR_HZ(100),
    SENSOR_RATE_ONCHANGE,
    0
};

static const uint64_t rateTimerValsAls[] = {
//should match "supported rates in length" and be the timer length for that rate in nanosecs
    1000000000ULL / 5,
    1000000000ULL / 15,
    1000000000ULL / 50,
    1000000000ULL / 100,
    1000000000ULL / 100,
};

static const uint32_t rgbwSupportedRates[] = {
    SENSOR_HZ(5),
    SENSOR_HZ(15),
    SENSOR_HZ(50),
    SENSOR_HZ(100),
    SENSOR_RATE_ONCHANGE,
    0
};

static const uint64_t rateTimerValsRGBW[] = {
//should match "supported rates in length" and be the timer length for that rate in nanosecs
    1000000000ULL / 5,
    1000000000ULL / 15,
    1000000000ULL / 50,
    1000000000ULL / 100,
    1000000000ULL / 100,
};

static const uint32_t psSupportedRates[] = {
    SENSOR_HZ(5),
    SENSOR_HZ(15),
    SENSOR_HZ(50),
    SENSOR_HZ(100),
    SENSOR_RATE_ONCHANGE,
    0
};

static const uint64_t rateTimerValsPs[] = {
//should match "supported rates in length" and be the timer length for that rate in nanosecs
    1000000000ULL / 5,
    1000000000ULL / 15,
    1000000000ULL / 50,
    1000000000ULL / 100,
    1000000000ULL / 100,
};

/* sensor callbacks from nanohub */

SRAM_REGION_FUNCTION static void i2cCallback(void *cookie, size_t tx, size_t rx, int err)
{
    //osLog(LOG_DEBUG, "alsPs: i2cCallback (%d) cookie:%d\n", err, (uint32_t)cookie);
    if (err == 0)
        osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
    else {
        osLog(LOG_ERROR, "alsPs: i2c error (%d)\n", err);
        osEnqueuePrivateEvt(EVT_ERROR_EVENT, (void *)ERR_SENSOR_I2C, NULL, mTask.id);
    }
}

/* sensor callbacks from nanohub */

SRAM_REGION_FUNCTION static void spiCallback(void *cookie, int err)
{
    //osLog(LOG_DEBUG, "alsPs: spiCallback (%d) cookie:%d\n", err, (uint32_t)cookie);
    if (err == 0)
        osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
    else {
        osLog(LOG_ERROR, "alsPs: spi error (%d)\n", err);
        osEnqueuePrivateEvt(EVT_ERROR_EVENT, (void *)ERR_SENSOR_SPI, NULL, mTask.id);
    }
}

SRAM_REGION_FUNCTION void alsPsInterruptOccur(void)
{
    osEnqueuePrivateEvt(EVT_SENSOR_INTR, NULL, NULL, mTask.id);
    extern TaskHandle_t CHRE_TaskHandle;
    if (xTaskResumeFromISR(CHRE_TaskHandle) == pdTRUE) {
        portYIELD_WITHIN_API();
    }
}

SRAM_REGION_FUNCTION static void alsTimerCallback(uint32_t timerId, void *cookie)
{
    /* osLog(LOG_DEBUG, "alsTimerCallback:cookie:%d\n", (uint32_t)cookie); */
    osEnqueuePrivateEvt(EVT_SENSOR_TIMER, cookie, NULL, mTask.id);
}

SRAM_REGION_FUNCTION static void psTimerCallback(uint32_t timerId, void *cookie)
{
    /* osLog(LOG_INFO, "psTimerCallback:cookie:%d\n", (uint32_t)cookie); */
    osEnqueuePrivateEvt(EVT_SENSOR_INTR, cookie, NULL, mTask.id);
}

SRAM_REGION_FUNCTION static void psOncePollingTimerCallback(uint32_t timerId, void *cookie)
{
    mTask.mSensorPerUnit[PS].timerHandle = 0;
    osEnqueuePrivateEvt(EVT_PS_SAMPLE_ONCE, cookie, NULL, mTask.id);
}


static bool sensorPowerAls(bool on, void *cookie)
{
    osLog(LOG_INFO, "sensorPowerAls on:%d, nowOn:%d\n", on, mTask.mSensorPerUnit[ALS].nowOn);
    if (!on) {
        //mTask.mSensorPerUnit[ALS].isReading = false;
        mTask.mSensorPerUnit[ALS].timerDelay = 0XFFFFFFFF;
    }
    if (!on && mTask.mSensorPerUnit[ALS].timerHandle && !mTask.mSensorPerUnit[RGBW].nowOn) {
        timTimerCancel(mTask.mSensorPerUnit[ALS].timerHandle);
        mTask.mSensorPerUnit[ALS].timerHandle = 0;
        mTask.mSensorPerUnit[ALS].isReading = false;
    } else if (!on && mTask.mSensorPerUnit[ALS].timerHandle && mTask.mSensorPerUnit[RGBW].nowOn) {
        mTask.mSensorPerUnit[RGBW].timerDelay =
            sensorTimerLookupCommon(rgbwSupportedRates, rateTimerValsRGBW,
                mTask.mSensorPerUnit[RGBW].rate);
        timTimerCancel(mTask.mSensorPerUnit[ALS].timerHandle);
        mTask.mSensorPerUnit[ALS].timerHandle =
            timTimerSet(mTask.mSensorPerUnit[RGBW].timerDelay, 0, 50, alsTimerCallback, NULL, false);
        osLog(LOG_INFO, "sensorPowerAls: timerDelay:%luns\n", mTask.mSensorPerUnit[RGBW].timerDelay);
    }
    if (on != mTask.mSensorPerUnit[ALS].nowOn) {
        if (mTask.nowState == CHIP_IDLE) {
            mTask.pendingConfig[ALS] = false;
            mTask.mSensorPerUnit[ALS].nowOn = on;
            if (on == true) {
                mTask.nowState = CHIP_ALS_ENABLE;
                sensorFsmRunState(NULL, &mTask.fsm, (const void *)CHIP_ALS_ENABLE, &i2cCallback, &spiCallback);
            } else {
                mTask.nowState = CHIP_ALS_DISABLE;
                sensorFsmRunState(NULL, &mTask.fsm, (const void *)CHIP_ALS_DISABLE, &i2cCallback, &spiCallback);
            }
        } else {
            osLog(LOG_INFO, "sensorPowerAls pending by: %d\n", mTask.nowState);
            mTask.pendingConfig[ALS] = true;
            mTask.mSensorPerUnit[ALS].pendConfig.enable = on;
        }
    } else
        osLog(LOG_ERROR, "sensorPowerAls on != nowOn\n");
    return true;
}

static bool sensorFirmwareAls(void *cookie)
{
    sensorSignalInternalEvt(mTask.mSensorPerUnit[ALS].handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static void setAlsHwRate(uint32_t rate)
{
    struct transferDataInfo dataInfo;
    uint32_t als_rate[1] = {rate};

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_ALS_RATECHG;
        dataInfo.inBuf = (void *)als_rate;
        dataInfo.inSize = 1;
        dataInfo.elemInSize = sizeof(uint32_t);
        dataInfo.outBuf = NULL;
        dataInfo.outSize = NULL;
        dataInfo.elemOutSize = NULL;
        sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ALS_RATECHG, &i2cCallback, &spiCallback);
    } else {
        osLog(LOG_INFO, "setAlsHwRate pending by:%d\n", mTask.nowState);
        mTask.pendingConfig[ALS] = true;
        mTask.mSensorPerUnit[ALS].pendConfig.enable = true;
        mTask.mSensorPerUnit[ALS].pendConfig.latency = mTask.mSensorPerUnit[ALS].latency;
        mTask.mSensorPerUnit[ALS].pendConfig.rate = mTask.mSensorPerUnit[ALS].rate;
    }
}

static bool sensorRateAls(uint32_t rate, uint64_t latency, void *cookie)
{
    osLog(LOG_INFO, "sensorRateAls rate:%lu, latency:%lld\n", rate, latency);
    mTask.mSensorPerUnit[ALS].rate = rate;
    mTask.mSensorPerUnit[ALS].latency = latency;
    setAlsHwRate(rate);
    return true;
}

static bool sensorFlushAls(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_ALS), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static bool sendLastSampleAls(void *cookie, uint32_t tid)
{
    return true;
}

static bool sensorCaliAls(void *cookie)
{
    struct transferDataInfo dataInfo;
    osLog(LOG_INFO, "alsps sensorCaliAls\n");

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_ALS_CALI;
        dataInfo.inBuf = NULL;
        dataInfo.inSize = 0;
        dataInfo.elemInSize = 0;
        dataInfo.outBuf = NULL;
        dataInfo.outSize = 0;
        dataInfo.elemOutSize = 0;
        if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ALS_CALI, &i2cCallback, &spiCallback) < 0)
        mTask.nowState = CHIP_IDLE;
    } else {
        osLog(LOG_INFO, "sensorCaliAls pending by:%d\n", mTask.nowState);
        mTask.pendingCali[ALS] = true;
    }
    return true;
}

static void setAlsHwCaliCfg(int32_t *data)
{
    struct alspsCaliCfgPacket caliCfgPacket;
    struct transferDataInfo dataInfo;

    caliCfgPacket.caliCfgData[0] = data[0];

    mTask.nowState = CHIP_ALS_CFG;
    dataInfo.inBuf = (void *)&caliCfgPacket;
    dataInfo.inSize = 1;
    dataInfo.elemInSize = sizeof(struct alspsCaliCfgPacket);
    dataInfo.outBuf = NULL;
    dataInfo.outSize = 0;
    dataInfo.elemOutSize = 0;
    if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_ALS_CFG, &i2cCallback, &spiCallback) < 0)
        mTask.nowState = CHIP_IDLE;
}

static bool sensorCfgAls(void *data, void *cookie)
{
    int32_t *values = data;

    osLog(LOG_INFO, "sensorCfgAls : [%ld]\n", values[0]);
    if (mTask.nowState == CHIP_IDLE) {
        setAlsHwCaliCfg(values);
    } else {
        osLog(LOG_INFO, "sensorCfgAls pending by:%d\n", mTask.nowState);
        mTask.pendingCaliCfg[ALS] = true;
        mTask.mSensorPerUnit[ALS].pendCaliCfg[0] = values[0];
    }
    return true;
}

static bool sensorPowerRGBW(bool on, void *cookie)
{
    osLog(LOG_INFO, "sensorPowerRGBW on:%d, nowOn:%d\n", on, mTask.mSensorPerUnit[RGBW].nowOn);
    if (!on) {
        //mTask.mSensorPerUnit[ALS].isReading = false;
        mTask.mSensorPerUnit[RGBW].timerDelay = 0XFFFFFFFF;
    }
    if (!on && mTask.mSensorPerUnit[ALS].timerHandle && !mTask.mSensorPerUnit[ALS].nowOn) {
        timTimerCancel(mTask.mSensorPerUnit[ALS].timerHandle);
        mTask.mSensorPerUnit[ALS].timerHandle = 0;
        mTask.mSensorPerUnit[RGBW].isReading = false;
    } else if (!on && mTask.mSensorPerUnit[ALS].timerHandle && mTask.mSensorPerUnit[ALS].nowOn) {
        mTask.mSensorPerUnit[ALS].timerDelay =
            sensorTimerLookupCommon(alsSupportedRates, rateTimerValsAls,
                mTask.mSensorPerUnit[ALS].rate);
        timTimerCancel(mTask.mSensorPerUnit[ALS].timerHandle);
        mTask.mSensorPerUnit[ALS].timerHandle =
            timTimerSet(mTask.mSensorPerUnit[ALS].timerDelay, 0, 50, alsTimerCallback, NULL, false);
        osLog(LOG_INFO, "sensorPowerRGBW: timerDelay:%luns\n", mTask.mSensorPerUnit[ALS].timerDelay);
    }
    if (on != mTask.mSensorPerUnit[RGBW].nowOn) {
        if (mTask.nowState == CHIP_IDLE) {
            mTask.pendingConfig[RGBW] = false;
            mTask.mSensorPerUnit[RGBW].nowOn = on;
            if (on == true) {
                mTask.nowState = CHIP_RGBW_ENABLE;
                sensorFsmRunState(NULL, &mTask.fsm, (const void *)CHIP_RGBW_ENABLE, &i2cCallback, &spiCallback);
            } else {
                mTask.nowState = CHIP_RGBW_DISABLE;
                sensorFsmRunState(NULL, &mTask.fsm, (const void *)CHIP_RGBW_DISABLE, &i2cCallback, &spiCallback);
            }
        } else {
            osLog(LOG_INFO, "sensorPowerAls pending by nowState:%d\n", mTask.nowState);
            mTask.pendingConfig[RGBW] = true;
            mTask.mSensorPerUnit[RGBW].pendConfig.enable = on;
        }
    } else
        osLog(LOG_ERROR, "sensorPowerAls on != nowOn\n");
    return true;
}

static bool sensorFirmwareRGBW(void *cookie)
{
    sensorSignalInternalEvt(mTask.mSensorPerUnit[RGBW].handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static void setRGBWHwRate(uint32_t rate)
{
    struct transferDataInfo dataInfo;
    uint32_t als_rate[1] = {rate};

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_RGBW_RATECHG;
        dataInfo.inBuf = (void *)als_rate;
        dataInfo.inSize = 1;
        dataInfo.elemInSize = sizeof(uint32_t);
        dataInfo.outBuf = NULL;
        dataInfo.outSize = NULL;
        dataInfo.elemOutSize = NULL;
        sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_RGBW_RATECHG, &i2cCallback, &spiCallback);
    } else {
        osLog(LOG_INFO, "%s pending by nowState:%d\n", __func__, mTask.nowState);
        mTask.pendingConfig[RGBW] = true;
        mTask.mSensorPerUnit[RGBW].pendConfig.enable = true;
        mTask.mSensorPerUnit[RGBW].pendConfig.latency = mTask.mSensorPerUnit[RGBW].latency;
        mTask.mSensorPerUnit[RGBW].pendConfig.rate = mTask.mSensorPerUnit[RGBW].rate;
    }
}

static bool sensorRateRGBW(uint32_t rate, uint64_t latency, void *cookie)
{
    osLog(LOG_INFO, "%s rate:%lu, latency:%lld\n", __func__, rate, latency);
    mTask.mSensorPerUnit[RGBW].rate = rate;
    mTask.mSensorPerUnit[RGBW].latency = latency;
    setRGBWHwRate(rate);
    return true;
}

static bool sensorFlushRGBW(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_RGBW), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static bool sendLastSampleRGBW(void *cookie, uint32_t tid)
{
    return true;
}

static bool sensorPowerPs(bool on, void *cookie)
{
    osLog(LOG_INFO, "sensorPowerPs on:%d, nowOn:%d\n", on, mTask.mSensorPerUnit[PS].nowOn);
    if ((mTask.intrMode == PS_POLLING_MODE) && !on) {
        mTask.mSensorPerUnit[PS].timerDelay = 0XFFFFFFFF;
    }
    if ((mTask.intrMode == PS_POLLING_MODE) && !on && mTask.mSensorPerUnit[PS].timerHandle) {
        timTimerCancel(mTask.mSensorPerUnit[PS].timerHandle);
        mTask.mSensorPerUnit[PS].timerHandle = 0;
        mTask.mSensorPerUnit[PS].isReading = false;
    }
    if (on != mTask.mSensorPerUnit[PS].nowOn) {
        if (mTask.nowState == CHIP_IDLE) {
            mTask.pendingConfig[PS] = false;
            mTask.mSensorPerUnit[PS].nowOn = on;
            if (on == true) {
                mTask.nowState = CHIP_PS_ENABLE;
                sensorFsmRunState(NULL, &mTask.fsm, (const void *)CHIP_PS_ENABLE, &i2cCallback, &spiCallback);
                mTask.mSensorPerUnit[PS].last_sample.prox_state = PROX_STATE_INIT;
            } else {
                mTask.mSensorPerUnit[PS].isReading = false;
                mTask.nowState = CHIP_PS_DISABLE;
                sensorFsmRunState(NULL, &mTask.fsm, (const void *)CHIP_PS_DISABLE, &i2cCallback, &spiCallback);
                mTask.mSensorPerUnit[PS].last_sample.prox_state = PROX_STATE_INIT;
            }
        } else {
            osLog(LOG_INFO, "sensorPowerPs pending by:%d\n", mTask.nowState);
            mTask.pendingConfig[PS] = true;
            mTask.mSensorPerUnit[PS].pendConfig.enable = on;
        }
    } else
        osLog(LOG_ERROR, "sensorPowerPs on != nowOn\n");

    return true;
}

static bool sensorFirmwarePs(void *cookie)
{

    sensorSignalInternalEvt(mTask.mSensorPerUnit[PS].handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool sensorRatePs(uint32_t rate, uint64_t latency, void *cookie)
{
    struct transferDataInfo dataInfo;
    uint32_t ps_rate[1] = {rate};

    osLog(LOG_INFO, "sensorRatePs rate:%lu, latency:%lld\n", rate, latency);
    mTask.mSensorPerUnit[PS].rate = rate;
    mTask.mSensorPerUnit[PS].latency = latency;
    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_PS_RATECHG;
        dataInfo.inBuf = (void *)ps_rate;
        dataInfo.inSize = 1;
        dataInfo.elemInSize = sizeof(uint32_t);
        dataInfo.outBuf = NULL;
        dataInfo.outSize = NULL;
        dataInfo.elemOutSize = NULL;
        sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_PS_RATECHG, &i2cCallback, &spiCallback);
    } else {
        osLog(LOG_INFO, "sensorRatePs pending by:%d\n", mTask.nowState);
        mTask.pendingConfig[PS] = true;
        mTask.mSensorPerUnit[PS].pendConfig.enable = true;
        mTask.mSensorPerUnit[PS].pendConfig.latency = mTask.mSensorPerUnit[PS].latency;
        mTask.mSensorPerUnit[PS].pendConfig.rate = mTask.mSensorPerUnit[PS].rate;
    }
    // do nothing as psensor is using interrupt
    return true;
}

static bool sensorFlushPs(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_PROX), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static bool sendLastSampleProx(void *cookie, uint32_t tid)
{
    union EmbeddedDataPoint sample;

    if (mTask.mSensorPerUnit[PS].last_sample.prox_state != PROX_STATE_INIT) {
        sample.fdata = mTask.mSensorPerUnit[PS].last_sample.prox_data;
        osEnqueuePrivateEvt(sensorGetMyEventType(SENS_TYPE_PROX), sample.vptr, NULL, tid);
    }
    return true;
}

static bool sensorCaliPs(void *cookie)
{
    struct transferDataInfo dataInfo;
    osLog(LOG_INFO, "alsps sensorCaliPs\n");

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_PS_CALI;
        dataInfo.inBuf = NULL;
        dataInfo.inSize = 0;
        dataInfo.elemInSize = 0;
        dataInfo.outBuf = NULL;
        dataInfo.outSize = 0;
        dataInfo.elemOutSize = 0;
        if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_PS_CALI, &i2cCallback, &spiCallback) < 0)
            mTask.nowState = CHIP_IDLE;
    } else {
        osLog(LOG_INFO, "sensorCaliPs pending by:%d\n", mTask.nowState);
        mTask.pendingCali[PS] = true;
    }
    return true;
}

static void setPsHwCaliCfg(int32_t *data)
{
    struct alspsCaliCfgPacket caliCfgPacket;
    struct transferDataInfo dataInfo;

    caliCfgPacket.caliCfgData[0] = data[0];
    caliCfgPacket.caliCfgData[1] = data[1];

    mTask.nowState = CHIP_PS_CFG;
    dataInfo.inBuf = (void *)&caliCfgPacket;
    dataInfo.inSize = 1;
    dataInfo.elemInSize = sizeof(struct alspsCaliCfgPacket);
    dataInfo.outBuf = NULL;
    dataInfo.outSize = 0;
    dataInfo.elemOutSize = 0;
    if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_PS_CFG, &i2cCallback, &spiCallback) < 0)
        mTask.nowState = CHIP_IDLE;
}

static bool sensorCfgPs(void *data, void *cookie)
{
    int32_t *values = data;

    osLog(LOG_INFO, "sensorCfgPs [high, low]: [%ld, %ld]\n", values[0], values[1]);
    if (mTask.nowState == CHIP_IDLE) {
        setPsHwCaliCfg(values);
    } else {
        osLog(LOG_INFO, "sensorCfgPs pending by:%d\n", mTask.nowState);
        mTask.pendingCaliCfg[PS] = true;
        mTask.mSensorPerUnit[PS].pendCaliCfg[0] = values[0];
        mTask.mSensorPerUnit[PS].pendCaliCfg[1] = values[1];
    }

    return true;
}

static const struct SensorInfo mSensorInfo[MAX_HANDLE] = {
    {
        .sensorName = "ALS",
        .supportedRates = alsSupportedRates,
        .sensorType = SENS_TYPE_ALS,
        .numAxis = NUM_AXIS_EMBEDDED,
        .interrupt = NANOHUB_INT_NONWAKEUP,
        .minSamples = 20
    },
    {
        .sensorName = "Proximity",
        .supportedRates = psSupportedRates,
        .sensorType = SENS_TYPE_PROX,
        .numAxis = NUM_AXIS_EMBEDDED,
        .interrupt = NANOHUB_INT_WAKEUP,
        .minSamples = 300
    },
    {
        .sensorName = "RGBW",
        .supportedRates = rgbwSupportedRates,
        .sensorType = SENS_TYPE_RGBW,
        .numAxis = NUM_AXIS_THREE,
        .interrupt = NANOHUB_INT_NONWAKEUP,
        .minSamples = 20
    },
};

static const struct SensorOps mSensorOps[MAX_HANDLE] = {
    {
        .sensorPower = sensorPowerAls,
        .sensorFirmwareUpload = sensorFirmwareAls,
        .sensorSetRate = sensorRateAls,
        .sensorFlush = sensorFlushAls,
        .sensorSendOneDirectEvt = sendLastSampleAls,
        .sensorCalibrate = sensorCaliAls,
        .sensorCfgData = sensorCfgAls,
    },
    {
        .sensorPower = sensorPowerPs,
        .sensorFirmwareUpload = sensorFirmwarePs,
        .sensorSetRate = sensorRatePs,
        .sensorFlush = sensorFlushPs,
        .sensorSendOneDirectEvt = sendLastSampleProx,
        .sensorCalibrate = sensorCaliPs,
        .sensorCfgData = sensorCfgPs,
    },
    {
        .sensorPower = sensorPowerRGBW,
        .sensorFirmwareUpload = sensorFirmwareRGBW,
        .sensorSetRate = sensorRateRGBW,
        .sensorFlush = sensorFlushRGBW,
        .sensorSendOneDirectEvt = sendLastSampleRGBW,
    },
};

static void configEvent(struct sensPerUnit *mSensor, struct ConfigStat *ConfigData)
{
    enum sensorHandleState handle;

    for (handle = ALS; &mTask.mSensorPerUnit[handle] != mSensor; handle++) ;

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
    if (mTask.pendingSampleOnce) {
        mTask.pendingSampleOnce = false;
        osEnqueuePrivateEvt(EVT_PS_SAMPLE_ONCE, NULL, NULL, mTask.id);
        return;
    }
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
    for (handle = ALS; handle < MAX_HANDLE; handle++) {
        if (mTask.pendingConfig[handle]) {
            mTask.pendingConfig[handle] = false;
            configEvent(&mTask.mSensorPerUnit[handle], &mTask.mSensorPerUnit[handle].pendConfig);
            return;
        }
    }
    /* Process ALS & PS pending evt */
    for (handle = ALS; handle < MAX_HANDLE; handle++) {
        if (mTask.pendingCali[handle]) {
            mTask.pendingCali[handle] = false;
            mSensorOps[handle].sensorCalibrate((void *)handle);
            return;
        }
    }
    /* Process ALS & PS pending evt */
    for (handle = ALS; handle < MAX_HANDLE; handle++) {
        if (mTask.pendingCaliCfg[handle]) {
            mTask.pendingCaliCfg[handle] = false;
            mSensorOps[handle].sensorCfgData(mTask.mSensorPerUnit[handle].pendCaliCfg,
                (void *)handle);
            return;
        }
    }
}

static bool allocateDataEvt(uint64_t timeStamp)
{
    mTask.data_evt = slabAllocatorAlloc(mTask.mDataSlab);

    if (mTask.data_evt == NULL) {
        osLog(LOG_ERROR, "alsPs: Slab allocation failed\n");
        return false;
    }
    memset(&mTask.data_evt->samples[0].firstSample, 0x00, sizeof(struct SensorFirstSample));
    mTask.data_evt->referenceTime = timeStamp;
    mTask.prev_rtc_time = timeStamp;

    return true;
}

static void dataEvtFree(void *ptr)
{
    struct TripleAxisDataEvent *ev = (struct TripleAxisDataEvent *)ptr;
    slabAllocatorFree(mTask.mDataSlab, ev);
}

static void flushRawData(void)
{
    if (mTask.data_evt) {
        if (!osEnqueueEvt(EVENT_TYPE_BIT_DISCARDABLE | sensorGetMyEventType(SENS_TYPE_RGBW),
                mTask.data_evt, dataEvtFree)) {
            // don't log error since queue is already full. silently drop
            // this data event.
            dataEvtFree(mTask.data_evt);
        }
        mTask.data_evt = NULL;
    }
}

static void parseRawData(struct AlsPsData *data, uint64_t timeStamp)
{
    uint32_t rgbwData[4] = {0};
    struct TripleAxisDataPoint *sample;
    uint32_t delta_time = 0;

    if (mTask.data_evt == NULL) {
        if (!allocateDataEvt(timeStamp))
            return;
    }
    if (mTask.data_evt->samples[0].firstSample.numSamples >= MAX_NUM_COMMS_EVENT_SAMPLES) {
        osLog(LOG_ERROR, "rgbw parseRawData BAD INDEX\n");
        return;
    }
    sample = &mTask.data_evt->samples[mTask.data_evt->samples[0].firstSample.numSamples++];

    // the first deltatime is for sample size
    if (mTask.data_evt->samples[0].firstSample.numSamples > 1) {
        delta_time = timeStamp - mTask.prev_rtc_time;
        delta_time = delta_time < 0 ? 0 : delta_time;
        sample->deltaTime = delta_time;
        mTask.prev_rtc_time = timeStamp;
    }
    rgbwData[0] = data->r;
    rgbwData[1] = data->g;
    rgbwData[2] = data->b;
    rgbwData[3] = data->w;
    rgbwDataToTripleAxisData(rgbwData, sample);
    flushRawData();
}

static void handleSensorEvent(const void *state)
{
    int i = 0;
    union EmbeddedDataPoint sample;
    enum AlsPsState mState = (enum AlsPsState)state;
    uint32_t timerDelay = 0;

    //osLog(LOG_DEBUG, "handleSensorEvent: %d\n", mState);
    switch (mState) {
        default: {
            sensorFsmSwitchState(NULL, &mTask.fsm, &i2cCallback, &spiCallback);
            break;
        }
        case CHIP_IDLE: {
            //osLog(LOG_DEBUG, "alsPs: STATE_IDLE done\n");
            break;
        }

        case CHIP_INIT_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            if (mTask.mSensorPerUnit[ALS].handle)
                sensorRegisterInitComplete(mTask.mSensorPerUnit[ALS].handle);
            if (mTask.mSensorPerUnit[PS].handle)
                sensorRegisterInitComplete(mTask.mSensorPerUnit[PS].handle);
            if (mTask.mSensorPerUnit[RGBW].handle)
                sensorRegisterInitComplete(mTask.mSensorPerUnit[RGBW].handle);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "alsPs: init done\n");
            processPendingEvt();
            break;
        }

        case CHIP_ALS_ENABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[ALS].handle,
                                    SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "als: enable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_ALS_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[ALS].handle,
                                    SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "als: disable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_ALS_RATECHG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.mSensorPerUnit[ALS].timerDelay =
                sensorTimerLookupCommon(alsSupportedRates, rateTimerValsAls,
                                                 mTask.mSensorPerUnit[ALS].rate);
            if (mTask.mSensorPerUnit[RGBW].nowOn) {
                timerDelay =
                    (mTask.mSensorPerUnit[RGBW].timerDelay > mTask.mSensorPerUnit[ALS].timerDelay) ?
                    mTask.mSensorPerUnit[ALS].timerDelay : mTask.mSensorPerUnit[RGBW].timerDelay;
            } else {
                timerDelay = mTask.mSensorPerUnit[ALS].timerDelay;
            }

            if (mTask.mSensorPerUnit[ALS].timerHandle)
                timTimerCancel(mTask.mSensorPerUnit[ALS].timerHandle);

            mTask.mSensorPerUnit[ALS].timerHandle = timTimerSet(timerDelay, 0, 50, alsTimerCallback, NULL, false);

            sensorSignalInternalEvt(mTask.mSensorPerUnit[ALS].handle,
                                    SENSOR_INTERNAL_EVT_RATE_CHG, mTask.mSensorPerUnit[ALS].rate, mTask.mSensorPerUnit[ALS].latency);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "als: ratechg done timerDelay:%luns\n", timerDelay);
            processPendingEvt();
            break;
        }

        case CHIP_RGBW_ENABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[RGBW].handle,
                                    SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "RGBW: enable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_RGBW_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[RGBW].handle,
                                    SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "RGBW: disable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_RGBW_RATECHG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            /* do something, turn on the sampling timer */
            mTask.mSensorPerUnit[RGBW].timerDelay =
                sensorTimerLookupCommon(rgbwSupportedRates, rateTimerValsRGBW,
                                                 mTask.mSensorPerUnit[RGBW].rate);
            if (mTask.mSensorPerUnit[ALS].nowOn) {
                timerDelay =
                    (mTask.mSensorPerUnit[ALS].timerDelay > mTask.mSensorPerUnit[RGBW].timerDelay) ?
                    mTask.mSensorPerUnit[RGBW].timerDelay : mTask.mSensorPerUnit[ALS].timerDelay;
            } else {
                timerDelay = mTask.mSensorPerUnit[RGBW].timerDelay;
            }

            if (mTask.mSensorPerUnit[ALS].timerHandle)
                timTimerCancel(mTask.mSensorPerUnit[ALS].timerHandle);

            mTask.mSensorPerUnit[ALS].timerHandle = timTimerSet(timerDelay, 0, 50, alsTimerCallback, NULL, false);

            sensorSignalInternalEvt(mTask.mSensorPerUnit[RGBW].handle,
                                    SENSOR_INTERNAL_EVT_RATE_CHG, mTask.mSensorPerUnit[RGBW].rate, mTask.mSensorPerUnit[RGBW].latency);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "RGBW: ratechg done timerDelay:%luns\n", timerDelay);
            processPendingEvt();
            break;
        }

        case CHIP_PS_ENABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit[PS].handle,
                                    SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "ps: enable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_PS_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);

            if (mTask.intrMode == PS_INTERRUPT_MODE && mTask.mSensorPerUnit[PS].timerHandle) {
                timTimerCancel(mTask.mSensorPerUnit[PS].timerHandle);
                mTask.mSensorPerUnit[PS].timerHandle = 0;
            }

            sensorSignalInternalEvt(mTask.mSensorPerUnit[PS].handle,
                                    SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "ps: disable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_PS_RATECHG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            if (mTask.intrMode == PS_POLLING_MODE) {
                /* do something, turn on the sampling timer */
                timerDelay = sensorTimerLookupCommon(psSupportedRates, rateTimerValsPs,
                                                  mTask.mSensorPerUnit[PS].rate);
                if (mTask.mSensorPerUnit[PS].timerHandle)
                    timTimerCancel(mTask.mSensorPerUnit[PS].timerHandle);

                mTask.mSensorPerUnit[PS].timerHandle = timTimerSet(timerDelay, 0, 50, psTimerCallback, NULL, false);
                if (mTask.mSensorPerUnit[PS].timerHandle == 0)
                    configASSERT(0);
            } else {
                if (mTask.mSensorPerUnit[PS].timerHandle)
                    timTimerCancel(mTask.mSensorPerUnit[PS].timerHandle);
                mTask.mSensorPerUnit[PS].timerHandle = timTimerSet(PS_DEBOUNCE_TIME, 0, 50, psOncePollingTimerCallback, NULL, true);
                if (mTask.mSensorPerUnit[PS].timerHandle == 0)
                    configASSERT(0);
            }
            sensorSignalInternalEvt(mTask.mSensorPerUnit[PS].handle,
                                    SENSOR_INTERNAL_EVT_RATE_CHG, mTask.mSensorPerUnit[PS].rate, mTask.mSensorPerUnit[PS].latency);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "ps: ratechg done, timerdelay = %lu\n", timerDelay);
            processPendingEvt();
            break;
        }

        case CHIP_PS_CFG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "ps: cfg done\n");
            processPendingEvt();
            break;
        }

        case CHIP_PS_CALI_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "ps: cali done\n");
            processPendingEvt();
            break;
        }

        case CHIP_SAMPLING_ALS_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            //for ALS
            if ((mTask.mSensorPerUnit[ALS].nowOn && mTask.mSensorPerUnit[ALS].isReading) ||
                    (mTask.mSensorPerUnit[RGBW].nowOn && mTask.mSensorPerUnit[RGBW].isReading)) {
                for (i = 0; i < mTask.outSize; ++i) {
                    if (mTask.current_sample[i].sensType == SENS_TYPE_ALS) {
                        if (mTask.mSensorPerUnit[ALS].last_sample.als_data !=
                                mTask.current_sample[i].als_data) {
                            mTask.mSensorPerUnit[ALS].last_sample.als_data =
                                mTask.current_sample[i].als_data;
                            sample.fdata = mTask.current_sample[i].als_data;
                            osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_ALS), sample.vptr, NULL);
                        }
                    } else if (mTask.current_sample[i].sensType == SENS_TYPE_RGBW) {
                        parseRawData(&mTask.current_sample[i], rtcGetTime());
                    }
                }
            }

            mTask.mSensorPerUnit[ALS].isReading = false;
            mTask.mSensorPerUnit[RGBW].isReading = false;
            mTask.nowState = CHIP_IDLE;
            processPendingEvt();
            //osLog(LOG_DEBUG, "mTask.isReading:%d\n", mTask.isReading);
            break;
        }

        case CHIP_SAMPLING_PS_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            //for PS
            if (mTask.mSensorPerUnit[PS].nowOn && mTask.mSensorPerUnit[PS].isReading) {
                for (i = 0; i < mTask.outSize; ++i) {
                     if (mTask.current_sample[i].sensType == SENS_TYPE_PROX) {
                         if (mTask.mSensorPerUnit[PS].last_sample.prox_state !=
                                 mTask.current_sample[i].prox_state) {
                             mTask.mSensorPerUnit[PS].last_sample.prox_state =
                                 mTask.current_sample[i].prox_state;
                             sample.fdata = mTask.mSensorPerUnit[PS].last_sample.prox_data =
                                 mTask.current_sample[i].prox_data;
                             osLog(LOG_INFO, "ps: data update:%"PRIu32"\n", mTask.mSensorPerUnit[PS].last_sample.prox_data);
                             osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_PROX), sample.vptr, NULL);
                         }
                     }
                }
            }

            mTask.mSensorPerUnit[PS].isReading = false;
            mTask.nowState = CHIP_IDLE;
            processPendingEvt();
            //osLog(LOG_DEBUG, "mTask.isReading:%d\n", mTask.isReading);
            break;
        }

        case CHIP_ALS_CFG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "als: cfg done\n");
            processPendingEvt();
            break;
        }

        case CHIP_ALS_CALI_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "als: cali done\n");
            processPendingEvt();
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
                osLog(LOG_INFO, "alsps: app start\n");
                /* Reset chip */
                sensorFsmRunState(NULL, &mTask.fsm, (const void *)CHIP_RESET, &i2cCallback, &spiCallback);
            } else
               osLog(LOG_INFO, "alsps: wait for auto detect\n");
            break;
        }

        case EVT_SENSOR_EVENT: {
            handleSensorEvent(evtData);
            break;
        }

        case EVT_SENSOR_TIMER: {
            /* Start sampling for a value */
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.mSensorPerUnit[ALS].isReading ||
                    !mTask.mSensorPerUnit[RGBW].isReading) {
                    mTask.nowState = CHIP_SAMPLING_ALS;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.current_sample;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING_ALS, &i2cCallback, &spiCallback);
                    mTask.mSensorPerUnit[ALS].isReading = true;
                    mTask.mSensorPerUnit[RGBW].isReading = true;
                }
            } else {
                osLog(LOG_INFO, "alsPs EST pending by:%d\n", mTask.nowState);
                mTask.pendingSample = true;
            }
            break;
        }

        case EVT_SENSOR_INTR: {
            /* Start sampling for a value */
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.mSensorPerUnit[PS].isReading) {
                    mTask.nowState = CHIP_SAMPLING_PS;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.current_sample;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING_PS, &i2cCallback, &spiCallback);
                    mTask.mSensorPerUnit[PS].isReading = true;
                }
            } else {
                osLog(LOG_INFO, "alsPs ESI pending by:%d\n", mTask.nowState);
                mTask.pendingInt = true;
            }
            break;
        }

        case EVT_PS_SAMPLE_ONCE: {
            /* Start sampling for a value */
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.mSensorPerUnit[PS].isReading) {
                    mTask.nowState = CHIP_SAMPLING_PS_ONCE;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.current_sample;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING_PS_ONCE, &i2cCallback, &spiCallback))
                        mTask.nowState = CHIP_IDLE;
                    else
                        mTask.mSensorPerUnit[PS].isReading = true;
                }
            } else {
                osLog(LOG_INFO, "alsPs EPSO pending by:%d\n", mTask.nowState);
                mTask.pendingSampleOnce = true;
            }
            break;
        }

        case EVT_ERROR_EVENT: {
            sendBusErrToAp(evtData);
            break;
        }
    }
}

void alsPsSendCalibrationResult(uint8_t sensorType, int32_t value[2]) {
    struct CalibrationData *data = heapAlloc(sizeof(struct CalibrationData));
    if (!data) {
        osLog(LOG_ERROR, "Couldn't alloc data cali result pkt");
        return;
    }
    data->header.appId = APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ALS, 0, 0));
    data->header.dataLen = (sizeof(struct CalibrationData) - sizeof(struct HostHubRawPacket));
    data->data_header.msgId = SENSOR_APP_MSG_ID_CAL_RESULT;
    data->data_header.sensorType = sensorType;
    data->data_header.status = 0;

    data->values[0] = value[0];
    data->values[1] = value[1];
    if (!osEnqueueEvtOrFree(EVT_APP_TO_HOST, data, heapFree))
        osLog(LOG_ERROR, "Couldn't send data cali result evt");
}

void registerPsInterruptMode(uint8_t mode)
{
    mTask.intrMode = mode;
}

void registerAlsPsDriverFsm(struct sensorFsm *mSensorFsm, uint8_t size)
{
    mTask.fsm.moduleName = "alsps";
    mTask.fsm.mSensorFsm = mSensorFsm;
    mTask.fsm.mSensorFsmSize = size;
    osEnqueuePrivateEvt(EVT_APP_START, NULL, NULL, mTask.id);
}

void alsSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_ALS, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.mSensorPerUnit[ALS].handle =
        sensorRegister(&mSensorInfo[ALS], &mSensorOps[ALS], NULL, false);
}

void psSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_PROX, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.mSensorPerUnit[PS].handle =
        sensorRegister(&mSensorInfo[PS], &mSensorOps[PS], NULL, false);
}

void rgbwSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_RGBW, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.mSensorPerUnit[RGBW].handle =
        sensorRegister(&mSensorInfo[RGBW], &mSensorOps[RGBW], NULL, false);
}

void __attribute__((weak)) alspsSecondaryOverlayRemap(void)
{
    return;
}

static bool startTask(uint32_t taskId)
{
    size_t slabSize;

    alsPsDebugPoint = &mTask;


    /* Register tid first */
    mTask.id = taskId;
    mTask.intrMode = PS_INTERRUPT_MODE;

    slabSize = sizeof(struct TripleAxisDataEvent) +
               MAX_NUM_COMMS_EVENT_SAMPLES * sizeof(struct TripleAxisDataPoint);

    mTask.mDataSlab = slabAllocatorNew(slabSize, 4, 1);
    if (!mTask.mDataSlab) {
        osLog(LOG_INFO, "Slab allocation failed\n");
        return false;
    }

    mTask.data_evt = NULL;

#ifndef CFG_OVERLAY_INIT_SUPPORT
    /* Register sensor fsm last */
    module_init(SENS_TYPE_ALS);
#else
    extern void alspsOverlayRemap(void);
    alspsOverlayRemap();
    extern void alspsSecondaryOverlayRemap(void);
    alspsSecondaryOverlayRemap();
#endif

    return true;
}

static void endTask(void)
{
    sensorUnregister(mTask.mSensorPerUnit[ALS].handle);
    sensorUnregister(mTask.mSensorPerUnit[PS].handle);
}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ALS, 0, 0)), 0, startTask, endTask,
                  handleEvent);
