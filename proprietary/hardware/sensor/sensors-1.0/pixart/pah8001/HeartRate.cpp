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
#include "HeartRate.h"
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <string.h>
#include <inttypes.h>
#include <Performance.h>
#include <pxialg.h>

#undef LOG_TAG
#define LOG_TAG "HeartRate"

#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH           "/dev/m_hrs_misc"

HeartRateSensor::HeartRateSensor()
    : mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS)
{
    mEnabled = 0;
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_HEART_RATE;
    mPendingEvent.type = SENSOR_TYPE_HEART_RATE;
    mPendingEvent.heart_rate.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mEnabledTime = 0;
    mPendingEvent.timestamp = 0;
    input_sysfs_path_len = 0;

    memset(input_sysfs_path, 0, sizeof(input_sysfs_path));
    char datapath[64] = "/sys/class/sensor/m_hrs_misc/hrsactive";
    int fd = -1;
    char buf[64] = {0};
    int len;

    if (mSensorReader.selectSensorEventFd(DEVICE_PATH) >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_hrs_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    } else {
        ALOGE("couldn't find sensor device");
    }
    ALOGD("misc path =%s", input_sysfs_path);

    ALOGD("Pixart PAH8001 v%d", PxiAlg_Version());
}

HeartRateSensor::~HeartRateSensor()
{
}

int HeartRateSensor::enable(int32_t handle, int en)
{
    int fd = -1;
    int flags = en ? 1 : 0;
    char buf[2] = {0};

    ALOGI("enable: handle:%d, en:%d\r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "hrsactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no enable control attr\r\n");
        return -1;
    }

    mEnabled = flags;
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

int HeartRateSensor::setDelay(int32_t handle, int64_t ns)
{
    int fd = -1;
    ALOGI("setDelay: (handle=%d, ns=%" PRId64 ")", handle, ns);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "hrsdelay", sizeof(input_sysfs_path) - input_sysfs_path_len);
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

int HeartRateSensor::batch(int handle, int flags,
                           int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
    ALOGD("Pixart PAG8001 oes not support batch");
    return 0;
}

int HeartRateSensor::flush(int handle)
{
    ALOGD("Pixart PAH8001 does not support flush");
    return 0;
}

int HeartRateSensor::readEvents(sensors_event_t *data, int count)
{
    if (count < 1)
        return -EINVAL;

    ssize_t n = mSensorReader.fill();
    if (n < 0)
        return n;
    int numEventReceived = 0;

    struct sensor_event const *event;

    while (count && mSensorReader.readEvent(&event)) {
        processEvent(event);
        /* we only report DATA_ACTION and FLUSH_ACTION to framework */
        if (event->flush_action < FLUSH_ACTION) {
            /* auto cts request flush event when sensor disable, ALPS03452281 */
            *data++ = mPendingEvent;
            numEventReceived++;
            count--;
        }
        mSensorReader.next();
    }
    return numEventReceived;
}

bool HeartRateSensor::pendingEvent(void)
{
    return mSensorReader.pendingEvent();
}

void HeartRateSensor::processEvent(struct sensor_event const *event)
{
    if (event->flush_action == DATA_ACTION) {
        int ret;
        static uint8_t HRD_Data[13] = {0};
        static float MEMS_Data[3] = {0};

        if (event->word[5] == 0x00) { // Reporting HeartRate Data
            memcpy(HRD_Data, event->word, sizeof(HRD_Data));
        } else if (event->word[5] == 0x01) { // Reporting Accelerometer Data
            memcpy(MEMS_Data, event->word, sizeof(MEMS_Data));
        }

        ret = PxiAlg_Process(HRD_Data, MEMS_Data);
        if (ret == FLAG_DATA_READY) {
            float grade;
            if (PxiAlg_GetSigGrade(&grade)) {
                if (grade >= 75) {
                    mPendingEvent.heart_rate.status = SENSOR_STATUS_ACCURACY_HIGH;
                } else if (grade >= 50) {
                    mPendingEvent.heart_rate.status = SENSOR_STATUS_ACCURACY_MEDIUM;
                } else if (grade >= 25) {
                    mPendingEvent.heart_rate.status = SENSOR_STATUS_ACCURACY_LOW;
                } else {
                    mPendingEvent.heart_rate.status = SENSOR_STATUS_UNRELIABLE;
                }
            } else {
                mPendingEvent.heart_rate.status = SENSOR_STATUS_ACCURACY_MEDIUM;
            }
            PxiAlg_HrGet(&mPendingEvent.heart_rate.bpm);
        } else {
            mPendingEvent.heart_rate.status = SENSOR_STATUS_NO_CONTACT;
            mPendingEvent.heart_rate.bpm = 0;
        }

        mPendingEvent.version = sizeof(sensors_event_t);
        mPendingEvent.sensor = ID_HEART_RATE;
        mPendingEvent.type = SENSOR_TYPE_HEART_RATE;
        mPendingEvent.timestamp = event->time_stamp;
#ifdef DEBUG_PERFORMANCE
        if (1 == event->reserved) {
            mPendingEvent.heart_rate.status = event->reserved;
            mark_timestamp(ID_HEART_RATE, android::elapsedRealtimeNano(), mPendingEvent.timestamp);
        }
#endif
    } else
        ALOGI("unknown action\n");
}
