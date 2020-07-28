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
#include <log/log.h>
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <string.h>
#include <inttypes.h>
#include "Biometric.h"
#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#endif

#undef LOG_TAG
#define LOG_TAG "BIOMETRIC"


#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH           "/dev/m_bio_misc"


/*****************************************************************************/
BiometricSensor::BiometricSensor()
    : SensorBase(NULL, "m_bio_input"),  // BIOMETRIC_INPUTDEV_NAME
      mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS) {
    mPendingEvent.version = 0;
    mPendingEvent.sensor = 0;
    mPendingEvent.type = 0;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mPendingEvent.timestamp =0;

    memset(mEnabledTime, 0, sizeof(mEnabledTime));
    memset(mEnabled, 0, sizeof(mEnabled));
    memset(mFlushCnt, 0, sizeof(mFlushCnt));
    memset(mDataDiv, 0, sizeof(mDataDiv));
    input_sysfs_path_len = 0;

    mdata_fd = FindDataFd();
    if (mdata_fd >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_bio_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    }
    // ALOGD("biometric misc path =%s", input_sysfs_path);
#if 0
    char datapath[64]={"/sys/class/sensor/m_bio_misc/bioactive"};
    int fd = open(datapath, O_RDWR);
    char buf[64];
    int len;

    if (fd >= 0) {
        len = read(fd, buf, sizeof(buf)-1);
        if (len <= 0) {
            ALOGD("read div err buf(%s)", buf);
        } else {
            buf[len] = '\0';
            sscanf(buf, "%d,%d,%d,%d,%d,%d,%d,%d,%d", &mDataDiv[orientation], &mDataDiv[grv], &mDataDiv[gmrv],
                &mDataDiv[rv], &mDataDiv[la], &mDataDiv[grav], &mDataDiv[ungyro], &mDataDiv[unmag], &mDataDiv[pdr]);
        }
        close(fd);
    } else {
        ALOGE("open biometric misc path %s fail ", datapath);
    }
#endif
}

BiometricSensor::~BiometricSensor() {
    if (mdata_fd >= 0)
        close(mdata_fd);
}
int BiometricSensor::FindDataFd() {
    int fd = -1;

    fd = TEMP_FAILURE_RETRY(open(DEVICE_PATH, O_RDONLY));
    // ALOGE_IF(fd<0, "couldn't find input device");
    return fd;
}
int BiometricSensor::HandleToIndex(int handle) {
    switch (handle) {
    case ID_EKG:
        return ekg;
    case ID_PPG1:
        return ppg1;
    case ID_PPG2:
        return ppg2;
    //defult:
        //ALOGE("HandleToIndex(%d) err\n", handle);
    }

    return -1;
}
int BiometricSensor::enable(int32_t handle, int en) {
    int fd, index = -1;
    int flags = en ? 1 : 0;
    char buf[8] = {0};

    ALOGI("enable: handle:%d, en:%d \r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "bioactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        return -1;
    }

    index = HandleToIndex(handle);
    if (index < 0) {
        ALOGE("HandleToIndex err, handle:%d, index:%d\n", handle, index);
        return -EINVAL;
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

int BiometricSensor::setDelay(int32_t handle, int64_t ns) {
    int fd = -1;
    ALOGI("setDelay: (handle=%d, ns=%" PRId64 ")", handle, ns);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "biodelay", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGD("no biometric setDelay control attr\r\n");
        return -1;
    }
    char buf[80] = {0};
    sprintf(buf, "%d,%" PRId64 "", handle, ns);
    TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf)+1));
    close(fd);
    return 0;
}

int BiometricSensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    int fd = -1;
    char buf[128] = {0};

    ALOGI("biometric batch: handle:%d, flag:%d,samplingPeriodNs:%" PRId64 " maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "biobatch", sizeof(input_sysfs_path) - input_sysfs_path_len);
    ALOGD("biometric batch path:%s \r\n", input_sysfs_path);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGD("no biometric batch control attr\r\n");
        return -1;
    }
    sprintf(buf, "%d,%d,%" PRId64 ",%" PRId64 "", handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);
    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}

int BiometricSensor::flush(int handle) {
    int fd = -1, index = -1;
    char buf[32] = {0};

    index = HandleToIndex(handle);
    if (index < 0)
        return -EINVAL;
    mFlushCnt[index]++;
    ALOGI("biometric flush, handle:%d, mFlushCnt:%d\n", handle, mFlushCnt[index]);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "bioflush", sizeof(input_sysfs_path) - input_sysfs_path_len);
    ALOGI("biometric flush path:%s \r\n", input_sysfs_path);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGD("no biometric flush control attr\r\n");
        return -1;
    }
    sprintf(buf, "%d", handle);
    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}

int BiometricSensor::readEvents(sensors_event_t* data, int count) {
    int index = -1;

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
        if (mEnabled[index]) {
            if (1) { /* mPendingEvent.timestamp > mEnabledTime[index]) { */
                *data++ = mPendingEvent;
                numEventReceived++;
                count--;
            }
        }
        mSensorReader.next();
    }
    return numEventReceived;
}

void BiometricSensor::processEvent(struct sensor_event const *event) {
    int index = HandleToIndex(event->handle);

    switch (event->handle) {
    case ID_EKG:
        if (event->flush_action == FLUSH_ACTION) {
            mPendingEvent.version = META_DATA_VERSION;
            mPendingEvent.sensor = 0;
            mPendingEvent.type = SENSOR_TYPE_META_DATA;
            mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
            mPendingEvent.meta_data.sensor = ID_EKG;
            // must fill timestamp, if not, readEvents may can not report flush to framework
            mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
            mFlushCnt[index]--;
            ALOGI("ekg flush complete, mFlushCnt:%d\n", mFlushCnt[index]);
        } else if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_EKG;
            mPendingEvent.type = SENSOR_TYPE_EKG;
            mPendingEvent.timestamp = event->time_stamp;
            mPendingEvent.data[0] = (float)event->word[0];
            mPendingEvent.data[1] = (float)event->word[1];
            mPendingEvent.data[4] = event->status;
#ifdef HAVE_AEE_FEATURE
            if ((mPendingEvent.data[1] != 0) && (mPendingEvent.data[1] - sn[ekg]) != 1)
                aee_system_warning(LOG_TAG, NULL, DB_OPT_DEFAULT, "%f (%f - %f) ekg data dropped!",
                    mPendingEvent.data[1] - sn[ekg] - 1, mPendingEvent.data[1], sn[ekg]);
            sn[ekg] = mPendingEvent.data[1];
#endif
        } else
            ALOGE("unknow action\n");
        break;
    case ID_PPG1:
        if (event->flush_action == FLUSH_ACTION) {
            mPendingEvent.version = META_DATA_VERSION;
            mPendingEvent.sensor = 0;
            mPendingEvent.type = SENSOR_TYPE_META_DATA;
            mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
            mPendingEvent.meta_data.sensor = ID_PPG1;
            // must fill timestamp, if not, readEvents may can not report flush to framework
            mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
            mFlushCnt[index]--;
            ALOGI("ppg1 flush complete, mFlushCnt:%d\n", mFlushCnt[index]);
        } else if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_PPG1;
            mPendingEvent.type = SENSOR_TYPE_PPG1;
            mPendingEvent.timestamp = event->time_stamp;
            mPendingEvent.data[0] = (float)event->word[0];
            mPendingEvent.data[1] = (float)event->word[1];
            mPendingEvent.data[2] = (float)event->word[2];
            mPendingEvent.data[3] = (float)event->word[3];
            mPendingEvent.data[4] = event->status;
#ifdef HAVE_AEE_FEATURE
            if ((mPendingEvent.data[1] != 0) && (mPendingEvent.data[1] - sn[ppg1]) != 1)
                aee_system_warning(LOG_TAG, NULL, DB_OPT_DEFAULT, "%f (%f - %f) ppg1 data dropped!",
                    mPendingEvent.data[1] - sn[ppg1] - 1, mPendingEvent.data[1], sn[ppg1]);
            sn[ppg1] = mPendingEvent.data[1];
#endif
        } else
            ALOGE("unknow action\n");
        break;
    case ID_PPG2:
        if (event->flush_action == FLUSH_ACTION) {
            mPendingEvent.version = META_DATA_VERSION;
            mPendingEvent.sensor = 0;
            mPendingEvent.type = SENSOR_TYPE_META_DATA;
            mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
            mPendingEvent.meta_data.sensor = ID_PPG2;
            // must fill timestamp, if not, readEvents may can not report flush to framework
            mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
            mFlushCnt[index]--;
            ALOGI("ppg2 flush complete, mFlushCnt:%d\n", mFlushCnt[index]);
        } else if (event->flush_action == DATA_ACTION) {
            mPendingEvent.version = sizeof(sensors_event_t);
            mPendingEvent.sensor = ID_PPG2;
            mPendingEvent.type = SENSOR_TYPE_PPG2;
            mPendingEvent.timestamp = event->time_stamp;
            mPendingEvent.data[0] = (float)event->word[0];
            mPendingEvent.data[1] = (float)event->word[1];
            mPendingEvent.data[2] = (float)event->word[2];
            mPendingEvent.data[3] = (float)event->word[3];
            mPendingEvent.data[4] = event->status;
#ifdef HAVE_AEE_FEATURE
            if ((mPendingEvent.data[1] != 0) && (mPendingEvent.data[1] - sn[ppg2]) != 1)
                aee_system_warning(LOG_TAG, NULL, DB_OPT_DEFAULT, "%f (%f - %f) ppg2 data dropped!",
                    mPendingEvent.data[1] - sn[ppg2] - 1, mPendingEvent.data[1], sn[ppg2]);
            sn[ppg2] = mPendingEvent.data[1];
#endif
        } else
            ALOGE("unknow action\n");
        break;
    default:
        ALOGI("Unsupported sensor type: %d\n", event->handle);
        break;
    }
}
