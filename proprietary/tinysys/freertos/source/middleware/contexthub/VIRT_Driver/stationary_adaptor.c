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
#include "math_method.h"
#include "tilt_common.h"
#include "stationary.h"

#define EVT_SENSOR_ACCEL        sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_STA          sensorGetMyEventType(SENS_TYPE_STATIONARY_DETECT)
#define EVT_SENSOR_NONSTA       sensorGetMyEventType(SENS_TYPE_MOTION_DETECT)
#define EVT_SENSOR_ANY_MOTION   sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#define EVT_SENSOR_NO_MOTION    sensorGetMyEventType(SENS_TYPE_NO_MOTION)

#define STATIONARY_VAR_THRE     2000  // average variance of Acc (unit: 10^5 * m/s^2)
#define TIME_TO_STATIONARY      6000  // timer for NONSTATIONARY -> STATIONARY (unit: ms)
#define TIME_TO_NONSTATIONARY   6000  // timer for STATIONARY -> NONSTATIONARY (unit: ms)
#define DEGREE_SECTION          90

#define DEBUG_LOG

// ----------------------------------------------
// 1. One-shot for SENS_TYPE_STATIONARY_DETECT and SENS_TYPE_MOTION_DETECT
// 2. Report first stable state
// 3. Only consider variance here, maybe should add norm ~= 1g ?
// ----------------------------------------------

enum StationaryType {
    STATIONARY,
    NON_STATIONARY,
    NUM_OF_STATIONARY
};

struct StationaryArgument {
    uint8_t idx;
    uint32_t handle;
    bool active;
    uint32_t time_stamp;     // unit: ns
    uint32_t last_set_time;  // unit: ms
    int8_t state;
    int8_t result;
};

struct StationaryTask {
    struct StationaryArgument sensors[NUM_OF_STATIONARY];
    uint32_t taskId;
    uint32_t anyMotionHandle;
    uint32_t noMotionHandle;
    uint32_t accelHandle;
    enum {
        STATE_DISABLED,
        STATE_AWAITING_ANY_MOTION,
        STATE_AWAITING_STATIONARY,
        STATE_INIT
    } taskState;
};

static resampling_t mResample;
static struct StationaryTask mTask;
static struct StationaryArgument mArg;
static swMotion_t mMotion;

static void algoInit()
{
    uint32_t init_time = rtcGetTime() / TIME_NANO_TO_MILLI;
    memset(&mArg, 0, sizeof(struct StationaryArgument));
    memset(&mMotion, false, sizeof(swMotion_t));
    init_data_resampling(&mResample, STATIONARY_INPUT_INTERVAL);
    stationary_set_ts(init_time);
    stationary_parameter_init(STATIONARY_VAR_THRE, TIME_TO_STATIONARY, TIME_TO_NONSTATIONARY, DEGREE_SECTION,
                              INTERPOLATION_UPPER_LIMIT * STATIONARY_INPUT_INTERVAL);
    stationary_algorithm_init(init_time);
}

static void runAlgo()
{
    if (mArg.state == INPUT_STATE_INIT) {
        mArg.last_set_time = mResample.algo_time_stamp;
        mArg.state = INPUT_STATE_PREPARE_DATA;
    }

    int32_t time_diff;
    time_diff = mResample.algo_time_stamp - mArg.last_set_time;
    if ((mArg.state == INPUT_STATE_PREPARE_DATA) || \
            ((mArg.state == INPUT_STATE_UPDATE) && (time_diff >= STATIONARY_FEATURE_UPDATE_TIME))) {
        mArg.state = INPUT_STATE_UPDATE;
        mArg.last_set_time = mResample.algo_time_stamp;
        stationary_set_ts(mResample.algo_time_stamp);
        stationary_alg_enter_point();

        int notify = get_stationary_notify();
        mArg.result = notify ? notify : mArg.result;
    }
}

static bool algoUpdate(struct TripleAxisDataEvent *ev)
{
    struct TripleAxisDataPoint *sample;
    uint64_t input_time_stamp_ms = ev->referenceTime / TIME_NANO_TO_MILLI;
    uint32_t count = ev->samples[0].firstSample.numSamples;
    uint32_t i;

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
            Directional_Sensor_Ms2_Data_t algoAccInput = {mResample.algo_time_stamp, mResample.algoAccData.x, mResample.algoAccData.y, mResample.algoAccData.z};
            get_signal_dir_acc(algoAccInput); /* using address acc_buff to update acc measurements */
            runAlgo();
            mResample.input_count--;
        }
    }
    return mArg.result;
}

static const struct SensorInfo mSi[NUM_OF_STATIONARY] = {
    {
        .sensorName = "Stationary",
        .sensorType = SENS_TYPE_STATIONARY_DETECT,
        .numAxis = NUM_AXIS_EMBEDDED,
        .interrupt = NANOHUB_INT_WAKEUP,
        .minSamples = 20
    },
    {
        .sensorName = "Motion",
        .sensorType = SENS_TYPE_MOTION_DETECT,
        .numAxis = NUM_AXIS_EMBEDDED,
        .interrupt = NANOHUB_INT_WAKEUP,
        .minSamples = 20
    }
};

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
#ifdef DEBUG_LOG
            osLog(LOG_DEBUG, "[Stationary] anyMotion sensor is not available\n");
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
#ifdef DEBUG_LOG
            osLog(LOG_DEBUG, "[Stationary] noMotion sensor is not available\n");
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
#ifdef DEBUG_LOG
            osLog(LOG_DEBUG, "[Stationary] Accel sensor is not available\n");
#endif
        }
    } else if ((!on) && mTask.accelHandle) {
        sensorRelease(mTask.taskId, mTask.accelHandle);
        mTask.accelHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
    }
}

static bool stationaryPower(bool on, void *cookie)
{
    struct StationaryArgument *mSensor = &mTask.sensors[(int)cookie];
    static bool prev_any_on = false;
    mSensor->active = on;
    bool any_on = (mTask.sensors[STATIONARY].active || mTask.sensors[NON_STATIONARY].active);
#ifdef DEBUG_LOG
    osLog(LOG_INFO, "[Stationary] Active: Stationary(%d), Motion(%d)\n", mTask.sensors[STATIONARY].active,
          mTask.sensors[NON_STATIONARY].active);
#endif

    //Initialize only when any sensor is the first registered sensor
    if ((!prev_any_on) && (any_on)) {
#ifdef DEBUG_LOG
        osLog(LOG_INFO, "[Stationary] configAccel on\n");
#endif
        algoInit();
        configAccel(true, NORMAL_ACC_WINDOW);
        configAnyMotion(true);
        configNoMotion(true);
        mTask.taskState = STATE_INIT;
    } else if ((prev_any_on) && (!any_on)) {
#ifdef DEBUG_LOG
        osLog(LOG_INFO, "[Stationary] configAccel off\n");
#endif
        configAnyMotion(false);
        configNoMotion(false);
        configAccel(false, NORMAL_ACC_WINDOW);
        configAccel(false, EXTENDED_ACC_WINDOW);
        mTask.taskState = STATE_DISABLED;
    }
    prev_any_on = any_on;
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool stationarySetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    struct StationaryArgument *mSensor = &mTask.sensors[(int)cookie];
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool stationaryFirmwareUpload(void *cookie)
{
    struct StationaryArgument *mSensor = &mTask.sensors[(int)cookie];
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool stationaryFlush(void *cookie)
{
    struct StationaryArgument *mSensor = &mTask.sensors[(int)cookie];
    return osEnqueueEvt(sensorGetMyEventType(mSi[mSensor->idx].sensorType), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void stationaryHandleEvent(uint32_t evtType, const void* evtData)
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
                configAnyMotion(false);
                if (mMotion.apply_sw_anymotion) {
                    configAccel(false, EXTENDED_ACC_WINDOW);
                }
                configAccel(true, NORMAL_ACC_WINDOW);
                mTask.taskState = STATE_AWAITING_STATIONARY;
            }
            break;
        case EVT_SENSOR_NO_MOTION:
            if ((mTask.taskState == STATE_AWAITING_STATIONARY) || (mTask.taskState == STATE_INIT)) {
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
            if (algoUpdate((struct TripleAxisDataEvent *)evtData)) {
                union EmbeddedDataPoint sample;
                sample.idata = 1;

                if (mArg.result - 1 == STATIONARY) {
#ifdef DEBUG_LOG
                    osLog(LOG_DEBUG, "[Stationary] change state to Stationary\n");
#endif
                    osEnqueueEvt(EVT_SENSOR_STA, sample.vptr, NULL);
                } else if (mArg.result - 1 == NON_STATIONARY) {
#ifdef DEBUG_LOG
                    osLog(LOG_DEBUG, "[Stationary] change state to Motion\n");
#endif
                    osEnqueueEvt(EVT_SENSOR_NONSTA, sample.vptr, NULL);
                }
                mArg.result = 0;
            }
            break;
    }
}

static const struct SensorOps mSops = {
    .sensorPower = stationaryPower,
    .sensorFirmwareUpload = stationaryFirmwareUpload,
    .sensorSetRate = stationarySetRate,
    .sensorFlush = stationaryFlush
};


static bool stationaryStart(uint32_t taskId)
{
    size_t i;
    mTask.taskId = taskId;
    for (i = 0; i < NUM_OF_STATIONARY; i++) {
        mTask.sensors[i].idx = i;
        mTask.sensors[i].handle = sensorRegister(&mSi[i], &mSops, (void *)i, true);
    }
    algoInit();
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void stationaryEnd()
{

}

INTERNAL_APP_INIT(
    APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_STATIONARY_DETECT, 0, 0)),
    0,
    stationaryStart,
    stationaryEnd,
    stationaryHandleEvent
);
