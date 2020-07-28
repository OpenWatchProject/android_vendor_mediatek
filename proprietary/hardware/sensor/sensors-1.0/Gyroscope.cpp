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

#include <utils/Timers.h>
#include <log/log.h>

#include "Gyroscope.h"
#include <utils/SystemClock.h>
#include <string.h>
#include <Performance.h>
#include <inttypes.h>

#undef LOG_TAG
#define LOG_TAG "Gyroscope"

#define IGNORE_EVENT_TIME 0
#define GYRO_TEMPERATURE_CONVERT 10000000

#define DEVICE_PATH           "/dev/m_gyro_misc"
#define GYRO_TAG_BIAS         "gyro_bias"
#define GYRO_TAG_CALI         "gyro_cali"
#define GYRO_TAG_TEMP         "gyro_temp"
#define GYRO_BIAS_SAVED_DIR   DEFINE_PATH(GYRO_TAG_BIAS)
#define GYRO_CALI_SAVED_DIR   DEFINE_PATH(GYRO_TAG_CALI)
#define GYRO_TEMP_SAVED_DIR   DEFINE_PATH(GYRO_TAG_TEMP)

HwGyroscopeSensor::HwGyroscopeSensor()
    : mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS) {
    mEnabled = 0;
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_GYROSCOPE;
    mPendingEvent.type = SENSOR_TYPE_GYROSCOPE;
    mPendingEvent.acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mEnabledTime =0;
    mFlushCnt.store(0, std::memory_order_relaxed);
    mDataDiv = 1;
    mPendingEvent.timestamp = 0;
    input_sysfs_path_len = 0;
    mSensorCalibration = NULL;

    memset(input_sysfs_path, 0, sizeof(input_sysfs_path));

    char datapath[64]={"/sys/class/sensor/m_gyro_misc/gyroactive"};
    int fd = -1;
    char buf[64]={0};
    int len;
    float gyroBias[3] = {0};
    int32_t gyroCali[3] = {0};
    int32_t gyroTemp[6] = {0};
    int32_t sendBias[12] = {0};

    if (mSensorReader.selectSensorEventFd(DEVICE_PATH) >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_gyro_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    } else {
        ALOGE("couldn't find sensor device");
    }
    ALOGD("misc path =%s", input_sysfs_path);

    fd = TEMP_FAILURE_RETRY(open(datapath, O_RDWR));
    if (fd >= 0) {
        len = TEMP_FAILURE_RETRY(read(fd,buf,sizeof(buf)-1));
        if (len <= 0) {
            ALOGE("read div err, len = %d", len);
        } else {
            buf[len] = '\0';
            sscanf(buf, "%d", &mDataDiv);
            ALOGI("read div buf(%s), mdiv %d", datapath, mDataDiv);
        }
        close(fd);
    } else {
        ALOGE("open misc path %s fail ", datapath);
    }
    if (!mSensorSaved.getCalibrationFloat(GYRO_BIAS_SAVED_DIR, GYRO_TAG_BIAS, gyroBias, 3))
        memset(gyroBias, 0, sizeof(gyroBias));
    ALOGI("read bias: [%f, %f, %f]\n", gyroBias[0], gyroBias[1], gyroBias[2]);
    if (!mSensorSaved.getCalibrationInt32(GYRO_CALI_SAVED_DIR, GYRO_TAG_CALI, gyroCali, 3))
        memset(gyroCali, 0, sizeof(gyroCali));
    ALOGI("read cali: [%d, %d, %d]\n", gyroCali[0], gyroCali[1], gyroCali[2]);
    if (!mSensorSaved.getCalibrationInt32(GYRO_TEMP_SAVED_DIR, GYRO_TAG_TEMP, gyroTemp, 6))
        memset(gyroTemp, 0, sizeof(gyroTemp));
    ALOGI("read temp: [%d, %d, %d, %d, %d, %d]\n", gyroTemp[0], gyroTemp[1], gyroTemp[2],
        gyroTemp[3], gyroTemp[4], gyroTemp[5]);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "gyrocali", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd >= 0) {
        /* dynamic bias */
        sendBias[0] = (int32_t)(gyroBias[0] * mDataDiv);
        sendBias[1] = (int32_t)(gyroBias[1] * mDataDiv);
        sendBias[2] = (int32_t)(gyroBias[2] * mDataDiv);
        /* static cali */
        sendBias[3] = gyroCali[0];
        sendBias[4] = gyroCali[1];
        sendBias[5] = gyroCali[2];
        /* temperature cali */
        sendBias[6] = gyroTemp[0];
        sendBias[7] = gyroTemp[1];
        sendBias[8] = gyroTemp[2];
        sendBias[9] = gyroTemp[3];
        sendBias[10] = gyroTemp[4];
        sendBias[11] = gyroTemp[5];
        TEMP_FAILURE_RETRY(write(fd, sendBias, sizeof(sendBias)));
        close(fd);
    } else
        ALOGE("no cali attr\r\n");
#ifndef ACC_GYRO_CALIBRATION_IN_SENSORHUB
#ifdef ACC_GYRO_CALIBRATION_SUPPORT
    struct sensorData inputData;
    memset(&inputData, 0, sizeof(struct sensorData));

    mSensorCalibration = SensorCalibration::getInstance();
    mSensorCalibration->gyroInitCalibration();
    // set bias to lib
    inputData.data[0] = gyroBias[0];
    inputData.data[1] = gyroBias[1];
    inputData.data[2] = gyroBias[2];
    mSensorCalibration->gyroSetCalibration(&inputData);
    // set temp cali paramter to lib
    inputData.data[0] = (float)gyroTemp[0] / GYRO_TEMPERATURE_CONVERT;
    inputData.data[1] = (float)gyroTemp[1] / GYRO_TEMPERATURE_CONVERT;
    inputData.data[2] = (float)gyroTemp[2] / GYRO_TEMPERATURE_CONVERT;
    inputData.data[3] = (float)gyroTemp[3] / GYRO_TEMPERATURE_CONVERT;
    inputData.data[4] = (float)gyroTemp[4] / GYRO_TEMPERATURE_CONVERT;
    inputData.data[5] = (float)gyroTemp[5] / GYRO_TEMPERATURE_CONVERT;
    mSensorCalibration->gyroSetTempCaliParameter(&inputData);
#endif
#endif
}

HwGyroscopeSensor::~HwGyroscopeSensor() {
}

int HwGyroscopeSensor::enable(int32_t handle, int en) {
    int fd=-1;
    int flags = en ? 1 : 0;
#ifndef ACC_GYRO_CALIBRATION_IN_SENSORHUB
#ifdef ACC_GYRO_CALIBRATION_SUPPORT
    float gyroBias[3] = {0};
    int32_t gyroTemp[6] = {0};
    struct sensorData outputData;

    memset(&outputData, 0, sizeof(struct sensorData));
#endif
#endif

    ALOGI("enable: handle:%d, en:%d\r\n", handle, en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "gyroactive", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no enable control attr\r\n" );
        return -1;
    }

    mEnabled = flags;
    char buf[2]={0};
    buf[1] = 0;
    if (flags) {
        buf[0] = '1';
        mEnabledTime = getTimestamp() + IGNORE_EVENT_TIME;
    } else {
        buf[0] = '0';
#ifndef ACC_GYRO_CALIBRATION_IN_SENSORHUB
#ifdef ACC_GYRO_CALIBRATION_SUPPORT
        if (mPendingEvent.gyro.status == 3) {
            mSensorCalibration->gyroGetCalibration(&outputData);
            gyroBias[0] = outputData.data[0];
            gyroBias[1] = outputData.data[1];
            gyroBias[2] = outputData.data[2];
            mSensorSaved.saveCalibrationFloat(GYRO_BIAS_SAVED_DIR, GYRO_TAG_BIAS, gyroBias, 3);
            ALOGI("write bias: [%f, %f, %f]\n", gyroBias[0], gyroBias[1], gyroBias[2]);
            mSensorCalibration->gyroGetTempCaliParameter(&outputData);
            gyroTemp[0] = outputData.data[0] * GYRO_TEMPERATURE_CONVERT;
            gyroTemp[1] = outputData.data[1] * GYRO_TEMPERATURE_CONVERT;
            gyroTemp[2] = outputData.data[2] * GYRO_TEMPERATURE_CONVERT;
            gyroTemp[3] = outputData.data[3] * GYRO_TEMPERATURE_CONVERT;
            gyroTemp[4] = outputData.data[4] * GYRO_TEMPERATURE_CONVERT;
            gyroTemp[5] = outputData.data[5] * GYRO_TEMPERATURE_CONVERT;
            mSensorSaved.saveCalibrationInt32(GYRO_TEMP_SAVED_DIR, GYRO_TAG_TEMP, gyroTemp, 6);
            ALOGI("write temp cali: [%d, %d, %d, %d, %d, %d]\n", gyroTemp[0], gyroTemp[1], gyroTemp[2],
                gyroTemp[3], gyroTemp[4], gyroTemp[5]);
        }
#endif
#endif
    }
    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return err < 0 ? err : 0;
}

int HwGyroscopeSensor::setDelay(int32_t handle, int64_t ns) {
    int fd=-1;
    ALOGI("setDelay: (handle=%d, ns=%" PRId64 ")", handle, ns);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "gyrodelay", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no setDelay control attr \r\n" );
        return -1;
    }
    char buf[80]={0};
    sprintf(buf, "%" PRId64 "", ns);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf)+1));
    close(fd);
    return err < 0 ? err : 0;
}

int HwGyroscopeSensor::batch(int handle, int flags,
        int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    int fd = -1;
    char buf[128] = {0};

    ALOGI("batch: handle:%d, flag:%d,samplingPeriodNs:%" PRId64 " maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags,samplingPeriodNs, maxBatchReportLatencyNs);

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "gyrobatch", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no batch control attr\r\n");
        return -1;
    }
    sprintf(buf, "%d,%d,%" PRId64 ",%" PRId64 "", handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return err < 0 ? err : 0;
}

int HwGyroscopeSensor::flush(int handle) {
    int fd = -1;
    char buf[32] = {0};

    mFlushCnt.fetch_add(1, std::memory_order_relaxed);
    ALOGI("flush, flushCnt:%d\n", mFlushCnt.load(std::memory_order_relaxed));
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "gyroflush", sizeof(input_sysfs_path) - input_sysfs_path_len);
    ALOGI("flush path:%s\r\n",input_sysfs_path);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no flush control attr\r\n");
        return -1;
    }
    sprintf(buf, "%d", handle);
    int err = TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return err < 0 ? err : 0;
}

int HwGyroscopeSensor::readEvents(sensors_event_t* data, int count) {
    if (count < 1)
        return -EINVAL;

    ssize_t n = mSensorReader.fill();
    if (n < 0)
        return n;
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

bool HwGyroscopeSensor::pendingEvent(void) {
    return mSensorReader.pendingEvent();
}

void HwGyroscopeSensor::processEvent(struct sensor_event const *event) {
    float gyroBias[3] = {0};
    int32_t gyroCali[3] = {0};
    int32_t gyroTemp[6] = {0};
#ifndef ACC_GYRO_CALIBRATION_IN_SENSORHUB
#ifdef ACC_GYRO_CALIBRATION_SUPPORT
    struct sensorData inputData;
    struct sensorData outputData;
    static int32_t lastStatus = 0;
    short int temp = 0;

    memset(&inputData, 0, sizeof(struct sensorData));
    memset(&outputData, 0, sizeof(struct sensorData));
#endif
#endif

    if (event->flush_action == DATA_ACTION) {
        mPendingEvent.version = sizeof(sensors_event_t);
        mPendingEvent.sensor = ID_GYROSCOPE;
        mPendingEvent.type = SENSOR_TYPE_GYROSCOPE;
        mPendingEvent.timestamp = event->time_stamp;
        mPendingEvent.gyro.status = event->status;
        mPendingEvent.gyro.x = (float)event->word[0] / mDataDiv;
        mPendingEvent.gyro.y = (float)event->word[1] / mDataDiv;
        mPendingEvent.gyro.z = (float)event->word[2] / mDataDiv;
#ifndef ACC_GYRO_CALIBRATION_IN_SENSORHUB
#ifdef ACC_GYRO_CALIBRATION_SUPPORT
        temp = (short int)event->word[3];
        if (temp < 0)
            inputData.data[3] = (float)(23 - (float)(0-temp)/512);
        else
            inputData.data[3] = (float)(23 + (float)temp / 512);
        inputData.data[0] = mPendingEvent.gyro.x;
        inputData.data[1] = mPendingEvent.gyro.y;
        inputData.data[2] = mPendingEvent.gyro.z;
        inputData.status = mPendingEvent.gyro.status;
        inputData.timeStamp = mPendingEvent.timestamp;
        mSensorCalibration->gyroRunCalibration(&inputData, &outputData);
        mPendingEvent.gyro.x = outputData.data[0];
        mPendingEvent.gyro.y = outputData.data[1];
        mPendingEvent.gyro.z = outputData.data[2];
        mPendingEvent.gyro.status = outputData.status;
        if (mPendingEvent.gyro.status != lastStatus) {
            mSensorCalibration->gyroGetCalibration(&outputData);
            gyroBias[0] = outputData.data[0];
            gyroBias[1] = outputData.data[1];
            gyroBias[2] = outputData.data[2];
            mSensorSaved.saveCalibrationFloat(GYRO_BIAS_SAVED_DIR, GYRO_TAG_BIAS, gyroBias, 3);
            ALOGI("write bias: [%f, %f, %f]\n", gyroBias[0], gyroBias[1], gyroBias[2]);
            mSensorCalibration->gyroGetTempCaliParameter(&outputData);
            gyroTemp[0] = outputData.data[0] * GYRO_TEMPERATURE_CONVERT;
            gyroTemp[1] = outputData.data[1] * GYRO_TEMPERATURE_CONVERT;
            gyroTemp[2] = outputData.data[2] * GYRO_TEMPERATURE_CONVERT;
            gyroTemp[3] = outputData.data[3] * GYRO_TEMPERATURE_CONVERT;
            gyroTemp[4] = outputData.data[4] * GYRO_TEMPERATURE_CONVERT;
            gyroTemp[5] = outputData.data[5] * GYRO_TEMPERATURE_CONVERT;
            mSensorSaved.saveCalibrationInt32(GYRO_TEMP_SAVED_DIR, GYRO_TAG_TEMP, gyroTemp, 6);
            ALOGI("write temp cali: [%d, %d, %d, %d, %d, %d]\n", gyroTemp[0], gyroTemp[1], gyroTemp[2],
                gyroTemp[3], gyroTemp[4], gyroTemp[5]);
        }
        lastStatus = mPendingEvent.gyro.status;
#endif
#endif
#ifdef DEBUG_PERFORMANCE
        if(1 == event->reserved) {
            mPendingEvent.gyro.status = event->reserved;
            mark_timestamp(ID_GYROSCOPE, android::elapsedRealtimeNano(), mPendingEvent.timestamp);
        }
#endif
    } else if (event->flush_action == FLUSH_ACTION) {
        mPendingEvent.version = META_DATA_VERSION;
        mPendingEvent.sensor = 0;
        mPendingEvent.type = SENSOR_TYPE_META_DATA;
        mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
        mPendingEvent.meta_data.sensor = ID_GYROSCOPE;
        // must fill timestamp, if not, readEvents may can not report flush to framework
        mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
        mFlushCnt.fetch_sub(1, std::memory_order_relaxed);
        ALOGI("flush complete, flushCnt:%d\n", mFlushCnt.load(std::memory_order_relaxed));
    } else if (event->flush_action == BIAS_ACTION) {
        gyroBias[0] = (float)event->word[0] / mDataDiv;
        gyroBias[1] = (float)event->word[1] / mDataDiv;
        gyroBias[2] = (float)event->word[2] / mDataDiv;
        mSensorSaved.saveCalibrationFloat(GYRO_BIAS_SAVED_DIR, GYRO_TAG_BIAS, gyroBias, 3);
        ALOGI("write bias: [%f, %f, %f]\n", gyroBias[0], gyroBias[1], gyroBias[2]);
    } else if (event->flush_action == CALI_ACTION) {
        gyroCali[0] = event->word[0];
        gyroCali[1] = event->word[1];
        gyroCali[2] = event->word[2];
        mSensorSaved.saveCalibrationInt32(GYRO_CALI_SAVED_DIR, GYRO_TAG_CALI, gyroCali, 3);
        ALOGI("write cali: [%d, %d, %d]\n", gyroCali[0], gyroCali[1], gyroCali[2]);
    } else if (event->flush_action == TEMP_ACTION) {
        gyroTemp[0] = event->word[0];
        gyroTemp[1] = event->word[1];
        gyroTemp[2] = event->word[2];
        gyroTemp[3] = event->word[3];
        gyroTemp[4] = event->word[4];
        gyroTemp[5] = event->word[5];
        mSensorSaved.saveCalibrationInt32(GYRO_TEMP_SAVED_DIR, GYRO_TAG_TEMP, gyroTemp, 6);
        ALOGI("write temp cali: [%d, %d, %d, %d, %d, %d]\n", gyroTemp[0], gyroTemp[1], gyroTemp[2],
            gyroTemp[3], gyroTemp[4], gyroTemp[5]);
    } else
        ALOGE("unknow action\n");
}

#undef LOG_TAG
#define LOG_TAG "VGyroscope"

#define FLOAT_TO_INTEGER 1000000

#define MAX_SAMPLE_DELAY_NS   0x7FFFFFFFFFFFFFFFLL
#define MAX_REPORT_LETANCY_NS 0x7FFFFFFFFFFFFFFFLL

#ifndef MAGNETOMETER_MINDELAY
#define MIN_MAG_SAMPLE_DELAY_NS  20000000
#else
#define MIN_MAG_SAMPLE_DELAY_NS  (MAGNETOMETER_MINDELAY * 1000)
#endif

#ifndef ACCELEROMETER_MINDELAY
#define MIN_ACC_SAMPLE_DELAY_NS  10000000
#else
#define MIN_ACC_SAMPLE_DELAY_NS  (ACCELEROMETER_MINDELAY * 1000)
#endif

VirtGyroscopeSensor::VirtGyroscopeSensor()
    : mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS) {
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_GYROSCOPE;
    mPendingEvent.type = SENSOR_TYPE_GYROSCOPE;
    mPendingEvent.acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mPendingEvent.timestamp = 0;

    mWritePipeFd = -1;

    mSamplingPeriodNs = MAX_SAMPLE_DELAY_NS;
    mBatchReportLatencyNs = MAX_REPORT_LETANCY_NS;

    accEnabled = 0;
    magEnabled = 0;

    if (mSensorReader.selectSensorEventFd(NULL) >= 0)
        mWritePipeFd = mSensorReader.getWriteFd();

    mVendorInterface = VendorInterface::getInstance();
    mSensorManager = SensorManager::getInstance();
    mSensorConnection = mSensorManager->createSensorConnection(gyro);
}

VirtGyroscopeSensor::~VirtGyroscopeSensor() {
    mVendorInterface = NULL;
    mSensorManager->removeSensorConnection(mSensorConnection);
    mSensorManager = NULL;
}

int VirtGyroscopeSensor::reportFlush() {
    struct sensor_event event;

    memset(&event, 0, sizeof(struct sensor_event));
    event.handle = ID_GYROSCOPE;
    event.flush_action = FLUSH_ACTION;
    mSensorReader.writeEvent(&event);
    return 0;
}

int VirtGyroscopeSensor::reportData(struct sensorData *data) {
    struct sensor_event event;

    memset(&event, 0, sizeof(struct sensor_event));
    event.handle = ID_GYROSCOPE;
    event.time_stamp = data->timeStamp;
    event.flush_action = DATA_ACTION;
    event.word[0] = data->data[0] * FLOAT_TO_INTEGER;
    event.word[1] = data->data[1] * FLOAT_TO_INTEGER;
    event.word[2] = data->data[2] * FLOAT_TO_INTEGER;
    event.status = data->status;
    mSensorReader.writeEvent(&event);
    return 0;
}

int VirtGyroscopeSensor::getVirtualGyroAndReport() {
    struct sensorData outputData;

    memset(&outputData, 0, sizeof(struct sensorData));
    mVendorInterface->getVirtualGyro(&outputData);
    reportData(&outputData);
    return 0;
}

int VirtGyroscopeSensor::enable(int32_t handle, int en) {
    int64_t samplingPeriodNs = 0, maxBatchReportLatencyNs = 0;

    ALOGI("enable, handle:%d, en:%d\n", handle, en);

    if (!!en) {
        if (mBatchReportLatencyNs < MAX_REPORT_LETANCY_NS)
            maxBatchReportLatencyNs = mBatchReportLatencyNs;
        else
            maxBatchReportLatencyNs = MAX_REPORT_LETANCY_NS;
        /* acc enable */
        if (mSamplingPeriodNs < MIN_ACC_SAMPLE_DELAY_NS)
            samplingPeriodNs = MIN_ACC_SAMPLE_DELAY_NS;
        else
            samplingPeriodNs = mSamplingPeriodNs;
        mSensorManager->batch(mSensorConnection, ID_ACCELEROMETER, samplingPeriodNs, maxBatchReportLatencyNs);
        if (!accEnabled) {
            accEnabled = true;
            mSensorManager->activate(mSensorConnection, ID_ACCELEROMETER, true);
        }
        /* mag enable */
        if (mSamplingPeriodNs < MIN_MAG_SAMPLE_DELAY_NS)
            samplingPeriodNs = MIN_MAG_SAMPLE_DELAY_NS;
        else
            samplingPeriodNs = mSamplingPeriodNs;
        mSensorManager->batch(mSensorConnection, ID_MAGNETIC, samplingPeriodNs, maxBatchReportLatencyNs);
        if (!magEnabled) {
            magEnabled = true;
            mSensorManager->activate(mSensorConnection, ID_MAGNETIC, true);
        }
    } else {
        mSamplingPeriodNs = MAX_SAMPLE_DELAY_NS;
        mBatchReportLatencyNs = MAX_REPORT_LETANCY_NS;
        /* acc disable */
        if (accEnabled) {
            accEnabled = false;
            mSensorManager->activate(mSensorConnection, ID_ACCELEROMETER, false);
        }
        /* mag disable */
        if (magEnabled) {
            magEnabled = false;
            mSensorManager->activate(mSensorConnection, ID_MAGNETIC, false);
        }
    }
    return 0;
}

int VirtGyroscopeSensor::setDelay(int32_t handle, int64_t ns) {
    ALOGD("setDelay: (handle=%d, ns=%" PRId64 ")", handle, ns);
    return 0;
}

int VirtGyroscopeSensor::batch(int handle, int flags,
    int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs) {
    ALOGI("batch: handle:%d, flag:%d,samplingPeriodNs:%" PRId64 " maxBatchReportLatencyNs:%" PRId64 "\n",
        handle, flags,samplingPeriodNs, maxBatchReportLatencyNs);

    mSamplingPeriodNs = samplingPeriodNs;
    mBatchReportLatencyNs = maxBatchReportLatencyNs;

    if (mBatchReportLatencyNs < MAX_REPORT_LETANCY_NS)
        maxBatchReportLatencyNs = mBatchReportLatencyNs;
    else
        maxBatchReportLatencyNs = MAX_REPORT_LETANCY_NS;
    /* acc batch */
    if (mSamplingPeriodNs < MIN_ACC_SAMPLE_DELAY_NS)
        samplingPeriodNs = MIN_ACC_SAMPLE_DELAY_NS;
    else
        samplingPeriodNs = mSamplingPeriodNs;
    if (accEnabled)
        mSensorManager->batch(mSensorConnection, ID_ACCELEROMETER, samplingPeriodNs, maxBatchReportLatencyNs);
    /* mag batch */
    if (mSamplingPeriodNs < MIN_MAG_SAMPLE_DELAY_NS)
        samplingPeriodNs = MIN_MAG_SAMPLE_DELAY_NS;
    else
        samplingPeriodNs = mSamplingPeriodNs;
    if (magEnabled)
        mSensorManager->batch(mSensorConnection, ID_MAGNETIC, samplingPeriodNs, maxBatchReportLatencyNs);
    return 0;
}

int VirtGyroscopeSensor::flush(int handle) {
    return mSensorManager->flush(mSensorConnection, ID_ACCELEROMETER);
}

int VirtGyroscopeSensor::setFlushEvent(sensors_event_t *data) {
    if (data->meta_data.sensor == ID_ACCELEROMETER) {
        reportFlush();
    }

    return 0;
}

int VirtGyroscopeSensor::setEvent(sensors_event_t *data) {
    struct sensorData inputData;

    /* ALOGD("setEvent handle=%d x=%f, y=%f, z=%f, timestamp=%lld\n",
		data->sensor, data->data[0], data->data[1], data->data[2], data->timestamp); */

    memset(&inputData, 0, sizeof(struct sensorData));

    inputData.timeStamp = data->timestamp;
    inputData.data[0] = data->data[0];
    inputData.data[1] = data->data[1];
    inputData.data[2] = data->data[2];
    if (data->sensor == ID_ACCELEROMETER) {
        inputData.status = data->acceleration.status;
        mVendorInterface->setAccData(&inputData);
        getVirtualGyroAndReport();
    } else if (data->sensor == ID_MAGNETIC) {
        inputData.status = data->magnetic.status;
        mVendorInterface->setMagData(&inputData);
    }
    return 0;
}

int VirtGyroscopeSensor::readEvents(sensors_event_t* data, int count) {
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

bool VirtGyroscopeSensor::pendingEvent(void) {
    return mSensorReader.pendingEvent();
}

void VirtGyroscopeSensor::processEvent(struct sensor_event const *event) {
    if (event->flush_action == DATA_ACTION) {
        mPendingEvent.version = sizeof(sensors_event_t);
        mPendingEvent.sensor = ID_GYROSCOPE;
        mPendingEvent.type = SENSOR_TYPE_GYROSCOPE;
        mPendingEvent.timestamp = event->time_stamp;
        mPendingEvent.gyro.status = event->status;
        mPendingEvent.gyro.x = (float)event->word[0] / FLOAT_TO_INTEGER;
        mPendingEvent.gyro.y = (float)event->word[1] / FLOAT_TO_INTEGER;
        mPendingEvent.gyro.z = (float)event->word[2] / FLOAT_TO_INTEGER;
    } else if (event->flush_action == FLUSH_ACTION) {
        mPendingEvent.version = META_DATA_VERSION;
        mPendingEvent.sensor = 0;
        mPendingEvent.type = SENSOR_TYPE_META_DATA;
        mPendingEvent.meta_data.what = META_DATA_FLUSH_COMPLETE;
        mPendingEvent.meta_data.sensor = ID_GYROSCOPE;
        // must fill timestamp, if not, readEvents may can not report flush to framework
        mPendingEvent.timestamp = android::elapsedRealtimeNano() + IGNORE_EVENT_TIME;
        // ALOGD("flush complete, flushCnt:%d\n", mFlushCnt);
    }
}
