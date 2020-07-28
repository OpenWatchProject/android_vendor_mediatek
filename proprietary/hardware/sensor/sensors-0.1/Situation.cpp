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
#include <log/log.h>
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <string.h>
#include "Situation.h"
#include <inttypes.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "Situation"
#endif

#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH "/dev/m_situ_misc"

SituationSensor::SituationSensor()
    : SensorBase(NULL, "GES_INPUTDEV_NAME"),
      mSensorReader(32){
    mPendingEvent.sensor = 0;
    mPendingEvent.type = 0;
    mPendingEvent.version = sizeof(sensors_event_t);
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mPendingEvent.timestamp =0;

    memset(mEnabledTime, 0, sizeof(mEnabledTime));
    memset(mEnabled, 0, sizeof(mEnabled));
    input_sysfs_path_len = 0;

    mdata_fd = FindDataFd();
    if (mdata_fd >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_situ_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    }
    ALOGI("misc path =%s", input_sysfs_path);
}

SituationSensor::~SituationSensor() {
    if (mdata_fd >= 0)
        close(mdata_fd);
}

int SituationSensor::FindDataFd() {
    int fd = -1;

    fd = TEMP_FAILURE_RETRY(open(DEVICE_PATH, O_RDONLY));
    ALOGE_IF(fd < 0, "couldn't find input device");
    return fd;
}

int SituationSensor::HandleToIndex(int handle) {
    int index = -1;

    switch (handle) {
    case ID_IN_POCKET:
        index = inpocket;
        break;
    case ID_STATIONARY_DETECT:
        index = stationary;
        break;
    case ID_WAKE_GESTURE:
        index = wake_gesture;
        break;
    case ID_GLANCE_GESTURE:
        index = glance_gesture;
        break;
    case ID_PICK_UP_GESTURE:
        index = pickup_gesture;
        break;
    case ID_ANSWER_CALL:
        index = answer_call;
        break;
    case ID_MOTION_DETECT:
        index = motion_detect;
        break;
    case ID_DEVICE_ORIENTATION:
        index = device_orientation;
        break;
    case ID_TILT_DETECTOR:
        index = tilt_detector;
        break;
    }

    return index;
}
int SituationSensor::enable(int32_t handle, int en) {
    int fd;
    int index;
    char buf[8];
    int flags = en ? 1 : 0;

    ALOGI("enable: handle:%d, en:%d \r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "situactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no enable control attr\r\n");
        return -1;
    }

    index = HandleToIndex(handle);
    if (index < 0) {
        ALOGE("HandleToIndex err, handle:%d, index:%d\n", handle, index);
        return -1;
    }

    sprintf(buf, "%d : %d", handle, flags);
    if (flags) {
        mEnabled[index] = true;
        mEnabledTime[index] = getTimestamp() + IGNORE_EVENT_TIME;
    } else {
        mEnabled[index] = false;
    }

    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}

int SituationSensor::setDelay(int32_t handle, int64_t ns) {
    ALOGI("setDelay: regardless of the setDelay() value (handle=%d, ns=%" PRId64 ")", handle, ns);
    return 0;
}

int SituationSensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    int fd = -1;
    char buf[128] = {0};

    ALOGI("batch: handle:%d, flag:%d,samplingPeriodNs:%" PRId64 " maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags,samplingPeriodNs, maxBatchReportLatencyNs);

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "situbatch", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if(fd < 0) {
        ALOGE("no control attr\r\n");
        return -1;
    }
    sprintf(buf, "%d,%d,%" PRId64 ",%" PRId64 "", handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);
    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}

int SituationSensor::flush(int handle) {
    int fd = -1;
    char buf[32] = {0};

    ALOGI("flush, handle:%d\n", handle);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "situflush", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if(fd < 0) {
        ALOGE("no flush control attr\r\n");
        return -1;
    }
    sprintf(buf, "%d", handle);
    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}

int SituationSensor::readEvents(sensors_event_t* data, int count) {
    int index;

    if (count < 1)
        return -EINVAL;

    ssize_t n = mSensorReader.fill(mdata_fd);
    if (n < 0)
        return n;
    int numEventReceived = 0;
    struct sensor_event const* event;

    while (count && mSensorReader.readEvent(&event)) {
        processEvent(event);
        index = HandleToIndex(event->handle);
        if (index < 0)
            return -EINVAL;
        if (mEnabled[index] == true) {
            if (mPendingEvent.timestamp >= mEnabledTime[index]) {
                *data++ = mPendingEvent;
                numEventReceived++;
                count--;
                if (mReportingMode[index] == oneShot)
                    enable(event->handle, false);
            }
        }
        mSensorReader.next();
    }
    return numEventReceived;
}
void SituationSensor::processEvent(struct sensor_event const *event) {
    ALOGE("processEvent, handle:%d, flush_action:%d\n", event->handle, event->flush_action);
    switch (event->handle) {
    case ID_IN_POCKET:
        if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_IN_POCKET;
            mPendingEvent.type = SENSOR_TYPE_IN_POCKET;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float) event->word[0];
        }
        break;
    case ID_STATIONARY_DETECT:
        if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_STATIONARY_DETECT;
            mPendingEvent.type = SENSOR_TYPE_STATIONARY_DETECT;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float) event->word[0];
        }
        break;
    case ID_WAKE_GESTURE:
        if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_WAKE_GESTURE;
            mPendingEvent.type = SENSOR_TYPE_WAKE_GESTURE;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float) event->word[0];
        }
        break;
    case ID_PICK_UP_GESTURE:
        if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_PICK_UP_GESTURE;
            mPendingEvent.type = SENSOR_TYPE_PICK_UP_GESTURE;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float) event->word[0];
        }
        break;
    case ID_GLANCE_GESTURE:
        if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_GLANCE_GESTURE;
            mPendingEvent.type = SENSOR_TYPE_GLANCE_GESTURE;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float) event->word[0];
        }
        break;
    case ID_ANSWER_CALL:
        if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_ANSWER_CALL;
            mPendingEvent.type = SENSOR_TYPE_ANSWER_CALL;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float) event->word[0];
        }
        break;
    case ID_MOTION_DETECT:
        if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_MOTION_DETECT;
            mPendingEvent.type = SENSOR_TYPE_MOTION_DETECT;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float) event->word[0];
        }
        break;
    case ID_DEVICE_ORIENTATION:
        if (event->flush_action == FLUSH_ACTION) {
            mPendingEvent.version = META_DATA_VERSION;
            mPendingEvent.sensor = 0;
            mPendingEvent.type = SENSOR_TYPE_META_DATA;
            mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
            mPendingEvent.meta_data.sensor = ID_DEVICE_ORIENTATION;
            // must fill timestamp, if not, readEvents may can not report flush to framework
            mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
        } else if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_DEVICE_ORIENTATION;
            mPendingEvent.type = SENSOR_TYPE_DEVICE_ORIENTATION;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float) event->word[0];
        } else
            ALOGE("unknow action\n");
        break;
    case ID_TILT_DETECTOR:
        if (event->flush_action == FLUSH_ACTION) {
            mPendingEvent.version = META_DATA_VERSION;
            mPendingEvent.sensor = 0;
            mPendingEvent.type = SENSOR_TYPE_META_DATA;
            mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
            mPendingEvent.meta_data.sensor = ID_TILT_DETECTOR;
            // must fill timestamp, if not, readEvents may can not report flush to framework
            mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
        } else if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_TILT_DETECTOR;
            mPendingEvent.type = SENSOR_TYPE_TILT_DETECTOR;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float) event->word[0];
        } else
            ALOGE("unknow action\n");
        break;
    }
}
