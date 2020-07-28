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

#include "StepCounter.h"
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <inttypes.h>

#ifdef LOG_TAG
#undef   LOG_TAG
#define LOG_TAG "StepRecognize"
#endif

#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH           "/dev/m_step_c_misc"


/*****************************************************************************/
StepCounterSensor::StepCounterSensor()
    : SensorBase(NULL, "m_step_c_input"),//_INPUTDEV_NAME
      mEnabled(0),
      mSensorReader(32)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_STEP_COUNTER;
    mPendingEvent.type = SENSOR_TYPE_STEP_COUNTER;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mEnabledTime =0;
    mDataDiv = 1;
    mPendingEvent.timestamp =0;
    input_sysfs_path_len = 0;
    memset(input_sysfs_path, 0, sizeof(input_sysfs_path));

    mdata_fd = FindDataFd();
    if (mdata_fd >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_step_c_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    }
    ALOGI("misc path =%s", input_sysfs_path);

    char datapath[64] = {"/sys/class/sensor/m_step_c_misc/step_cactive"};
    int fd = TEMP_FAILURE_RETRY(open(datapath, O_RDWR));
    char buf[64];
    int len;

    if (fd >= 0) {
        len = TEMP_FAILURE_RETRY(read(fd,buf,sizeof(buf)-1));
        if(len <= 0) {
            ALOGE("read div err, len = %d", len);
        } else {
            buf[len] = '\0';
            sscanf(buf, "%d", &mDataDiv);
            ALOGI("read div buf(%s)", datapath);
            ALOGI("mdiv %d",mDataDiv );
        }
        close(fd);
    } else {
        ALOGE("open misc path %s fail ", datapath);
    }
}

StepCounterSensor::~StepCounterSensor() {
    if (mdata_fd >= 0)
        close(mdata_fd);
}

int StepCounterSensor::FindDataFd() {
    int fd = -1;

    fd = TEMP_FAILURE_RETRY(open(DEVICE_PATH, O_RDONLY));
    ALOGE_IF(fd < 0, "couldn't find input device");
    return fd;
}

int StepCounterSensor::enable(int32_t handle, int en)
{
    int fd;
    int flags = en ? 1 : 0;

    ALOGI("enable: handle:%d, en:%d\r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "step_cactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no enable control attr\r\n");
        return -1;
    }

    //mEnabled = flags;
    mEnabled = en ? mEnabled | (1 << handle) : mEnabled & (~(1 << handle));
    char buf[120] = {0};
    sprintf(buf, "%d,%d", handle, en);
    TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf)+1));
    close(fd);
    return 0;
}
int StepCounterSensor::setDelay(int32_t handle, int64_t ns)
{
    ALOGI("setDelay: regardless of the setDelay() value (handle=%d, ns=%" PRId64 ")", handle, ns);
    return 0;
}
int StepCounterSensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
    int fd = -1;
    char buf[128] = {0};

    ALOGI("batch: handle:%d, flag:%d,samplingPeriodNs:%" PRId64 " maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags,samplingPeriodNs, maxBatchReportLatencyNs);

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "step_cbatch", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if(fd < 0) {
        ALOGE("no batch control attr\r\n");
        return -1;
    }
    sprintf(buf, "%d,%d,%" PRId64 ",%" PRId64 "", handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);
    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}


int StepCounterSensor::flush(int handle)
{
    int fd = -1;
    char buf[32] = {0};

    ALOGI("handle: %d flush\n", handle);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "step_cflush", sizeof(input_sysfs_path) - input_sysfs_path_len);
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

int StepCounterSensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1)
        return -EINVAL;

    ssize_t n = mSensorReader.fill(mdata_fd);
    if (n < 0)
        return n;
    int numEventReceived = 0;
    struct sensor_event const* event;

    while (count && mSensorReader.readEvent(&event)) {
        processEvent(event);
        if (mEnabled) {
            if (mPendingEvent.timestamp >= mEnabledTime) {
                *data++ = mPendingEvent;
                numEventReceived++;
                count--;
                if(mPendingEvent.sensor == ID_SIGNIFICANT_MOTION)
                    enable(ID_SIGNIFICANT_MOTION, false); //one shot sensor need auto disable
            }
        }
        mSensorReader.next();
    }
    return numEventReceived;
}

void StepCounterSensor::processEvent(struct sensor_event const* event)
{
    switch (event->handle) {
    case ID_STEP_COUNTER:
        if (event->flush_action == FLUSH_ACTION) {
            mPendingEvent.version = META_DATA_VERSION;
            mPendingEvent.sensor = 0;
            mPendingEvent.type = SENSOR_TYPE_META_DATA;
            mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
            mPendingEvent.meta_data.sensor = ID_STEP_COUNTER;
            // must fill timestamp, if not, readEvents may can not report flush to framework
            mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
            ALOGI("step counter flush complete\n");
        } else if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_STEP_COUNTER;
            mPendingEvent.type = SENSOR_TYPE_STEP_COUNTER;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.u64.step_counter= event->word[0];
        } else
            ALOGE("unknow action\n");
        break;
    case ID_STEP_DETECTOR:
        if (event->flush_action == FLUSH_ACTION) {
            mPendingEvent.version = META_DATA_VERSION;
            mPendingEvent.sensor = 0;
            mPendingEvent.type = SENSOR_TYPE_META_DATA;
            mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
            mPendingEvent.meta_data.sensor = ID_STEP_DETECTOR;
            // must fill timestamp, if not, readEvents may can not report flush to framework
            mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
            ALOGI("step detect flush complete\n");
        } else if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_STEP_DETECTOR;
            mPendingEvent.type = SENSOR_TYPE_STEP_DETECTOR;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float)event->word[0];
        } else
            ALOGE("unknow action\n");
        break;
    case ID_SIGNIFICANT_MOTION:
        if (event->flush_action == FLUSH_ACTION) {
            mPendingEvent.version = META_DATA_VERSION;
            mPendingEvent.sensor = 0;
            mPendingEvent.type = SENSOR_TYPE_META_DATA;
            mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
            mPendingEvent.meta_data.sensor = ID_SIGNIFICANT_MOTION;
            // must fill timestamp, if not, readEvents may can not report flush to framework
            mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
            ALOGI("smd flush complete\n");
        } else if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_SIGNIFICANT_MOTION;
            mPendingEvent.type = SENSOR_TYPE_SIGNIFICANT_MOTION;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float)event->word[0];
        } else
            ALOGE("unknow action\n");
        break;
    case ID_FLOOR_COUNTER:
        if (event->flush_action == FLUSH_ACTION) {
            mPendingEvent.version = META_DATA_VERSION;
            mPendingEvent.sensor = 0;
            mPendingEvent.type = SENSOR_TYPE_META_DATA;
            mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
            mPendingEvent.meta_data.sensor = ID_FLOOR_COUNTER;
            // must fill timestamp, if not, readEvents may can not report flush to framework
            mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
            ALOGI("fc flush complete\n");
        } else if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_FLOOR_COUNTER;
            mPendingEvent.type = SENSOR_TYPE_FLOOR_COUNTER;
            mPendingEvent.timestamp = android::elapsedRealtimeNano();
            mPendingEvent.data[0] = (float)event->word[0];
        } else
            ALOGE("unknow action\n");
        break;
    }
}
