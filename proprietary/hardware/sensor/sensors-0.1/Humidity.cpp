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

#include "Humidity.h"
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <inttypes.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "HUMIDITY"
#endif


#define IGNORE_EVENT_TIME 350000000
#define DEVICE_PATH           "/dev/m_hmdy_misc"


/*****************************************************************************/
HumiditySensor::HumiditySensor()
    : SensorBase(NULL, "m_hmdy_input"),  // HUMIDITY_INPUTDEV_NAME
      mEnabled(0),
      mSensorReader(32) {
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_RELATIVE_HUMIDITY;
    mPendingEvent.type = SENSOR_TYPE_RELATIVE_HUMIDITY;
    mPendingEvent.acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mEnabledTime = 0;
    mDataDiv = 1;
    mPendingEvent.timestamp = 0;
    input_sysfs_path_len = 0;
    memset(input_sysfs_path, 0, sizeof(input_sysfs_path));
    m_hmdy_last_ts = 0;
    m_hmdy_delay = 0;
    mdata_fd = FindDataFd();
    if (mdata_fd >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_hmdy_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    } else {
        ALOGE("couldn't find input device ");
        return;
    }
    ALOGD("humidity misc path =%s", input_sysfs_path);

    char datapath[64] = {"/sys/class/sensor/m_hmdy_misc/hmdyactive"};
    int fd = TEMP_FAILURE_RETRY(open(datapath, O_RDWR));
    char buf[64] = {0};
    int len;
    if (fd >= 0) {
        len = TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf)-1));
        if (len <= 0) {
            ALOGD("read div err, len = %d", len);
        } else {
            buf[len] = '\0';
            sscanf(buf, "%d", &mDataDiv);
            ALOGD("read div buf(%s), mdiv %d", datapath, mDataDiv);
        }
        close(fd);
    } else {
    ALOGE("open hmdy misc path %s fail ", datapath);
    }
}

HumiditySensor::~HumiditySensor() {
    if (mdata_fd >= 0)
        close(mdata_fd);
}

int HumiditySensor::FindDataFd() {
    int fd = -1;

    fd = TEMP_FAILURE_RETRY(open(DEVICE_PATH, O_RDONLY));
    ALOGE_IF(fd < 0, "couldn't find sensor device");
    return fd;
}

int HumiditySensor::enable(int32_t handle, int en) {
    int fd = -1;
    int flags = en ? 1 : 0;

    ALOGD("humidity enable: handle:%d, en:%d\r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "hmdyactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    ALOGD("path:%s \r\n", input_sysfs_path);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
          ALOGD("no humidity enable control attr\r\n");
          return -1;
    }

    mEnabled = flags;
    char buf[2]={0};
    buf[1] = 0;
    if (flags) {
         buf[0] = '1';
             mEnabledTime = getTimestamp() + IGNORE_EVENT_TIME;
        m_hmdy_last_ts = 0;
    } else {
              buf[0] = '0';
    }
        TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
      close(fd);

        ALOGD("humidity enable(%d) done", mEnabled);
        return 0;
}
int HumiditySensor::setDelay(int32_t handle, int64_t ns) {
    int fd = -1;

    ALOGD("setDelay: (handle=%d, ns=%" PRId64 ")", handle, ns);
    m_hmdy_delay = ns;
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "hmdydelay", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
           ALOGD("no humidity setDelay control attr\r\n");
          return -1;
    }

    char buf[80]={0};
    sprintf(buf, "%" PRId64 "", ns);
    TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf)+1));
    close(fd);
        return 0;
}
int HumiditySensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    int flag = 0;
    int fd = -1;

    ALOGE("humidity batch: handle:%d, en:%d,samplingPeriodNs:%" PRId64 ", maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags,samplingPeriodNs, maxBatchReportLatencyNs);

        // Don't change batch status if dry run.
        if (flags & SENSORS_BATCH_DRY_RUN)
                return 0;

    if (maxBatchReportLatencyNs == 0) {
        flag = 0;
    } else {
        flag = 1;
    }

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "hmdybatch", sizeof(input_sysfs_path) - input_sysfs_path_len);
    ALOGD("path:%s \r\n", input_sysfs_path);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
          ALOGD("no humidity batch control attr\r\n");
          return -1;
    }

    char buf[2]={0};
    buf[1] = 0;
    if (flag) {
         buf[0] = '1';
    } else {
              buf[0] = '0';
    }
       TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
      close(fd);

    ALOGD("humidity batch(%d) done", flag);
    return 0;
}

int HumiditySensor::flush(int handle) {
    ALOGD("handle=%d\n", handle);
    return -errno;
}

int HumiditySensor::readEvents(sensors_event_t* data, int count) {
    // ALOGE("fwq read Event 1\r\n");
    if (count < 1)
        return -EINVAL;

    ssize_t n = mSensorReader.fill(mdata_fd);
    if (n < 0)
        return n;
    int numEventReceived = 0;
    struct sensor_event const* event;

    while (count && mSensorReader.readEvent(&event)) {
        processEvent(event);
        int64_t time = android::elapsedRealtimeNano();  // systemTime(SYSTEM_TIME_MONOTONIC);
        mPendingEvent.timestamp = time;
        if (mEnabled) {
            if (mPendingEvent.timestamp >= mEnabledTime) {
                float delta_mod = (float)(mPendingEvent.timestamp - m_hmdy_last_ts) / (float)(m_hmdy_delay);
                int loopcout = delta_mod;

                if (loopcout >= 1 && loopcout < 100) {
                    for (int i = 0; i < loopcout; i++) {
                        mPendingEvent.timestamp = time- (loopcout-i)*m_hmdy_delay;
                        if ((time - mPendingEvent.timestamp) < 450000000) {
                            *data++ = mPendingEvent;
                            numEventReceived++;
                            count--;
                            if (0 == count) {
                                break;
                            }
                        }
                    }
                }

                if (count != 0) {
                    mPendingEvent.timestamp = time;
                    *data++ = mPendingEvent;
                    numEventReceived++;
                }
            }
            if (count != 0)
                count--;
        }
        m_hmdy_last_ts = mPendingEvent.timestamp;

        mSensorReader.next();
    }
    // ALOGE("fwq read Event 2\r\n");
    return numEventReceived;
}

void HumiditySensor::processEvent(struct sensor_event const *event) {
    // ALOGD("processEvent code=%d,value=%d\r\n",code, value);
        mPendingEvent.relative_humidity = (float) event->word[0] / mDataDiv;
}
