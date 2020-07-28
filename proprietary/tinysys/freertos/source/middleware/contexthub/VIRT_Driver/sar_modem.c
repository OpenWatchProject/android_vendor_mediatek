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

#include <stdio.h>
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

#include <ccci.h>

#define EVT_SENSOR_SAR           sensorGetMyEventType(SENS_TYPE_SAR)
#define DEBOUNCE_TIME            3000000000


struct sar_modem_data_t
{
    uint8_t action;
    uint8_t data[1];
};

enum {
    SAR_MODEM_DISABLE_ACTION = 0,
    SAR_MODEM_ENABLE_ACTION = 1,
    SAR_MODEM_REBOOT_NOTIFY_ACTION = 2,
    SAR_MODEM_DATA_ACTION = 3,
} action_t;

enum {
    SAR_INIT = -1,
    SAR_NEAR = 0,
    SAR_FAR = 1,
} data_t;

enum sarModemEvents {
    EVT_DISABLE_SAR = EVT_APP_START + 1,
    EVT_ENABLE_SAR,
    EVT_SENSOR_REBOOT_NOTIFY
};

struct sarModemTask {
    uint32_t taskId;
    uint32_t sarHandle;
    uint32_t timerHandle;
    uint32_t sar_is_shaded;

    enum {
        STATE_DISABLED,
        STATE_INIT,
    } taskState;
};
static struct sarModemTask mTask;

static void sarModemInit()
{
    mTask.sar_is_shaded = 0;
    //osLog(LOG_INFO, "SAR_MODEM INIT DONE\n");
}

static void configSar(bool on)
{
    uint32_t i;

    if ((on == true) && (mTask.sarHandle == 0)) {
        for (i = 0; sensorFind(SENS_TYPE_SAR, i, &mTask.sarHandle) != NULL; i++) {
            if (sensorRequest(mTask.taskId, mTask.sarHandle, SENSOR_RATE_ONCHANGE, 0)) {
                osEventSubscribe(mTask.taskId, EVT_SENSOR_SAR);
                break;
            }
        }
        if (sensorFind(SENS_TYPE_SAR, i, &mTask.sarHandle) == NULL) {
            osLog(LOG_INFO, "sarModem SAR sensor is not available\n");
        }

    } else if ((on == false) && (mTask.sarHandle != 0)) {
        sensorRelease(mTask.taskId, mTask.sarHandle);
        mTask.sarHandle = 0;
        osEventUnsubscribe(mTask.taskId, EVT_SENSOR_SAR);
    }
}

static bool sarModemPower(bool on)
{
    static bool prev_sar_on = false;
    bool sar_on = on;

    osLog(LOG_INFO, "sarModem power:%d preOn:%d\n", on, prev_sar_on);

    if ((!prev_sar_on) && (sar_on)) {
        sarModemInit();
        configSar(true);
        mTask.taskState = STATE_INIT;
    } else if ((prev_sar_on) && (!sar_on)) {
        configSar(false);
        mTask.taskState = STATE_DISABLED;
    } else
         osLog(LOG_INFO, "sarModem repower by modem\n");
    prev_sar_on = sar_on;

    return true;
}

void sarModemRecieveHandle(void *data)
{
    struct sar_modem_data_t *modem_data = (struct sar_modem_data_t *)data;
    //osLog(LOG_INFO, "sarModem data action:%d from modem\n", modem_data->action);
    switch (modem_data->action)
    {
        case SAR_MODEM_ENABLE_ACTION :
            osEnqueuePrivateEvt(EVT_ENABLE_SAR, NULL, NULL, mTask.taskId);
            break;
        case SAR_MODEM_DISABLE_ACTION :
            osEnqueuePrivateEvt(EVT_DISABLE_SAR, NULL, NULL, mTask.taskId);
            break;
    }
}

static void restartTimerCallBack(uint32_t timerId, void *cookie) {
    osEnqueuePrivateEvt(EVT_SENSOR_REBOOT_NOTIFY, NULL, NULL, mTask.taskId);
}

static void sarModemHandleEvent(uint32_t evtType, const void* evtData)
{
    int err = 0;
    static uint8_t power_notify_retry = 0;
    struct sar_modem_data_t sar_modem_data;

    if (evtData == SENSOR_DATA_EVENT_FLUSH) {
        return;
    }

    switch (evtType) {
        case EVT_APP_START :
            osEventUnsubscribe(mTask.taskId, EVT_APP_START);
            err = ccci_register_rx(MD_SYS1, CCCI_SCP_USR0_RX, sarModemRecieveHandle);
            if (err < 0)
                osLog(LOG_INFO, "sarModem data callback register fail\n");

            mTask.timerHandle = timTimerSet(DEBOUNCE_TIME, 0, 50, restartTimerCallBack, NULL, true);
            if (mTask.timerHandle == 0)
                configASSERT(0);
            osLog(LOG_INFO, "sarModem EVT_APP_START\n");
            break;

        case EVT_SENSOR_REBOOT_NOTIFY :
            if (mTask.taskState != STATE_INIT && power_notify_retry < 8) {
                power_notify_retry++;

                sar_modem_data.action = SAR_MODEM_REBOOT_NOTIFY_ACTION;
                err = ccci_user_msg_send(MD_SYS1, CCCI_SCP_USR0_TX, &sar_modem_data, sizeof(struct sar_modem_data_t));
                if (err < 0)
                {
                    mTask.timerHandle = timTimerSet(DEBOUNCE_TIME, 0, 50, restartTimerCallBack, NULL, true);
                    if (mTask.timerHandle == 0)
                        configASSERT(0);
                }
                //osLog(LOG_INFO, "sarModem ready notify fail\n");
            }
            break;

        case EVT_ENABLE_SAR :
            sarModemPower(true);
            break;

        case EVT_DISABLE_SAR :
            sarModemPower(false);
            break;

        case EVT_SENSOR_SAR :
            /*osLog(LOG_INFO, "sarModem data notify %d %d %d\n",
                (uint32_t)((struct TripleAxisDataEvent *)evtData)->samples[0].ix,
                (uint32_t)((struct TripleAxisDataEvent *)evtData)->samples[0].iy,
                (uint32_t)((struct TripleAxisDataEvent *)evtData)->samples[0].iz);*/
            if(mTask.taskState == STATE_INIT) {
                mTask.sar_is_shaded = (uint32_t)((struct TripleAxisDataEvent *)evtData)->samples[0].ix;
                if (mTask.sar_is_shaded) {
                    sar_modem_data.action = SAR_MODEM_DATA_ACTION;
                    sar_modem_data.data[0] = SAR_FAR;
                } else {
                    sar_modem_data.action = SAR_MODEM_DATA_ACTION;
                    sar_modem_data.data[0] = SAR_NEAR;
                }

                //osLog(LOG_INFO, "EVT_SENSOR_SAR:%u, sar_is_shaded:%u\n", (uint32_t)ev.fdata, mTask.sar_is_shaded);
                err = ccci_user_msg_send(MD_SYS1, CCCI_SCP_USR0_TX, &sar_modem_data, sizeof(struct sar_modem_data_t));
                if (err < 0)
                    osLog(LOG_INFO, "sarModem data notify fail\n");
            }

            break;

        default:
            break;
    }
}

static bool sarModemStart(uint32_t taskId)
{
    mTask.taskId = taskId;
    mTask.taskState = STATE_DISABLED;
    osEventSubscribe(taskId, EVT_APP_START);
     osLog(LOG_INFO, "sarModemStart\n");
    return true;
}

static void sarModemEnd()
{
    return;
}

INTERNAL_APP_INIT(APP_ID_MAKE(APP_ID_VENDOR_MTK, MTK_APP_ID_WRAP(SENS_TYPE_INVALID, 0, 1)), 0,
    sarModemStart,
    sarModemEnd,
    sarModemHandleEvent);
