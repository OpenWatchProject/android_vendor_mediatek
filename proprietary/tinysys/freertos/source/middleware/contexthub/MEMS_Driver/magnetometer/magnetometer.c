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
#include <interrupt.h>

#include <contexthub_core.h>
#include <magnetometer.h>
#include <API_timestamp_calibration.h>
#include <performance.h>

#ifdef CFG_MTK_CALIBRATION_V1_0
#include <calibration/gyroscope/gyro_cal.h>
#endif

#define MAX_NUM_COMMS_EVENT_SAMPLES 15

enum magSensorEvents {
    EVT_SENSOR_EVENT = EVT_APP_START + 1,
    EVT_SENSOR_TIMER,
    EVT_SENSOR_INTR,
    EVT_SENSOR_CONTROL,
    EVT_ERROR_EVENT,
};
struct ConfigStat {
    uint64_t latency;
    uint32_t rate;
    bool enable;
};
SRAM_REGION_BSS static struct magTask {
    uint32_t id;
    uint32_t magHandle;
    uint32_t gyroHandle;
    uint32_t hwGyroSupported;
    uint32_t magTimerHandle;
    uint32_t rate;
    uint64_t latency;
    struct ConfigStat pendConfig;
    bool pendingConfig;
    bool pendingSample;
    bool pendingInt;
    bool pendingFlushFifo;
    bool pendingSelfTest;
    bool magNowOn;
    bool magReading;
    bool configed;
    bool fifoEnabled;
    uint8_t flush;
    uint8_t intrMode;
    uint8_t nowState;
    /* receive data which convert from rawdata, real output for algo and ap */
    struct magDataPacket *packet;
    uint8_t outSize;
    uint8_t elemOutSize;
    uint32_t timerDelay;
    uint32_t fifoDelay;
    uint64_t fifoStartTime;
    uint64_t prev_rtc_time;
    struct SlabAllocator *mDataSlab;
    struct TripleAxisDataEvent *data_evt;
    struct fsmCurrInfo fsm;
    struct magCalibrationLibAPI caliAPI;
    magTimerCbkF timerInterruptCbk;
} mTask;
SRAM_REGION_BSS static uint64_t last_mag_time;

static struct magTask *magDebugPoint;
static const uint32_t magSupportedRates[] = {
    SENSOR_HZ(5.0f),
    SENSOR_HZ(10.0f),
    SENSOR_HZ(16.0f),
    SENSOR_HZ(50.0f),
    SENSOR_HZ(100.0f),
    0
};

static const uint64_t rateTimerValsMag[] = {
//should match "supported rates in length" and be the timer length for that rate in nanosecs
    1000000000ULL / 5,
    1000000000ULL / 10,
    1000000000ULL / 16,
    1000000000ULL / 50,
    1000000000ULL / 100,
};

SRAM_REGION_FUNCTION static void i2cCallback(void *cookie, size_t tx, size_t rx, int err)
{
    //osLog(LOG_DEBUG, "mag: i2cCallback (%d) cookie:%d\n", err, (uint32_t)cookie);
    if (err == 0)
        osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
    else {
        osLog(LOG_ERROR, "mag: i2c error (%d)\n", err);
        osEnqueuePrivateEvt(EVT_ERROR_EVENT, (void *)ERR_SENSOR_I2C, NULL, mTask.id);
    }
}

/* sensor callbacks from nanohub */

SRAM_REGION_FUNCTION static void spiCallback(void *cookie, int err)
{
    //osLog(LOG_DEBUG, "mag: spiCallback (%d) cookie:%d\n", err, (uint32_t)cookie);
    if (err == 0)
        osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
    else {
        osLog(LOG_ERROR, "mag: spi error (%d)\n", err);
        osEnqueuePrivateEvt(EVT_ERROR_EVENT, (void *)ERR_SENSOR_SPI, NULL, mTask.id);
    }
}

SRAM_REGION_FUNCTION void magInterruptOccur(void)
{
    osEnqueuePrivateEvt(EVT_SENSOR_INTR, NULL, NULL, mTask.id);
    extern TaskHandle_t CHRE_TaskHandle;
    if (xTaskResumeFromISR(CHRE_TaskHandle) == pdTRUE) {
        portYIELD_WITHIN_API();
    }
}

void magRegisterInterruptMode(uint8_t mode)
{
    mTask.intrMode = mode;
}

SRAM_REGION_FUNCTION static void magTimerCallback(uint32_t timerId, void *cookie)
{
    if(mTask.timerInterruptCbk)
        mTask.timerInterruptCbk(rtcGetTime());
    osEnqueuePrivateEvt(EVT_SENSOR_TIMER, cookie, NULL, mTask.id);
}
#define GYRO_MIN_RATE                   SENSOR_HZ(200)
#define GYRO_FASTEST_LATENCY            20000000ull     // 20 ms
#define EVT_SENSOR_GYR_DATA_RDY         sensorGetMyEventType(SENS_TYPE_GYRO)
static void configGyro(bool on)
{
#ifndef CFG_MAG_CALIBRATION_IN_AP

    const struct SensorInfo *si = sensorFind(SENS_TYPE_GYRO, 0, &mTask.gyroHandle);

    if (!si)
        return;
    mTask.hwGyroSupported = !strcmp(si->sensorName, "Gyroscope");
    if (on) {
        if (mTask.hwGyroSupported) {
            sensorRequest(mTask.id, mTask.gyroHandle, GYRO_MIN_RATE, GYRO_FASTEST_LATENCY);
            osEventSubscribe(mTask.id, EVT_SENSOR_GYR_DATA_RDY);
        }
        if (mTask.caliAPI.initLib != NULL)
            mTask.caliAPI.initLib(mTask.hwGyroSupported);
    } else {
        if (mTask.hwGyroSupported) {
            sensorRelease(mTask.id, mTask.gyroHandle);
            osEventUnsubscribe(mTask.id, EVT_SENSOR_GYR_DATA_RDY);
        }
    }
#endif
}
static void handleGyroData(const void *evtData)
{
    struct magCaliDataInPut inputData;
    struct TripleAxisDataEvent *ev;
    struct TripleAxisDataPoint *sample;
    uint64_t timeStamp = 0;
    uint8_t i = 0;

    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    ev = (struct TripleAxisDataEvent *)evtData;
    timeStamp = ev->referenceTime;
    for (i = 0; i < ev->samples[0].firstSample.numSamples; i++) {
        sample = &ev->samples[i];
        if (i > 0)
            timeStamp += sample->deltaTime;
        inputData.timeStamp = timeStamp;
        inputData.x = sample->x;
        inputData.y = sample->y;
        inputData.z = sample->z;
        if (mTask.caliAPI.caliApiSetGyroData != NULL)
            mTask.caliAPI.caliApiSetGyroData(&inputData);
    }
}
static uint32_t calcuWaterMark(void)
{
    uint64_t min_latency = 0XFFFFFFFFFFFFFFFFULL;
    uint32_t max_rate = 0;
    uint8_t min_watermark = 4;
    uint8_t max_watermark = MAX_RECV_PACKET;
    uint8_t watermark = 0;
    uint32_t temp_cnt, total_cnt = 0;
    uint32_t temp_delay = 0;

    if (mTask.magNowOn && mTask.latency != SENSOR_LATENCY_NODATA) {
        min_latency =
            mTask.latency < min_latency ? mTask.latency : min_latency;
        max_rate = mTask.rate > max_rate ? mTask.rate : max_rate;
    }

    // if max_rate is less than or equal to 50Hz, we lower the minimum watermark level
    if (max_rate <= SENSOR_HZ(50.0f)) {
        min_watermark = 3;
    }

    if (min_latency == 0) {
        //osLog(LOG_DEBUG, "mag, watermark:%d\n", min_watermark);
        return min_watermark;
    }

    if (mTask.magNowOn && mTask.latency != SENSOR_LATENCY_NODATA && mTask.rate) {
        temp_delay = 1024000000000ULL / mTask.rate;
        temp_cnt = min_latency / temp_delay;
        total_cnt += temp_cnt;
    }
    watermark = total_cnt;
    watermark = watermark < min_watermark ? min_watermark : watermark;
    watermark = watermark > max_watermark ? max_watermark : watermark;
    //osLog(LOG_DEBUG, "mag, watermark:%d\n", watermark);
    return watermark;
}
static inline bool anyFifoEnabled(void)
{
    return mTask.fifoEnabled;
}

static void flushFifo(void)
{
    struct transferDataInfo dataInfo;
    bool any_fifo_enabled_prev;

    if (mTask.intrMode != MAG_UNFIFO) {
        any_fifo_enabled_prev = anyFifoEnabled();

        if (mTask.configed && mTask.latency != SENSOR_LATENCY_NODATA)
            mTask.fifoEnabled = true;
        else
            mTask.fifoEnabled = false;

        if (any_fifo_enabled_prev && anyFifoEnabled()) {
            if (!mTask.magReading && mTask.magNowOn) {
                mTask.nowState = CHIP_SAMPLING;
                dataInfo.inBuf = NULL;
                dataInfo.inSize = 0;
                dataInfo.elemInSize = 0;
                dataInfo.outBuf = &mTask.packet;
                dataInfo.outSize = &mTask.outSize;
                dataInfo.elemOutSize = &mTask.elemOutSize;
                sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
                mTask.magReading = true;
            }
        }
    }
}
static void setMagHwEnable(bool on)
{
    struct transferDataInfo dataInfo;

    mTask.pendingFlushFifo = false;
    configGyro(on);
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
}
static void magPowerCalculate(bool on)
{
    if (mTask.nowState == CHIP_IDLE) {
        if (!on) {
            if (mTask.magTimerHandle && mTask.intrMode != MAG_FIFO_INTERRUPTIBLE) {
                timTimerCancel(mTask.magTimerHandle);
                mTask.magTimerHandle = 0;
            }
            mTask.magReading = false;
        }
        mTask.pendingConfig = false;
        mTask.magNowOn = on;
        setMagHwEnable(on);
    } else {
        osLog(LOG_INFO, "magPowerCalculate pending by:%d\n", mTask.nowState);
        mTask.pendingConfig = true;
        mTask.pendConfig.enable = on;
    }
}
static void setMagHwRate(uint32_t rate, uint32_t waterMark)
{
    struct transferDataInfo dataInfo;
    struct magCntlPacket cntlPacket;

    mTask.pendingFlushFifo = false;
    cntlPacket.rate = rate;
    cntlPacket.waterMark = waterMark;

    mTask.nowState = CHIP_RATECHG;
    dataInfo.inBuf = (void *)&cntlPacket;
    dataInfo.inSize = 1;
    dataInfo.elemInSize = sizeof(struct magCntlPacket);
    dataInfo.outBuf = NULL;
    dataInfo.outSize = NULL;
    dataInfo.elemOutSize = NULL;
    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_RATECHG, &i2cCallback, &spiCallback);
}
static void magRateCalculate(uint32_t rate, uint64_t latency)
{
    uint32_t waterMark = 0;

    if (mTask.nowState == CHIP_IDLE) {
        mTask.rate = rate;
        mTask.latency = latency;
        switch (mTask.intrMode) {
            case MAG_FIFO_INTERRUPTIBLE:
                setMagHwRate(rate, calcuWaterMark());
                break;
            case MAG_FIFO_UNINTERRUPTIBLE:
                waterMark = calcuWaterMark();
                if (waterMark >= MAX_NUM_COMMS_EVENT_SAMPLES)
                    waterMark = MAX_NUM_COMMS_EVENT_SAMPLES;
                else if (waterMark >= MAX_NUM_COMMS_EVENT_SAMPLES >> 1)
                    waterMark = 6;
                mTask.timerDelay = waterMark * (1024000000000ULL / rate);
                setMagHwRate(rate, waterMark);
                break;
            case MAG_UNFIFO:
                mTask.timerDelay = sensorTimerLookupCommon(magSupportedRates, rateTimerValsMag, rate);
                setMagHwRate(rate, 0);
                break;
        }
    } else {
        osLog(LOG_INFO, "magRateCalculate pending by:%d\n", mTask.nowState);
        mTask.pendingConfig = true;
        mTask.pendConfig.enable = true;
        mTask.pendConfig.latency = latency;
        mTask.pendConfig.rate = rate;
    }
}
static bool sensorPowerMag(bool on, void *cookie)
{
    osLog(LOG_INFO, "sensorPowerMag on:%d, nowOn:%d\n", on, mTask.magNowOn);
    if (on != mTask.magNowOn) {
        if (mTask.nowState == CHIP_IDLE) {
            if (!on && !mTask.pendingFlushFifo) {
                mTask.configed = false;
                flushFifo();
            }
            magPowerCalculate(on);
        } else {
            osLog(LOG_INFO, "sensorPowerMag pending by:%d\n", mTask.nowState);
            mTask.pendingConfig = true;
            mTask.pendConfig.enable = on;
        }
    } else
        osLog(LOG_ERROR, "sensorPowerMag on != nowOn\n");
    return true;
}

static bool sensorFirmwareMag(void *cookie)
{
    sensorSignalInternalEvt(mTask.magHandle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}
static bool sensorRateMag(uint32_t rate, uint64_t latency, void *cookie)
{
    osLog(LOG_INFO, "sensorRateMag, rate:%lu, latency:%lld\n", rate, latency);

    if (mTask.nowState == CHIP_IDLE) {
        mTask.configed = true;
        if (!mTask.pendingFlushFifo)
            flushFifo();
        magRateCalculate(rate, latency);
    } else {
        osLog(LOG_INFO, "sensorRateMag pending by:%d\n", mTask.nowState);
        mTask.pendingConfig = true;
        mTask.pendConfig.enable = true;
        mTask.pendConfig.latency = latency;
        mTask.pendConfig.rate = rate;
    }
    return true;
}

static bool sensorFlushMag(void *cookie)
{
    struct transferDataInfo dataInfo;

    osLog(LOG_INFO, "mag sensorFlushMag\n");
    switch (mTask.intrMode) {
        case MAG_FIFO_INTERRUPTIBLE:
        case MAG_FIFO_UNINTERRUPTIBLE: {
            mTask.flush++;
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.magReading && mTask.magNowOn) {
                    mTask.nowState = CHIP_SAMPLING;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.packet;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
                    mTask.magReading = true;
                }
            } else {
                osLog(LOG_INFO, "mag flush pending by:%d\n", mTask.nowState);
                mTask.pendingSample = true;
            }
            break;
        }
        case MAG_UNFIFO:
            osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_MAG), SENSOR_DATA_EVENT_FLUSH, NULL);
            break;
    }
    return 0;
}

static bool sensorCfgMag(void *data, void *cookie)
{
    int err = 0;
    int32_t *values = data;
    float offset[AXES_NUM];
    int32_t caliParameter[6] = {0};

    osLog(LOG_ERROR, "sensorCfgMag:\n");
    osLog(LOG_ERROR, "values: %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld, %ld\n",
        values[0], values[1], values[2], values[3], values[4], values[5],
        values[6], values[7], values[8]);

    offset[AXIS_X] = (float)values[0] / MAGNETOMETER_INCREASE_NUM_AP;
    offset[AXIS_Y] = (float)values[1] / MAGNETOMETER_INCREASE_NUM_AP;
    offset[AXIS_Z] = (float)values[2] / MAGNETOMETER_INCREASE_NUM_AP;
    osLog(LOG_INFO, "bias: %f, %f, %f\n", (double)offset[AXIS_X],
            (double)offset[AXIS_Y], (double)offset[AXIS_Z]);
    if (mTask.caliAPI.caliApiSetOffset != NULL)
        mTask.caliAPI.caliApiSetOffset(offset);

    caliParameter[0] = values[3];
    caliParameter[1] = values[4];
    caliParameter[2] = values[5];
    caliParameter[3] = values[6];
    caliParameter[4] = values[7];
    caliParameter[5] = values[8];

    if (mTask.caliAPI.caliApiSetCaliParam != NULL) {
        err = mTask.caliAPI.caliApiSetCaliParam(&caliParameter[0]);
        if (err < 0) {
            osLog(LOG_ERROR, "mag mTask.caliAPI.caliApiSetCaliParam fail\n");
            return false;
        }
    }
    return true;
}

static bool sensorSelfTestMag(void *data)
{
    struct transferDataInfo dataInfo;

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_SELFTEST;
        dataInfo.inBuf = NULL;
        dataInfo.inSize = 0;
        dataInfo.elemInSize = 0;
        dataInfo.outBuf = NULL;
        dataInfo.outSize = 0;
        dataInfo.elemOutSize = 0;
        if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SELFTEST, &i2cCallback, &spiCallback) < 0)
            mTask.nowState = CHIP_IDLE;
    } else {
        osLog(LOG_INFO, "magSelfTest pending by:%d\n", mTask.nowState);
        mTask.pendingSelfTest = true;
    }
    return true;
}

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

#define DEC_OPS_CFG_SELFTEST(power, firmware, rate, flush, cfg, selftest) \
    DEC_OPS(power, firmware, rate, flush), \
    .sensorCfgData = cfg, \
    .sensorSelfTest = selftest

static const struct SensorInfo sensorInfoMag = {
    DEC_INFO_RATE_BIAS("Magnetometer", magSupportedRates,
        SENS_TYPE_MAG, NUM_AXIS_THREE, NANOHUB_INT_NONWAKEUP, 600, SENS_TYPE_MAG_BIAS),
};

static const struct SensorOps sensorOpsMag = {
    DEC_OPS_CFG_SELFTEST(sensorPowerMag, sensorFirmwareMag, sensorRateMag, sensorFlushMag, sensorCfgMag, sensorSelfTestMag),
};

static bool allocateDataEvt(uint64_t timeStamp)
{
    mTask.data_evt = slabAllocatorAlloc(mTask.mDataSlab);

    if (mTask.data_evt == NULL) {
        osLog(LOG_ERROR, "mag: Slab allocation failed\n");
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

static void sendFlushEvt(void)
{
    while (mTask.flush > 0) {
        osLog(LOG_INFO, "mag sendFlushEvt\n");
        osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_MAG), SENSOR_DATA_EVENT_FLUSH, NULL);
        mTask.flush--;
    }
}
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
        if (mTask.pendConfig.enable == 0 && mTask.magNowOn)
            sensorOpsMag.sensorPower(false, NULL);
        else if (mTask.pendConfig.enable == 1 && !mTask.magNowOn)
            sensorOpsMag.sensorPower(true, NULL);
        else
            sensorOpsMag.sensorSetRate(mTask.pendConfig.rate, mTask.pendConfig.latency, NULL);
        return;
    }
    if (mTask.pendingSelfTest) {
        mTask.pendingSelfTest = false;
        sensorOpsMag.sensorSelfTest((void *)0);
        return;
    }
}

// Returns temperature in units of 0.01 degrees celsius.

static void flushRawData(void)
{
    if (mTask.data_evt) {
        if (!osEnqueueEvt(EVENT_TYPE_BIT_DISCARDABLE | sensorGetMyEventType(SENS_TYPE_MAG),
                mTask.data_evt, dataEvtFree)) {
            // don't log error since queue is already full. silently drop
            // this data event.
            dataEvtFree(mTask.data_evt);
        }
        mTask.data_evt = NULL;
    }
}

static void flushBiasData(void)
{
    if (mTask.data_evt) {
        if (!osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_MAG_BIAS),
                mTask.data_evt, dataEvtFree)) {
            // don't log error since queue is already full. silently drop
            // this data event.
            dataEvtFree(mTask.data_evt);
        }
        mTask.data_evt = NULL;
    }
}


void magSendTestResult(uint8_t status) {
    struct TestResultData *data = heapAlloc(sizeof(struct TestResultData));
    if (!data) {
        osLog(LOG_WARN, "Couldn't alloc test result packet");
        return;
    }
    data->header.appId = APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_MAG, 0, 0));
    data->header.dataLen = (sizeof(struct TestResultData) - sizeof(struct HostHubRawPacket));
    data->data_header.msgId = SENSOR_APP_MSG_ID_TEST_RESULT;
    data->data_header.sensorType = SENS_TYPE_MAG;
    data->data_header.status = status;

    if (!osEnqueueEvtOrFree(EVT_APP_TO_HOST, data, heapFree))
        osLog(LOG_WARN, "Couldn't send test result packet");
}

void magSendCalibrationParameter(uint8_t sensorType, int32_t param[6])
{
    uint8_t i = 0;
    struct CalibrationData *data = heapAlloc(sizeof(struct CalibrationData));
    if (!data) {
        osLog(LOG_ERROR, "Couldn't alloc data cali result pkt");
        return;
    }
    data->header.appId = APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_MAG, 0, 0));
    data->header.dataLen = (sizeof(struct CalibrationData) - sizeof(struct HostHubRawPacket));
    data->data_header.msgId = SENSOR_APP_MSG_ID_CAL_RESULT;
    data->data_header.sensorType = sensorType;
    data->data_header.status = 0;
    for (i = 0; i < 6; i++) {
        data->values[i] = param[i];
    }
    if (!osEnqueueEvtOrFree(EVT_APP_TO_HOST, data, heapFree))
        osLog(LOG_ERROR, "Couldn't send data cali result evt");
}

static void parseRawData(const struct magData *outBuf, uint64_t timeStamp)
{
    struct magCaliDataInPut inputData;
    struct magCaliDataOutPut outputData;
    struct TripleAxisDataPoint *sample;
    uint32_t delta_time = 0;
    static uint8_t last_accurancy = 0;
    bool newMagBias = false;
    int err = 0;
    int32_t caliParameter[6] = {0};

    if (mTask.data_evt == NULL) {
        if (!allocateDataEvt(timeStamp))
            return;
    }
    if (mTask.data_evt->samples[0].firstSample.numSamples >= MAX_NUM_COMMS_EVENT_SAMPLES) {
        osLog(LOG_ERROR, "mag parseRawData BAD INDEX\n");
        return;
    }
    inputData.timeStamp = timeStamp;
    inputData.x = outBuf->x;
    inputData.y = outBuf->y;
    inputData.z = outBuf->z;
    if (mTask.caliAPI.doCaliApi != NULL) {
        err = mTask.caliAPI.doCaliApi(&inputData, &outputData);
        if (err < 0) {
            osLog(LOG_ERROR, "mag mTask.caliAPI.doCaliApi fail\n");
            return;
        }
    } else {
        memset(&outputData, 0, sizeof(struct magCaliDataOutPut));
        outputData.x = inputData.x;
        outputData.y = inputData.y;
        outputData.z = inputData.z;
    }

    if (outputData.status != last_accurancy)
        newMagBias = true;
    else
        newMagBias = false;
    last_accurancy = outputData.status;
    if (newMagBias) {
        if (mTask.data_evt->samples[0].firstSample.numSamples > 0) {
            // flush existing samples so the bias appears after them
            flushRawData();
            if (!allocateDataEvt(timeStamp))
                return;
        }
        if (outputData.status == 3)
            mTask.data_evt->samples[0].firstSample.biasCurrent = 1;
        else
            mTask.data_evt->samples[0].firstSample.biasCurrent = 0;
        mTask.data_evt->samples[0].firstSample.biasPresent = 1;
        mTask.data_evt->samples[0].firstSample.biasSample =
                mTask.data_evt->samples[0].firstSample.numSamples;
        sample = &mTask.data_evt->samples[mTask.data_evt->samples[0].firstSample.numSamples++];
        sample->x = outputData.x_bias;
        sample->y = outputData.y_bias;
        sample->z = outputData.z_bias;
        flushBiasData();

        // bias is non-discardable, if we fail to enqueue, don't clear new_mag_bias
        if (!allocateDataEvt(timeStamp))
            return;
        if (mTask.caliAPI.caliApiGetCaliParam != NULL) {
            err = mTask.caliAPI.caliApiGetCaliParam(&caliParameter[0]);
            if (err < 0) {
                osLog(LOG_ERROR, "mag mTask.caliAPI.caliApiGetCaliParam fail\n");
                return;
            }
            magSendCalibrationParameter(SENS_TYPE_MAG, &caliParameter[0]);
        }
    }
    sample = &mTask.data_evt->samples[mTask.data_evt->samples[0].firstSample.numSamples++];

    // the first deltatime is for sample size
    if (mTask.data_evt->samples[0].firstSample.numSamples > 1) {
        delta_time = timeStamp - mTask.prev_rtc_time;
        delta_time = delta_time < 0 ? 0 : delta_time;
        sample->deltaTime = delta_time;
        mTask.prev_rtc_time = timeStamp;
    }
    //osLog(LOG_ERROR, "mag: numSamples:%d, deltaTime:%d, rtc_time:%lld, prev_rtc_time:%lld\n",
        //mTask.data_evt->samples[0].firstSample.numSamples, sample->deltaTime,
        //rtc_time, mTask.prev_rtc_time);

    sample->x = outputData.x;
    sample->y = outputData.y;
    sample->z = outputData.z;
#if defined(CFG_MTK_CALIBRATION_V1_0) && !defined(CFG_MAG_CALIBRATION_IN_AP)
    extern struct GyroCal *GetGyroCalBase(void);
    gyroCalUpdateMag(GetGyroCalBase(), timeStamp, sample->x, sample->y, sample->z);
#endif
    // osLog(LOG_DEBUG, "mag: x: %d, y: %d, z: %d\n", sample->ix, sample->iy, sample->iz);

    if (mTask.data_evt->samples[0].firstSample.numSamples == MAX_NUM_COMMS_EVENT_SAMPLES)
        flushRawData();
}
static void handleSensorEvent(const void *state)
{
    uint8_t i = 0;
    uint64_t timeStamp = 0, final_sample_time = 0;
    uint64_t magFifoStartTime = 0, nowTime = 0, destTime = 0, eventTimeStamp = 0;
    uint8_t magOutSize = 0;
    enum MagState mState = (enum MagState)state;

    switch (mState) {
        default: {
            sensorFsmSwitchState(NULL, &mTask.fsm, &i2cCallback, &spiCallback);
            break;
        }
        case CHIP_IDLE: {
            //osLog(LOG_DEBUG, "mag: STATE_IDLE done\n");
            break;
        }

        case CHIP_INIT_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            if (mTask.magHandle)
                sensorRegisterInitComplete(mTask.magHandle);
            mTask.nowState = CHIP_IDLE;
            processPendingEvt();
            osLog(LOG_INFO, "mag: init done\n");
            break;
        }

        case CHIP_ENABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.magHandle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "mag: enable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.magHandle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "mag: disable done\n");
            flushRawData();
            processPendingEvt();
            break;
        }

        case CHIP_RATECHG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            switch (mTask.intrMode) {
                case MAG_FIFO_INTERRUPTIBLE:
                    break;
                case MAG_FIFO_UNINTERRUPTIBLE:
                    if (mTask.magTimerHandle)
                        timTimerCancel(mTask.magTimerHandle);
                    mTask.magTimerHandle = timTimerSet(mTask.timerDelay, 0, 50, magTimerCallback, NULL, false);
                    osLog(LOG_INFO, "mag intrMode: MFU, timerDelay:%lu\n", mTask.timerDelay);
                    break;
                case MAG_UNFIFO:
                    if (mTask.magTimerHandle)
                        timTimerCancel(mTask.magTimerHandle);
                    resetMagTimeStamp(); /*before timer started, reset the timeStamp Buffer, no matter whether device driver call moving average or not*/
                    mTask.magTimerHandle = timTimerSet(mTask.timerDelay, 0, 50, magTimerCallback, NULL, false);
                    osLog(LOG_INFO, "mag intrMode: MU, timerDelay:%lu\n", mTask.timerDelay);
                    break;
            }
            sensorSignalInternalEvt(mTask.magHandle, SENSOR_INTERNAL_EVT_RATE_CHG, mTask.rate, mTask.latency);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "mag: ratechg done\n");
            processPendingEvt();
            break;
        }

        case CHIP_SELFTEST_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "mag: selftest done\n");
            processPendingEvt();
            break;
        }

        case CHIP_SAMPLING_DONE: {
            mark_timestamp(SENS_TYPE_MAG, SENS_TYPE_MAG, SAMPLE_DONE, rtcGetTime());
            sensorFsmReleaseState(&mTask.fsm);
            if (mTask.magNowOn && mTask.magReading) {
                timeStamp = mTask.packet->timeStamp;
                nowTime = magFifoStartTime = rtcGetTime();
                magOutSize = mTask.outSize;
                if (mTask.intrMode != MAG_UNFIFO) {
                    if (mTask.magNowOn && mTask.outSize != 0) {
                        MagSensor_calibrate_fifo_timestamp(timeStamp, mTask.outSize,
                        (unsigned long long *)&final_sample_time, (unsigned long long *)&mTask.fifoDelay);
                        //osLog(LOG_DEBUG, "mag final_sample_time:%lld, nowtime:%lld, fifoDelay:%d, cnt:%d\n",
                            //final_sample_time, timeStamp, mTask.fifoDelay, mTask.outSize);
                    } else if (mTask.magNowOn && mTask.outSize == 0) {
                        if (mTask.fifoStartTime <= magFifoStartTime)
                            mTask.fifoStartTime = magFifoStartTime;
                        MagSensor_set_init_timestamp((long long)mTask.fifoStartTime, (int)mTask.fifoDelay);
                    }
                }
                for (i = 0; i < mTask.outSize; ++i) {
                    if (mTask.intrMode != MAG_UNFIFO) {
                        eventTimeStamp = mTask.fifoStartTime + mTask.fifoDelay;
                        if (eventTimeStamp > final_sample_time) {
                            if (nowTime > final_sample_time)
                                eventTimeStamp = final_sample_time;
                            else
                                eventTimeStamp = nowTime;
                        }
                        mTask.fifoStartTime = eventTimeStamp;
                    } else {
                        destTime = (uint64_t)mTask.timerDelay * (--magOutSize);
                        eventTimeStamp = timeStamp - destTime;
                        if (eventTimeStamp <= last_mag_time) {
                            osLog(LOG_INFO, "mag time reverse nowtime=%lld, lasttime=%lld\n",
                                eventTimeStamp, last_mag_time);
                            eventTimeStamp = last_mag_time + 10000;
                        }
                        last_mag_time = eventTimeStamp;
                        if (eventTimeStamp >= nowTime)
                            osLog(LOG_INFO, "mag eventTimeStamp - nowtime = %lld\n", eventTimeStamp - nowTime);
                    }
                    parseRawData(&mTask.packet->outBuf[i], eventTimeStamp);
                }
            }
            mTask.magReading = false;
            flushRawData();
            sendFlushEvt();
            mTask.nowState = CHIP_IDLE;
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
                osLog(LOG_INFO, "magnetometer: app start\n");
                /* Reset chip */
                dataInfo.inBuf = NULL;
                dataInfo.inSize = 0;
                dataInfo.elemInSize = 0;
                dataInfo.outBuf = NULL;
                dataInfo.outSize = NULL;
                dataInfo.elemOutSize = NULL;
                sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_RESET, &i2cCallback, &spiCallback);
            } else
                osLog(LOG_INFO, "magnetometer: wait for auto detect\n");
            break;
        }

        case EVT_SENSOR_EVENT: {
            handleSensorEvent(evtData);
            break;
        }

        case EVT_SENSOR_TIMER: {
            /* Start sampling for a value */
            mark_timestamp(SENS_TYPE_MAG, SENS_TYPE_MAG, SAMPLE_BEGIN, rtcGetTime());
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.magReading) {
                    mTask.nowState = CHIP_SAMPLING;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.packet;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
                    mTask.magReading = true;
                }
            } else {
                osLog(LOG_INFO, "mag EST pending by:%d\n", mTask.nowState);
                mTask.pendingSample = true;
            }
            break;
        }

        case EVT_SENSOR_INTR: {
            /* Start sampling for a value */
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.magReading) {
                    mTask.nowState = CHIP_SAMPLING;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.packet;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAMPLING, &i2cCallback, &spiCallback);
                    mTask.magReading = true;
                }
            } else {
                osLog(LOG_INFO, "mag ESI pending by:%d\n", mTask.nowState);
                mTask.pendingInt = true;
            }
            break;
        }
        case EVT_SENSOR_GYR_DATA_RDY: {
            handleGyroData(evtData);
            break;
        }
        case EVT_ERROR_EVENT: {
            sendBusErrToAp(evtData);
            break;
        }
    }
}
void registerMagDriverFsm(struct sensorFsm *mSensorFsm, uint8_t size)
{
    mTask.fsm.moduleName = "mag";
    mTask.fsm.mSensorFsm = mSensorFsm;
    mTask.fsm.mSensorFsmSize = size;
    osEnqueuePrivateEvt(EVT_APP_START, NULL, NULL, mTask.id);
}
void registerMagCaliAPI(struct magCalibrationLibAPI *API)
{
    memset(&mTask.caliAPI, 0x00, sizeof(struct magCalibrationLibAPI));
    mTask.caliAPI.initLib = API->initLib;
    mTask.caliAPI.caliApiGetOffset = API->caliApiGetOffset;
    mTask.caliAPI.caliApiSetOffset = API->caliApiSetOffset;
    mTask.caliAPI.caliApiGetCaliParam = API->caliApiGetCaliParam;
    mTask.caliAPI.caliApiSetCaliParam = API->caliApiSetCaliParam;
    mTask.caliAPI.caliApiSetGyroData = API->caliApiSetGyroData;
    mTask.caliAPI.doCaliApi = API->doCaliApi;
}
void registerMagFifoInfo(uint64_t fifoStartTime, uint32_t fifoDelay)
{
    uint64_t magFifoStartTime;

    magFifoStartTime = rtcGetTime();
    if (mTask.fifoStartTime <= magFifoStartTime)
        mTask.fifoStartTime = magFifoStartTime;

    mTask.fifoDelay = fifoDelay;

    MagSensor_set_init_timestamp((long long)mTask.fifoStartTime, (int)fifoDelay);
}
void registerMagTimerCbk(magTimerCbkF cbk)
{
    mTask.timerInterruptCbk = cbk;
}

#define timeStampArraySize 5
uint64_t timeStampArray[timeStampArraySize];
uint64_t timeStampReport;
int timeStampArrayIndex;
int timeStampArrayCount;

void resetMagTimeStamp(void)
{
    memset(timeStampArray, 0x0, sizeof(timeStampArray));
    timeStampArrayIndex = 0;
    timeStampArrayCount = 0;
    timeStampReport = 0;
    /*osLog(LOG_ERROR, "resetMagTimer\n");*/
}
uint64_t addThenRetreiveAverageMagTimeStamp(uint64_t newTime)
{
    /*1st element*/
    if (timeStampArrayIndex == 0 && timeStampArrayCount == 0){
        timeStampArray[timeStampArrayIndex] = newTime;
        timeStampReport = newTime;
        timeStampArrayIndex ++;
        timeStampArrayCount ++;
    }
    else if(timeStampArrayCount == 1) { /*one there and 2nd comes*/
        timeStampArray[timeStampArrayIndex] = newTime;
        timeStampReport = timeStampReport + (timeStampArray[1] - timeStampArray[0]);
        timeStampArrayIndex ++;
        timeStampArrayCount ++;
    }
    else if(timeStampArrayCount < timeStampArraySize) { /*less than 5 elements in array, starts from 3rd element*/
        timeStampArray[timeStampArrayIndex] = newTime;
        timeStampReport = timeStampReport + (timeStampArray[timeStampArrayIndex] - timeStampArray[0])/(timeStampArrayCount);
        if(timeStampArrayIndex == (timeStampArraySize-1))
            timeStampArrayIndex = 0;
        else
            timeStampArrayIndex ++;
        if (timeStampArrayCount < timeStampArraySize) /*count will never larger than 5*/
            timeStampArrayCount ++;
    }
    else { /*5 element in array, adding new item remove oldest item in the same time*/
        timeStampArray[timeStampArrayIndex] = newTime;
        timeStampReport = timeStampReport + (timeStampArray[timeStampArrayIndex] - timeStampArray[(timeStampArrayIndex==4)?0:(timeStampArrayIndex+1)])/(timeStampArraySize-1);
        if(timeStampArrayIndex == (timeStampArraySize-1))
            timeStampArrayIndex = 0;
        else
            timeStampArrayIndex ++;
        if (timeStampArrayCount < timeStampArraySize) /*count will never larger than 5*/
            timeStampArrayCount ++;

    }

    /*osLog(LOG_ERROR, "addThenRetreiveMagTimer : %d, %d, %lld, %lld\n", timeStampArrayIndex, timeStampArrayCount, newTime, timeStampReport);*/
    return timeStampReport;
}
SRAM_REGION_FUNCTION int8_t magTimeBufferWrite(struct magTimeStampBuffer *record, uint64_t inTime)
{
    int8_t err = 0;
    SRAM_REGION_RO static const char STRING_FULL[] = "mag timer buffer full\n";

    if (!is_in_isr())
        taskENTER_CRITICAL();
    if (record->filledSize < MAX_MAG_TIMER_LENGTH) {
        record->buffer[record->head] = inTime;
        record->head ++;
        record->filledSize ++;
        //osLog(LOG_ERROR, "size:%d in: %lld\n", record->filledSize, inTime);
        if (record->head == MAX_MAG_TIMER_LENGTH)
            record->head = 0;
    } else {
        osLog(LOG_ERROR, STRING_FULL);
        err = -1;
    }
    if (!is_in_isr())
        taskEXIT_CRITICAL();

    return err;
}
SRAM_REGION_FUNCTION int8_t magTimeBufferRead(struct magTimeStampBuffer *record, uint64_t *outTime)
{
    int8_t err = 0;

    if (!is_in_isr())
        taskENTER_CRITICAL();
    if (record->filledSize > 0) {
        *outTime = record->buffer[record->tail];
        record->filledSize--;
        record->tail++;
        //osLog(LOG_ERROR, "size:%d out: %lld\n", record->filledSize, *outTime);
        if (record->tail == MAX_MAG_TIMER_LENGTH)
            record->tail = 0;
    } else
        err = -1;
    //osLog(LOG_ERROR, "mag timer not ready\n");
    if (!is_in_isr())
        taskEXIT_CRITICAL();

    return err;
}

SRAM_REGION_FUNCTION uint8_t magTimeBufferSize(struct magTimeStampBuffer *record)
{
    uint8_t filledSize;
    if (!is_in_isr())
        taskENTER_CRITICAL();
    filledSize = record->filledSize;
    if (!is_in_isr())
        taskEXIT_CRITICAL();
    return filledSize;
}

SRAM_REGION_FUNCTION void magTimeBufferReset(struct magTimeStampBuffer *record)
{
    if (!is_in_isr())
        taskENTER_CRITICAL();
    memset(record, 0x00, sizeof(struct magTimeStampBuffer));
    if (!is_in_isr())
        taskEXIT_CRITICAL();
}

void magSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_MAG, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.magHandle = sensorRegister(&sensorInfoMag, &sensorOpsMag, NULL, false);
}
static bool startTask(uint32_t taskId)
{
    size_t slabSize;


    magDebugPoint = &mTask;

    slabSize = sizeof(struct TripleAxisDataEvent) +
        MAX_NUM_COMMS_EVENT_SAMPLES * sizeof(struct TripleAxisDataPoint);

    mTask.mDataSlab = slabAllocatorNew(slabSize, 4, 5);
    if (!mTask.mDataSlab) {
        osLog(LOG_ERROR, "Slab allocation failed\n");
        return false;
    }
    mTask.data_evt = NULL;
    /* Register tid first */
    mTask.id = taskId;

#ifndef CFG_OVERLAY_INIT_SUPPORT
    /* Register sensor fsm last */
    module_init(SENS_TYPE_MAG);
#else
    extern void magOverlayRemap(void);
    magOverlayRemap();
#endif
    return true;
}

static void endTask(void)
{
}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_MAG, 0, 0)), 0, startTask, endTask, handleEvent);
