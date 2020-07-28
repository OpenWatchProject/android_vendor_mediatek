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
#include <string.h>

#include <log/log.h>

#include "Pressure.h"
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <inttypes.h>

#undef LOG_TAG
#define LOG_TAG "Pressure"

#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH           "/dev/m_baro_misc"

PressureSensor::PressureSensor()
    : mSensorReader(32) {
    mEnabled = 0;
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_PRESSURE;
    mPendingEvent.type = SENSOR_TYPE_PRESSURE;
    mPendingEvent.acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mEnabledTime =0;
    mDataDiv = 1;
    mPendingEvent.timestamp =0;
    input_sysfs_path_len = 0;
    memset(input_sysfs_path, 0, sizeof(input_sysfs_path));

    if (mSensorReader.selectSensorEventFd(DEVICE_PATH) >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_baro_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    } else {
        ALOGE("couldn't find sensor device");
        return;
    }
    ALOGD("misc path =%s", input_sysfs_path);

    char datapath[64]={"/sys/class/sensor/m_baro_misc/baroactive"};
    int fd = TEMP_FAILURE_RETRY(open(datapath, O_RDWR));
    char buf[64]={0};
    int len;
    if (fd >= 0) {
        len = TEMP_FAILURE_RETRY(read(fd,buf,sizeof(buf)-1));
        if (len <= 0) {
            ALOGE("read div err, len = %d", len);
        } else {
            buf[len] = '\0';
            sscanf(buf, "%d", &mDataDiv);
            ALOGI("read div buf(%s), mdiv %d", datapath,mDataDiv);
        }
        close(fd);
    } else {
        ALOGE("open misc path %s fail ", datapath);
    }
}

PressureSensor::~PressureSensor() {
}

int PressureSensor::enable(int32_t handle, int en)
{
    int fd = -1;
    int flags = en ? 1 : 0;

    ALOGI("enable: handle:%d, en:%d \r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "baroactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no enable control attr\r\n");
        return -1;
    }

    mEnabled = flags;
    char buf[2] = {0};
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
int PressureSensor::setDelay(int32_t handle, int64_t ns)
{
    int fd = -1;

    ALOGI("setDelay: (handle=%d, ns=%" PRId64 ")",handle, ns);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "barodelay", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no setDelay control attr \r\n" );
        return -1;
    }

    char buf[80] = {0};
    sprintf(buf, "%" PRId64 "", ns);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf) + 1));
    close(fd);
    return err < 0 ? err : 0;
}
int PressureSensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
    int fd = -1;
    char buf[128] = {0};

    ALOGI("batch: handle:%d, flag:%d,samplingPeriodNs:%" PRId64 " maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags,samplingPeriodNs, maxBatchReportLatencyNs);

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "barobatch", sizeof(input_sysfs_path) - input_sysfs_path_len);
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

int PressureSensor::flush(int handle)
{
    int fd = -1;
    char buf[32] = {0};

    ALOGI("flush\n");
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "baroflush", sizeof(input_sysfs_path) - input_sysfs_path_len);
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

int PressureSensor::readEvents(sensors_event_t* data, int count)
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
            /* auto cts may request flush event when sensor disable, ALPS03452281 */
            *data++ = mPendingEvent;
            numEventReceived++;
            count--;
        }
        mSensorReader.next();
    }
    return numEventReceived;
}

bool PressureSensor::pendingEvent(void) {
    return mSensorReader.pendingEvent();
}

void PressureSensor::processEvent(struct sensor_event const *event)
{
    if (event->flush_action == FLUSH_ACTION) {
        mPendingEvent.version = META_DATA_VERSION;
        mPendingEvent.sensor = 0;
        mPendingEvent.type = SENSOR_TYPE_META_DATA;
        mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
        mPendingEvent.meta_data.sensor = ID_PRESSURE;
        // must fill timestamp, if not, readEvents may can not report flush to framework
        mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
        ALOGI("flush complete\n");
    } else if (event->flush_action == DATA_ACTION) {
        mPendingEvent.version = sizeof(sensors_event_t);
        mPendingEvent.sensor = ID_PRESSURE;
        mPendingEvent.type = SENSOR_TYPE_PRESSURE;
        mPendingEvent.timestamp = event->time_stamp;
        mPendingEvent.pressure = (float) event->word[0]/ mDataDiv;
    } else
        ALOGE("unknow action\n");
}
