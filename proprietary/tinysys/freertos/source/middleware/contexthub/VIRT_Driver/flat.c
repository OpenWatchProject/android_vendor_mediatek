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
#include "math_method.h"
#include "FreeRTOS.h"

/* Math defines */
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define EVT_SENSOR_ACCEL         sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_FLAT          sensorGetMyEventType(SENS_TYPE_FLAT)
#define EVT_SENSOR_ANY_MOTION    sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#define EVT_SENSOR_NO_MOTION     sensorGetMyEventType(SENS_TYPE_NO_MOTION)

#define FLAT_ACC_MAX_VALUE      38220           // 9.8 * 1000 * 3.9
#define FLAT_STABLE_TIME        5000            // timer for getting stable result (unit: ms)
#define FLAT_DEGREE_TOLERANCE   10              // tolerance of degree for whether device is flat (unit: degree)
#define FLAT_R2D                (180/M_PI)

#define FLAT_STATE_UNKNOWN      0
#define FLAT_STATE_NOTFLAT      1
#define FLAT_STATE_FLAT         2

#define FLAT_DEBUG_LOG

// ----------------------------------------------
// 1. One-shot for SENS_TYPE_FLAT
// 2. Report first stable state
// 3. Only consider variance here, maybe should add norm ~= 1g ?
// ----------------------------------------------

struct FlatArgument {
    uint32_t curr_ms;               // current time
    uint32_t prev_ms;               // previous time
    uint32_t flat_state_ref_ms;     // last flat state changing time
    int8_t flat_state;              // flat state
    int8_t flat_first_state;        // first flat unstable state
    int8_t result;                  // final result state

    int8_t device_is_flat;
};

struct FlatTask {
    uint32_t taskId;
    uint32_t handle;
    uint32_t anyMotionHandle;
    uint32_t noMotionHandle;
    uint32_t accelHandle;
    uint32_t alsHandle;
    uint32_t proxHandle;
    enum {
        STATE_DISABLED,
        STATE_AWAITING_ANY_MOTION,
        STATE_AWAITING_FLAT,
        STATE_INIT
    } taskState;
};

static resampling_t mResample;
static struct FlatArgument mArg;
static struct FlatTask mTask;
static swMotion_t mMotion;

static void algoInit()
{
    memset(&mArg, 0, sizeof(struct FlatArgument));
    memset(&mMotion, false, sizeof(swMotion_t));
    init_data_resampling(&mResample, FLAT_INPUT_INTERVAL);
    mArg.device_is_flat = -1;
}

static int get_device_flat(int x, int y, int z)
{
    // check signal range
    x = (x > FLAT_ACC_MAX_VALUE) ? FLAT_ACC_MAX_VALUE : x;
    y = (y > FLAT_ACC_MAX_VALUE) ? FLAT_ACC_MAX_VALUE : y;
    z = (z > FLAT_ACC_MAX_VALUE) ? FLAT_ACC_MAX_VALUE : z;
    x = (x < -FLAT_ACC_MAX_VALUE) ? -FLAT_ACC_MAX_VALUE : x;
    y = (y < -FLAT_ACC_MAX_VALUE) ? -FLAT_ACC_MAX_VALUE : y;
    z = (z < -FLAT_ACC_MAX_VALUE) ? -FLAT_ACC_MAX_VALUE : z;
    x = x >> 8;
    y = y >> 8;
    z = z >> 8;

    int mean = dsp_fixed_sqrt(x * x + y * y + z * z);
    int degree = (mean != 0) ? (int)(asinf((float)z / mean) * FLAT_R2D) : 0;
    if (abs_value(degree) < (90 + FLAT_DEGREE_TOLERANCE) && abs_value(degree) >= (90 - FLAT_DEGREE_TOLERANCE)) {
        return 1;
    } else {
        return 0;
    }
}

static int get_flat_state()
{
    if (mArg.device_is_flat) {
        return FLAT_STATE_FLAT;
    } else {
        return FLAT_STATE_NOTFLAT;
    }
}

static bool algoUpdate(struct TripleAxisDataEvent *ev, uint32_t evtType)
{
    struct TripleAxisDataPoint *sample;
    uint64_t input_time_stamp_ms = ev->referenceTime / TIME_NANO_TO_MILLI;
    uint32_t count = ev->samples[0].firstSample.numSamples;
    uint32_t i;

    // Initial
    mArg.result = FLAT_STATE_UNKNOWN;
    for (i = 0; i < count; i++) {
        sample = &ev->samples[i];
        if (i > 0) {
            input_time_stamp_ms += sample->deltaTime / TIME_NANO_TO_MILLI;
        }

        sensor_input_data_resampling_preparation(&mResample, input_time_stamp_ms, EVT_SENSOR_ACCEL);
        mResample.currentAccData = (AccelData_t) {
            (int32_t)(sample->x * 1000), (int32_t)(sample->y * 1000), (int32_t)(sample->z * 1000)
        };
        while (mResample.input_count > 0) {
            algo_input_data_generation(&mResample, EVT_SENSOR_ACCEL);
            mArg.curr_ms = mResample.algo_time_stamp;

            // If timestamp interval exceed large gap
            if (mArg.prev_ms && (mArg.curr_ms - mArg.prev_ms) >= INTERPOLATION_UPPER_LIMIT * FLAT_INPUT_INTERVAL) {
                mArg.flat_state_ref_ms += (mArg.curr_ms - mArg.prev_ms);
            }

            mArg.device_is_flat = get_device_flat(mResample.algoAccData.x, mResample.algoAccData.y, mResample.algoAccData.z);
            int current_state = get_flat_state();
            if (mArg.flat_state == FLAT_STATE_UNKNOWN) {
                // [Note] One-shot sensor should report first stable state
                if ((mArg.flat_first_state == FLAT_STATE_UNKNOWN) || (mArg.flat_first_state != current_state)) {
                    mArg.flat_first_state = current_state;
                    mArg.flat_state_ref_ms = mResample.algo_time_stamp;
                } else if (mResample.algo_time_stamp - mArg.flat_state_ref_ms >= FLAT_STABLE_TIME) {
                    mArg.flat_state = mArg.flat_first_state;
                    mArg.flat_state_ref_ms = mResample.algo_time_stamp;
                    mArg.result = mArg.flat_state;
                }
            } else if (mArg.flat_state == current_state) {
                mArg.flat_state_ref_ms = mResample.algo_time_stamp;
            } else if (mResample.algo_time_stamp - mArg.flat_state_ref_ms >= FLAT_STABLE_TIME) {
                mArg.flat_state = (mArg.flat_state == FLAT_STATE_NOTFLAT) ? FLAT_STATE_FLAT : FLAT_STATE_NOTFLAT;
                mArg.flat_state_ref_ms = mResample.algo_time_stamp;
                mArg.result = mArg.flat_state;
            }
            mArg.prev_ms = mArg.curr_ms;
            mResample.input_count--;
        }
    }
    return mArg.result;
}

static void configAnyMotion(bool on)
{
    uint32_t i;
    const struct SensorInfo *getAnyMotionSrc;
    if (on && (!mTask.anyMotionHandle)) {
        for (i = 0; (getAnyMotionSrc = sensorFind(SENS_TYPE_ANY_MOTION, i, &mTask.anyMotionHandle))
                && (getAnyMotionSrc != NULL); i++) {
            if (sensorRequest(mTask.taskId, mTask.anyMotionHandle, SENSOR_RATE_ONCHANGE, 0)) {
                mMotion.apply_sw_anymotion = !(strcmp(getAnyMotionSrc->sensorName, (char*)SW_AnyMotion));
                osEventSubscribe(mTask.taskId, EVT_SENSOR_ANY_MOTION);
                break;
            }
        }
        if (!getAnyMotionSrc) {
#ifdef FLAT_DEBUG_LOG
            osLog(LOG_DEBUG, "[Flat] anyMotion sensor is not available\n");
#endif
        }
    } else if ((!on) && mTask.anyMotionHandle) {
        sensorRelease(mTask.taskId, mTask.anyMotionHandle);
        mTask.anyMotionHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ANY_MOTION);
    }
}

static void configNoMotion(bool on)
{
    uint32_t i;
    const struct SensorInfo *getNoMotionSrc;
    if (on && (!mTask.noMotionHandle)) {
        for (i = 0; (getNoMotionSrc = sensorFind(SENS_TYPE_NO_MOTION, i, &mTask.noMotionHandle))
                && (getNoMotionSrc != NULL); i++) {
            if (sensorRequest(mTask.taskId, mTask.noMotionHandle, SENSOR_RATE_ONCHANGE, 0)) {
                mMotion.apply_sw_nomotion = !(strcmp(getNoMotionSrc->sensorName, (char*)SW_NoMotion));
                osEventSubscribe(mTask.taskId, EVT_SENSOR_NO_MOTION);
                break;
            }
        }
        if (!getNoMotionSrc) {
#ifdef FLAT_DEBUG_LOG
            osLog(LOG_DEBUG, "[Flat] noMotion sensor is not available\n");
#endif
        }
    } else if ((!on) && mTask.noMotionHandle) {
        sensorRelease(mTask.taskId, mTask.noMotionHandle);
        mTask.noMotionHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_NO_MOTION);
    }
}

static void configAccel(bool on, uint32_t acc_class)
{
    uint32_t i;
    bool getAccelSensor = false;
    if (on && (!mTask.accelHandle)) {
        for (i = 0; sensorFind(SENS_TYPE_ACCEL, i, &mTask.accelHandle) != NULL; i++) {
            if (acc_class == NORMAL_ACC_WINDOW) {
                if (sensorRequest(mTask.taskId, mTask.accelHandle, ACCEL_MIN_RATE, ACCEL_NORMAL_LATENCY)) {
                    getAccelSensor = true;
                    osEventSubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
                    break;
                }
            } else {
                if (sensorRequest(mTask.taskId, mTask.accelHandle, ACCEL_MIN_RATE, ACCEL_EXTENDED_LATENCY)) {
                    getAccelSensor = true;
                    osEventSubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
                    break;
                }
            }
        }
        if (!getAccelSensor) {
#ifdef FLAT_DEBUG_LOG
            osLog(LOG_DEBUG, "[Flat] Accel sensor is not available\n");
#endif
        }
    } else if ((!on) && mTask.accelHandle) {
        sensorRelease(mTask.taskId, mTask.accelHandle);
        mTask.accelHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
    }
}
static bool flatPower(bool on, void *cookie)
{
#ifdef FLAT_DEBUG_LOG
    osLog(LOG_INFO, "[Flat] Active flat(%d)\n", on);
#endif
    if (on) {
        algoInit();
        configAccel(true, NORMAL_ACC_WINDOW);
        configAnyMotion(true);
        configNoMotion(true);
        mTask.taskState = STATE_INIT;

    } else {
        configAnyMotion(false);
        configNoMotion(false);
        configAccel(false, NORMAL_ACC_WINDOW);
        configAccel(false, EXTENDED_ACC_WINDOW);
        mTask.taskState = STATE_DISABLED;
    }
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool flatSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool flatFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool flatFlush(void *cookie)
{
    return osEnqueueEvt(EVT_SENSOR_FLAT, SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void flatHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }
    switch (evtType) {
        case EVT_APP_START:
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            osLog(LOG_DEBUG, "FLAT EVT_APP_START\n");
            break;
        case EVT_SENSOR_ANY_MOTION:
            if ((mTask.taskState == STATE_AWAITING_ANY_MOTION)  || (mTask.taskState == STATE_INIT)) {
                configNoMotion(true);
                configAnyMotion(false);
                if (mMotion.apply_sw_anymotion) {
                    configAccel(false, EXTENDED_ACC_WINDOW);
                }
                configAccel(true, NORMAL_ACC_WINDOW);
                mTask.taskState = STATE_AWAITING_FLAT;
            }
            break;
        case EVT_SENSOR_NO_MOTION:
            if ((mTask.taskState == STATE_AWAITING_FLAT)  || (mTask.taskState == STATE_INIT)) {
                configAnyMotion(true);
                configNoMotion(false);
                configAccel(false, NORMAL_ACC_WINDOW);
                if (mMotion.apply_sw_anymotion) {
                    configAccel(true, EXTENDED_ACC_WINDOW);
                }
                mTask.taskState = STATE_AWAITING_ANY_MOTION;
            }
            break;
        case EVT_SENSOR_ACCEL:
            if (algoUpdate((struct TripleAxisDataEvent *)evtData, evtType)) {
                union EmbeddedDataPoint sample;
                sample.idata = 1;

                if (mArg.result == FLAT_STATE_NOTFLAT) {
#ifdef FLAT_DEBUG_LOG
                    osLog(LOG_DEBUG, "[Flat] changed state to not_flat\n");
#endif
                } else if (mArg.result == FLAT_STATE_FLAT) {
#ifdef FLAT_DEBUG_LOG
                    osLog(LOG_DEBUG, "[Flat] changed state to flat\n");
#endif
                    osEnqueueEvt(EVT_SENSOR_FLAT, sample.vptr, NULL);
                }
            }
            break;
    }
}

static const struct SensorInfo mSi = {
    .sensorName = "Flat",
    .sensorType = SENS_TYPE_FLAT,
    .numAxis = NUM_AXIS_EMBEDDED,
    .interrupt = NANOHUB_INT_WAKEUP,
    .minSamples = 20
};

static const struct SensorOps mSops = {
    .sensorPower = flatPower,
    .sensorFirmwareUpload = flatFirmwareUpload,
    .sensorSetRate = flatSetRate,
    .sensorFlush = flatFlush
};

static bool flatStart(uint32_t taskId)
{
    mTask.taskId = taskId;
    mTask.handle = sensorRegister(&mSi, &mSops, NULL, true);
    algoInit();
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void flatEnd()
{

}

INTERNAL_APP_INIT(
    APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_FLAT, 0, 0)),
    0,
    flatStart,
    flatEnd,
    flatHandleEvent
);
