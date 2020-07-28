/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2012. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>

#include <utils/Timers.h>
#include <log/log.h>

#include "WakeUpSet.h"
#include <utils/SystemClock.h>
#include <string.h>
#include <Performance.h>
#include <inttypes.h>

#undef LOG_TAG
#define LOG_TAG "WakeUpSet"

static int32_t wakeUpToNonWakeUp(int32_t wakeUp) {
    int32_t nonWakeUp = -1;

    switch (wakeUp) {
    case ID_STEP_DETECTOR_WAKEUP:
        nonWakeUp = ID_STEP_DETECTOR;
        break;
    default:
        nonWakeUp = -1;
        break;
    }
    return nonWakeUp;
}

static int32_t nonWakeUpToWakeUp(int32_t nonWakeUp) {
    int32_t wakeUp = -1;

    switch (nonWakeUp) {
    case ID_STEP_DETECTOR:
        wakeUp = ID_STEP_DETECTOR_WAKEUP;
        break;
    default:
        wakeUp = -1;
        break;
    }
    return wakeUp;
}

WakeUpSetSensor::WakeUpSetSensor()
    : mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS) {
    mPendingEvent.version = 0;
    mPendingEvent.sensor = 0;
    mPendingEvent.type = 0;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mPendingEvent.timestamp = 0;

    mWritePipeFd = -1;

    if (mSensorReader.selectSensorEventFd(NULL) >= 0)
        mWritePipeFd = mSensorReader.getWriteFd();

    mSensorManager = SensorManager::getInstance();
    mSensorConnection = mSensorManager->createSensorConnection(wakeupset);
}

WakeUpSetSensor::~WakeUpSetSensor() {
    mSensorManager->removeSensorConnection(mSensorConnection);
    mSensorManager = NULL;
}

int WakeUpSetSensor::reportFlush(int handle) {
    struct sensor_event event;

    memset(&event, 0, sizeof(struct sensor_event));
    event.handle = handle;
    event.flush_action = FLUSH_ACTION;
    mSensorReader.writeEvent(&event);
    return 0;
}

int WakeUpSetSensor::reportData(int handle, struct sensors_event_t *data) {
    struct sensor_event event;

    memset(&event, 0, sizeof(struct sensor_event));
    event.handle = handle;
    event.time_stamp = data->timestamp;
    event.flush_action = DATA_ACTION;

    if (handle == ID_STEP_DETECTOR_WAKEUP) {
        event.word[0] = (int32_t)data->data[0];
    }

    mSensorReader.writeEvent(&event);
    return 0;
}

int WakeUpSetSensor::enable(int32_t handle, int en) {
    int32_t nonWakeUpHandle = -1;

    ALOGI("enable, handle:%d, en:%d\n", handle, en);

    nonWakeUpHandle = wakeUpToNonWakeUp(handle);
    if (nonWakeUpHandle < 0)
        return -1;
    return mSensorManager->activate(mSensorConnection, nonWakeUpHandle, en);
}

int WakeUpSetSensor::setDelay(int32_t handle, int64_t ns) {
    ALOGD("setDelay: (handle=%d, ns=%" PRId64 ")", handle, ns);
    return 0;
}

int WakeUpSetSensor::batch(int handle, int flags,
    int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    int32_t nonWakeUpHandle = -1;

    ALOGI("batch: handle:%d, flag:%d,samplingPeriodNs:%" PRId64 " maxBatchReportLatencyNs:%" PRId64 "\n",
        handle, flags,samplingPeriodNs, maxBatchReportLatencyNs);

    nonWakeUpHandle = wakeUpToNonWakeUp(handle);
    if (nonWakeUpHandle < 0)
        return -1;
    return mSensorManager->batch(mSensorConnection, nonWakeUpHandle, samplingPeriodNs, maxBatchReportLatencyNs);
}

int WakeUpSetSensor::flush(int handle) {
    reportFlush(handle);
    return 0;
}

int WakeUpSetSensor::setEvent(sensors_event_t *data) {
    int32_t wakeUpHandle = -1;

    /* ALOGD("setEvent handle=%d x=%f, y=%f, z=%f, timestamp=%lld\n",
        data->sensor, data->data[0], data->data[1], data->data[2], data->timestamp); */

    if ((wakeUpHandle = nonWakeUpToWakeUp(data->sensor)) < 0) {
        ALOGI("setEvent: wrong event handle=%d\n", data->sensor);
        return 0;
    }
    reportData(wakeUpHandle, data);
    return 0;
}

int WakeUpSetSensor::readEvents(sensors_event_t* data, int count) {
    if (count < 1)
        return -EINVAL;

    ssize_t n = mSensorReader.fill();
    if (n < 0)
        return n;
    int numEventReceived = 0;
    struct sensor_event const* event;

    while (count && mSensorReader.readEvent(&event)) {
        processEvent(event);
        if (event->flush_action <= FLUSH_ACTION) {
            *data++ = mPendingEvent;
            numEventReceived++;
            count--;
        }
        mSensorReader.next();
    }
    return numEventReceived;
}

bool WakeUpSetSensor::pendingEvent(void) {
    return mSensorReader.pendingEvent();
}

void WakeUpSetSensor::processEvent(struct sensor_event const *event) {
    switch (event->handle) {
    case ID_STEP_DETECTOR_WAKEUP:
        if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_STEP_DETECTOR_WAKEUP;
            mPendingEvent.type = SENSOR_TYPE_STEP_DETECTOR;
            if (event->time_stamp)
                mPendingEvent.timestamp = event->time_stamp;
            else
                mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float)event->word[0];
        } else if (event->flush_action == FLUSH_ACTION) {
            mPendingEvent.version = META_DATA_VERSION;
            mPendingEvent.sensor = 0;
            mPendingEvent.type = SENSOR_TYPE_META_DATA;
            mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
            mPendingEvent.meta_data.sensor = ID_STEP_DETECTOR_WAKEUP;
            // must fill timestamp, if not, readEvents may can not report flush to framework
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
        } else
            ALOGE("unknow action\n");
        break;
    }
}
