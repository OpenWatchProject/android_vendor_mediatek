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
#include <Performance.h>
#include <log/log.h>

#include "Magnetic.h"
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <inttypes.h>

#undef LOG_TAG
#define LOG_TAG "Magnetic"

#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH           "/dev/m_mag_misc"
#define MAG_TAG_BIAS          "mag_bias"
#define MAG_TAG_CALI          "mag_cali"
#define MAG_BIAS_SAVED_DIR    DEFINE_PATH(MAG_TAG_BIAS)
#define MAG_CALI_SAVED_DIR    DEFINE_PATH(MAG_TAG_CALI)

MagneticSensor::MagneticSensor()
    : mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS) {
    mEnabled = 0;
    input_sysfs_path_len = 0;
    memset(input_sysfs_path, 0, sizeof(input_sysfs_path));
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_MAGNETIC;
    mPendingEvent.type = SENSOR_TYPE_MAGNETIC_FIELD;
    mPendingEvent.magnetic.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mPendingEvent.timestamp =0;

    mDataDiv = 1;
    mEnabledTime = 0;
    mFlushCnt = 0;
    magAccuracy = SENSOR_STATUS_UNRELIABLE;

    mSensorCalibration = NULL;

    char datapath1[64] = {"/sys/class/sensor/m_mag_misc/magactive"};
    int fd = -1;
    char buf[64] = {0};
    int len;
    float magBias[3] = {0};
    int32_t magCaliParam[6] = {0};
    int32_t sendBias[9] = {0};

    if (mSensorReader.selectSensorEventFd(DEVICE_PATH) >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_mag_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    } else {
        ALOGE("couldn't find input device\n");
    }
    ALOGD("misc path =%s", input_sysfs_path);

    fd = TEMP_FAILURE_RETRY(open(datapath1, O_RDWR));
    if (fd >= 0) {
        len = TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf) - 1));
        if (len <= 0) {
            ALOGE("read div err, len = %d", len);
        } else {
            buf[len] = '\0';
            sscanf(buf, "%d", &mDataDiv);
            ALOGI("read div buf(%s), mdiv_M %d\n", datapath1, mDataDiv);
        }
        close(fd);
    } else {
        ALOGE("open misc path %s fail\n", datapath1);
    }
    if (!mSensorSaved.getCalibrationFloat(MAG_BIAS_SAVED_DIR, MAG_TAG_BIAS, magBias, 3)) {
        memset(magBias, 0, sizeof(magBias));
        magAccuracy = SENSOR_STATUS_ACCURACY_MEDIUM;
    } else
        magAccuracy = SENSOR_STATUS_UNRELIABLE;
    ALOGI("read bias: [%f, %f, %f]\n", magBias[0], magBias[1], magBias[2]);
    if (!mSensorSaved.getCalibrationInt32(MAG_CALI_SAVED_DIR, MAG_TAG_CALI, magCaliParam, 6)) {
        memset(magCaliParam, 0, sizeof(magCaliParam));
    }
    ALOGI("read cali parameter: [%d, %d, %d, %d, %d, %d]\n",
        magCaliParam[0], magCaliParam[1], magCaliParam[2],
        magCaliParam[3], magCaliParam[4], magCaliParam[5]);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "magcali", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd >= 0) {
        sendBias[0] = (int32_t)(magBias[0] * mDataDiv);
        sendBias[1] = (int32_t)(magBias[1] * mDataDiv);
        sendBias[2] = (int32_t)(magBias[2] * mDataDiv);

        sendBias[3] = magCaliParam[0];
        sendBias[4] = magCaliParam[1];
        sendBias[5] = magCaliParam[2];
        sendBias[6] = magCaliParam[3];
        sendBias[7] = magCaliParam[4];
        sendBias[8] = magCaliParam[5];
        TEMP_FAILURE_RETRY(write(fd, sendBias, sizeof(sendBias)));
        close(fd);
    } else
        ALOGE("no cali attr\r\n");
#ifndef MAG_CALIBRATION_IN_SENSORHUB
    struct sensorData inputData;
    memset(&inputData, 0, sizeof(struct sensorData));

    inputData.data[0] = magBias[0];
    inputData.data[1] = magBias[1];
    inputData.data[2] = magBias[2];
    mSensorCalibration = SensorCalibration::getInstance();
    mSensorCalibration->magInitCalibration();
    mSensorCalibration->magSetCalibration(&inputData);
    mSensorManager = SensorManager::getInstance();
    mSensorConnection = mSensorManager->createSensorConnection(magnetic);
    gyroEnabled = false;
    accEnabled = false;
#endif
}

MagneticSensor::~MagneticSensor() {
#ifndef MAG_CALIBRATION_IN_SENSORHUB
    mSensorCalibration = NULL;
    mSensorManager->removeSensorConnection(mSensorConnection);
    mSensorManager = NULL;
#endif
}

int MagneticSensor::enable(int32_t handle, int en) {
    int fd = -1;
    char buf[2] = {0};

    ALOGI("enable: handle:%d, en:%d \r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "magactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no magntic enable attr\r\n");
        return -1;
    }

    if (0 == en) {
       mEnabled = false;
       buf[1] = 0;
       buf[0] = '0';
    }

    if (1 == en) {
        mEnabledTime = getTimestamp() + IGNORE_EVENT_TIME;
        mEnabled = true;
        buf[1] = 0;
        buf[0] = '1';
    }

    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);

#ifndef MAG_CALIBRATION_IN_SENSORHUB

    mSensorCalibration->magEnableCalibration(en);
#ifdef MAG_CALIBRATION_FAST
    if (!!en) {
        accEnabled = true;
        mSensorManager->batch(mSensorConnection, ID_ACCELEROMETER, 20000000, 0);
        mSensorManager->activate(mSensorConnection, ID_ACCELEROMETER, true);
        gyroEnabled = true;
        mSensorManager->batch(mSensorConnection, ID_GYROSCOPE, 20000000, 0);
        mSensorManager->activate(mSensorConnection, ID_GYROSCOPE, true);
    } else {
        if (!!accEnabled) {
            accEnabled = false;
            mSensorManager->activate(mSensorConnection, ID_ACCELEROMETER, false);
        }
        if (!!gyroEnabled) {
            gyroEnabled = false;
            mSensorManager->activate(mSensorConnection, ID_GYROSCOPE, false);
        }
    }
#endif
#endif
    return err < 0 ? err : 0;
}
int MagneticSensor::setDelay(int32_t handle, int64_t ns) {
    int fd;
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "magdelay", sizeof(input_sysfs_path) - input_sysfs_path_len);

    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no MAG setDelay control attr\r\n" );
        return -1;
    }

    ALOGI("setDelay: (handle=%d, ms=%" PRId64 ")", handle, ns);
    char buf[80] = {0};
    sprintf(buf, "%" PRId64 "", ns);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf)+1));

    close(fd);
    return err < 0 ? err : 0;
}
int MagneticSensor::batch(int handle, int flags,
        int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    int fd = -1;
    char buf[128] = {0};

    ALOGI("batch: handle:%d, flag:%d, samplingPeriodNs:%" PRId64 ",maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags, samplingPeriodNs,maxBatchReportLatencyNs);

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "magbatch", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no enable attr\r\n");
        return -1;
    }
    sprintf(buf, "%d,%d,%" PRId64 ",%" PRId64 "", handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return err < 0 ? err : 0;
}

int MagneticSensor::flush(int handle) {
    int fd = -1;
    char buf[32] = {0};

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "magflush", sizeof(input_sysfs_path) - input_sysfs_path_len);
    mFlushCnt++;
    ALOGI("flush, flushCnt:%d\n", mFlushCnt);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no handle: %d flush control attr\n", handle);
        return -1;
    }
    sprintf(buf, "%d", handle);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return err < 0 ? err : 0;
}

int MagneticSensor::setEvent(sensors_event_t * /*data*/) {
    struct sensorData inputData;

    // ALOGD("handle=%d x=%f, y=%f, z=%f\n", data->sensor, data->data[0], data->data[1], data->data[2]);
    memset(&inputData, 0, sizeof(struct sensorData));
#ifndef MAG_CALIBRATION_IN_SENSORHUB
#ifdef MAG_CALIBRATION_FAST
    inputData.timeStamp = data->timestamp;
    inputData.data[0] = data->data[0];
    inputData.data[1] = data->data[1];
    inputData.data[2] = data->data[2];
    if (data->sensor == ID_ACCELEROMETER) {
        inputData.status = data->acceleration.status;
        mSensorCalibration->magSetAccData(&inputData);
    } else if (data->sensor == ID_GYROSCOPE) {
        inputData.status = data->gyro.status;
        mSensorCalibration->magSetGyroData(&inputData);
    }
#endif
#endif
    return 0;
}

int MagneticSensor::readEvents(sensors_event_t* data, int count) {
    if (count < 1) {
        return -EINVAL;
    }

    ssize_t n = mSensorReader.fill();
    if (n < 0) {
        return n;
    }
    int numEventReceived = 0;
    struct sensor_event const* event;

    while (count && mSensorReader.readEvent(&event)) {
        processEvent(event);
        /* we only report DATA_ACTION and FLUSH_ACTION to framework */
        if (event->flush_action <= FLUSH_ACTION) {
            *data++ = mPendingEvent;
            numEventReceived++;
            count--;
        }
        mSensorReader.next();
    }
    return numEventReceived;
}

bool MagneticSensor::pendingEvent(void) {
    return mSensorReader.pendingEvent();
}

#define MIN_MAG_SQ              (10.0f * 10.0f)
#define MAX_MAG_SQ              (80.0f * 80.0f)

uint8_t MagneticSensor::magAccuracyUpdate(float x, float y, float z, uint8_t status) {
    float magSq = x * x + y * y + z * z;

    if (status == SENSOR_STATUS_ACCURACY_HIGH) {
        magAccuracy = SENSOR_STATUS_ACCURACY_HIGH;
        return magAccuracy;
    }
    if (magSq < MIN_MAG_SQ || magSq > MAX_MAG_SQ){
        magAccuracy = SENSOR_STATUS_UNRELIABLE;
        return magAccuracy;
    } else {
        magAccuracy = SENSOR_STATUS_ACCURACY_MEDIUM;
        return magAccuracy;
    }
    return SENSOR_STATUS_UNRELIABLE;
}

void MagneticSensor::processEvent(struct sensor_event const *event) {
    float magBias[3] = {0};
    int32_t magCaliParam[6] = {0};
#ifndef MAG_CALIBRATION_IN_SENSORHUB
    struct sensorData inputData;
    struct sensorData outputData;
    static int32_t lastStatus = 0;

    memset(&inputData, 0, sizeof(struct sensorData));
    memset(&outputData, 0, sizeof(struct sensorData));
#endif

    if (event->flush_action == DATA_ACTION) {
        mPendingEvent.version = sizeof(sensors_event_t);
        mPendingEvent.sensor = ID_MAGNETIC;
        mPendingEvent.type = SENSOR_TYPE_MAGNETIC_FIELD;
        mPendingEvent.timestamp = event->time_stamp;
        mPendingEvent.magnetic.status = event->status;
        mPendingEvent.magnetic.x = (float)event->word[0] / (float)mDataDiv;
        mPendingEvent.magnetic.y = (float)event->word[1] / (float)mDataDiv;
        mPendingEvent.magnetic.z = (float)event->word[2] / (float)mDataDiv;

#ifndef MAG_CALIBRATION_IN_SENSORHUB
        inputData.data[0] = mPendingEvent.magnetic.x;
        inputData.data[1] = mPendingEvent.magnetic.y;
        inputData.data[2] = mPendingEvent.magnetic.z;
        inputData.status = mPendingEvent.magnetic.status;
        inputData.timeStamp = mPendingEvent.timestamp;
        mSensorCalibration->magRunCalibration(&inputData, &outputData);
        mPendingEvent.magnetic.x = outputData.data[0];
        mPendingEvent.magnetic.y = outputData.data[1];
        mPendingEvent.magnetic.z = outputData.data[2];
        mPendingEvent.magnetic.status = outputData.status;
        if (mPendingEvent.magnetic.status != lastStatus) {
            mSensorCalibration->magGetCalibration(&outputData);
            magBias[0] = outputData.data[0];
            magBias[1] = outputData.data[1];
            magBias[2] = outputData.data[2];
            mSensorSaved.saveCalibrationFloat(MAG_BIAS_SAVED_DIR, MAG_TAG_BIAS, magBias, 3);
            ALOGI("write bias: [%f, %f, %f]\n", magBias[0], magBias[1], magBias[2]);
        }
        lastStatus = mPendingEvent.magnetic.status;
#else
        mPendingEvent.magnetic.status = magAccuracyUpdate(mPendingEvent.magnetic.x,
            mPendingEvent.magnetic.y, mPendingEvent.magnetic.z, event->status);
#endif
#ifdef DEBUG_PERFORMANCE
        if(1 == event->reserved) {
            mPendingEvent.gyro.status = event->reserved;
            mark_timestamp(ID_MAGNETIC, android::elapsedRealtimeNano(), mPendingEvent.timestamp);
        }
#endif
    } else if (event->flush_action == FLUSH_ACTION) {
        mPendingEvent.version = META_DATA_VERSION;
        mPendingEvent.sensor = 0;
        mPendingEvent.type = SENSOR_TYPE_META_DATA;
        mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
        mPendingEvent.meta_data.sensor = ID_MAGNETIC;
        // must fill timestamp, if not, readEvents may can not report flush to framework
        mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
        mFlushCnt--;
        ALOGI("flush complete, flushCnt:%d\n", mFlushCnt);
    } else if (event->flush_action == BIAS_ACTION) {
        magBias[0] = (float)event->word[0] / (float)mDataDiv;
        magBias[1] = (float)event->word[1] / (float)mDataDiv;
        magBias[2] = (float)event->word[2] / (float)mDataDiv;
        mSensorSaved.saveCalibrationFloat(MAG_BIAS_SAVED_DIR, MAG_TAG_BIAS, magBias, 3);
        ALOGI("write bias: [%f, %f, %f]\n", magBias[0], magBias[1], magBias[2]);
    }  else if (event->flush_action == CALI_ACTION) {
        magCaliParam[0] = event->word[0];
        magCaliParam[1] = event->word[1];
        magCaliParam[2] = event->word[2];
        magCaliParam[3] = event->word[3];
        magCaliParam[4] = event->word[4];
        magCaliParam[5] = event->word[5];
        mSensorSaved.saveCalibrationInt32(MAG_CALI_SAVED_DIR, MAG_TAG_CALI, magCaliParam, 6);
        ALOGI("write cali parameter: [%d, %d, %d, %d, %d, %d]\n", magCaliParam[0], magCaliParam[1], magCaliParam[2],
            magCaliParam[3], magCaliParam[4], magCaliParam[5]);
    } else
        ALOGE("unknow action\n");
}
