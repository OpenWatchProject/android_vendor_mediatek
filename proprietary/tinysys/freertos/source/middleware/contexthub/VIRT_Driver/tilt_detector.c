#include <stdio.h>
#include "tilt_common.h"
#include "algoConfig.h"
#include "tilt.h"
#include "FreeRTOS.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <plat/inc/rtc.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <seos.h>
#include <nanohub_math.h>
#include <sensors.h>
#include <limits.h>
#include <performance.h>

#define EVT_SENSOR_ACC  sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_ANY_MOTION    sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#define EVT_SENSOR_NO_MOTION     sensorGetMyEventType(SENS_TYPE_NO_MOTION)

struct tiltDetectorTask {
    uint32_t taskId;
    uint32_t handle;
    uint32_t anyMotionHandle;
    uint32_t noMotionHandle;
    uint32_t accelHandle;
    uint8_t tilt_init_flag;
    uint32_t prev_set_data_time;
    int32_t input_state;
    enum {
        STATE_DISABLED,
        STATE_AWAITING_ANY_MOTION,
        STATE_AWAITING_GESTURE,
        STATE_INIT
    } taskState;
};
static struct tiltDetectorTask mTask;
static resampling_t mResample;
static swMotion_t mMotion;
// *****************************************************************************

static void algoInit(struct TripleAxisDataEvent *ev)
{
    uint32_t init_time = (uint32_t)(ev->referenceTime / TIME_NANO_TO_MILLI);
    tilt_algorithm_init(init_time);
    tilt_set_ts(init_time);
    tilt_original_reinit();
    mTask.prev_set_data_time = 0;
    mTask.input_state = INPUT_STATE_INIT;
    init_data_resampling(&mResample, TILT_INPUT_SAMPLE_DELAY);
    memset(&mMotion, false, sizeof(swMotion_t));
    mTask.tilt_init_flag = 1;
}

static void run_tilt_algorithm()
{
    if (mTask.input_state == INPUT_STATE_INIT) {
        mTask.prev_set_data_time = mResample.algo_time_stamp;
        mTask.input_state = INPUT_STATE_PREPARE_DATA;
    }

    int32_t time_diff;
    time_diff = mResample.algo_time_stamp - mTask.prev_set_data_time;
    if ((mTask.input_state == INPUT_STATE_PREPARE_DATA) ||
            ((mTask.input_state == INPUT_STATE_UPDATE) && (time_diff >= TILT_FEATURE_UPDATE_TIME))) {
        mTask.input_state = INPUT_STATE_UPDATE;
        mTask.prev_set_data_time = mResample.algo_time_stamp;
        tilt_set_ts(mResample.algo_time_stamp);
        tilt_alg_enter_point();
        int* tilt_output = get_tilt_result_ptr(0);
        if (*tilt_output) {
            union EmbeddedDataPoint sample;
            sample.idata = 1;
            osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_TILT), sample.vptr, NULL);
            get_tilt_result_ptr(1);
            osLog(LOG_DEBUG, "tilt notify\n");
        }
    }
}

static void tiltDetectorAlgo(struct TripleAxisDataEvent *ev)
{
    struct TripleAxisDataPoint *sample;
    uint32_t input_time_stamp_ms = ev->referenceTime / TIME_NANO_TO_MILLI;
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
            Directional_Sensor_Ms2_Data_t dirAccInput = {mResample.algo_time_stamp, mResample.algoAccData.x, mResample.algoAccData.y, mResample.algoAccData.z};
            get_signal_dir_acc(dirAccInput);
            run_tilt_algorithm();
            mResample.input_count--;
        }
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
            osLog(LOG_DEBUG, "[WARNING] TILT: anyMotion sensor is not available\n");
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
            osLog(LOG_DEBUG, "[WARNING] TILT: noMotion sensor is not available\n");
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
            osLog(LOG_DEBUG, "[WARNING] TILT: Accel sensor is not available\n");
        }
    } else if ((!on) && mTask.accelHandle) {
        sensorRelease(mTask.taskId, mTask.accelHandle);
        mTask.accelHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
    }
}

static bool tiltDetectorPower(bool on, void *cookie)
{
    //Initialize only when any sensor is the first registered sensor
    if (on) {
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
        mTask.tilt_init_flag = 0;
    }
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool tiltDetectorSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool tiltDetectorFirmwareUpload(void *cookie)
{

    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool tiltDetectorFlush(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_TILT), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void tiltDetectorHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    switch (evtType) {
        case EVT_APP_START:
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            osLog(LOG_DEBUG, "TILT EVT_APP_START\n");
            break;

        case EVT_SENSOR_ANY_MOTION:
            if ((mTask.taskState == STATE_AWAITING_ANY_MOTION)  || (mTask.taskState == STATE_INIT)) {
                configNoMotion(true);
                configAnyMotion(false);
                if (mMotion.apply_sw_anymotion) {
                    configAccel(false, EXTENDED_ACC_WINDOW);
                }
                configAccel(true, NORMAL_ACC_WINDOW);
                mTask.taskState = STATE_AWAITING_GESTURE;
            }
            break;

        case EVT_SENSOR_NO_MOTION:
            if ((mTask.taskState == STATE_AWAITING_GESTURE)  || (mTask.taskState == STATE_INIT)) {
                configAnyMotion(true);
                configNoMotion(false);
                configAccel(false, NORMAL_ACC_WINDOW);
                if (mMotion.apply_sw_anymotion) {
                    configAccel(true, EXTENDED_ACC_WINDOW);
                }
                mTask.taskState = STATE_AWAITING_ANY_MOTION;
            }
            break;

        case EVT_SENSOR_ACC:
            mark_timestamp(ACC_GYRO, GESTURE, ALGO_BEGIN, rtcGetTime());
            if (!mTask.tilt_init_flag) {
                algoInit((struct TripleAxisDataEvent *)evtData);
                osLog(LOG_DEBUG, "TILT ALGO INIT!!\n");
            }

            tiltDetectorAlgo((struct TripleAxisDataEvent *)evtData);
            mark_timestamp(ACC_GYRO, GESTURE, ALGO_DONE, rtcGetTime());
            break;

        default:
            break;
    }
}

static const struct SensorInfo mSi = {
    .sensorName = "TILT",
    .sensorType = SENS_TYPE_TILT,
    .numAxis = NUM_AXIS_EMBEDDED,
    .interrupt = NANOHUB_INT_WAKEUP,
    .minSamples = 20
};

static const struct SensorOps mSops = {
    .sensorPower = tiltDetectorPower,
    .sensorFirmwareUpload = tiltDetectorFirmwareUpload,
    .sensorSetRate = tiltDetectorSetRate,
    .sensorFlush = tiltDetectorFlush,
};

static bool tiltDetectorStart(uint32_t taskId)
{
    mTask.taskId = taskId;
    mTask.handle = sensorRegister(&mSi, &mSops, NULL, true);;
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void tiltDetectorEnd()
{
}
INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_TILT, 0, 0)), 0, tiltDetectorStart,
                  tiltDetectorEnd, tiltDetectorHandleEvent);

