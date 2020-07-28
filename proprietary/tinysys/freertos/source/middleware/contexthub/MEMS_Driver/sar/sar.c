/* MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdio.h>
#include <stdint.h>

#include <heap.h>
#include <i2c.h>
#include <spi.h>
#include <timer.h>
#include <util.h>
#include <slab.h>
#include <plat/inc/rtc.h>

#include <eventnums.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <sensors.h>
#include <seos.h>

#include <contexthub_core.h>
#include <sar.h>

#define MAX_NUM_COMMS_EVENT_SAMPLES 1

enum SarSensorEvents {
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

struct sensPerUnit {
    uint32_t handle;

    uint32_t rate;
    uint64_t latency;
    uint32_t timerDelay;

    struct ConfigStat pendConfig;
    //int32_t pendCaliCfg[2];
    bool isReading;
    bool nowOn;
};
SRAM_REGION_BSS static struct sarTask {
    uint32_t id;
    uint32_t timerHandle;
    uint8_t intrMode;

    struct fsmCurrInfo fsm;
    struct sensPerUnit mSensorPerUnit;
    enum SarFsmState nowState;

    struct SarData_t last_sample;
    struct SarData_t *current_sample;
    uint8_t outSize;
    uint8_t elemOutSize;

    uint64_t prev_rtc_time;
    struct SlabAllocator *mDataSlab;
    struct TripleAxisDataEvent *data_evt;

    bool pendingSample;
    bool pendingConfig;
    bool pendingCali;
    bool pendingInt;
} mTask;

static struct sarTask *sarDebugPoint;

static const uint32_t sarSupportedRates[] = {
    SENSOR_HZ(5),
    SENSOR_HZ(15),
    SENSOR_HZ(50),
    SENSOR_HZ(100),
    SENSOR_RATE_ONCHANGE,
    0
};

static const uint64_t rateTimerValsSar[] = {
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
    //osLog(LOG_INFO, "sar: i2cCallback (%d) cookie:%d\n", err, (uint32_t)cookie);
    if (err == 0)
        osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
    else {
        osLog(LOG_ERROR, "sar: i2c error (%d)\n", err);
        osEnqueuePrivateEvt(EVT_ERROR_EVENT, (void *)ERR_SENSOR_I2C, NULL, mTask.id);
    }
}

/* sensor callbacks from nanohub */
SRAM_REGION_FUNCTION static void spiCallback(void *cookie, int err)
{
    //osLog(LOG_INFO, "sar: spiCallback (%d) cookie:%d\n", err, (uint32_t)cookie);
    if (err == 0)
        osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
    else {
        osLog(LOG_ERROR, "sar: spi error (%d)\n", err);
        osEnqueuePrivateEvt(EVT_ERROR_EVENT, (void *)ERR_SENSOR_SPI, NULL, mTask.id);
    }
}

SRAM_REGION_FUNCTION void sarInterruptOccur(void)
{
    osEnqueuePrivateEvt(EVT_SENSOR_INTR, NULL, NULL, mTask.id);
    extern TaskHandle_t CHRE_TaskHandle;
    if (xTaskResumeFromISR(CHRE_TaskHandle) == pdTRUE) {
        portYIELD_WITHIN_API();
    }
}

SRAM_REGION_FUNCTION void delayCallback(uint32_t timerId, void *cookie)
{
    /* osLog(LOG_DEBUG, "sarTimerCallback:cookie:%d\n", (uint32_t)cookie); */
    osEnqueuePrivateEvt(EVT_SENSOR_EVENT, cookie, NULL, mTask.id);
}

SRAM_REGION_FUNCTION static void sarTimerCallback(uint32_t timerId, void *cookie)
{
    /* osLog(LOG_INFO, "sarTimerCallback:cookie:%d\n", (uint32_t)cookie); */
    osEnqueuePrivateEvt(EVT_SENSOR_TIMER, cookie, NULL, mTask.id);
}

static bool sensorPowerSar(bool on, void *cookie)
{
    osLog(LOG_INFO, "sensorPowerSar on:%d, nowOn:%d\n", on, mTask.mSensorPerUnit.nowOn);
    if ((mTask.intrMode == SAR_POLLING_MODE) && !on) {
        mTask.mSensorPerUnit.timerDelay = 0XFFFFFFFF;
    }
    if ((mTask.intrMode == SAR_POLLING_MODE) && !on && mTask.timerHandle) {
        timTimerCancel(mTask.timerHandle);
        mTask.timerHandle = 0;
        mTask.mSensorPerUnit.isReading = false;
    }
    if (on != mTask.mSensorPerUnit.nowOn) {
        if (mTask.nowState == CHIP_IDLE) {
            mTask.pendingConfig = false;
            mTask.mSensorPerUnit.nowOn = on;
            if (on == true) {
                mTask.nowState = CHIP_SAR_ENABLE;
                sensorFsmRunState(NULL, &mTask.fsm, (const void *)CHIP_SAR_ENABLE, &i2cCallback, &spiCallback);
                mTask.last_sample.sarState = SAR_STATE_INIT;
            } else {
                mTask.mSensorPerUnit.isReading = false;
                mTask.nowState = CHIP_SAR_DISABLE;
                sensorFsmRunState(NULL, &mTask.fsm, (const void *)CHIP_SAR_DISABLE, &i2cCallback, &spiCallback);
                mTask.last_sample.sarState = SAR_STATE_INIT;
            }
        } else {
            osLog(LOG_INFO, "sensorPowerSar pending by nowState:%d\n", mTask.nowState);
            mTask.pendingConfig = true;
            mTask.mSensorPerUnit.pendConfig.enable = on;
        }
    } else
        osLog(LOG_ERROR, "sensorPowerSar on != nowOn\n");
    return true;
}

static bool sensorFirmwareSar(void *cookie)
{
    osLog(LOG_INFO, "%s \n", __func__);

    sensorSignalInternalEvt(mTask.mSensorPerUnit.handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static void setSarHwRate(uint32_t rate)
{
    struct transferDataInfo dataInfo;

    struct SarCntlPacket cntlPacket = {0};
    cntlPacket.rate = rate;

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_SAR_RATECHG;
        dataInfo.inBuf = (void *)&cntlPacket;
        dataInfo.inSize = 1;
        dataInfo.elemInSize = sizeof(struct SarCntlPacket);
        dataInfo.outBuf = NULL;
        dataInfo.outSize = NULL;
        dataInfo.elemOutSize = NULL;
        sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAR_RATECHG, &i2cCallback, &spiCallback);
    } else {
        osLog(LOG_INFO, "%s pending by nowState:%d\n", __func__, mTask.nowState);
        mTask.pendingConfig = true;
        mTask.mSensorPerUnit.pendConfig.enable = true;
        mTask.mSensorPerUnit.pendConfig.latency = mTask.mSensorPerUnit.latency;
        mTask.mSensorPerUnit.pendConfig.rate = mTask.mSensorPerUnit.rate;
    }
}

static bool sensorRateSar(uint32_t rate, uint64_t latency, void *cookie)
{
    osLog(LOG_INFO, "%s rate:%d, latency:%lld\n", __func__, rate, latency);
    mTask.mSensorPerUnit.rate = rate;
    mTask.mSensorPerUnit.latency = latency;
    mTask.mSensorPerUnit.timerDelay = rate;
    setSarHwRate(rate);
    return true;
}

static bool sensorFlushSar(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_SAR), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static bool sensorCaliSar(void *cookie)
{
    struct transferDataInfo dataInfo;
    osLog(LOG_INFO, "sensorCaliSar\n");

    if (mTask.nowState == CHIP_IDLE) {
        mTask.nowState = CHIP_SAR_CALI;
        dataInfo.inBuf = NULL;
        dataInfo.inSize = 0;
        dataInfo.elemInSize = 0;
        dataInfo.outBuf = NULL;
        dataInfo.outSize = 0;
        dataInfo.elemOutSize = 0;
        if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAR_CALI, &i2cCallback, &spiCallback) < 0)
            mTask.nowState = CHIP_IDLE;
    } else {
        osLog(LOG_INFO, "sensorCaliSar pending by:%d\n", mTask.nowState);
        mTask.pendingCali = true;
    }
    return true;
}
/*

static void setPsHwCaliCfg(int32_t *data)
{
    struct alspsCaliCfgPacket caliCfgPacket;
    struct transferDataInfo dataInfo;

    caliCfgPacket.caliCfgData[0] = data[0];
    caliCfgPacket.caliCfgData[1] = data[1];

    mTask.nowState = CHIP_SAR_CFG;
    dataInfo.inBuf = (void *)&caliCfgPacket;
    dataInfo.inSize = 1;
    dataInfo.elemInSize = sizeof(struct alspsCaliCfgPacket);
    dataInfo.outBuf = NULL;
    dataInfo.outSize = 0;
    dataInfo.elemOutSize = 0;
    if (sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAR_CFG, &i2cCallback, &spiCallback) < 0)
        mTask.nowState = CHIP_IDLE;
}
*/
static bool sensorCfgSar(void *data, void *cookie)
{
   /* int32_t *values = data;

    osLog(LOG_INFO, "sensorCfgSar [high, low]: [%d, %d]\n", values[0], values[1]);
    if (mTask.nowState == CHIP_IDLE) {
        setPsHwCaliCfg(values);
    } else {
        osLog(LOG_INFO, "sensorCfgSar pending by:%d\n", mTask.nowState);
        mTask.pendingCaliCfg = true;
        mTask.mSensorPerUnit.pendCaliCfg[0] = values[0];
        mTask.mSensorPerUnit.pendCaliCfg[1] = values[1];
    }*/

    return true;
}

static bool allocateDataEvt(uint64_t timeStamp)
{
    mTask.data_evt = slabAllocatorAlloc(mTask.mDataSlab);

    if (mTask.data_evt == NULL) {
        osLog(LOG_ERROR, "sar: Slab allocation failed\n");
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
        if (!osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_SAR),
                mTask.data_evt, dataEvtFree)) {
            // don't log error since queue is already full. silently drop
            // this data event.
            dataEvtFree(mTask.data_evt);
        }
        mTask.data_evt = NULL;
    }
}

static void parseRawData(struct SarData_t *data, uint64_t timeStamp)
{
    struct TripleAxisDataPoint *sample;
    uint32_t delta_time = 0;

    if (mTask.data_evt == NULL) {
        if (!allocateDataEvt(timeStamp))
            return;
    }
    if (mTask.data_evt->samples[0].firstSample.numSamples >= MAX_NUM_COMMS_EVENT_SAMPLES) {
        osLog(LOG_ERROR, "sar parseRawData BAD INDEX\n");
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

    sample->ix = data->sarData[0];
    sample->iy = data->sarData[1];
    sample->iz = data->sarData[2];

    flushRawData();
}

static bool sendLastSampleSar(void *cookie, uint32_t tid)
{
    if (mTask.last_sample.sarState != SAR_STATE_INIT) {
        parseRawData(&mTask.last_sample, rtcGetTime());
    }
    return true;
}

static const struct SensorInfo sensorInfoSar = {
    .sensorName = "sx9310",
    .supportedRates = sarSupportedRates,
    .sensorType = SENS_TYPE_SAR,
    .numAxis = NUM_AXIS_THREE,
    .interrupt = NANOHUB_INT_WAKEUP,
    .minSamples = 300
};

static const struct SensorOps sensorOpsSar = {
    .sensorPower = sensorPowerSar,
    .sensorFirmwareUpload = sensorFirmwareSar,
    .sensorSetRate = sensorRateSar,
    .sensorFlush = sensorFlushSar,
    .sensorSendOneDirectEvt = sendLastSampleSar,
    .sensorCalibrate = sensorCaliSar,
    .sensorCfgData = sensorCfgSar,
};


static void configEvent(struct sensPerUnit *mSensor, struct ConfigStat *ConfigData)
{
    if (ConfigData->enable == 0 && mSensor->nowOn)
        sensorOpsSar.sensorPower(false, NULL);
    else if (ConfigData->enable == 1 && !mSensor->nowOn)
        sensorOpsSar.sensorPower(true, NULL);
    else
        sensorOpsSar.sensorSetRate(ConfigData->rate, ConfigData->latency, NULL);
}

static void processPendingEvt(void)
{
    if (mTask.pendingSample) {
        mTask.pendingSample = false;
        osEnqueuePrivateEvt(EVT_SENSOR_TIMER, NULL, NULL, mTask.id);
        return;
    }
    /* pending event int first */
    if (mTask.pendingInt) {
        mTask.pendingInt = false;
        osEnqueuePrivateEvt(EVT_SENSOR_INTR, NULL, NULL, mTask.id);
        return;
    }
    /* pending event control third */
    if (mTask.pendingConfig) {
        mTask.pendingConfig = false;
        configEvent(&mTask.mSensorPerUnit, &mTask.mSensorPerUnit.pendConfig);
        return;
    }
    /* Process SAR pending evt */
    if (mTask.pendingCali) {
        mTask.pendingCali = false;
        sensorOpsSar.sensorCalibrate(NULL);
        return;
    }
}

static void handleSensorEvent(const void *state)
{
    enum SarFsmState mState = (enum SarFsmState)state;
    uint32_t timerDelay = 0;

    switch (mState) {
        default: {
            sensorFsmSwitchState(NULL, &mTask.fsm, &i2cCallback, &spiCallback);
            break;
        }
        case CHIP_IDLE: {
            //osLog(LOG_INFO, "sar: STATE_IDLE done\n");
            break;
        }

        case CHIP_INIT_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            if (mTask.mSensorPerUnit.handle)
                sensorRegisterInitComplete(mTask.mSensorPerUnit.handle);

            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "sar: init done\n");
            processPendingEvt();
            break;
        }

        case CHIP_SAR_ENABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit.handle,
                                    SENSOR_INTERNAL_EVT_POWER_STATE_CHG, true, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "sar: enable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_SAR_DISABLE_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            sensorSignalInternalEvt(mTask.mSensorPerUnit.handle,
                                    SENSOR_INTERNAL_EVT_POWER_STATE_CHG, false, 0);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "sar: disable done\n");
            processPendingEvt();
            break;
        }

        case CHIP_SAR_CALI_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "sar: cali done\n");
            processPendingEvt();
            break;
        }

        case CHIP_SAR_RATECHG_DONE: {
            sensorFsmReleaseState(&mTask.fsm);
             if (mTask.intrMode == SAR_POLLING_MODE) {
                /* do something, turn on the sampling timer */
                timerDelay = sensorTimerLookupCommon(sarSupportedRates, rateTimerValsSar,
                                                     mTask.mSensorPerUnit.timerDelay);

                if (mTask.timerHandle)
                    timTimerCancel(mTask.timerHandle);

                mTask.timerHandle = timTimerSet(timerDelay, 0, 50, sarTimerCallback, NULL, false);
                if (mTask.timerHandle == 0)
                    configASSERT(0);
             }

            sensorSignalInternalEvt(mTask.mSensorPerUnit.handle,
                SENSOR_INTERNAL_EVT_RATE_CHG, mTask.mSensorPerUnit.rate, mTask.mSensorPerUnit.latency);
            mTask.nowState = CHIP_IDLE;
            osLog(LOG_INFO, "sar: ratechg done timerDelay:%dns\n", timerDelay);
            processPendingEvt();
            break;
        }

        case CHIP_SAR_SAMPLING_DONE: {
            sensorFsmReleaseState(&mTask.fsm);

            if (mTask.mSensorPerUnit.nowOn && mTask.mSensorPerUnit.isReading) {
                if (mTask.last_sample.sarState != mTask.current_sample->sarState) {
                    mTask.last_sample.sarState = mTask.current_sample->sarState;

                    mTask.last_sample.sarData[0] = mTask.current_sample->sarData[0];
                    mTask.last_sample.sarData[1] = mTask.current_sample->sarData[1];
                    mTask.last_sample.sarData[2] = mTask.current_sample->sarData[2];
                    parseRawData(mTask.current_sample, rtcGetTime());
                }
            }

            mTask.mSensorPerUnit.isReading = false;
            mTask.nowState = CHIP_IDLE;
            processPendingEvt();
            //osLog(LOG_INFO, "mTask.isReading:%d\n", mTask.isReading);
            break;
        }
    }
}


static void handleEvent(uint32_t evtType, const void *evtData)
{
    struct transferDataInfo dataInfo;
    //osLog(LOG_INFO, "evtType :%d evtdata:%d\n", evtType, (enum SarFsmState)evtData);

    switch (evtType) {
        case EVT_APP_START: {
            if (mTask.fsm.mSensorFsm != NULL) {
                osLog(LOG_INFO, "sar: app start\n");
                /* Reset chip */
                dataInfo.inBuf = NULL;
                dataInfo.inSize = 0;
                dataInfo.elemInSize = 0;
                dataInfo.outBuf = NULL;
                dataInfo.outSize = NULL;
                dataInfo.elemOutSize = NULL;
                sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_RESET, &i2cCallback, &spiCallback);
            } else
                osLog(LOG_INFO, "sar: wait for auto detect\n");
            break;
        }

        case EVT_SENSOR_EVENT: {
            handleSensorEvent(evtData);
            break;
        }

        case EVT_SENSOR_TIMER: {
            /* Start sampling for a value */
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.mSensorPerUnit.isReading) {
                    mTask.nowState = CHIP_SAR_SAMPLING;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.current_sample;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAR_SAMPLING, &i2cCallback, &spiCallback);
                    mTask.mSensorPerUnit.isReading = true;
                }
            } else {
                osLog(LOG_INFO, "sar EST pending by nowState:%d\n", mTask.nowState);
                mTask.pendingSample = true;
            }
            break;
        }

        case EVT_SENSOR_INTR: {
        /* Start sampling for a value */
            if (mTask.nowState == CHIP_IDLE) {
                if (!mTask.mSensorPerUnit.isReading) {
                    mTask.nowState = CHIP_SAR_SAMPLING;
                    dataInfo.inBuf = NULL;
                    dataInfo.inSize = 0;
                    dataInfo.elemInSize = 0;
                    dataInfo.outBuf = &mTask.current_sample;
                    dataInfo.outSize = &mTask.outSize;
                    dataInfo.elemOutSize = &mTask.elemOutSize;
                    sensorFsmRunState(&dataInfo, &mTask.fsm, (const void *)CHIP_SAR_SAMPLING, &i2cCallback, &spiCallback);
                    mTask.mSensorPerUnit.isReading = true;
                }
            } else {
                osLog(LOG_INFO, "sar EST pending by nowState:%d\n", mTask.nowState);
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

void registerSarInterruptMode(uint8_t mode)
{
    mTask.intrMode = mode;
}

void registerSarDriverFsm(struct sensorFsm *mSensorFsm, uint8_t size)
{
    mTask.fsm.moduleName = "sar";
    mTask.fsm.mSensorFsm = mSensorFsm;
    mTask.fsm.mSensorFsmSize = size;
    osEnqueuePrivateEvt(EVT_APP_START, NULL, NULL, mTask.id);
}
void sarSensorRegister(void)
{
    uint32_t handle = 0;

    if (sensorFind(SENS_TYPE_SAR, 0, &handle) != NULL)
        sensorUnregister(handle);
    mTask.mSensorPerUnit.handle = sensorRegister(&sensorInfoSar, &sensorOpsSar, NULL, false);
}

void sarSendCalibrationResult(uint8_t sensorType,
        int32_t bias[3], int8_t status) {
    struct CalibrationData *data = heapAlloc(sizeof(struct CalibrationData));
    if (!data) {
        osLog(LOG_ERROR, "Couldn't alloc data cali result pkt");
        return;
    }

    data->header.appId = APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_SAR, 0, 0));
    data->header.dataLen = (sizeof(struct CalibrationData) - sizeof(struct HostHubRawPacket));
    data->data_header.msgId = SENSOR_APP_MSG_ID_CAL_RESULT;
    data->data_header.sensorType = sensorType;
    data->data_header.status = (uint8_t)status;

    data->values[0] = bias[0];
    data->values[1] = bias[1];
    data->values[2] = bias[2];

    if (!osEnqueueEvtOrFree(EVT_APP_TO_HOST, data, heapFree))
        osLog(LOG_ERROR, "Couldn't send data cali result evt");
}

static bool startTask(uint32_t taskId)
{
    size_t slabSize;

    slabSize = sizeof(struct TripleAxisDataEvent) +
               MAX_NUM_COMMS_EVENT_SAMPLES * sizeof(struct TripleAxisDataPoint);

    mTask.mDataSlab = slabAllocatorNew(slabSize, 4, 2);
    if (!mTask.mDataSlab) {
        osLog(LOG_INFO, "Slab allocation failed\n");
        return false;
    }

    mTask.data_evt = NULL;

    sarDebugPoint = &mTask;
    /* Register tid first */
    mTask.id = taskId;

#ifndef CFG_OVERLAY_INIT_SUPPORT
        /* Register sensor fsm last */
        module_init(SENS_TYPE_SAR);
#else
        extern void sarOverlayRemap(void);
        sarOverlayRemap();
#endif

    return true;
}

static void endTask(void)
{
    sensorUnregister(mTask.mSensorPerUnit.handle);
}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_SAR, 0, 0)),
    0, startTask, endTask, handleEvent);
