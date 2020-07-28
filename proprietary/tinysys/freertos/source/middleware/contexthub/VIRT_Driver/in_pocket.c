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

#define EVT_SENSOR_ACCEL        sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_ALS          sensorGetMyEventType(SENS_TYPE_ALS)
#define EVT_SENSOR_PROX         sensorGetMyEventType(SENS_TYPE_PROX)
#define EVT_SENSOR_INPK         sensorGetMyEventType(SENS_TYPE_INPOCKET)
#define EVT_SENSOR_ANY_MOTION   sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#define EVT_SENSOR_NO_MOTION    sensorGetMyEventType(SENS_TYPE_NO_MOTION)
#define ALS_MIN_RATE            SENSOR_HZ(5)
#define ALS_MAX_LATENCY         0ull            // 0 ms

#define INPK_ACC_MAX_VALUE      38220           // 9.8 * 1000 * 3.9
#define INPK_STABLE_TIME        5000            // timer for getting stable result (unit: ms)
#define INPK_DEGREE_TOLERANCE   10              // tolerance of degree for whether device is flat (unit: degree)
#define INPK_ALS_TOLERANCE      6               // tolerance of als for whether device is shaded
#define INPK_R2D                (180/M_PI)

#define INPK_STATE_UNKNOWN      0
#define INPK_STATE_NOTINPK      1
#define INPK_STATE_INPK         2

#define INPK_DEBUG_LOG

// ----------------------------------------------
// 1. One-shot for SENS_TYPE_INPOCKET
// 2. Report first stable state
// 3. Need low-power mechanism
// ----------------------------------------------

struct InpkArgument {
    uint32_t curr_ms;               // current time
    uint32_t prev_ms;               // previous time
    uint32_t inpk_state_ref_ms;     // last inpk state changing time (unit: ms)
    int8_t inpk_state;              // inpk state
    int8_t inpk_first_state;        // first inpk unstable state
    int8_t result;                  // final result state

    int8_t device_is_flat;
    int8_t als_is_shaded;
    int8_t prox_is_shaded;
};

struct InpkTask {
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
        STATE_AWAITING_IN_POCKET,
        STATE_INIT
    } taskState;
};

static resampling_t mResample;
static struct InpkArgument mArg;
static struct InpkTask mTask;
static swMotion_t mMotion;

static void algoInit()
{
    memset(&mArg, 0, sizeof(struct InpkArgument));
    memset(&mMotion, false, sizeof(swMotion_t));
    init_data_resampling(&mResample, INPK_INPUT_INTERVAL);
    mArg.device_is_flat = mArg.als_is_shaded = mArg.prox_is_shaded = -1;
}

static int get_device_flat(int x, int y, int z)
{
    // check signal range
    x = (x > INPK_ACC_MAX_VALUE) ? INPK_ACC_MAX_VALUE : x;
    y = (y > INPK_ACC_MAX_VALUE) ? INPK_ACC_MAX_VALUE : y;
    z = (z > INPK_ACC_MAX_VALUE) ? INPK_ACC_MAX_VALUE : z;
    x = (x < -INPK_ACC_MAX_VALUE) ? -INPK_ACC_MAX_VALUE : x;
    y = (y < -INPK_ACC_MAX_VALUE) ? -INPK_ACC_MAX_VALUE : y;
    z = (z < -INPK_ACC_MAX_VALUE) ? -INPK_ACC_MAX_VALUE : z;
    x = x >> 8;
    y = y >> 8;
    z = z >> 8;

    int mean = dsp_fixed_sqrt(x * x + y * y + z * z);
    int degree = (mean != 0) ? (int)(asinf((float)z / mean) * INPK_R2D) : 0;
    if (abs_value(degree) < (90 + INPK_DEGREE_TOLERANCE) && abs_value(degree) >= (90 - INPK_DEGREE_TOLERANCE)) {
        return 1;
    } else {
        return 0;
    }
}

static int ready_to_change_state()
{
    return (mArg.device_is_flat != -1 && mArg.als_is_shaded != -1 && mArg.prox_is_shaded != -1);
}

static int get_in_pocket_state()
{
    if (!mArg.device_is_flat && mArg.als_is_shaded && mArg.prox_is_shaded) {
        return INPK_STATE_INPK;
    } else {
        return INPK_STATE_NOTINPK;
    }
}

static bool algoUpdate(struct TripleAxisDataEvent *ev, uint32_t evtType)
{
    struct TripleAxisDataPoint *sample;
    uint64_t input_time_stamp_ms = ev->referenceTime / TIME_NANO_TO_MILLI;
    uint32_t count = ev->samples[0].firstSample.numSamples;
    uint32_t i;

    // Initial
    mArg.result = INPK_STATE_UNKNOWN;
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
            if (mArg.prev_ms && (mArg.curr_ms - mArg.prev_ms) >= INTERPOLATION_UPPER_LIMIT * INPK_INPUT_INTERVAL) {
                mArg.inpk_state_ref_ms += (mArg.curr_ms - mArg.prev_ms);
            }

            // Check whether the data is all ready
            mArg.device_is_flat = get_device_flat(mResample.algoAccData.x, mResample.algoAccData.y, mResample.algoAccData.z);
            if (!ready_to_change_state()) {
                mArg.prev_ms = mArg.curr_ms;
                mResample.input_count--;
                continue;
            }

            int current_state = get_in_pocket_state();
            if (mArg.inpk_state == INPK_STATE_UNKNOWN) {
                // [Note] One-shot sensor should report first stable state
                if ((mArg.inpk_first_state == INPK_STATE_UNKNOWN) || (mArg.inpk_first_state != current_state)) {
                    mArg.inpk_first_state = current_state;
                    mArg.inpk_state_ref_ms = mResample.algo_time_stamp;
                } else if (mResample.algo_time_stamp - mArg.inpk_state_ref_ms >= INPK_STABLE_TIME) {
                    mArg.inpk_state = mArg.inpk_first_state;
                    mArg.inpk_state_ref_ms = mResample.algo_time_stamp;
                    mArg.result = mArg.inpk_state;
                }
            } else if (mArg.inpk_state == current_state) {
                mArg.inpk_state_ref_ms = mResample.algo_time_stamp;
            } else if (mResample.algo_time_stamp - mArg.inpk_state_ref_ms >= INPK_STABLE_TIME) {
                mArg.inpk_state = (mArg.inpk_state == INPK_STATE_NOTINPK) ? INPK_STATE_INPK : INPK_STATE_NOTINPK;
                mArg.inpk_state_ref_ms = mResample.algo_time_stamp;
                mArg.result = mArg.inpk_state;
            }
            mArg.prev_ms = mArg.curr_ms;
            mResample.input_count--;
        }
    }
    return mArg.result;
}

static void algoEmbeddedUpdate(uint32_t x, uint32_t evtType)
{
    if (evtType == EVT_SENSOR_ALS) {
        mArg.als_is_shaded = (x < INPK_ALS_TOLERANCE);
    } else if (evtType == EVT_SENSOR_PROX) {
        mArg.prox_is_shaded = (x == 0) ? 1 : 0;
    }
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
#ifdef INPK_DEBUG_LOG
            osLog(LOG_DEBUG, "[In_Pocket] anyMotion sensor is not available\n");
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
#ifdef INPK_DEBUG_LOG
            osLog(LOG_DEBUG, "[In_Pocket] noMotion sensor is not available\n");
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
#ifdef INPK_DEBUG_LOG
            osLog(LOG_DEBUG, "[In_Pocket] Accel sensor is not available\n");
#endif
        }
    } else if ((!on) && mTask.accelHandle) {
        sensorRelease(mTask.taskId, mTask.accelHandle);
        mTask.accelHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
    }
}

static void configAls(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.alsHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_ALS, i, &mTask.alsHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.alsHandle, ALS_MIN_RATE, ALS_MAX_LATENCY)) {
                osEventSubscribe(mTask.taskId, EVT_SENSOR_ALS);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_ALS, i, &mTask.alsHandle) == NULL) {
#ifdef INPK_DEBUG_LOG
            osLog(LOG_DEBUG, "[In_Pocket] ALS sensor is not available\n");
#endif
        }
    } else if ((on == false) && (mTask.alsHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.alsHandle);
        mTask.alsHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ALS);
    }
}

static void configProx(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.proxHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_PROX, i, &mTask.proxHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.proxHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osEventSubscribe(mTask.taskId, EVT_SENSOR_PROX);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_PROX, i, &mTask.proxHandle) == NULL) {
#ifdef INPK_DEBUG_LOG
            osLog(LOG_DEBUG, "[In_Pocket] Prox sensor is not available\n");
#endif
        }
    } else if ((on == false) && (mTask.proxHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.proxHandle);
        mTask.proxHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_PROX);
    }
}

static bool inpkPower(bool on, void *cookie)
{
#ifdef INPK_DEBUG_LOG
    osLog(LOG_INFO, "[In_Pocket] Active in_pocket(%d)\n", on);
#endif
    if (on) {
        algoInit();
        configAccel(true, NORMAL_ACC_WINDOW);
        configAls(true);
        configProx(true);
        configAnyMotion(true);
        configNoMotion(true);
        mTask.taskState = STATE_INIT;
    } else {
        configAnyMotion(false);
        configNoMotion(false);
        configAccel(false, NORMAL_ACC_WINDOW);
        configAccel(false, EXTENDED_ACC_WINDOW);
        configAls(false);
        configProx(false);
        mTask.taskState = STATE_DISABLED;
    }
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool inpkSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool inpkFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool inpkFlush(void *cookie)
{
    return osEnqueueEvt(EVT_SENSOR_INPK, SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void inpkHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }
    switch (evtType) {
        case EVT_APP_START:
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            break;
        case EVT_SENSOR_ANY_MOTION:
            if ((mTask.taskState == STATE_AWAITING_ANY_MOTION)  || (mTask.taskState == STATE_INIT)) {
                configNoMotion(true);
                configAnyMotion(false);
                if (mMotion.apply_sw_anymotion) {
                    configAccel(false, EXTENDED_ACC_WINDOW);
                }
                configAls(true);
                configProx(true);
                configAccel(true, NORMAL_ACC_WINDOW);
                mTask.taskState = STATE_AWAITING_IN_POCKET;
            }
            break;
        case EVT_SENSOR_NO_MOTION:
            if ((mTask.taskState == STATE_AWAITING_IN_POCKET)  || (mTask.taskState == STATE_INIT)) {
                configAnyMotion(true);
                configNoMotion(false);
                configAccel(false, NORMAL_ACC_WINDOW);
                if (mMotion.apply_sw_anymotion) {
                    configAccel(true, EXTENDED_ACC_WINDOW);
                }
                configAls(false);
                configProx(false);
                mTask.taskState = STATE_AWAITING_ANY_MOTION;
            }
            break;
        case EVT_SENSOR_ACCEL:
            if (algoUpdate((struct TripleAxisDataEvent *)evtData, evtType)) {
                union EmbeddedDataPoint sample;
                sample.idata = 1;

                if (mArg.result == INPK_STATE_NOTINPK) {
#ifdef INPK_DEBUG_LOG
                    osLog(LOG_DEBUG, "[In_Pocket] changed state to not_in_pocket\n");
#endif
                } else if (mArg.result == INPK_STATE_INPK) {
#ifdef INPK_DEBUG_LOG
                    osLog(LOG_DEBUG, "[In_Pocket] changed state to in_pocket\n");
#endif
                    osEnqueueEvt(EVT_SENSOR_INPK, sample.vptr, NULL);
                }
            }
            break;
        case EVT_SENSOR_ALS: {
            union EmbeddedDataPoint ev;
            ev.idata = (uint32_t)evtData;
            algoEmbeddedUpdate((uint32_t)ev.fdata, evtType);
            break;
        }
        case EVT_SENSOR_PROX: {
            union EmbeddedDataPoint ev;
            ev.idata = (uint32_t)evtData;
            algoEmbeddedUpdate((uint32_t)ev.fdata, evtType);
            break;
        }
    }
}

static const struct SensorInfo mSi = {
    .sensorName = "In Pocket",
    .sensorType = SENS_TYPE_INPOCKET,
    .numAxis = NUM_AXIS_EMBEDDED,
    .interrupt = NANOHUB_INT_WAKEUP,
    .minSamples = 20
};

static const struct SensorOps mSops = {
    .sensorPower = inpkPower,
    .sensorFirmwareUpload = inpkFirmwareUpload,
    .sensorSetRate = inpkSetRate,
    .sensorFlush = inpkFlush
};

static bool inpkStart(uint32_t taskId)
{
    mTask.taskId = taskId;
    mTask.handle = sensorRegister(&mSi, &mSops, NULL, true);
    algoInit();
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void inpkEnd()
{

}

INTERNAL_APP_INIT(
    APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_INPOCKET, 0, 0)),
    0,
    inpkStart,
    inpkEnd,
    inpkHandleEvent
);
