#include <stdio.h>
#include "algoConfig.h"
#include "activity_algorithm.h"
#include "init_learning.h"
#include "signal.h"
#include "context.h"
#include "context_setting.h"
#include "context_setting_private.h"
#include "context_private.h"
#include "tilt.h"
#include "math_method.h"
#include "FreeRTOS.h"
#include "contexthub_fw.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <slab.h>
#include <plat/inc/rtc.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <seos.h>
#include <nanohub_math.h>
#include <sensors.h>
#include <limits.h>

#define EVT_SENSOR_ACCEL      sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_TILT       sensorGetMyEventType(SENS_TYPE_TILT)
#define EVT_SENSOR_ANY_MOTION sensorGetMyEventType(SENS_TYPE_ANY_MOTION)
#define EVT_SENSOR_NO_MOTION  sensorGetMyEventType(SENS_TYPE_NO_MOTION)


#define MAX_NUM_COMMS_EVENT_SAMPLES 15
#define TILT_MIN_RATE    SENSOR_HZ(1)
#define TILT_MAX_LATENCY 1000000000ull   // 1000 ms
//#define DEBUG_LOG

struct activity_adaptor_t {
    uint32_t notifity_data_count;
    uint32_t prev_set_data_time;
    uint64_t min_update_interval;
    uint32_t set_data_timing;
    int32_t input_state;

    // tilt part
    uint32_t last_tilt_time;
    uint32_t tilt_event_state;
};

static struct activityDetectionTask {
    uint32_t taskId;
    uint32_t handle;
    uint32_t anyMotionHandle;
    uint32_t noMotionHandle;
    uint32_t accelHandle;
    uint32_t tiltHandle;
    uint64_t prev_rtc_time;
    struct SlabAllocator *mDataSlab;
    struct TripleAxisDataEvent *data_evt;
    enum {
        STATE_DISABLED,
        STATE_AWAITING_ANY_MOTION,
        STATE_AWAITING_ACTIVITY,
        STATE_INIT
    } taskState;
} mTask;

static uint64_t last_update_timestamp = 0;
static resampling_t mResample;
static Activity_algorithm_output_t act_output;
static activity_type_t act_last_state, act_temp_state;
static struct activity_adaptor_t activity_adaptor;

static void activity_adaptor_init()
{
    activity_adaptor.notifity_data_count    = 0;
    activity_adaptor.prev_set_data_time     = 0;
    activity_adaptor.set_data_timing        = ACTIVITY_FEATURE_UPDATE_TIME;
    activity_adaptor.input_state            = INPUT_STATE_INIT;
    activity_adaptor.last_tilt_time         = 0;
    activity_adaptor.tilt_event_state       = TILT_INIT_STATE;
}

static void algoInit()
{
    uint64_t init_time = rtcGetTime() / TIME_NANO_TO_MILLI;
    activity_algorithm_init(init_time);
    act_last_state = UNKNOWN;
    act_temp_state = UNKNOWN;
    memset(&act_output, 0, sizeof(Activity_algorithm_output_t));
    activity_adaptor_init();
    init_data_resampling(&mResample, ACTIVITY_INPUT_SAMPLE_DELAY);
#ifdef DEBUG_LOG
    osLog(LOG_INFO, "ACTIVITY: INIT DONE\n");
#endif
}

static void set_tilt_data()
{
    // tilt event trigger TILT_HOLD for TILT_HOLD_TIME
    if (activity_adaptor.tilt_event_state == TILT_TRIGGERED) {
        activity_adaptor.tilt_event_state = TILT_HOLD;
        activity_adaptor.last_tilt_time = mResample.algo_time_stamp;
        act_output.tilt = DOMINATE_CONFIDENCE;
        act_output.unknown = 0;
    }
    // if TILT_HOLD and exceed the latency, trigger UNKNOWN_HOLD
    else if ((activity_adaptor.tilt_event_state == TILT_HOLD) &&
             (mResample.algo_time_stamp - activity_adaptor.last_tilt_time) > TILT_HOLD_TIME) {
        activity_adaptor.tilt_event_state = UNKNOWN_HOLD;
        act_output.tilt = 0;
        act_output.unknown = DOMINATE_CONFIDENCE;
    }
    // the UNKNOWN_HOLD state remains unknown for extra UNKNOWN_HOLD_TIME ms
    else if ((activity_adaptor.tilt_event_state == UNKNOWN_HOLD) &&
             (mResample.algo_time_stamp - activity_adaptor.last_tilt_time) > TILT_HOLD_TIME + UNKNOWN_HOLD_TIME) {
        activity_adaptor.tilt_event_state = TILT_INIT_STATE;
        act_output.unknown = 0;
    }
#ifdef DEBUG_LOG
    osLog(LOG_INFO, "ACTIVITY: REPORT TILT & UNKNOWN:%u, %u, %u, %u\n", act_output.tilt, act_output.unknown,
          mResample.algo_time_stamp, activity_adaptor.last_tilt_time);
#endif
}

static void run_activity_algorithm()
{
    if (activity_adaptor.input_state == INPUT_STATE_INIT) {
        activity_adaptor.prev_set_data_time = mResample.algo_time_stamp;
        activity_adaptor.input_state = INPUT_STATE_PREPARE_DATA;
    }

    int32_t time_diff;
    time_diff = mResample.algo_time_stamp - activity_adaptor.prev_set_data_time;

    if ((activity_adaptor.input_state == INPUT_STATE_PREPARE_DATA) ||
            ((activity_adaptor.input_state == INPUT_STATE_UPDATE) && (time_diff >= activity_adaptor.set_data_timing))) {
        activity_adaptor.input_state = INPUT_STATE_UPDATE;
        activity_adaptor.prev_set_data_time = mResample.algo_time_stamp;
        context_alg_enter_point();
    }
}

static bool algoUpdate(uint32_t evtType, const void* evtData)
{
#ifdef DEBUG_LOG
    uint64_t start_time = rtcGetTime();
    uint64_t end_time;
#endif

    if ((evtType == EVT_SENSOR_TILT) || (activity_adaptor.tilt_event_state == TILT_HOLD)
            || (activity_adaptor.tilt_event_state == UNKNOWN_HOLD)) {
        if (evtType == EVT_SENSOR_TILT) {
            activity_adaptor.tilt_event_state = TILT_TRIGGERED;
        }
        set_tilt_data();
    }

    if (evtType == EVT_SENSOR_ACCEL) {
        struct TripleAxisDataEvent* sampleRoot = (struct TripleAxisDataEvent*)evtData;
        struct TripleAxisDataPoint* sample;
        uint64_t input_time_stamp_ms = sampleRoot->referenceTime / TIME_NANO_TO_MILLI;
        uint32_t count = sampleRoot->samples[0].firstSample.numSamples;
        uint32_t i;

        for (i = 0; i < count; i++) {
            sample = &sampleRoot->samples[i];
            if (i > 0) {
                input_time_stamp_ms += sample->deltaTime / TIME_NANO_TO_MILLI;
            }

            sensor_input_data_resampling_preparation(&mResample, input_time_stamp_ms, evtType);
            mResample.currentAccData = (AccelData_t) {
                (int32_t)(sample->x * 1000), (int32_t)(sample->y * 1000), (int32_t)(sample->z * 1000)
            };
            while (mResample.input_count > 0) {
                algo_input_data_generation(&mResample, evtType);
                Activity_Accelerometer_Ms2_Data_t algoAccInput = {mResample.algo_time_stamp, mResample.algoAccData.x, mResample.algoAccData.y, mResample.algoAccData.z};
                get_signal_acc(algoAccInput); /* using address acc_buff to update acc measurements */
                run_activity_algorithm();
                mResample.input_count--;
            }
#ifdef DEBUG_LOG
            osLog(LOG_DEBUG, "ACTIVITY_UPDATE:%d, %llu, %u, %llu, %d, %d, %d\n", count, ev->referenceTime / TIME_NANO_TO_MILLI,
                  sample->deltaTime / TIME_NANO_TO_MILLI, input_time_stamp_ms, acc_input.x, acc_input.y, acc_input.z);
#endif
        }

#ifdef DEBUG_LOG
        end_time = rtcGetTime();
        osLog(LOG_INFO, "ACTIVITY: execution time=%llu\n", end_time - start_time);
#endif
    }
    uint64_t current_timestamp = rtcGetTime();
    if ((current_timestamp - last_update_timestamp) > activity_adaptor.min_update_interval) {
        last_update_timestamp = current_timestamp;
        return 1;
    } else {
        return 0;
    }
}

static void configAccel(bool on)
{
    uint32_t i;
    bool getAccelSensor = false;
    if ((on == true) && (mTask.accelHandle == 0)) {
        osLog(LOG_INFO, "ACTIVITY: SUBSCRIBE ACCEL SENSOR DONE\n");
        for (i = 0; sensorFind(SENS_TYPE_ACCEL, i, &mTask.accelHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.accelHandle, ACCEL_MIN_RATE, ACCEL_EXTENDED_LATENCY)) {
                getAccelSensor = true;
                osEventSubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
                break;
            }
        }
        if (!getAccelSensor) {
            osLog(LOG_DEBUG, "[WARNING] ACTIVITY: ACCEL SNEOSR IS NOT AVAILABLE\n");
        }
    } else if ((on == false) && (mTask.accelHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.accelHandle);
        mTask.accelHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_ACCEL);
    }
}


static void configTilt(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.tiltHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_TILT, i, &mTask.tiltHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.tiltHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osLog(LOG_INFO, "ACTIVITY: SUBSCRIBE TILT SENSOR DONE\n");
                osEventSubscribe(mTask.taskId, EVT_SENSOR_TILT);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_TILT, i, &mTask.tiltHandle) == NULL) {
            osLog(LOG_DEBUG, "[WARNING] ACTIVITY: TILT SNEOSR IS NOT AVAILABLE\n");
        }
    } else if ((on == false) && (mTask.tiltHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.tiltHandle);
        mTask.tiltHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_TILT);
    }
}

// *****************************************************************************
static bool activityDetectionPower(bool on, void *cookie)
{
#ifdef DEBUG_LOG
    osLog(LOG_DEBUG, "ACTIVITY POWER:%d\n", on);
#endif
    if (on) {
        algoInit();
        configAccel(true);
        configTilt(true);
        mTask.taskState = STATE_INIT;
    } else {
        configAccel(false);
        configTilt(false);
        mTask.taskState = STATE_DISABLED;
    }

    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool activityDetectionSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    activity_adaptor.min_update_interval = (TIME_SEC_TO_NANO / ((rate * RATE_PRESERVE) / RATE_SCALING)) * RATE_PRESERVE;
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool activityDetectionFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool activityDetectionFlush(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_ACTIVITY), SENSOR_DATA_EVENT_FLUSH, NULL);
}

// Returns temperature in units of 0.01 degrees celsius.

static void dataEvtFree(void *ptr)
{
    struct TripleAxisDataEvent *ev = (struct TripleAxisDataEvent *)ptr;
    slabAllocatorFree(mTask.mDataSlab, ev);
}

static void parseRawData(const uint8_t *activityData, uint64_t timeStamp)
{
    struct TripleAxisDataPoint *sample;
    uint32_t delta_time = 0;

    if (mTask.data_evt == NULL) {
        mTask.data_evt = slabAllocatorAlloc(mTask.mDataSlab);
        if (mTask.data_evt == NULL) {
            // slab allocation failed
            osLog(LOG_ERROR, "activity: Slab allocation failed\n");
            return;
        }
        // delta time for the first sample is sample count
        mTask.data_evt->samples[0].firstSample.numSamples = 0;
        mTask.data_evt->samples[0].firstSample.biasCurrent = 0;
        mTask.data_evt->samples[0].firstSample.biasPresent = 0;
        mTask.data_evt->samples[0].firstSample.biasSample = 0;
        mTask.data_evt->referenceTime = timeStamp;
        mTask.prev_rtc_time = timeStamp;
    }
    if (mTask.data_evt->samples[0].firstSample.numSamples >= MAX_NUM_COMMS_EVENT_SAMPLES) {
        osLog(LOG_ERROR, "activity parseRawData BAD INDEX\n");
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
#ifdef DEBUG_LOG
    osLog(LOG_ERROR, "activity: numSamples:%d, deltaTime:%d, rtc_time:%lld, prev_rtc_time:%lld\n",
          mTask.data_evt->samples[0].firstSample.numSamples, sample->deltaTime,
          timeStamp, mTask.prev_rtc_time);
#endif
    activityDataToTripleAxisData(activityData, sizeof(activityData), sample);
    if (mTask.data_evt) {
        if (!osEnqueueEvt(EVENT_TYPE_BIT_DISCARDABLE | sensorGetMyEventType(SENS_TYPE_ACTIVITY),
                          mTask.data_evt, dataEvtFree)) {
            // don't log error since queue is already full. silently drop
            // this data event.
            dataEvtFree(mTask.data_evt);
        }
        mTask.data_evt = NULL;
    }
}

static void activityReportResult()
{
    context_get_alg_result(&act_output);
#ifdef DEBUG_LOG
    osLog(LOG_INFO, "ACTIVITY OUTPUT: still:%d, cycle:%d, walk:%d, run:%d, stairs:%d, veh:%d, tilt:%d, unknown:%d\n",
          act_output.still,
          act_output.cycle, act_output.walk, act_output.run, act_output.stairs, act_output.veh, act_output.tilt,
          act_output.unknown);
#endif
    if (act_output.tilt || act_output.unknown) {
        act_output.still = act_output.cycle = act_output.walk = act_output.run = \
                                              act_output.stairs = act_output.veh = 0;
    }
    uint8_t _activityData[AR_ACTIVITY_MAX] = {0};
    uint8_t activityData[ACTIVITY_MAX] = {0};
    _activityData[AR_STILL]      = act_output.still;
    _activityData[AR_ON_BICYCLE] = act_output.cycle;
    _activityData[AR_WALKING]    = act_output.walk;
    _activityData[AR_RUNNING]    = act_output.run;
    _activityData[AR_CLIMBING]   = act_output.stairs;
    _activityData[AR_IN_VEHICLE] = act_output.veh;
    _activityData[AR_TILTING]    = act_output.tilt;
    _activityData[AR_UNKNOWN]    = act_output.unknown;
    _activityData[AR_ON_FOOT] = (uint8_t) max3(act_output.walk, act_output.run, act_output.stairs);

    activityData[STILL] = _activityData[AR_STILL];
    activityData[ON_BICYCLE] = _activityData[AR_ON_BICYCLE];
    activityData[WALKING] = _activityData[AR_WALKING];
    activityData[RUNNING] = _activityData[AR_RUNNING];
    activityData[CLIMBING] = _activityData[AR_CLIMBING];
    activityData[IN_VEHICLE] = _activityData[AR_IN_VEHICLE];
    activityData[TILTING]    = _activityData[AR_TILTING];
    activityData[UNKNOWN]    = _activityData[AR_UNKNOWN];
    activityData[ON_FOOT] = _activityData[AR_ON_FOOT];

    parseRawData(activityData, rtcGetTime());
}

static void activityDetectionHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    switch (evtType) {
        case EVT_APP_START:
            osLog(LOG_INFO, "ACTIVITY EVT_APP_START\n");
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            break;

        case EVT_SENSOR_ACCEL:
            if (algoUpdate(evtType, evtData)) {
                activityReportResult();
            }
            break;

        case EVT_SENSOR_TILT:
            osLog(LOG_INFO, "ACTIVITY: RECEIVE EVT_SENSOR_TILT\n");
            if (algoUpdate(evtType, evtData)) {
                activityReportResult();
            }
            break;

        default:
            break;
    }
}
static const uint32_t actSupportedRates[] = {
    SENSOR_HZ(0.2f),
    SENSOR_HZ(0.5f),
    SENSOR_HZ(1.0f),
    SENSOR_HZ(2.0f),
    SENSOR_HZ(5.0f),
};

static const struct SensorInfo mSi = {
    .sensorName = "Activity",
    .sensorType = SENS_TYPE_ACTIVITY,
    .numAxis = NUM_AXIS_THREE,
    .interrupt = NANOHUB_INT_WAKEUP,
    .supportedRates = actSupportedRates,
    .minSamples = 600
};

static const struct SensorOps mSops = {
    .sensorPower = activityDetectionPower,
    .sensorFirmwareUpload = activityDetectionFirmwareUpload,
    .sensorSetRate = activityDetectionSetRate,
    .sensorFlush = activityDetectionFlush,
};

static bool activityDetectionStart(uint32_t taskId)
{
    size_t slabSize;

    slabSize = sizeof(struct TripleAxisDataEvent) +
               MAX_NUM_COMMS_EVENT_SAMPLES * sizeof(struct TripleAxisDataPoint);

    mTask.mDataSlab = slabAllocatorNew(slabSize, 4, 5);
    if (!mTask.mDataSlab) {
        osLog(LOG_INFO, "Slab allocation failed\n");
        return false;
    }
    mTask.data_evt = NULL;
    mTask.taskId = taskId;
    mTask.handle = sensorRegister(&mSi, &mSops, NULL, true);
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}
static void activityDetectionEnd()
{
}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_ACTIVITY, 0, 0)), 0, activityDetectionStart,
                  activityDetectionEnd, activityDetectionHandleEvent);
