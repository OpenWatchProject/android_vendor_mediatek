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
#include "tilt_common.h"
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
#include <performance.h>

#define EVT_SENSOR_ACCEL      sensorGetMyEventType(SENS_TYPE_ACCEL)
#define EVT_SENSOR_BARO       sensorGetMyEventType(SENS_TYPE_BARO)

#define MAX_NUM_COMMS_EVENT_SAMPLES 15
//#define DEBUG_LOG
//#define ADOPT_BARO

static const uint32_t actSupportedRates[] = {
    SENSOR_HZ(0.2f),
    SENSOR_HZ(0.5f),
    SENSOR_HZ(1.0f),
    SENSOR_HZ(2.0f),
    SENSOR_HZ(5.0f),
};

static struct activityDetectionTask {
    uint32_t taskId;
    uint32_t handle;
    uint32_t accelHandle;
    uint32_t baroHandle;
    uint64_t prev_rtc_time;
    uint32_t tilt_algo_init;
    uint32_t prev_set_data_time;
    uint64_t min_update_interval;
    uint32_t set_data_timing;
    int32_t input_state;
    uint32_t last_tilt_time;
    uint32_t tilt_event_state;
    uint64_t last_update_timestamp;
    struct SlabAllocator *mDataSlab;
    struct TripleAxisDataEvent *data_evt;
    enum {
        STATE_DISABLED,
        STATE_AWAITING_ANY_MOTION,
        STATE_AWAITING_ACTIVITY,
        STATE_INIT
    } taskState;
} mTask;

static resampling_t mResample_acc;
static resampling_t mResample_baro;
static Activity_algorithm_output_t act_output;
extern uint8_t activity_debug_trace;

static void algoInit()
{
    uint64_t init_time = rtcGetTime() / TIME_NANO_TO_MILLI;
    activity_algorithm_init(init_time);
    activity_pararmeter_init();
    memset(&act_output, 0, sizeof(Activity_algorithm_output_t));
    init_data_resampling(&mResample_acc, ACTIVITY_INPUT_SAMPLE_DELAY);
    init_data_resampling(&mResample_baro, ACTIVITY_INPUT_BARO_SAMPLE_DELAY);
    mTask.set_data_timing        = ACTIVITY_FEATURE_UPDATE_TIME;
    mTask.input_state            = INPUT_STATE_INIT;
    mTask.tilt_event_state       = TILT_INIT_STATE;
    mTask.min_update_interval    = 10000000000ull;
    mTask.prev_set_data_time     = 0;
    mTask.last_tilt_time         = 0;
    mTask.last_update_timestamp  = 0;
    osLog(LOG_INFO, "ACTIVITY: INIT DONE\n");
}

static void set_tilt_data()
{
    // tilt event trigger TILT_HOLD for TILT_HOLD_TIME
    if (mTask.tilt_event_state == TILT_TRIGGERED) {
        mTask.tilt_event_state = TILT_HOLD;
        mTask.last_tilt_time = mResample_acc.algo_time_stamp;
        act_output.tilt = DOMINATE_CONFIDENCE;
        act_output.unknown = 0;
    }
    // if TILT_HOLD and exceed the latency, trigger UNKNOWN_HOLD
    else if ((mTask.tilt_event_state == TILT_HOLD) &&
             (mResample_acc.algo_time_stamp - mTask.last_tilt_time) > TILT_HOLD_TIME) {
        mTask.tilt_event_state = UNKNOWN_HOLD;
        act_output.tilt = 0;
        act_output.unknown = DOMINATE_CONFIDENCE;
    }
    // the UNKNOWN_HOLD state remains unknown for extra UNKNOWN_HOLD_TIME ms
    else if ((mTask.tilt_event_state == UNKNOWN_HOLD) &&
             (mResample_acc.algo_time_stamp - mTask.last_tilt_time) > TILT_HOLD_TIME + UNKNOWN_HOLD_TIME) {
        mTask.tilt_event_state = TILT_INIT_STATE;
        act_output.unknown = 0;
    }
}

static void run_activity_algorithm()
{
    if (mTask.input_state == INPUT_STATE_INIT) {
        mTask.prev_set_data_time = mResample_acc.algo_time_stamp;
        mTask.input_state = INPUT_STATE_PREPARE_DATA;
    }

    int32_t time_diff;
    time_diff = mResample_acc.algo_time_stamp - mTask.prev_set_data_time;

    if ((mTask.input_state == INPUT_STATE_PREPARE_DATA) ||
            ((mTask.input_state == INPUT_STATE_UPDATE) && (time_diff >= mTask.set_data_timing))) {
        mTask.input_state = INPUT_STATE_UPDATE;
        mTask.prev_set_data_time = mResample_acc.algo_time_stamp;
        tilt_set_ts(mResample_acc.algo_time_stamp);
        if (!mTask.tilt_algo_init) {
            tilt_original_reinit();
            mTask.tilt_algo_init = 1;
        }
        context_alg_enter_point();
        tilt_alg_enter_point();

        if ((*get_tilt_result_ptr(0)) || (mTask.tilt_event_state == TILT_HOLD)
                || (mTask.tilt_event_state == UNKNOWN_HOLD)) {
            if (*get_tilt_result_ptr(0)) {
                mTask.tilt_event_state = TILT_TRIGGERED;
                get_tilt_result_ptr(1);
            }
            set_tilt_data();
        }
    }
}

static bool algoUpdate(uint32_t evtType, const void* evtData)
{
    if (evtType == EVT_SENSOR_ACCEL) {
        struct TripleAxisDataEvent* sampleRoot = (struct TripleAxisDataEvent*)evtData;
        struct TripleAxisDataPoint* sample;
        uint32_t input_time_stamp_ms = sampleRoot->referenceTime / TIME_NANO_TO_MILLI;
        uint32_t count = sampleRoot->samples[0].firstSample.numSamples;
        uint32_t i;

        for (i = 0; i < count; i++) {
            sample = &sampleRoot->samples[i];
            if (i > 0) {
                input_time_stamp_ms += sample->deltaTime / TIME_NANO_TO_MILLI;
            }

            sensor_input_data_resampling_preparation(&mResample_acc, input_time_stamp_ms, evtType);
            mResample_acc.currentAccData = (AccelData_t) {
                (int32_t)(sample->x * 1000), (int32_t)(sample->y * 1000), (int32_t)(sample->z * 1000)
            };

            if (activity_debug_trace) {
                static uint32_t accumulated_acc_count = 0;
                accumulated_acc_count++;
                int32_t checksum = input_time_stamp_ms + (int32_t)(sample->x * 1000) + (int32_t)(sample->y * 1000) + (int32_t)(
                                       sample->z * 1000) + accumulated_acc_count;
                osLog(LOG_DEBUG, "Macc:%u,%d,%d,%d,%u,%d\n", input_time_stamp_ms, (int32_t)(sample->x * 1000),
                      (int32_t)(sample->y * 1000), (int32_t)(sample->z * 1000), accumulated_acc_count, checksum);
            }

            while (mResample_acc.input_count > 0) {
                algo_input_data_generation(&mResample_acc, evtType);
                Activity_Accelerometer_Ms2_Data_t algoAccInput = {mResample_acc.algo_time_stamp, mResample_acc.algoAccData.x, mResample_acc.algoAccData.y, mResample_acc.algoAccData.z};
                get_signal_acc(algoAccInput); /* using address acc_buff to update acc measurements */
                Directional_Sensor_Ms2_Data_t dirAccInput = {mResample_acc.algo_time_stamp, mResample_acc.algoAccData.x, mResample_acc.algoAccData.y, mResample_acc.algoAccData.z};
                get_signal_dir_acc(dirAccInput);
                run_activity_algorithm();
                mResample_acc.input_count--;
            }
        }
    }

#ifdef ADOPT_BARO
    if (evtType == EVT_SENSOR_BARO) {
        uint32_t input_time_stamp_ms = rtcGetTime() / TIME_NANO_TO_MILLI;
        union EmbeddedDataPoint ev;
        ev.idata = (uint32_t)evtData;

        sensor_input_data_resampling_preparation(&mResample_baro, input_time_stamp_ms, evtType);
        mResample_baro.currentBaroData = (int32_t)(ev.fdata * 100);

        if (activity_debug_trace) {
            static uint32_t accumulated_baro_count = 0;
            accumulated_baro_count++;
            osLog(LOG_DEBUG, "A_baro:%u,%d,%u\n", input_time_stamp_ms, (int32_t)(ev.fdata * 100), accumulated_baro_count);
        }

        while (mResample_baro.input_count > 0) {
            algo_input_data_generation(&mResample_baro, evtType);
            Activity_Barometer_Data_t algoBaroInput = {mResample_baro.algo_time_stamp, mResample_baro.algoBaroData};
            get_signal_baro(algoBaroInput); /* using address baro_buff to update baro measurements */
            mResample_baro.input_count--;
        }
    }
#endif

    uint64_t current_timestamp = rtcGetTime();
    if ((current_timestamp - mTask.last_update_timestamp) > mTask.min_update_interval) {
        mTask.last_update_timestamp = current_timestamp;
        return 1;
    } else {
        return 0;
    }
}

static void configAccel(bool on)
{
    uint32_t i;
    bool getAccelSensor = false;
    uint64_t acc_batch_interval = 0;
    if ((on == true) && (mTask.accelHandle == 0)) {
        osLog(LOG_INFO, "ACTIVITY: SUBSCRIBE ACCEL SENSOR DONE\n");
        for (i = 0; sensorFind(SENS_TYPE_ACCEL, i, &mTask.accelHandle) != NULL; i++) {
            acc_batch_interval = (mTask.min_update_interval > ACCEL_EXTENDED_LATENCY) ? ACCEL_EXTENDED_LATENCY :
                                 mTask.min_update_interval;
            if (sensorRequest(mTask.taskId, mTask.accelHandle, ACCEL_MIN_RATE, acc_batch_interval)) {
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

#ifdef ADOPT_BARO
static void configBaro(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.baroHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_BARO, i, &mTask.baroHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.baroHandle, BARO_MIN_RATE, BARO_LATENCY)) {
                osLog(LOG_INFO, "ACTIVITY: SUBSCRIBE BARO SENSOR DONE\n");
                osEventSubscribe(mTask.taskId, EVT_SENSOR_BARO);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_BARO, i, &mTask.baroHandle) == NULL) {
            osLog(LOG_DEBUG, "[WARNING] ACTIVITY: BARO SNEOSR IS NOT AVAILABLE\n");
        }
    } else if ((on == false) && (mTask.baroHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.baroHandle);
        mTask.baroHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_BARO);
    }
}
#endif

// *****************************************************************************
static bool activityDetectionPower(bool on, void *cookie)
{

    osLog(LOG_DEBUG, "ACTIVITY POWER:%d\n", on);

    if (on) {
        algoInit();
        configAccel(true);
#ifdef ADOPT_BARO
        configBaro(true);
#endif
        mTask.taskState = STATE_INIT;
    } else {
#ifdef ADOPT_BARO
        configBaro(false);
#endif
        configAccel(false);
        mTask.taskState = STATE_DISABLED;
        mTask.tilt_algo_init = 0;
    }

    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool activityDetectionSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    uint32_t i;
    uint32_t supportRateClass = sizeof(actSupportedRates) / sizeof(actSupportedRates[0]);
    for (i = 0; i < supportRateClass - 1; i++) {
        if (rate <= actSupportedRates[0]) {
            rate = actSupportedRates[0];
        } else if ((rate > actSupportedRates[i]) && (rate <= actSupportedRates[i + 1])) {
            rate = actSupportedRates[i + 1];
        } else if (rate >= actSupportedRates[supportRateClass - 1]) {
            rate = actSupportedRates[supportRateClass - 1];
        }
    }
    mTask.min_update_interval = (TIME_SEC_TO_NANO / ((rate * RATE_PRESERVE) / RATE_SCALING)) * RATE_PRESERVE;
    configAccel(false);
    configAccel(true);
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
    context_get_alg_result(&act_output, mTask.min_update_interval);
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
    _activityData[AR_ON_FOOT] = act_output.walk + act_output.run + act_output.stairs;

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
            mark_timestamp(SENS_TYPE_ACCEL, SENS_TYPE_ACTIVITY, ALGO_BEGIN, rtcGetTime());
            if (algoUpdate(evtType, evtData)) {
                activityReportResult();
            }
            mark_timestamp(SENS_TYPE_ACCEL, SENS_TYPE_ACTIVITY, ALGO_DONE, rtcGetTime());
            break;
#ifdef ADOPT_BARO
        case EVT_SENSOR_BARO:
            mark_timestamp(SENS_TYPE_BARO, SENS_TYPE_ACTIVITY, ALGO_BEGIN, rtcGetTime());
            if (algoUpdate(evtType, evtData)) {
                activityReportResult();
            }
            mark_timestamp(SENS_TYPE_BARO, SENS_TYPE_ACTIVITY, ALGO_DONE, rtcGetTime());
            break;
#endif
        default:
            break;
    }
}

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
