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

#include "Activity.h"
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <inttypes.h>

#undef LOG_TAG
#define LOG_TAG "Activity"

#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH           "/dev/m_act_misc"

ActivitySensor::ActivitySensor()
    : mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS) {
    mEnabled = 0;
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_ACTIVITY;
    mPendingEvent.type = SENSOR_TYPE_ACTIVITY;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mEnabledTime =0;
    mDataDiv = 1;
    mPendingEvent.timestamp =0;
    input_sysfs_path_len = 0;
    //input_sysfs_path[PATH_MAX] = {0};
    memset(input_sysfs_path, 0, sizeof(char)*sizeof(input_sysfs_path));
    m_act_last_ts = 0;
    m_act_delay= 0;
    batchMode=0;
    firstData = 1;
    char datapath[64]={"/sys/class/sensor/m_act_misc/actactive"};
    int fd = -1;
    char buf[64];
    int len;

    if (mSensorReader.selectSensorEventFd(DEVICE_PATH) >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_act_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    } else {
        ALOGE("couldn't find sensor device");
        return;
    }
    ALOGD("misc path =%s", input_sysfs_path);

    fd = TEMP_FAILURE_RETRY(open(datapath, O_RDWR));
    if (fd >= 0) {
        len = TEMP_FAILURE_RETRY(read(fd,buf,sizeof(buf)-1));
        if (len <= 0) {
            ALOGE("read div err buf(%s)",buf );
        } else {
            buf[len] = '\0';
            ALOGI("len = %d, buf = %s",len, buf);
            sscanf(buf, "%d", &mDataDiv);
            ALOGI("read div buf(%s)", datapath);
            ALOGI("mdiv %d",mDataDiv );
        }
        close(fd);
    } else {
        ALOGE("open misc path %s fail ", datapath);
    }
}

ActivitySensor::~ActivitySensor() {
}

int ActivitySensor::enable(int32_t handle, int en)
{
    int fd;
    int flags = en ? 1 : 0;

    ALOGI("enable: handle:%d, en:%d \r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "actactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no enable control attr\r\n" );
        return -1;
    }

    mEnabled = flags;
    char buf[2];
    buf[1] = 0;
    if (flags)  {
        buf[0] = '1';
        mEnabledTime = getTimestamp() + IGNORE_EVENT_TIME;
    } else {
        buf[0] = '0';
    }
    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return err < 0 ? err : 0;
}

int ActivitySensor::setDelay(int32_t handle, int64_t ns)
{
    int fd;
    ALOGI("setDelay: (handle=%d, ns=%" PRId64 ")", handle, ns);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "actdelay", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no setDelay control attr \r\n" );
        return -1;
    }
    char buf[80];
    sprintf(buf, "%" PRId64 "", ns);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf)+1));
    close(fd);
    return err < 0 ? err : 0;
}

int ActivitySensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
    int fd = -1;
    char buf[128] = {0};

    ALOGI("batch: handle:%d, flag:%d,samplingPeriodNs:%" PRId64 " maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags,samplingPeriodNs, maxBatchReportLatencyNs);

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "actbatch", sizeof(input_sysfs_path) - input_sysfs_path_len);
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

int ActivitySensor::flush(int handle)
{
    int fd = -1;
    char buf[32] = {0};

    ALOGI("flush\n");
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "actflush", sizeof(input_sysfs_path) - input_sysfs_path_len);
    ALOGD("flush path:%s \r\n",input_sysfs_path);
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

int ActivitySensor::readEvents(sensors_event_t* data, int count)
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

bool ActivitySensor::pendingEvent(void) {
    return mSensorReader.pendingEvent();
}

void ActivitySensor::processEvent(struct sensor_event const *event)
{
    if (event->flush_action == FLUSH_ACTION) {
        mPendingEvent.version = META_DATA_VERSION;
        mPendingEvent.sensor = 0;
        mPendingEvent.type = SENSOR_TYPE_META_DATA;
        mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
        mPendingEvent.meta_data.sensor = ID_ACTIVITY;
        // must fill timestamp, if not, readEvents may can not report flush to framework
        mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
        ALOGI("flush complete\n");
    } else if (event->flush_action == DATA_ACTION) {
        mPendingEvent.version = sizeof(sensors_event_t);
        mPendingEvent.sensor = ID_ACTIVITY;
        mPendingEvent.type = SENSOR_TYPE_ACTIVITY;
        mPendingEvent.timestamp = event->time_stamp;
        mPendingEvent.data[0] = event->byte[0];
        mPendingEvent.data[1] = event->byte[1];
        mPendingEvent.data[2] = event->byte[2];
        mPendingEvent.data[3] = event->byte[3];
        mPendingEvent.data[4] = event->byte[4];
        mPendingEvent.data[5] = event->byte[5];
        mPendingEvent.data[6] = event->byte[6];
        mPendingEvent.data[7] = event->byte[7];
        mPendingEvent.data[8] = event->byte[8];
        mPendingEvent.data[9] = event->byte[9];
        mPendingEvent.data[10] = event->byte[10];
        mPendingEvent.data[11] = event->byte[11];
    } else
        ALOGE("unknow action\n");
}
