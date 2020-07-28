#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <plat/inc/rtc.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <seos.h>
// #include <accelerometer.h>
#include <nanohub_math.h>
#include <sensors.h>
#include <limits.h>

#include "algoConfig.h"
#include "FreeRTOS.h"
#include "floor_count.h"
#include "pedometer.h"

/* Math defines */
#define EVT_SENSOR_STEP         sensorGetMyEventType(SENS_TYPE_STEP_COUNT)
#define EVT_SENSOR_BARO         sensorGetMyEventType(SENS_TYPE_BARO)
#define EVT_SENSOR_FLOOR_COUNT  sensorGetMyEventType(SENS_TYPE_FLOOR_COUNT)
#define EVT_SENSOR_ANY_MOTION   sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#define EVT_SENSOR_NO_MOTION    sensorGetMyEventType(SENS_TYPE_NO_MOTION)
#define FC_BARO_INPUT_INTERVAL          1000 //ms

#define FC_DEBUG_LOG

// ----------------------------------------------
// 1. On Change for SENS_TYPE_FLOOR_COUNT
// 2. Use step_count_for_floor_counting() for faster step count result
// ----------------------------------------------

struct FcArgument {
    uint32_t step_num;      // has latency to prevent backtrack
    uint32_t result;
    uint32_t prev_result;
};

struct FcTask {
    uint32_t taskId;
    uint32_t handle;
    uint32_t anyMotionHandle;
    uint32_t noMotionHandle;
    uint32_t stepHandle;
    uint32_t baroHandle;
    enum {
        STATE_DISABLED,
        STATE_AWAITING_ANY_MOTION,
        STATE_AWAITING_FLOOR_COUNT,
        STATE_INIT
    } taskState;
};

static resampling_t mResample_baro;
static struct FcArgument mArg;
static struct FcTask mTask;

static void algoInit()
{
    memset(&mArg, 0, sizeof(struct FcArgument));
    init_data_resampling(&mResample_baro, FC_BARO_INPUT_INTERVAL);
    init_floor_counting();
}

static bool algoUpdate(uint32_t evtType, const void* evtData)
{
    if (evtType == EVT_SENSOR_BARO) {
        uint32_t input_time_stamp_ms = rtcGetTime() / TIME_NANO_TO_MILLI;
        union EmbeddedDataPoint ev;
        ev.idata = (uint32_t)evtData;

        sensor_input_data_resampling_preparation(&mResample_baro, input_time_stamp_ms, evtType);
        mResample_baro.currentBaroData = (int32_t)(ev.fdata * 100);
        while (mResample_baro.input_count > 0) {
            algo_input_data_generation(&mResample_baro, evtType);
            // osLog(LOG_DEBUG, "[Floor_Count] Update baro to %d, step count to %u\n", mResample_baro.algoBaroData, mArg.step_num);
            run_floor_counting((uint64_t)mResample_baro.algo_time_stamp * TIME_NANO_TO_MILLI, mResample_baro.algoBaroData,
                               step_count_for_floor_counting());
            mResample_baro.input_count--;
        }
        mArg.prev_result = mArg.result;
        mArg.result = get_floor_counting_result();
    } else if (evtType == EVT_SENSOR_STEP) {
        union EmbeddedDataPoint ev;
        ev.idata = (uint32_t)evtData;
        mArg.step_num = (uint32_t)ev.idata;
    }

    if (mArg.prev_result != mArg.result) return 1;
    return 0;
}

static void configStep(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.stepHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_STEP_COUNT, i, &mTask.stepHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.stepHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osEventSubscribe(mTask.taskId, EVT_SENSOR_STEP);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_STEP_COUNT, i, &mTask.stepHandle) == NULL) {
#ifdef FC_DEBUG_LOG
            osLog(LOG_DEBUG, "[Floor_Count] Step sensor is not available\n");
#endif
        }
    } else if ((on == false) && (mTask.stepHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.stepHandle);
        mTask.stepHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_STEP);
    }
}

static void configBaro(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.baroHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_BARO, i, &mTask.baroHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.baroHandle, BARO_MIN_RATE, BARO_LATENCY)) {
                osEventSubscribe(mTask.taskId, EVT_SENSOR_BARO);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_BARO, i, &mTask.baroHandle) == NULL) {
#ifdef FC_DEBUG_LOG
            osLog(LOG_DEBUG, "[Floor_Count] Baro sensor is not available\n");
#endif
        }
    } else if ((on == false) && (mTask.baroHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.baroHandle);
        mTask.baroHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_BARO);
    }
}

static void configAnyMotion(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.anyMotionHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_ANY_MOTION, i, &mTask.anyMotionHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.anyMotionHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osEventSubscribe(mTask.taskId, EVT_SENSOR_ANY_MOTION);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_ANY_MOTION, i, &mTask.anyMotionHandle) == NULL) {
#ifdef FC_DEBUG_LOG
            osLog(LOG_DEBUG, "[Floor_Count] anyMotion sensor is not available\n");
#endif
        }
    } else if ((on == false) && (mTask.anyMotionHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.anyMotionHandle);
        mTask.anyMotionHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ANY_MOTION);
    }
}

static void configNoMotion(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.noMotionHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_NO_MOTION, i, &mTask.noMotionHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.noMotionHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osEventSubscribe(mTask.taskId, EVT_SENSOR_NO_MOTION);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_NO_MOTION, i, &mTask.noMotionHandle) == NULL) {
#ifdef FC_DEBUG_LOG
            osLog(LOG_DEBUG, "[Floor_Count] noMotion sensor is not available\n");
#endif
        }
    } else if ((on == false) && (mTask.noMotionHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.noMotionHandle);
        mTask.noMotionHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_NO_MOTION);
    }
}

static bool fcPower(bool on, void *cookie)
{
#ifdef FC_DEBUG_LOG
    osLog(LOG_INFO, "[Floor_Count] Active floor_count(%d)\n", on);
#endif
    if (on) {
        algoInit();
        configStep(true);
        configBaro(true);
        configAnyMotion(true);
        configNoMotion(true);
        mTask.taskState = STATE_INIT;
    } else {
        configAnyMotion(false);
        configNoMotion(false);
        configBaro(false);
        configStep(false);
        mTask.taskState = STATE_DISABLED;
    }
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool fcSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool fcFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask .handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool fcFlush(void *cookie)
{
    return osEnqueueEvt(EVT_SENSOR_FLOOR_COUNT, SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void fcHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    switch (evtType) {
        case EVT_APP_START:
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            break;
        case EVT_SENSOR_ANY_MOTION:
            if ((mTask.taskState == STATE_AWAITING_ANY_MOTION) || (mTask.taskState == STATE_INIT)) {
                configNoMotion(true);
                configBaro(true);
                configAnyMotion(false);
                mTask.taskState = STATE_AWAITING_FLOOR_COUNT;
            }
            break;
        case EVT_SENSOR_NO_MOTION:
            if ((mTask.taskState == STATE_AWAITING_FLOOR_COUNT) || (mTask.taskState == STATE_INIT)) {
                configAnyMotion(true);
                configBaro(false);
                configNoMotion(false);
                mTask.taskState = STATE_AWAITING_ANY_MOTION;
            }
            break;
        case EVT_SENSOR_BARO:
            if (algoUpdate(evtType, evtData)) {
                union EmbeddedDataPoint sample;
#ifdef FC_DEBUG_LOG
                osLog(LOG_INFO, "[Floor_Count] Report floor count to %u\n", mArg.result);
#endif
                sample.idata = (int) mArg.result;
                osEnqueueEvt(EVT_SENSOR_FLOOR_COUNT, sample.vptr, NULL);
            }
            break;
        case EVT_SENSOR_STEP:
            algoUpdate(evtType, evtData);
            break;
        default:
            break;
    }
}

static const struct SensorInfo mSi = {
    .sensorName = "Floor Count",
    .sensorType = SENS_TYPE_FLOOR_COUNT,
    .numAxis = NUM_AXIS_EMBEDDED,
    .interrupt = NANOHUB_INT_NONWAKEUP,
    .minSamples = 20
};

static const struct SensorOps mSops = {
    .sensorPower = fcPower,
    .sensorFirmwareUpload = fcFirmwareUpload,
    .sensorSetRate = fcSetRate,
    .sensorFlush = fcFlush
};

static bool fcStart(uint32_t taskId)
{
    mTask.taskId = taskId;
    mTask.handle = sensorRegister(&mSi, &mSops, NULL, true);
    algoInit();
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void fcEnd()
{

}

INTERNAL_APP_INIT(
    APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_FLOOR_COUNT, 0, 0)),
    0,
    fcStart,
    fcEnd,
    fcHandleEvent
);
