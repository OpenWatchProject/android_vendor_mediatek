/*
 * Copyright (C) 2008 The Android Open Source Project
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

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <string.h>

#include <log/log.h>

#include "Pedometer.h"
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <inttypes.h>

#undef LOG_TAG
#define LOG_TAG "Pedometer"

#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH           "/dev/m_pedo_misc"

PedometerSensor::PedometerSensor()
    : mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS) {
    mEnabled = 0;
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_PEDOMETER;
    mPendingEvent.type = SENSOR_TYPE_PEDOMETER;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mEnabledTime =0;
    mDataDiv = 1;
    mPendingEvent.timestamp =0;
    input_sysfs_path_len = 0;
    memset(input_sysfs_path, 0, sizeof(input_sysfs_path));

    if (mSensorReader.selectSensorEventFd(DEVICE_PATH) >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_pedo_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    }
    ALOGD("misc path =%s", input_sysfs_path);

    char datapath[64] = {"/sys/class/sensor/m_pedo_misc/pedoactive"};
    int fd = TEMP_FAILURE_RETRY(open(datapath, O_RDWR));
    char buf[64];
    int len;
    if (fd >= 0) {
        len = TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf) - 1));
        if(len <= 0) {
            ALOGE("read div err buf(%s)", buf);
        } else {
            buf[len] = '\0';
            sscanf(buf, "%d", &mDataDiv);
            ALOGI("read div buf(%s)", datapath);
            ALOGI("mdiv %d", mDataDiv);
        }
        close(fd);
    } else {
        ALOGE("open misc path %s fail", datapath);
    }
}

PedometerSensor::~PedometerSensor() {
}

int PedometerSensor::enable(int32_t handle, int en)
{
    int fd;
    int flags = en ? 1 : 0;

    ALOGI("enable: handle:%d, en:%d\r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "pedoactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no enable control attr\r\n");
        return -1;
    }

    mEnabled = flags;
    char buf[2];
    buf[1] = 0;
    if (flags) {
        buf[0] = '1';
        mEnabledTime = getTimestamp() + IGNORE_EVENT_TIME;
    } else {
        buf[0] = '0';
    }
    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return err < 0 ? err : 0;
}

int PedometerSensor::setDelay(int32_t handle, int64_t ns)
{
    int fd;
    ALOGI("setDelay: (handle=%d, ns=%" PRId64 ")", handle, ns);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "pedodelay", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no setDelay control attr\r\n");
        return -1;
    }
    char buf[80];
    sprintf(buf, "%" PRId64 "", ns);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf) + 1));
    close(fd);
    return err < 0 ? err : 0;
}

int PedometerSensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
    int fd;
    char buf[128];

    ALOGI("batch: handle:%d, flag:%d,samplingPeriodNs:%" PRId64 " maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags,samplingPeriodNs, maxBatchReportLatencyNs);

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "pedobatch", sizeof(input_sysfs_path) - input_sysfs_path_len);

    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if(fd < 0) {
        ALOGE("no batch control attr\r\n");
        return -1;
    }
    sprintf(buf, "%d,%d,%" PRId64 ",%" PRId64 "", handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return err < 0 ? err : 0;
}

int PedometerSensor::flush(int handle)
{
    int fd = -1;
    char buf[32] = {0};

    ALOGI("flush\n");
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "pedoflush", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if(fd < 0) {
        ALOGE("no flush control attr\r\n");
        return -1;
    }
    sprintf(buf, "%d", handle);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return err < 0 ? err : 0;
}

int PedometerSensor::readEvents(sensors_event_t* data, int count)
{
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

bool PedometerSensor::pendingEvent(void) {
    return mSensorReader.pendingEvent();
}

void PedometerSensor::processEvent(struct sensor_event const *event)
{
    if (event->flush_action == FLUSH_ACTION) {
        mPendingEvent.version = META_DATA_VERSION;
        mPendingEvent.sensor = 0;
        mPendingEvent.type = SENSOR_TYPE_META_DATA;
        mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
        mPendingEvent.meta_data.sensor = ID_PEDOMETER;
        // must fill timestamp, if not, readEvents may can not report flush to framework
        mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
        ALOGI("flush complete\n");
    } else if (event->flush_action == DATA_ACTION) {
        mPendingEvent.version = sizeof(sensors_event_t);
        mPendingEvent.sensor = ID_PEDOMETER;
        mPendingEvent.type = SENSOR_TYPE_PEDOMETER;
        mPendingEvent.timestamp = event->time_stamp;
        mPendingEvent.data[0] = (float) event->word[0] /1000; //change from milli-meter to meter
        mPendingEvent.data[1] = (float) event->word[1] /1024; //frequency div SHIFT_VALUE (define in SCP)
        mPendingEvent.data[2] = event->word[2];
        mPendingEvent.data[3] = (float) event->word[3] /1000; //change from milli-meter to meter
    } else
        ALOGE("unknow action\n");
}
