/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <string.h>
#include <timer.h>
#include <heap.h>
#include <plat/inc/rtc.h>
#include <hostIntf.h>
#include <nanohubPacket.h>
#include <seos.h>
#include <sensors.h>
#include <limits.h>
#include <slab.h>

#ifdef CFG_CCCI_SUPPORT
#include "ccci.h"
#endif

#define GEOFENCE_LOG
#ifdef GEOFENCE_LOG
#define GLOG(fmt, args...)    osLog(LOG_INFO, "[GF]: "fmt, ##args)
#else
#define GLOG(fmt, args...)
#endif

#define EVT_SENSOR_SIG_MOTION   sensorGetMyEventType(SENS_TYPE_SIG_MOTION)
#define EVT_SENSOR_NO_MOTION    sensorGetMyEventType(SENS_TYPE_NO_MOTION)

#ifdef CFG_CCCI_SUPPORT
enum MTK_MDM_CMD {
    MTK_MDM_DISABLE = 0x0, // default
    MTK_MDM_ENABLE_CHANGE_OF_CELL_ID, // enable phase I: only report when camping cell ID has change
    MTK_MDM_ENABLE_CHANGE_OF_CELL_RSSI, // enable phase II: report under phase I + report when camping cells RSSI has change
    MTK_MDM_CMD_END
};

typedef struct {
    unsigned short MDM_TYPE;
    unsigned short MCC;
    unsigned short NET;
    unsigned short AREA;
    unsigned short OTHER;
    unsigned short SIGNAL_STRENGTH;
    unsigned int CELL_ID;
    unsigned int isCamping;
} modem_vec_t;
#endif

enum MTK_GEOFENCE_STATUS {
    UNDEFINED = 0x00,
    STATIC,
    MOVING
};

enum MTK_GEOFENCE_SOURCE {
    MOTION_SENSOR = 0x00,
    MODEM
};

struct GeofenceArgument {
    uint8_t data_source;
    uint8_t status;
    uint8_t operation_state;
};

struct geofenceTask {
    uint32_t taskId;
    uint32_t handle;
    uint32_t sigMotionHandle;
    uint32_t noMotionHandle;
    enum {
        REBOOT_DONE,
        GEOFENCE_DISABLED,
        GEOFENCE_ENABLED
    } taskState;
};

#ifdef CFG_CCCI_SUPPORT
modem_vec_t modem_data1[10] = {{0}};
unsigned int current_cell_id = 0;
#endif

static struct geofenceTask mTask;
static struct GeofenceArgument GeoRes;

static const struct SensorInfo mSi = {
    .sensorName = "Geofence",
    .sensorType = SENS_TYPE_GEOFENCE,
    .numAxis = NUM_AXIS_EMBEDDED,
    .interrupt = NANOHUB_INT_WAKEUP,
    .minSamples = 20
};

#ifdef CFG_CCCI_SUPPORT
static int Geo_mdm_motion_detect(modem_vec_t *mdm_data)
{
    int ret = false;
    union EmbeddedDataPoint sample;
    if(mdm_data->CELL_ID != current_cell_id) {
        current_cell_id = mdm_data->CELL_ID;

        //trigger notification to host
        GeoRes.data_source = MODEM;
        GeoRes.status = MOVING;
        GeoRes.operation_state = mTask.taskState;
        sample.idata = (GeoRes.data_source <<8) | (GeoRes.status << 4) | (GeoRes.operation_state << 0);
        osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_GEOFENCE), sample.vptr, NULL);
        GLOG("cell notify \r\n");
        ret = true;
    }
    return ret;
}

static void GeoImpl_md1_handler(void *buffer)
{
    unsigned int mdm_size = 0;
    unsigned int mdm1_cnt = 0;
    int i;

    if(mTask.taskState == GEOFENCE_DISABLED) {
        GLOG("bypass redundant mdm data\n");
        return;
    }

    if(buffer != NULL) {
        memcpy(&mdm_size, (unsigned char *)buffer, sizeof(mdm_size));
        if((mdm_size< sizeof(modem_vec_t)) || (mdm_size > 10*sizeof(modem_vec_t)) || ((mdm_size % sizeof(modem_vec_t)) != 0)) {
            GLOG("incorrect mdm size, %d\n",mdm_size );
            return;
        }
        mdm1_cnt = mdm_size/sizeof(modem_vec_t);
        GLOG("mdm_size = %d. cnt = %d\n",mdm_size, mdm1_cnt);
        if (mdm1_cnt > 0) {
            memcpy(modem_data1, (unsigned char *)buffer+sizeof(unsigned int), mdm1_cnt*sizeof(modem_vec_t));
            for (i = 0; i < mdm1_cnt; i++) {
                if(i==0) {
                    Geo_mdm_motion_detect(modem_data1+i);
                }
                GLOG("mdm data %d,type:%d,mcc:%d,mnc:%d,lac:%d,signal:%d,cellid:%d,iscamping:%d\n", i, \
                     modem_data1[i].MDM_TYPE, modem_data1[i].MCC, \
                     modem_data1[i].NET, modem_data1[i].AREA, \
                     modem_data1[i].SIGNAL_STRENGTH, modem_data1[i].CELL_ID, modem_data1[i].isCamping);
            }
        }
    }
}
#endif

static void GeofenceConfigSigMotion(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.sigMotionHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_SIG_MOTION, i, &mTask.sigMotionHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.sigMotionHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osEventSubscribe(mTask.taskId, EVT_SENSOR_SIG_MOTION);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_SIG_MOTION, i, &mTask.sigMotionHandle) == NULL) {
            GLOG("sigMotion sensor N.A\n");
        }
    } else if ((on == false) && (mTask.sigMotionHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.sigMotionHandle);
        mTask.sigMotionHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_SIG_MOTION);
    }
}

static void GeofenceConfigNoMotion(bool on)
{
    uint32_t i;
    if ((on == true) && (mTask.noMotionHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_NO_MOTION, i, &mTask.noMotionHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.noMotionHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osEventSubscribe(mTask.taskId, EVT_SENSOR_NO_MOTION);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_NO_MOTION, i, &mTask.noMotionHandle) == NULL) {
            GLOG("noMotion sensor N.A\n");
        }
    } else if ((on == false) && (mTask.noMotionHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.noMotionHandle);
        mTask.noMotionHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_NO_MOTION);
    }
}

static bool GeofenceSetRate(uint32_t rate, uint64_t latency, void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_RATE_CHG, rate, latency);
    return true;
}


bool GeofencePower(bool on, void *cookie)
{
#ifdef CFG_CCCI_SUPPORT
    unsigned int mdm_command[2] = {0};
    int status = 0;
#endif

    if (on) {
        GeofenceConfigSigMotion(true);
        GeofenceConfigNoMotion(true);
        mTask.taskState = GEOFENCE_ENABLED;

#ifdef CFG_CCCI_SUPPORT
        //trigger cell init
        ccci_register_rx(MD_SYS1,CCCI_CELLINFO_CHANNEL_RX,GeoImpl_md1_handler);
        mdm_command[0] = sizeof(unsigned int);
        mdm_command[1] = MTK_MDM_ENABLE_CHANGE_OF_CELL_ID;
        status = ccci_geo_fence_send(MD_SYS1, mdm_command, sizeof(mdm_command));
        GLOG("cell start %d\n", status);
#endif
    } else {
        GeofenceConfigSigMotion(false);
        GeofenceConfigNoMotion(false);
        mTask.taskState = GEOFENCE_DISABLED;

#ifdef CFG_CCCI_SUPPORT
        //trigger cell de-init
        mdm_command[0] = sizeof(unsigned int);
        mdm_command[1] = MTK_MDM_DISABLE;
        status = ccci_geo_fence_send(MD_SYS1, mdm_command, sizeof(mdm_command));
        GLOG("cell stopped OK, %d\n",status);
#endif
    }
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_POWER_STATE_CHG, on, 0);
    return true;
}


static bool GeofenceFirmwareUpload(void *cookie)
{
    sensorSignalInternalEvt(mTask.handle, SENSOR_INTERNAL_EVT_FW_STATE_CHG, 1, 0);
    return true;
}

static bool GeofenceFlush(void *cookie)
{
    return osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_GEOFENCE), SENSOR_DATA_EVENT_FLUSH, NULL);
}

static void GeofenceHandleEvent(uint32_t evtType, const void* evtData)
{
  union EmbeddedDataPoint sample;

    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    switch (evtType) {
        case EVT_APP_START:
            osLog(LOG_INFO, "GEOFENCE EVT_APP_START\n");
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            break;

        case EVT_SENSOR_SIG_MOTION:
            if ((mTask.taskState == GEOFENCE_ENABLED)) {
                GeofenceConfigNoMotion(true);
                GeofenceConfigSigMotion(false);
                GeoRes.data_source = MOTION_SENSOR;
                GeoRes.status = MOVING;
                GeoRes.operation_state = mTask.taskState;
                sample.idata = (GeoRes.data_source <<8) | (GeoRes.status << 4) | (GeoRes.operation_state << 0);
                osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_GEOFENCE), sample.vptr, NULL);
                GLOG("sig motion notify\r\n");
            }
            break;

        case EVT_SENSOR_NO_MOTION:
            if ((mTask.taskState == GEOFENCE_ENABLED)) {
                GeofenceConfigSigMotion(true);
                GeofenceConfigNoMotion(false);
                GeoRes.data_source = MOTION_SENSOR;
                GeoRes.status = STATIC;
                GeoRes.operation_state = mTask.taskState;
                sample.idata = (GeoRes.data_source <<8) | (GeoRes.status << 4) | (GeoRes.operation_state << 0);
                osEnqueueEvt(sensorGetMyEventType(SENS_TYPE_GEOFENCE), sample.vptr, NULL);
                GLOG("no motion notify\r\n");
            }
            break;
    }
}

static const struct SensorOps mSops = {
    .sensorPower = GeofencePower,
    .sensorFirmwareUpload = GeofenceFirmwareUpload,
    .sensorSetRate = GeofenceSetRate,
    .sensorFlush = GeofenceFlush
};

static bool GeofenceStart(uint32_t taskId)
{
    osLog(LOG_INFO, "GEOFENCE start:  %ld\n", taskId);
    mTask.taskId = taskId;
    mTask.sigMotionHandle = 0;
    mTask.noMotionHandle = 0;
    mTask.taskState = REBOOT_DONE;
    mTask.handle = sensorRegister(&mSi, &mSops, NULL, true);
    osEventSubscribe(taskId, EVT_APP_START);
    return true;
}

static void GeofenceEnd()
{

}

INTERNAL_APP_INIT(
        APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_GEOFENCE, 0, 0)),
        0,
        GeofenceStart,
        GeofenceEnd,
        GeofenceHandleEvent);

