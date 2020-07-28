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
#include "motion.h"

#define EVT_SENSOR_ACCEL        sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_ANY_MOTION   sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#define EVT_SENSOR_NO_MOTION    sensorGetMyEventType(SENS_TYPE_NO_MOTION)

#define ACCEL_MIN_RATE          SENSOR_HZ(50)
#define ACCEL_MAX_LATENCY       2000000000ull   // 2000 ms
#define NANO_TO_MICRO           1000000

#define MOTION_GRAVITY          9806            // 9.8 * 10^3
#define ACC_NOISE_DENSITY       250             // ug/(Hz)^0.5
#define ACC_F_CUT               111.0           // Hz
#define ACC_PTP                 ACC_NOISE_DENSITY*sqrtf(ACC_F_CUT)/NANO_TO_MICRO*MOTION_GRAVITY*12.0    // m/s^2 (6.0*sigma*3.0) peak to mean
#define ACC_VAR                 ACC_PTP*ACC_PTP

#define MOTION_VAR              ACC_VAR*3
#define MOTION_TIME_TO_ANY      100
#define MOTION_TIME_TO_NO       10000
#define MOTION_G_SECTION        MOTION_GRAVITY*MOTION_GRAVITY*0.2

#define MOTION_TIMESTAMP_ERROR  1.1
#define MOTION_MAX(x, y)        (x >= y ? x : y)

#define MOTION_DEBUG_LOG

// ----------------------------------------------
// 1. One-shot for SENS_TYPE_ANY_MOTION and SENS_TYPE_NO_MOTION
// 2. Report first stable state
// 3. Only consider variance here, maybe should add norm ~= 1g ?
// ----------------------------------------------

enum MotionType {
    ANY_MOTION,
    NO_MOTION,
    NUM_OF_MOTION
};

struct MotionArgument {
    uint8_t idx;
    uint32_t handle;
    bool regDone;
    bool active;
    int8_t result;
};

struct MotionTask {
    struct MotionArgument sensors[NUM_OF_MOTION];
    uint32_t taskId;
    uint32_t accelHandle;
    bool anyRegDone;
};

static struct MotionArgument mArg;
static struct MotionTask mTask;
static resampling_t mResample;
static bool hwAnymo_has_check = false;
static bool hwNomo_has_check = false;

static void algoInit()
{
    uint32_t init_time = rtcGetTime() / NANO_TO_MICRO;
    memset(&mArg, 0, sizeof(struct MotionArgument));
    init_data_resampling(&mResample, MOTION_INPUT_INTERVAL);
    motion_set_ts(init_time);
    motion_stored_init();
    motion_parameter_init(MOTION_VAR, MOTION_TIME_TO_ANY, MOTION_TIME_TO_NO, MOTION_G_SECTION,
                          MOTION_MAX(INTERPOLATION_UPPER_LIMIT * MOTION_INPUT_INTERVAL, (int32_t)(ACCEL_MAX_LATENCY * MOTION_TIMESTAMP_ERROR)));
}

static bool algoUpdate(struct TripleAxisDataEvent *ev)
{
    struct TripleAxisDataPoint *sample;
    uint32_t input_time_stamp_ms = ev->referenceTime / NANO_TO_MICRO;
    uint32_t count = ev->samples[0].firstSample.numSamples;
    uint32_t i;

    for (i = 0; i < count; i++) {
        sample = &ev->samples[i];
        if (i > 0) {
            input_time_stamp_ms += sample->deltaTime / NANO_TO_MICRO;
        }

        sensor_input_data_resampling_preparation(&mResample, input_time_stamp_ms, EVT_SENSOR_ACCEL);
        mResample.currentAccData = (AccelData_t) {
            (int32_t)(sample->x * 1000), (int32_t)(sample->y * 1000), (int32_t)(sample->z * 1000)
        };

        while (mResample.input_count > 0) {
            algo_input_data_generation(&mResample, EVT_SENSOR_ACCEL);
            motion_set_ts(mResample.algo_time_stamp);
            motion_set_acc(mResample.algoAccData.x, mResample.algoAccData.y, mResample.algoAccData.z);
            motion_run_algo();
            mResample.input_count--;
        }
    }
    mArg.result = get_motion_notify(mResample.algo_time_stamp);
    return mArg.result;
}

static const struct SensorInfo mSi[NUM_OF_MOTION] = {
    {
        .sensorName = SW_AnyMotion,
        .sensorType = SENS_TYPE_ANY_MOTION,
        .numAxis = NUM_AXIS_EMBEDDED,
        .interrupt = NANOHUB_INT_WAKEUP,
        .minSamples = 20
    },
    {
        .sensorName = SW_NoMotion,
        .sensorType = SENS_TYPE_NO_MOTION,
        .numAxis = NUM_AXIS_EMBEDDED,
        .interrupt = NANOHUB_INT_WAKEUP,
        .minSamples = 20
    }
};

static uint32_t SensorHasRegisterReturnHandle(uint32_t sensorType)
{
    uint32_t Handle = 0;

    sensorFind(sensorType, 0, &Handle);
    return Handle;
}

static bool motionSensorHasRegisteredCheck(uint32_t sensorType)
{
    uint32_t Handle = 0;

    if (sensorFind(sensorType, 0, &Handle) == NULL)
        return false;
    else
        return true;
}

static void configAccel(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.accelHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_ACCEL, i, &mTask.accelHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.accelHandle, ACCEL_MIN_RATE, ACCEL_MAX_LATENCY)) {
                osEventSubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_ACCEL, i, &mTask.accelHandle) == NULL) {
#ifdef MOTION_DEBUG_LOG
            osLog(LOG_DEBUG, "[Motion] Accel sensor is not available\n");
#endif
        }
    } else if ((on == false) && (mTask.accelHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.accelHandle);
        mTask.accelHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
    }
}

static bool motionPower(bool on, void *cookie)
{
    struct MotionArgument *mSensor = &mTask.sensors[(int)cookie];
    static bool prev_any_on = false;
    mSensor->active = on;
    bool any_on = (mTask.sensors[ANY_MOTION].active || mTask.sensors[NO_MOTION].active);

#ifdef MOTION_DEBUG_LOG
    osLog(LOG_INFO, "[Motion] Active anyMotion(%d), noMotion(%d)\n", mTask.sensors[ANY_MOTION].active,
          mTask.sensors[NO_MOTION].active);
#endif

    //Initialize only when any sensor is the first registered sensor
    if ((!prev_any_on) && (any_on)) {
#ifdef MOTION_DEBUG_LOG
        osLog(LOG_INFO, "[Motion] configAccel on\n");
#endif
        algoInit();
        configAccel(true);
    } else if ((prev_any_on) && (!any_on)) {
#ifdef MOTION_DEBUG_LOG
        osLog(LOG_INFO, "[Motion] configAccel off\n");
#endif
        configAccel(false);
        hwAnymo_has_check = false;
        hwNomo_has_check = false;
    }
    prev_any_on = any_on;
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool motionSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    struct MotionArgument *mSensor = &mTask.sensors[(int)cookie];
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool motionFirmwareUpload(void *cookie)
{
    struct MotionArgument *mSensor = &mTask.sensors[(int)cookie];
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool motionFlush(void *cookie)
{
    struct MotionArgument *mSensor = &mTask.sensors[(int)cookie];
    return osEnqueueEvt(sensorGetMyEventType(mSi[mSensor->idx].sensorType), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void motionSchemeRecognition(uint32_t sensorType, uint32_t sensor, bool *checkStatus)
{
    if (*checkStatus == false) {
        if (mTask.sensors[sensor].handle != SensorHasRegisterReturnHandle(sensorType)) {
            motionPower(false, (void*)sensor);
        }
        *checkStatus = true;
    }
}

static void motionHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }
    switch (evtType) {
        case EVT_APP_START:
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            break;
        case EVT_SENSOR_ACCEL:
            motionSchemeRecognition(SENS_TYPE_ANY_MOTION, ANY_MOTION, &hwAnymo_has_check);
            motionSchemeRecognition(SENS_TYPE_NO_MOTION, NO_MOTION, &hwNomo_has_check);

            if (algoUpdate((struct TripleAxisDataEvent *)evtData)) {
                union EmbeddedDataPoint sample;
                sample.idata = 1;

                if ((mArg.result == ANY_MOTION_STATE) && mTask.sensors[ANY_MOTION].active) {
#ifdef MOTION_DEBUG_LOG
                    osLog(LOG_DEBUG, "[Motion] changed state to anyMotion\n");
#endif
                    osEnqueueEvt(EVT_SENSOR_ANY_MOTION, sample.vptr, NULL);
                } else if ((mArg.result == NO_MOTION_STATE) && mTask.sensors[NO_MOTION].active) {
#ifdef MOTION_DEBUG_LOG
                    osLog(LOG_DEBUG, "[Motion] changed state to noMotion\n");
#endif
                    osEnqueueEvt(EVT_SENSOR_NO_MOTION, sample.vptr, NULL);
                }

            }
            break;
    }
}

static const struct SensorOps mSops = {
    .sensorPower = motionPower,
    .sensorFirmwareUpload = motionFirmwareUpload,
    .sensorSetRate = motionSetRate,
    .sensorFlush = motionFlush
};

static void swMotionRegistation(uint32_t sensorType, uint32_t sensor)
{
    if (motionSensorHasRegisteredCheck(sensorType) == false) {
        mTask.sensors[sensor].idx = sensor;
        mTask.sensors[sensor].handle = sensorRegister(&mSi[sensor], &mSops, (void *)sensor, true);
        mTask.sensors[sensor].regDone = true;
    }
}

static bool motionStart(uint32_t taskId)
{
    mTask.taskId = taskId;
    swMotionRegistation(SENS_TYPE_ANY_MOTION, ANY_MOTION);
    swMotionRegistation(SENS_TYPE_NO_MOTION, NO_MOTION);
    mTask.anyRegDone = mTask.sensors[ANY_MOTION].regDone || mTask.sensors[NO_MOTION].regDone;
    if (mTask.anyRegDone) {
        algoInit();
        osEventSubscribe(taskId, EVT_APP_START);
    }
    return true;
}

static void motionEnd()
{

}

INTERNAL_APP_INIT(
    APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ANY_MOTION, 0, 0)),
    0,
    motionStart,
    motionEnd,
    motionHandleEvent
);
