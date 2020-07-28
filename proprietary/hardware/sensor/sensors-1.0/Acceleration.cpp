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
#include "Acceleration.h"
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <string.h>
#include <inttypes.h>
#include <Performance.h>

#undef LOG_TAG
#define LOG_TAG "Accelerometer"

#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH           "/dev/m_acc_misc"
#define ACC_TAG_BIAS          "acc_bias"
#define ACC_TAG_CALI          "acc_cali"
#define ACC_BIAS_SAVED_DIR    DEFINE_PATH(ACC_TAG_BIAS)
#define ACC_CALI_SAVED_DIR    DEFINE_PATH(ACC_TAG_CALI)

AccelerationSensor::AccelerationSensor()
    : mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS) {
    mEnabled = 0;
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_ACCELEROMETER;
    mPendingEvent.type = SENSOR_TYPE_ACCELEROMETER;
    mPendingEvent.acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mEnabledTime = 0;
    mFlushCnt = 0;
    mDataDiv = 1;
    mPendingEvent.timestamp =0;
    input_sysfs_path_len = 0;
    mSensorCalibration = NULL;

    memset(input_sysfs_path, 0, sizeof(input_sysfs_path));
    char datapath[64]={"/sys/class/sensor/m_acc_misc/accactive"};
    int fd = -1;
    char buf[64]={0};
    int len;
    float accBias[3] = {0};
    int32_t accCali[3] = {0};
    int32_t sendBias[6] = {0};

    if (mSensorReader.selectSensorEventFd(DEVICE_PATH) >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_acc_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    } else {
        ALOGE("couldn't find sensor device");
    }
    ALOGD("misc path =%s", input_sysfs_path);

    fd = TEMP_FAILURE_RETRY(open(datapath, O_RDWR));
    if (fd >= 0) {
        len = TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf) - 1));
        if (len <= 0) {
            ALOGE("read dev err, len = %d", len);
        } else {
            buf[len] = '\0';
            sscanf(buf, "%d", &mDataDiv);
            ALOGI("read div buf(%s), mdiv %d", datapath, mDataDiv);
        }
        close(fd);
    } else {
        ALOGE("open misc path %s fail ", datapath);
    }
    if (!mSensorSaved.getCalibrationFloat(ACC_BIAS_SAVED_DIR, ACC_TAG_BIAS, accBias, 3))
        memset(accBias, 0, sizeof(accBias));
    ALOGI("read bias: [%f, %f, %f]\n", accBias[0], accBias[1], accBias[2]);
    if (!mSensorSaved.getCalibrationInt32(ACC_CALI_SAVED_DIR, ACC_TAG_CALI, accCali, 3))
        memset(accCali, 0, sizeof(accCali));
    ALOGI("read cali: [%d, %d, %d]\n", accCali[0], accCali[1], accCali[2]);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "acccali", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd >= 0) {
        /* dynamic bias */
        sendBias[0] = (int32_t)(accBias[0] * mDataDiv);
        sendBias[1] = (int32_t)(accBias[1] * mDataDiv);
        sendBias[2] = (int32_t)(accBias[2] * mDataDiv);
        /* static cali */
        sendBias[3] = accCali[0];
        sendBias[4] = accCali[1];
        sendBias[5] = accCali[2];
        TEMP_FAILURE_RETRY(write(fd, sendBias, sizeof(sendBias)));
        close(fd);
    } else
        ALOGE("no cali attr\r\n");
#ifndef ACC_GYRO_CALIBRATION_IN_SENSORHUB
#ifdef ACC_GYRO_CALIBRATION_SUPPORT
    struct sensorData inputData;
    memset(&inputData, 0, sizeof(struct sensorData));

    inputData.data[0] = accBias[0];
    inputData.data[1] = accBias[1];
    inputData.data[2] = accBias[2];
    mSensorCalibration = SensorCalibration::getInstance();
    mSensorCalibration->accInitCalibration();
    mSensorCalibration->accSetCalibration(&inputData);
#endif
#endif
}

AccelerationSensor::~AccelerationSensor() {
}

int AccelerationSensor::enable(int32_t handle, int en) {
    int fd = -1;
    int flags = en ? 1 : 0;
    char buf[2] = {0};
#ifndef ACC_GYRO_CALIBRATION_IN_SENSORHUB
#ifdef ACC_GYRO_CALIBRATION_SUPPORT
    float accBias[3] = {0};
    struct sensorData outputData;

    memset(&outputData, 0, sizeof(struct sensorData));
#endif
#endif

    ALOGI("enable: handle:%d, en:%d\r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "accactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if(fd < 0) {
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
#ifndef ACC_GYRO_CALIBRATION_IN_SENSORHUB
#ifdef ACC_GYRO_CALIBRATION_SUPPORT
       if (mPendingEvent.acceleration.status == 3) {
           mSensorCalibration->accGetCalibration(&outputData);
           accBias[0] = outputData.data[0];
           accBias[1] = outputData.data[1];
           accBias[2] = outputData.data[2];
           mSensorSaved.saveCalibrationFloat(ACC_BIAS_SAVED_DIR, ACC_TAG_BIAS, accBias, 3);
           ALOGI("write bias: [%f, %f, %f]\n", accBias[0], accBias[1], accBias[2]);
       }
#endif
#endif
    }
    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return err < 0 ? err : 0;
}
int AccelerationSensor::setDelay(int32_t handle, int64_t ns) {
    int fd = -1;
    ALOGI("setDelay: (handle=%d, ns=%" PRId64 ")", handle, ns);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "accdelay", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no setDelay control attr \r\n" );
        return -1;
    }
    char buf[80] = {0};
    sprintf(buf, "%" PRId64 "", ns);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf)+1));
    close(fd);
    return err < 0 ? err : 0;
}

int AccelerationSensor::batch(int handle, int flags,
        int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    int fd = -1;
    char buf[128] = {0};

    ALOGI("batch: handle:%d, flag:%d,samplingPeriodNs:%" PRId64 " maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags,samplingPeriodNs, maxBatchReportLatencyNs);

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "accbatch", sizeof(input_sysfs_path) - input_sysfs_path_len);
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

int AccelerationSensor::flush(int handle) {
    int fd = -1;
    char buf[32] = {0};

    mFlushCnt++;
    ALOGI("flush, flushCnt:%d\n", mFlushCnt);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "accflush", sizeof(input_sysfs_path) - input_sysfs_path_len);
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

int AccelerationSensor::readEvents(sensors_event_t* data, int count) {
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
        if (event->flush_action <= FLUSH_ACTION) {
            /* auto cts request flush event when sensor disable, ALPS03452281 */
            *data++ = mPendingEvent;
            numEventReceived++;
            count--;
        }
        mSensorReader.next();
    }
    return numEventReceived;
}

bool AccelerationSensor::pendingEvent(void) {
    return mSensorReader.pendingEvent();
}

void AccelerationSensor::processEvent(struct sensor_event const *event) {
    float accBias[3] = {0};
    int32_t accCali[3] = {0};
#ifndef ACC_GYRO_CALIBRATION_IN_SENSORHUB
#ifdef ACC_GYRO_CALIBRATION_SUPPORT
    struct sensorData inputData;
    struct sensorData outputData;
    static int32_t lastStatus = 0;

    memset(&inputData, 0, sizeof(struct sensorData));
    memset(&outputData, 0, sizeof(struct sensorData));
#endif
#endif

    if (event->flush_action == DATA_ACTION) {
        mPendingEvent.version = sizeof(sensors_event_t);
        mPendingEvent.sensor = ID_ACCELEROMETER;
        mPendingEvent.type = SENSOR_TYPE_ACCELEROMETER;
        mPendingEvent.timestamp = event->time_stamp;
        mPendingEvent.acceleration.status = event->status;
        mPendingEvent.acceleration.x = (float)event->word[0] / mDataDiv;
        mPendingEvent.acceleration.y = (float)event->word[1] / mDataDiv;
        mPendingEvent.acceleration.z = (float)event->word[2] / mDataDiv;
#ifndef ACC_GYRO_CALIBRATION_IN_SENSORHUB
#ifdef ACC_GYRO_CALIBRATION_SUPPORT
        inputData.data[0] = mPendingEvent.acceleration.x;
        inputData.data[1] = mPendingEvent.acceleration.y;
        inputData.data[2] = mPendingEvent.acceleration.z;
        inputData.status = mPendingEvent.acceleration.status;
        inputData.timeStamp = mPendingEvent.timestamp;
        mSensorCalibration->accRunCalibration(&inputData, &outputData);
        mPendingEvent.acceleration.x = outputData.data[0];
        mPendingEvent.acceleration.y = outputData.data[1];
        mPendingEvent.acceleration.z = outputData.data[2];
        mPendingEvent.acceleration.status = outputData.status;
        if (mPendingEvent.acceleration.status != lastStatus) {
            mSensorCalibration->accGetCalibration(&outputData);
            accBias[0] = outputData.data[0];
            accBias[1] = outputData.data[1];
            accBias[2] = outputData.data[2];
            mSensorSaved.saveCalibrationFloat(ACC_BIAS_SAVED_DIR, ACC_TAG_BIAS, accBias, 3);
            ALOGI("write bias: [%f, %f, %f]\n", accBias[0], accBias[1], accBias[2]);
        }
        lastStatus = mPendingEvent.acceleration.status;
#endif
#endif
#ifdef DEBUG_PERFORMANCE
        if(1 == event->reserved) {
            mPendingEvent.acceleration.status = event->reserved;
            mark_timestamp(ID_ACCELEROMETER, android::elapsedRealtimeNano(), mPendingEvent.timestamp);
        }
#endif

    } else if (event->flush_action == FLUSH_ACTION) {
        mPendingEvent.version = META_DATA_VERSION;
        mPendingEvent.sensor = 0;
        mPendingEvent.type = SENSOR_TYPE_META_DATA;
        mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
        mPendingEvent.meta_data.sensor = ID_ACCELEROMETER;
        // must fill timestamp, if not, readEvents may can not report flush to framework
        mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
        mFlushCnt--;
        ALOGI("flush complete, flushCnt:%d\n", mFlushCnt);
    } else if (event->flush_action == BIAS_ACTION) {
        accBias[0] = (float)event->word[0] / mDataDiv;
        accBias[1] = (float)event->word[1] / mDataDiv;
        accBias[2] = (float)event->word[2] / mDataDiv;
        mSensorSaved.saveCalibrationFloat(ACC_BIAS_SAVED_DIR, ACC_TAG_BIAS, accBias, 3);
        ALOGI("write bias: [%f, %f, %f]\n", accBias[0], accBias[1], accBias[2]);
    } else if (event->flush_action == CALI_ACTION) {
        accCali[0] = event->word[0];
        accCali[1] = event->word[1];
        accCali[2] = event->word[2];
        mSensorSaved.saveCalibrationInt32(ACC_CALI_SAVED_DIR, ACC_TAG_CALI, accCali, 3);
        ALOGI("write cali: [%d, %d, %d]\n", accCali[0], accCali[1], accCali[2]);
    } else
        ALOGI("unknow action\n");
}
