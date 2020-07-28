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
#include "tilt.h"

/* Math defines */
#define EVT_SENSOR_TILT         sensorGetMyEventType(SENS_TYPE_TILT)
#define EVT_SENSOR_PICK_UP      sensorGetMyEventType(SENS_TYPE_PICK_UP)

#define PICK_DEBUG_LOG

// ----------------------------------------------
// 1. One-shot for SENS_TYPE_PICK_UP
// 2. Use tilt event
// ----------------------------------------------

struct PickTask {
    uint32_t taskId;
    uint32_t handle;
    uint32_t tiltHandle;
    enum {
        STATE_DISABLED,
        STATE_INIT
    } taskState;
};

static struct PickTask mTask;

static void configTilt(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.tiltHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_TILT, i, &mTask.tiltHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.tiltHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osLog(LOG_INFO, "PICKUP: SUBSCRIBE TILT SENSOR DONE\n");
                osEventSubscribe(mTask.taskId, EVT_SENSOR_TILT);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_TILT, i, &mTask.tiltHandle) == NULL) {
            osLog(LOG_DEBUG, "[WARNING] PICKUP: TILT SNEOSR IS NOT AVAILABLE\n");
        }
    } else if ((on == false) && (mTask.tiltHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.tiltHandle);
        mTask.tiltHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_TILT);
    }
}

static bool PickPower(bool on, void *cookie)
{
#ifdef PICK_DEBUG_LOG
    osLog(LOG_INFO, "[Pick] Active pick(%d)\n", on);
#endif
    if (on) {
        configTilt(true);
        mTask.taskState = STATE_INIT;
    } else {
        configTilt(false);
        mTask.taskState = STATE_DISABLED;
    }
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}

static bool PickSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}

static bool PickFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask .handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool PickFlush(void *cookie)
{
    return osEnqueueEvt(EVT_SENSOR_PICK_UP, SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void PickHandleEvent(uint32_t evtType, const void* evtData)
{
    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    switch (evtType) {
        case EVT_APP_START:
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            break;
        case EVT_SENSOR_TILT: {
            union EmbeddedDataPoint sample;
#ifdef PICK_DEBUG_LOG
            osLog(LOG_DEBUG, "[Pick] Pick-up notify\n");
#endif
            sample.idata = 1;
            osEnqueueEvt(EVT_SENSOR_PICK_UP, sample.vptr, NULL);
            break;
        }
        default:
            break;
    }
}

static const struct SensorInfo mSi = {
    .sensorName = "Pick up",
    .sensorType = SENS_TYPE_PICK_UP,
    .numAxis = NUM_AXIS_EMBEDDED,
    .interrupt = NANOHUB_INT_WAKEUP,
    .minSamples = 20
};

static const struct SensorOps mSops = {
    .sensorPower = PickPower,
    .sensorFirmwareUpload = PickFirmwareUpload,
    .sensorSetRate = PickSetRate,
    .sensorFlush = PickFlush
};

static bool PickStart(uint32_t taskId)
{
    mTask.taskId = taskId;
    mTask.handle = sensorRegister(&mSi, &mSops, NULL, true);
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void PickEnd()
{

}

INTERNAL_APP_INIT(
    APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_PICK_UP, 0, 0)),
    0,
    PickStart,
    PickEnd,
    PickHandleEvent
);
