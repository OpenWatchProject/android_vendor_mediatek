#include <stdio.h>
#include "activity_algorithm.h"
#include "init_learning.h"
#include "gesture.h"
#include "signal.h"
#include "algoConfig.h"
#ifdef _SHAKE_ENABLE_
#include "gesture_shake_predictor.h"
#endif
#ifdef _SNAPSHOT_ENABLE_
#include "gesture_snapshot_predictor.h"
#endif
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
//#define _GESTURE_DEBUG_LOG_

struct gestureRecognitionArgument {
    uint8_t idx;
    uint32_t handle;
    bool active;
};

struct gestureRecognitionTask {
    struct gestureRecognitionArgument sensors[GESTURE_TYPE_NUM];
    uint32_t taskId;
    uint32_t anyMotionHandle;
    uint32_t noMotionHandle;
    uint32_t accelHandle;
    uint32_t proxHandle;
    uint8_t algo_init_flag;
    uint8_t snapshot_notifity_data_count;
    uint8_t shake_notifity_data_count;
    uint8_t freefall_notifity_data_count;
    uint32_t prev_set_data_time;
    uint32_t set_data_timing;
    int32_t input_state;
    bool any_on;
    bool prev_any_on;
    enum {
        STATE_DISABLED,
        STATE_AWAITING_ANY_MOTION,
        STATE_AWAITING_GESTURE,
        STATE_INIT
    } taskState;
};

static struct gestureRecognitionTask mTask = {.any_on = false, .prev_any_on = false};
static resampling_t mResample;
static swMotion_t mMotion;
// *****************************************************************************

static void gesture_task_init()
{
    mTask.set_data_timing = GESTURE_FEATURE_UPDATE_TIME - FEATURE_UPDATE_TIME_MARGIN;
    mTask.prev_set_data_time = 0;
    mTask.input_state = INPUT_STATE_INIT;
    mTask.snapshot_notifity_data_count   = GESTURE_NOTIFY_DATA_NUM + 1;
    mTask.shake_notifity_data_count      = GESTURE_NOTIFY_DATA_NUM + 1;
    mTask.algo_init_flag = 1;
}

static void algoInit(struct TripleAxisDataEvent *ev)
{
    init_data_resampling(&mResample, GESTURE_INPUT_SAMPLE_DELAY);
    memset(&mMotion, false, sizeof(swMotion_t));
    uint32_t init_time = (uint32_t)(ev->referenceTime / TIME_NANO_TO_MILLI);
    gesture_algorithm_init(init_time);
    gesture_task_init();
}

static uint8_t check_notify_or_not(gesture_notify_result_t *results, uint8_t *notify_count)
{
    if (*notify_count < GESTURE_NOTIFY_DATA_NUM) {
        *notify_count += 1;
        return 0;
    } else {
        if ((results->last_result <= results->threshold) && (results->current_result > results->threshold)) {
            *notify_count = 0;
            return 1;
        } else {
            return 0;
        }
    }
}

static void run_gesture_algorithm()
{
    if (mTask.input_state == INPUT_STATE_INIT) {
        mTask.prev_set_data_time = mResample.algo_time_stamp;
        mTask.input_state = INPUT_STATE_PREPARE_DATA;
    }

    int32_t time_diff;
    time_diff = mResample.algo_time_stamp - mTask.prev_set_data_time;
    if ((mTask.input_state == INPUT_STATE_PREPARE_DATA) ||
            ((mTask.input_state == INPUT_STATE_UPDATE) && (time_diff >= mTask.set_data_timing))) {
        union EmbeddedDataPoint sample;
        mTask.input_state = INPUT_STATE_UPDATE;
        mTask.prev_set_data_time = mResample.algo_time_stamp;
        int predictor_flag = 0;
        gesture_notify_result_t gesture_results;
        if (mTask.sensors[SHAKE].active || mTask.sensors[SNAPSHOT].active) {
            predictor_flag = gesutre_alg_enter_point();
        }
#ifdef _SHAKE_ENABLE_
        //gesture--shake
        if (mTask.sensors[SHAKE].active) {
            if (predictor_flag == 1) {
                shake_predictor(mResample.algo_time_stamp);
            }
            gesture_get_shake_result_ptr(&gesture_results);
            if (check_notify_or_not(&gesture_results, &mTask.shake_notifity_data_count)) {
                sample.idata = 1;
                osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_SHAKE), sample.vptr, NULL);
                osLog(LOG_DEBUG, "shake notify\n");
            }
        }
#endif
#ifdef _SNAPSHOT_ENABLE_
        //gesture--snapshot
        if (mTask.sensors[SNAPSHOT].active) {
            if (predictor_flag == 1) {
                snapshot_predictor(mResample.algo_time_stamp);
            }
            gesture_get_snapshot_result_ptr(&gesture_results);
            if (check_notify_or_not(&gesture_results, &mTask.snapshot_notifity_data_count)) {
                sample.idata = 1;
                osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_SNAPSHOT), sample.vptr, NULL);
                osLog(LOG_DEBUG, "snapshot notify\n");
            }
        }
#endif
    }
}

static void gestureRecognitionAlgo(struct TripleAxisDataEvent *ev)
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
            Activity_Accelerometer_Ms2_Data_t algoAccInput = {mResample.algo_time_stamp, mResample.algoAccData.x, mResample.algoAccData.y, mResample.algoAccData.z};
            get_signal_acc(algoAccInput); /* using address acc_buff to update acc measurements */
            run_gesture_algorithm();
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
            osLog(LOG_DEBUG, "[WARNING] GESTURE: anyMotion sensor is not available\n");
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
            osLog(LOG_DEBUG, "[WARNING] GESTURE: noMotion sensor is not available\n");
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
            osLog(LOG_DEBUG, "[WARNING] GESTURE: Accel sensor is not available\n");
        }
    } else if ((!on) && mTask.accelHandle) {
        sensorRelease(mTask.taskId, mTask.accelHandle);
        mTask.accelHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
    }
}

static const struct SensorInfo mSi[GESTURE_TYPE_NUM] = {
    {.sensorName = "SHAKE",       .sensorType = SENS_TYPE_SHAKE,       .numAxis = NUM_AXIS_EMBEDDED, .interrupt = NANOHUB_INT_WAKEUP, .minSamples = 20},
    {.sensorName = "SNAPSHOT",    .sensorType = SENS_TYPE_SNAPSHOT,    .numAxis = NUM_AXIS_EMBEDDED, .interrupt = NANOHUB_INT_WAKEUP, .minSamples = 20},
};

static bool gestureRecognitionPower(bool on, void *cookie)
{
    struct gestureRecognitionArgument *mSensor = &mTask.sensors[(int)cookie];
    mSensor->active = on;

    mTask.any_on = (mTask.sensors[SHAKE].active || mTask.sensors[SNAPSHOT].active);
    osLog(LOG_DEBUG,
          "GESTURE_RECOGNITION: Power: idx:%d, prev_any_on:%d, any_on:%d, shake:%d, snapshot:%d\n",
          mSensor->idx,
          mTask.prev_any_on, mTask.any_on, mTask.sensors[SHAKE].active, mTask.sensors[SNAPSHOT].active);

    //Initialize only when any sensor is the first registered sensor
    if ((!mTask.prev_any_on) && (mTask.any_on)) {
        configAccel(true, NORMAL_ACC_WINDOW);
        configAnyMotion(true);
        configNoMotion(true);
        mTask.taskState = STATE_INIT;
    } else if ((mTask.prev_any_on) && (!mTask.any_on)) {
        configAnyMotion(false);
        configNoMotion(false);
        configAccel(false, NORMAL_ACC_WINDOW);
        configAccel(false, EXTENDED_ACC_WINDOW);
        mTask.taskState = STATE_DISABLED;
        mTask.algo_init_flag = 0;
    }

    mTask.prev_any_on = mTask.any_on;
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool gestureRecognitionSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    struct gestureRecognitionArgument *mSensor = &mTask.sensors[(int)cookie];
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool gestureRecognitionFirmwareUpload(void *cookie)
{
    struct gestureRecognitionArgument *mSensor = &mTask.sensors[(int)cookie];
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool gestureRecognitionFlush(void *cookie)
{
    struct gestureRecognitionArgument *mSensor = &mTask.sensors[(int)cookie];
    return osEnqueueEvt(sensorGetMyEventType(mSi[mSensor->idx].sensorType), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void gestureRecognitionHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    switch (evtType) {
        case EVT_APP_START:
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            osLog(LOG_DEBUG, "GESTURE EVT_APP_START\n");
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
            if (!mTask.algo_init_flag) {
                algoInit((struct TripleAxisDataEvent *)evtData);
                osLog(LOG_DEBUG, "GESTURE ALGO INIT!!\n");
            }
            gestureRecognitionAlgo((struct TripleAxisDataEvent *)evtData);
            mark_timestamp(ACC_GYRO, GESTURE, ALGO_DONE, rtcGetTime());
            break;

        default:
            break;
    }
}

static const struct SensorOps mSops = {
    .sensorPower = gestureRecognitionPower,
    .sensorFirmwareUpload = gestureRecognitionFirmwareUpload,
    .sensorSetRate = gestureRecognitionSetRate,
    .sensorFlush = gestureRecognitionFlush,
};

static bool gestureRecognitionStart(uint32_t taskId)
{
    size_t i;
    mTask.taskId = taskId;
    for (i = 0; i < GESTURE_MAX; i++) {
        mTask.sensors[i].idx = i;
        mTask.sensors[i].handle = sensorRegister(&mSi[i], &mSops, (void *)i, true);
    }
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void gestureRecognitionEnd()
{
}
INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_SNAPSHOT, 0, 0)), 0, gestureRecognitionStart,
                  gestureRecognitionEnd, gestureRecognitionHandleEvent);

