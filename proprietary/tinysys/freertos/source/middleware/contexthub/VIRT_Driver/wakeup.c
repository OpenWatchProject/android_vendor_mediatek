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

/* Math defines */
#define EVT_SENSOR_LIFT         sensorGetMyEventType(SENS_TYPE_LIFT)
#define EVT_SENSOR_WAKE_UP      sensorGetMyEventType(SENS_TYPE_WAKE_UP)

#define WAKE_DEBUG_LOG

// ----------------------------------------------
// 1. One-shot for SENS_TYPE_WAKE_UP
// 2. Use lift event
// ----------------------------------------------

struct WakeTask {
    uint32_t taskId;
    uint32_t handle;
    uint32_t liftHandle;
    enum {
        STATE_DISABLED,
        STATE_INIT
    } taskState;
};

static struct WakeTask mTask;

static void configLift(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.liftHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_LIFT, i, &mTask.liftHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.liftHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osLog(LOG_INFO, "WAKEUP: SUBSCRIBE LIFT SENSOR DONE\n");
                osEventSubscribe(mTask.taskId, EVT_SENSOR_LIFT);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_LIFT, i, &mTask.liftHandle) == NULL) {
            osLog(LOG_DEBUG, "[WARNING] WAKEUP: LIFT SNEOSR IS NOT AVAILABLE\n");
        }
    } else if ((on == false) && (mTask.liftHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.liftHandle);
        mTask.liftHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_LIFT);
    }
}

static bool WakePower(bool on, void *cookie)
{
#ifdef WAKE_DEBUG_LOG
    osLog(LOG_INFO, "[Wake] Active wake(%d)\n", on);
#endif
    if (on) {
        configLift(true);
        mTask.taskState = STATE_INIT;
    } else {
        configLift(false);
        mTask.taskState = STATE_DISABLED;
    }
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool WakeSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool WakeFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask .handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool WakeFlush(void *cookie)
{
    return osEnqueueEvt(EVT_SENSOR_WAKE_UP, SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void WakeHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    switch (evtType) {
        case EVT_APP_START:
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            break;
        case EVT_SENSOR_LIFT: {
            union EmbeddedDataPoint sample;
#ifdef PICK_DEBUG_LOG
            osLog(LOG_DEBUG, "[Wake] Wake-up notify\n");
#endif
            sample.idata = 1;
            osEnqueueEvt(EVT_SENSOR_WAKE_UP, sample.vptr, NULL);
            break;
        }
        default:
            break;
    }
}

static const struct SensorInfo mSi = {
    .sensorName = "Wake up",
    .sensorType = SENS_TYPE_WAKE_UP,
    .numAxis = NUM_AXIS_EMBEDDED,
    .interrupt = NANOHUB_INT_WAKEUP,
    .minSamples = 20
};

static const struct SensorOps mSops = {
    .sensorPower = WakePower,
    .sensorFirmwareUpload = WakeFirmwareUpload,
    .sensorSetRate = WakeSetRate,
    .sensorFlush = WakeFlush
};

static bool WakeStart(uint32_t taskId)
{
    mTask.taskId = taskId;
    mTask.handle = sensorRegister(&mSi, &mSops, NULL, true);
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void WakeEnd()
{

}

INTERNAL_APP_INIT(
    APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_WAKE_UP, 0, 0)),
    0,
    WakeStart,
    WakeEnd,
    WakeHandleEvent
);
