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

#include <seos.h>
#include <i2c.h>
#include <timer.h>
#include <sensors.h>
#include <heap.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <eventnums.h>

#define DRIVER_NAME                            "FAC: "

#if 0
#define AMS_TMD2772_ATIME_SETTING              0xdb
#define AMS_TMD2772_ATIME_MS                   ((256 - AMS_TMD2772_ATIME_SETTING) * 2.73) // in milliseconds
#define AMS_TMD2772_PTIME_SETTING              0xff
#define AMS_TMD2772_PTIME_MS                   ((256 - AMS_TMD2772_PTIME_SETTING) * 2.73) // in milliseconds
#define AMS_TMD2772_WTIME_SETTING_ALS_ON       0xdd // (256 - 221) * 2.73 ms = 95.55 ms
#define AMS_TMD2772_WTIME_SETTING_ALS_OFF      0xb8 // (256 - 184) * 2.73 ms = 196.56 ms
#define AMS_TMD2772_PPULSE_SETTING             8

#define AMS_TMD2772_CAL_DEFAULT_OFFSET         0
#define AMS_TMD2772_CAL_MAX_OFFSET             500

/* AMS_TMD2772_REG_ENABLE */
#define POWER_ON_BIT                           (1 << 0)
#define ALS_ENABLE_BIT                         (1 << 1)
#define PROX_ENABLE_BIT                        (1 << 2)
#define WAIT_ENABLE_BIT                        (1 << 3)

/* AMS_TMD2772_REG_STATUS */
#define PROX_INT_BIT                           (1 << 5)
#define ALS_INT_BIT                            (1 << 4)

#define AMS_TMD2772_REPORT_NEAR_VALUE          0.0f // centimeters
#define AMS_TMD2772_REPORT_FAR_VALUE           5.0f // centimeters

#define AMS_TMD2772_THRESHOLD_ASSERT_NEAR      213  // in PS units
#define AMS_TMD2772_THRESHOLD_DEASSERT_NEAR    96   // in PS units

#define AMS_TMD2772_ALS_MAX_CHANNEL_COUNT      37888 // in raw data
#define AMS_TMD2772_ALS_MAX_REPORT_VALUE       10000 // in lux

#define AMS_TMD2772_ALS_INVALID                UINT32_MAX

/* Used when SENSOR_RATE_ONCHANGE is requested */
#define AMS_TMD2772_DEFAULT_RATE               SENSOR_HZ(5)
#endif
/* Private driver events */
enum FakeSensorEvents
{
    EVT_SENSOR_TIMER = EVT_APP_START + 1,
    EVT_SENSOR_PRIVATE_EVT
};

/* I2C state machine */
enum SensorState
{
    SENSOR_STATE_VERIFY_ID,
    SENSOR_STATE_INIT,

    SENSOR_STATE_CALIBRATE_RESET,
    SENSOR_STATE_CALIBRATE_START,
    SENSOR_STATE_CALIBRATE_ENABLING,
    SENSOR_STATE_CALIBRATE_POLLING_STATUS,
    SENSOR_STATE_CALIBRATE_AWAITING_SAMPLE,
    SENSOR_STATE_CALIBRATE_DISABLING,

    SENSOR_STATE_ENABLING_ALS,
    SENSOR_STATE_ENABLING_PROX,
    SENSOR_STATE_DISABLING_ALS,
    SENSOR_STATE_DISABLING_PROX,

    SENSOR_STATE_IDLE,
    SENSOR_STATE_SAMPLING,
};

enum ProxState
{
    PROX_STATE_INIT,
    PROX_STATE_NEAR,
    PROX_STATE_FAR,
};

struct FakeSensorData
{
    union {
        uint8_t bytes[16];
        struct {
            uint8_t status;
            uint16_t als[2];
            uint16_t prox;
        } __attribute__((packed)) sample;
        struct {
            uint16_t prox;
        } calibration;
    } txrxBuf;

    uint32_t tid;

    uint32_t alsHandle;
    uint32_t TimerHandle;

    union EmbeddedDataPoint lastAlsSample;

    uint8_t calibrationSampleCount;
    uint8_t proxState; // enum ProxState

    bool alsOn;
    bool alsReading;
    //bool proxOn;
    //bool proxReading;
};

static struct FakeSensorData mData;

/* TODO: check rates are supported */
static const uint32_t supportedRates[] =
{
    SENSOR_HZ(0.1),
    SENSOR_HZ(1),
    SENSOR_HZ(4),
    SENSOR_HZ(5),
    SENSOR_RATE_ONCHANGE,
    0
};

static const uint64_t rateTimerVals[] = //should match "supported rates in length" and be the timer length for that rate in nanosecs
{
    10 * 1000000000ULL,
     1 * 1000000000ULL,
    1000000000ULL / 4,
    1000000000ULL / 5,
};

/*
 * Helper functions
 */

static bool sensorPowerAls(bool on, void *cookie)
{
    osLog(LOG_INFO, "sensorPowerAls: %d\n", on);

    return true;
}

static bool sensorFirmwareAls(void *cookie)
{
    osLog(LOG_INFO, "%s\n", __func__);
    sensorSignalInternalEvt(mData.alsHandle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool sensorRateAls(uint32_t rate, uint64_t latency, void *cookie)
{

    osLog(LOG_INFO, "sensorRateAls: %ld/%lld\n", rate, latency);
#if 0
    if (mData.TimerHandle)
        timTimerCancel(mData.TimerHandle);
    mData.TimerHandle = timTimerSet(sensorTimerLookupCommon(supportedRates, rateTimerVals, rate), 0, 50, alsTimerCallback, NULL, false);
    osEnqueuePrivateEvt(EVT_SENSOR_ALS_TIMER, NULL, NULL, mData.tid);
    sensorSignalInternalEvt(mData.alsHandle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
#endif
    return true;
}

static bool sensorFlushAls(void *cookie)
{
    osLog(LOG_INFO, "%s\n", __func__);
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_ALS), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static bool sendLastSampleAls(void *cookie, uint32_t tid) {
    bool result = true;
    osLog(LOG_INFO, "%s\n", __func__);
    // If we don't end up doing anything here, the expectation is that we are powering up/haven't got the
    // first sample yet, so a broadcast event will go out soon with the first sample
   // if (mData.lastAlsSample.idata != AMS_TMD2772_ALS_INVALID) {
        result = osEnqueuePrivateEvt(sensorGetMyEventType(SENS_TYPE_ALS), mData.lastAlsSample.vptr, NULL, tid);
   // }
    return result;
}

static const struct SensorInfo sensorInfoAls =
{
    .sensorName = "ALS",
    .supportedRates = supportedRates,
    .sensorType = SENS_TYPE_ALS,
    .numAxis = NUM_AXIS_EMBEDDED,
    .interrupt = NANOHUB_INT_NONWAKEUP,
    .minSamples = 20
};

static const struct SensorOps sensorOpsAls =
{
    .sensorPower = sensorPowerAls,
    .sensorFirmwareUpload = sensorFirmwareAls,
    .sensorSetRate = sensorRateAls,
    .sensorFlush = sensorFlushAls,
    .sensorTriggerOndemand = NULL,
    .sensorCalibrate = NULL,
    .sensorSendOneDirectEvt = sendLastSampleAls
};

static void FakeTimerCallback(uint32_t timerId, void *cookie)
{
    osLog(LOG_INFO, "FakeTimerCallback:cookie:%lx\n", (uint32_t)cookie);
    osEnqueuePrivateEvt(EVT_SENSOR_TIMER, cookie, NULL, mData.tid);
}

static int count = 0;
static void handle_event(uint32_t evtType, const void* evtData)
{
    switch (evtType) {
    case EVT_APP_START:
        osLog(LOG_INFO, "%s EVT_APP_START\n", __func__);
        osEventUnsubscribe(mData.tid, EVT_APP_START);
        mData.TimerHandle = timTimerSet(2000000/*ns*/, 0, 50, FakeTimerCallback, &count, true);
        osEnqueuePrivateEvt(EVT_SENSOR_PRIVATE_EVT, NULL, NULL, mData.tid);
        break;

    case EVT_SENSOR_TIMER:
        count++;
        osLog(LOG_INFO, "%s EVT_SENSOR_TIMER %d\n", __func__, *(int *)evtData);
        break;

    case EVT_SENSOR_PRIVATE_EVT:
        osLog(LOG_INFO, "%s EVT_SENSOR_PRIVATE_EVT\n", __func__);
        if (mData.TimerHandle){	//cancel test
            osLog(LOG_INFO, "%s Timer cancel test ok\n", __func__);
            timTimerCancel(mData.TimerHandle);
        }
        mData.TimerHandle = timTimerSet(10000000000/*ns*/, 0, 50, FakeTimerCallback, &count, false);
        break;
    }
}

/*
 * Main driver entry points
 */

static bool init_app(uint32_t myTid)
{
    osLog(LOG_INFO, DRIVER_NAME "Sample Sensor starting\n");

    /* Set up driver private data */
    mData.tid = myTid;
    mData.alsOn = false;
    mData.alsReading = false;
    //mData.proxOn = false;
    //mData.proxReading = false;
//    mData.lastAlsSample.idata = AMS_TMD2772_ALS_INVALID;
//    mData.proxState = PROX_STATE_INIT;

    /* Register sensors */
    mData.alsHandle = sensorRegister(&sensorInfoAls, &sensorOpsAls, NULL, true);
//    mData.proxHandle = sensorRegister(&sensorInfoProx, &sensorOpsProx, NULL, false);

    osEventSubscribe(myTid, EVT_APP_START);

    return true;
}

static void end_app(void)
{
    sensorUnregister(mData.alsHandle);
}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ACCEL, 0, 0)), 0, init_app, end_app, handle_event);
