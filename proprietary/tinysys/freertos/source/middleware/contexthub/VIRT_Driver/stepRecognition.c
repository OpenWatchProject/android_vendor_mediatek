#include <stdio.h>
#include "pedometer.h"
#include "algoConfig.h"
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

#define EVT_SENSOR_ACCEL         sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_ANY_MOTION    sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#define EVT_SENSOR_NO_MOTION     sensorGetMyEventType(SENS_TYPE_NO_MOTION)
//#define PEDOMETER_RT_INFO        // MUST be disabled when evaluating power consumption
#define _ENABLE_MOTION_MONITORING_ // Comment this macro to keep step counter in the moving state
#define FEATURE_SWITCHING_TH     3000

enum stepRecognitionType {
    SMD,
    STEP_COUNTER,
    STEP_DETECTOR,
    NUM_OF_STEP_RECOGNITION
};

typedef struct SensorAttribute {
    uint8_t idx;
    uint32_t handle;
    bool active;
} SensorAttribute_t;

typedef struct stepRecognitionArgument {
    uint32_t prev_step_count_result;
    uint32_t current_step_count_result;
    uint32_t prev_accumulated_step_count;
    uint32_t step_counter_start_timing;
    uint32_t accumulate_step_from_detector;
    uint32_t elasped_time;
    bool step_counter_get_start_timing;
    bool step_counter_early_state;
} stepRecognitionArgument_t;

typedef struct stepRecognitionTask {
    SensorAttribute_t sensors[NUM_OF_STEP_RECOGNITION];
    uint32_t taskId;
    uint32_t anyMotionHandle;
    uint32_t noMotionHandle;
    uint32_t accelHandle;
    enum {
        STATE_DISABLED,
        STATE_AWAITING_ANY_MOTION,
        STATE_AWAITING_STEP,
        STATE_INIT
    } taskState;
} stepRecognitionTask_t;

static stepRecognitionTask_t mTask;
static stepRecognitionArgument_t mArg;
static resampling_t mResample;
static uint32_t accumulated_step_count = 0;
static swMotion_t mMotion;
extern uint8_t step_debug_trace;

// *****************************************************************************

static void algoInit()
{
    memset(&mArg, 0, sizeof(stepRecognitionArgument_t));
    memset(&mMotion, false, sizeof(swMotion_t));
    init_data_resampling(&mResample, STEP_RECOGNITION_INPUT_INTERVAL);
    pedometer_algorithm_init();
    mArg.prev_accumulated_step_count = accumulated_step_count;
    mResample.modify_sample_scheme = 1;
    osLog(LOG_INFO, "STEP_RECOGNITION_INTI_DONE\n");
}

static bool update_step_result(uint32_t input_time_stamp_ms)
{
    bool update_result = false;
    static bool terminate_early_state = false;
    if (!mArg.step_counter_early_state) {
        uint32_t step_counter_batch_result = get_pedometer_step_count();
        if (mArg.accumulate_step_from_detector && !terminate_early_state) {
            accumulated_step_count += mArg.accumulate_step_from_detector;
            mArg.prev_accumulated_step_count = accumulated_step_count;
            mArg.prev_step_count_result = mArg.prev_accumulated_step_count + step_counter_batch_result;
            terminate_early_state = true;
            return false;
        }

        if (mArg.accumulate_step_from_detector && (mArg.accumulate_step_from_detector < step_counter_batch_result)) {
            accumulated_step_count -= mArg.accumulate_step_from_detector;
            mArg.prev_accumulated_step_count = accumulated_step_count;
            mArg.accumulate_step_from_detector = 0;
        }

        mArg.current_step_count_result = mArg.prev_accumulated_step_count + step_counter_batch_result;
        if (mArg.current_step_count_result != mArg.prev_step_count_result) {
            mArg.prev_step_count_result = mArg.current_step_count_result;
            accumulated_step_count = mArg.current_step_count_result;
            update_result = true;
            osLog(LOG_INFO, "STEP_COUNTER RESULT UPDATE: driver=%" PRIu32 "algo=%" PRIu32 "@ %" PRIu64 "\n",
                accumulated_step_count, step_counter_batch_result, rtcGetTime());
        }
    }
#ifdef PEDOMETER_RT_INFO
    osLog(LOG_INFO, "STEP_COUNTER STATUS=%d @ %llu", get_pedometer_status(), rtcGetTime());
    if (get_pedometer_status() == TIMEOUT_NOW) osLog(LOG_INFO, "STEP_COUNTER TIMEOUT @ %llu\n", rtcGetTime());
#endif
    return update_result;
}

static bool stepRecognitionAlgo(struct TripleAxisDataEvent *ev)
{
    struct TripleAxisDataPoint *sample;
    uint32_t input_time_stamp_ms = ev->referenceTime / TIME_NANO_TO_MILLI;
    uint32_t count = ev->samples[0].firstSample.numSamples;
    uint32_t i = 0;
    if (mTask.sensors[STEP_COUNTER].active && !mArg.step_counter_get_start_timing) {
        mArg.step_counter_start_timing = input_time_stamp_ms;
        mArg.accumulate_step_from_detector = 0;
        mArg.elasped_time = 0;
        mArg.step_counter_early_state = true;
        mArg.step_counter_get_start_timing = true;
    }
    for (i = 0; i < count; i++) {
        sample = &ev->samples[i];
        if (i > 0) {
            input_time_stamp_ms += sample->deltaTime / TIME_NANO_TO_MILLI;
        }

        sensor_input_data_resampling_preparation(&mResample, input_time_stamp_ms, EVT_SENSOR_ACCEL);
        mResample.currentAccData = (AccelData_t) {
            (int32_t)(sample->x * 1000), (int32_t)(sample->y * 1000), (int32_t)(sample->z * 1000)
        };

        if (step_debug_trace) {
            static uint32_t accumulated_acc_count = 0;
            accumulated_acc_count++;
            int32_t checksum = input_time_stamp_ms + (int32_t)(sample->x * 1000) + (int32_t)(sample->y * 1000) + (int32_t)(
                                   sample->z * 1000) + accumulated_acc_count;
            osLog(LOG_INFO, "Macc:%lu,%ld,%ld,%ld,%lu,%ld\n", input_time_stamp_ms, (int32_t)(sample->x * 1000),
                  (int32_t)(sample->y * 1000), (int32_t)(sample->z * 1000), accumulated_acc_count, checksum);
        }

        while (mResample.input_count > 0) {
            algo_input_data_generation(&mResample, EVT_SENSOR_ACCEL);
            set_pedometer_t_s(mResample.algo_time_stamp);
            pedometer_update_acc(mResample.algoAccData.x, mResample.algoAccData.y, mResample.algoAccData.z);

            uint8_t step_detector_result = get_pedometer_step_detector();
#ifdef _STEP_DETECTOR_
            if (mTask.sensors[STEP_DETECTOR].active && step_detector_result) {
                union EmbeddedDataPoint sample;
                sample.idata = step_detector_result;
                osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_STEP_DETECT), sample.vptr, NULL);
            }
#endif
            if (mArg.step_counter_early_state) {
                mArg.elasped_time = input_time_stamp_ms - mArg.step_counter_start_timing;
                if (mTask.sensors[STEP_COUNTER].active && step_detector_result && mArg.elasped_time < FEATURE_SWITCHING_TH) {
                    mArg.accumulate_step_from_detector += step_detector_result;
                    union EmbeddedDataPoint sample;
                    sample.idata = accumulated_step_count + mArg.accumulate_step_from_detector;
                    osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_STEP_COUNT), sample.vptr, NULL);
                } else if (mArg.elasped_time >= FEATURE_SWITCHING_TH) {
                    mArg.step_counter_early_state = false;
                }
            }
            mResample.input_count--;
        }
    }
#ifdef _SMD_ENABLE_
    if (mTask.sensors[SMD].active && get_pedometer_smd_event()) {
        union EmbeddedDataPoint sample;
        sample.idata = 1;
        osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_SIG_MOTION), sample.vptr, NULL);
    }
#endif
    return update_step_result(input_time_stamp_ms);
}

#ifdef _ENABLE_MOTION_MONITORING_
static void configAnyMotion(bool on)
{
    uint32_t i;
    const struct SensorInfo *getAnyMotionSrc;
    if (on && (!mTask.anyMotionHandle)) {
        for (i = 0; (getAnyMotionSrc = sensorFind(SENS_TYPE_ANY_MOTION, i, &mTask.anyMotionHandle))
                && (getAnyMotionSrc != NULL); i++) {
            if (sensorRequest(mTask.taskId, mTask.anyMotionHandle, SENSOR_RATE_ONCHANGE, 0)) {
                mMotion.apply_sw_anymotion = !(strcmp(getAnyMotionSrc->sensorName, (char*)SW_AnyMotion));
                osLog(LOG_INFO, "STEP_RECOGNITION: anyMotion SRC: %s\n", getAnyMotionSrc->sensorName);
                osEventSubscribe(mTask.taskId, EVT_SENSOR_ANY_MOTION);
                break;
            }
        }
        if (!getAnyMotionSrc) {
            osLog(LOG_INFO, "[WARNING] STEP_RECOGNITION: anyMotion is not available\n");
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
                osLog(LOG_INFO, "STEP_RECOGNITION: noMotion SRC: %s\n", getNoMotionSrc->sensorName);
                osEventSubscribe(mTask.taskId, EVT_SENSOR_NO_MOTION);
                break;
            }
        }
        if (!getNoMotionSrc) {
            osLog(LOG_INFO, "[WARNING] STEP_RECOGNITION: noMotion is not available\n");
        }
    } else if ((!on) && mTask.noMotionHandle) {
        sensorRelease(mTask.taskId, mTask.noMotionHandle);
        mTask.noMotionHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_NO_MOTION);
    }
}
#endif

static void configAccel(bool on)
{
    uint32_t i;
    bool getAccelSensor = false;
    if ((on == true) && (mTask.accelHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_ACCEL, i, &mTask.accelHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.accelHandle, ACCEL_MIN_RATE, ACCEL_UINT_LATENCY)) {
                getAccelSensor = true;
                osEventSubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
                break;
            }
        }
        if (!getAccelSensor) {
            osLog(LOG_INFO, "[WARNING] STEP_RECOGNITION: accel is not available\n");
        }
    } else if ((on == false) && (mTask.accelHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.accelHandle);
        mTask.accelHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
    }
}

static void forceReport()
{
    union EmbeddedDataPoint sample;
    sample.idata = accumulated_step_count;
    osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_STEP_COUNT), sample.vptr, NULL);
}

static const struct SensorInfo mSi[NUM_OF_STEP_RECOGNITION] = {
    {.sensorName = "Significant Motion Detection", .sensorType = SENS_TYPE_SIG_MOTION,  .numAxis = NUM_AXIS_EMBEDDED, .interrupt = NANOHUB_INT_WAKEUP, .minSamples = 20},
    {.sensorName = "Step Counter",                 .sensorType = SENS_TYPE_STEP_COUNT,  .numAxis = NUM_AXIS_EMBEDDED, .interrupt = NANOHUB_INT_NONWAKEUP, .minSamples = 20},
    {.sensorName = "Step Detector",                .sensorType = SENS_TYPE_STEP_DETECT, .numAxis = NUM_AXIS_EMBEDDED, .interrupt = NANOHUB_INT_WAKEUP, .minSamples = 20},
};

static bool stepRecognitionPower(bool on, void *cookie)
{
    SensorAttribute_t *mSensor = &mTask.sensors[(int)cookie];
    static bool prev_any_on = false;
    mSensor->active = on;
    if (((int)cookie == 1) && mTask.sensors[STEP_COUNTER].active) mArg.step_counter_get_start_timing = false;
    bool any_on = (mTask.sensors[SMD].active || mTask.sensors[STEP_COUNTER].active || mTask.sensors[STEP_DETECTOR].active);
    osLog(LOG_INFO, "STEP_RECOGNITION: Power: idx:%d, prev_any_on:%d, any_on:%d, smd:%d, counter:%d, detector:%d\n",
          mSensor->idx, prev_any_on, any_on, mTask.sensors[SMD].active, mTask.sensors[STEP_COUNTER].active,
          mTask.sensors[STEP_DETECTOR].active);

    //Initialize only when an enable sensor is the first registered sensor
    if ((!prev_any_on) && (any_on)) {
        algoInit();
        configAccel(true);
#ifdef _ENABLE_MOTION_MONITORING_
        configAnyMotion(true);
        configNoMotion(true);
#endif
        mTask.taskState = STATE_INIT;
    } else if ((prev_any_on) && (!any_on)) {
        set_pedometer_transition(DISABLE);
        update_step_result(mResample.algo_time_stamp);
#ifdef _ENABLE_MOTION_MONITORING_
        configAnyMotion(false);
        configNoMotion(false);
#endif
        configAccel(false);
        mTask.taskState = STATE_DISABLED;
    }
    prev_any_on = any_on;
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool stepRecognitionSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    SensorAttribute_t *mSensor = &mTask.sensors[(int)cookie];
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool stepRecognitionFirmwareUpload(void *cookie)
{
    SensorAttribute_t *mSensor = &mTask.sensors[(int)cookie];
    sensorSignalInternalEvt(mSensor->handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool stepRecognitionFlush(void *cookie)
{
    SensorAttribute_t *mSensor = &mTask.sensors[(int)cookie];
    return osEnqueueEvt(sensorGetMyEventType(mSi[mSensor->idx].sensorType), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void stepRecognitionHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    switch (evtType) {
        case EVT_APP_START:
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            osLog(LOG_INFO, "STEP_RECOGNITION EVT_APP_START\n");
            break;

#ifdef _ENABLE_MOTION_MONITORING_
        case EVT_SENSOR_ANY_MOTION:
            if ((mTask.taskState == STATE_AWAITING_ANY_MOTION)  || (mTask.taskState == STATE_INIT)) {
                osLog(LOG_INFO, "STEP_RECOGNITION: EVT_SENSOR_ANY_MOTION RECEIVED\n");
                set_pedometer_transition(NO_TO_ANY);
                configNoMotion(true);
                configAnyMotion(false);
                if (!mMotion.apply_sw_anymotion) {
                    configAccel(true);
                }
                mTask.taskState = STATE_AWAITING_STEP;
            }
            break;

        case EVT_SENSOR_NO_MOTION:
            if ((mTask.taskState == STATE_AWAITING_STEP)  || (mTask.taskState == STATE_INIT)) {
                osLog(LOG_INFO, "STEP_RECOGNITION: EVT_SENSOR_NO_MOTION RECEIVED\n");
                set_pedometer_transition(ANY_TO_NO);
                update_step_result(mResample.algo_time_stamp);
                forceReport();
                configAnyMotion(true);
                configNoMotion(false);
                if (!mMotion.apply_sw_anymotion) {
                    configAccel(false);
                }
                mTask.taskState = STATE_AWAITING_ANY_MOTION;
            }
            break;
#endif

        case EVT_SENSOR_ACCEL:
            mark_timestamp(SENS_TYPE_ACCEL, SENS_TYPE_STEP_COUNT, ALGO_BEGIN, rtcGetTime());
            if (stepRecognitionAlgo((struct TripleAxisDataEvent *)evtData)) {
#ifdef _STEP_COUNTER_
                if (mTask.sensors[STEP_COUNTER].active) {
                    forceReport();
                }
#endif
            }
            mark_timestamp(SENS_TYPE_ACCEL, SENS_TYPE_STEP_COUNT, ALGO_DONE, rtcGetTime());
            break;
    }
}

static const struct SensorOps mSops = {
    .sensorPower = stepRecognitionPower,
    .sensorFirmwareUpload = stepRecognitionFirmwareUpload,
    .sensorSetRate = stepRecognitionSetRate,
    .sensorFlush = stepRecognitionFlush,
};

static bool stepRecognitionStart(uint32_t taskId)
{
    size_t i;
    mTask.taskId = taskId;
    uint32_t handle = 0;
    for (i = 0; i < NUM_OF_STEP_RECOGNITION; i++) {
        mTask.sensors[i].idx = i;
        if (sensorFind(mSi[i].sensorType, 0, &handle) == NULL)
            mTask.sensors[i].handle = sensorRegister(&mSi[i], &mSops, (void *)i, true);
    }
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void stepRecognitionEnd()
{
}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_STEP_COUNT, 0, 0)), 0, stepRecognitionStart,
                  stepRecognitionEnd, stepRecognitionHandleEvent);

