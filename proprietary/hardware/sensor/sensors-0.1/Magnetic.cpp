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
#ifndef CUSTOM_KERNEL_SENSORHUB
#include <dlfcn.h>
#endif

#include <log/log.h>

#include "Magnetic.h"
#ifndef CUSTOM_KERNEL_SENSORHUB
#include "mag_calibration_lib.h"
#endif
#include <utils/SystemClock.h>
#include <utils/Timers.h>
#include <inttypes.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "Magnetic"
#endif

#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH           "/dev/m_mag_misc"
#define MAG_BIAS_SAVED_DIR    "/data/misc/sensor/mag_bias.json"
#define MAG_TAG_BIAS          "mag_bias"

/*****************************************************************************/
MagneticSensor::MagneticSensor()
    : SensorBase(NULL, "m_mag_input"),//ACC_INPUTDEV_NAME
      mEnabled(0),
      mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS)
{
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
    lib_interface = NULL;
    lib_handle = NULL;

    char datapath1[64] = {"/sys/class/sensor/m_mag_misc/magactive"};
    int fd = -1;
    char buf[64] = {0};
    int len;
    float magBias[3] = {0};
    int32_t sendBias[3] = {0};

    mdata_fd = FindDataFd();
    if (mdata_fd >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_mag_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    } else {
        ALOGE("couldn't find input device\n");
        return;
    }
    ALOGI("misc path =%s", input_sysfs_path);

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
    if (!mSensorCali.getCalibrationFloat(MAG_BIAS_SAVED_DIR, MAG_TAG_BIAS, magBias, 3))
        memset(magBias, 0, sizeof(magBias));
    ALOGI("read bias: [%f, %f, %f]\n", magBias[0], magBias[1], magBias[2]);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "magcali", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd >= 0) {
        sendBias[0] = (int32_t)(magBias[0] * mDataDiv);
        sendBias[1] = (int32_t)(magBias[1] * mDataDiv);
        sendBias[2] = (int32_t)(magBias[2] * mDataDiv);
        TEMP_FAILURE_RETRY(write(fd, sendBias, sizeof(sendBias)));
        close(fd);
    } else
        ALOGE("no cali attr\r\n");
#ifndef CUSTOM_KERNEL_SENSORHUB
    char libinfoPatch[64] = {"/sys/class/sensor/m_mag_misc/maglibinfo"};
    struct mag_libinfo_t libinfo;
    struct magChipInfo chipinfo;

    memset(&libinfo, 0, sizeof(struct mag_libinfo_t));
    memset(&chipinfo, 0, sizeof(struct magChipInfo));

    fd = TEMP_FAILURE_RETRY(open(libinfoPatch, O_RDWR));
    if (fd >= 0) {
        len = TEMP_FAILURE_RETRY(read(fd, &libinfo, sizeof(struct mag_libinfo_t)));
        libinfo.libname[sizeof(libinfo.libname) - 1] = '\0';
        if (len <= 0) {
            ALOGE("read libinfo err, len = %d\n", len);
            close(fd);
            return;
        }
        close(fd);
    } else
        ALOGE("open vendor libinfo fail\n");
    if (strlen(libinfo.libname) > (64 - 3 - 3 - 1)) { /* 3bytes "lib", 3bytes ".so", 1byte "\n" */
        ALOGE("invalid libname, length too long\n");
        return;
    }
    strlcpy(buf, "lib", sizeof(buf));
    strlcat(buf, libinfo.libname, sizeof(buf));
    strlcat(buf, ".so", sizeof(buf));

    lib_handle = dlopen(buf, RTLD_NOW);
    if (!lib_handle) {
        ALOGE("dlopen fail\n");
        return;
    }
    dlerror();
    lib_interface = (struct mag_lib_interface_t *)dlsym(lib_handle, "MAG_LIB_API_INTERFACE");
    if (!lib_interface) {
        ALOGE("dlsym fail\n");
        dlclose(lib_handle);
        lib_handle = NULL;
        return;
    }
    chipinfo.deviceid = libinfo.deviceid;
    chipinfo.layout = libinfo.layout;
    lib_interface->initLib(&chipinfo);
    lib_interface->caliApiSetOffset(magBias);
#endif
}

MagneticSensor::~MagneticSensor() {
    if (mdata_fd >= 0)
        close(mdata_fd);
#ifndef CUSTOM_KERNEL_SENSORHUB
    if (lib_handle)
        dlclose(lib_handle);
#endif
}

int MagneticSensor::FindDataFd() {
    int fd = -1;

    fd = TEMP_FAILURE_RETRY(open(DEVICE_PATH, O_RDONLY));
    ALOGE_IF(fd<0, "couldn't find sensor device");
    return fd;
}
int MagneticSensor::enable(int32_t handle, int en)
{
    int fd = -1;
    int flags = en ? 1 : 0;
    int err = 0;
    char buf[2] = {0};
    int index = 0;
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

    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}
int MagneticSensor::setDelay(int32_t handle, int64_t ns)
{
    //uint32_t ms=0;
    //ms = ns/1000000;
    int err;
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
    TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf)+1));

    close(fd);
    return 0;
}
int MagneticSensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
    int fd = -1;
    int flag = 0;
    char buf[128] = {0};

    ALOGI("batch: handle:%d, en:%d, samplingPeriodNs:%" PRId64 ",maxBatchReportLatencyNs:%" PRId64 "\r\n",
        handle, flags, samplingPeriodNs,maxBatchReportLatencyNs);

    strlcpy(&input_sysfs_path[input_sysfs_path_len], "magbatch", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no enable attr\r\n");
        return -1;
    }
    sprintf(buf, "%d,%d,%" PRId64 ",%" PRId64 "", handle, flags, samplingPeriodNs, maxBatchReportLatencyNs);
    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}

int MagneticSensor::flush(int handle)
{
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
    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}
int MagneticSensor::readEvents(sensors_event_t* data, int count)
{
    if (count < 1) {
        return -EINVAL;
    }

    ssize_t n = mSensorReader.fill(mdata_fd);
    if (n < 0) {
        return n;
    }
    int numEventReceived = 0;
    struct sensor_event const* event;

    while (count && mSensorReader.readEvent(&event)) {
        processEvent(event);
        /* we only report DATA_ACTION and FLUSH_ACTION to framework */
        if (event->flush_action <= FLUSH_ACTION) {
            if (mPendingEvent.timestamp > mEnabledTime) {
                *data++ = mPendingEvent;
                numEventReceived++;
                count--;
            }
        }
        mSensorReader.next();
    }
    return numEventReceived;
}

void MagneticSensor::processEvent(struct sensor_event const *event)
{
    float magBias[3] = {0};
#ifndef CUSTOM_KERNEL_SENSORHUB
    struct magCaliDataInPut inputData;
    struct magCaliDataOutPut outputData;
    static int32_t lastStatus = 0;

    memset(&inputData, 0, sizeof(struct magCaliDataInPut));
    memset(&outputData, 0, sizeof(struct magCaliDataOutPut));
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

#ifndef CUSTOM_KERNEL_SENSORHUB
        inputData.x = mPendingEvent.magnetic.x ;
        inputData.y = mPendingEvent.magnetic.y ;
        inputData.z = mPendingEvent.magnetic.z ;
        inputData.status = mPendingEvent.magnetic.status;
        inputData.timeStamp = mPendingEvent.timestamp;
        if (lib_interface != NULL) {
            lib_interface->doCaliApi(&inputData,&outputData);
        }
        mPendingEvent.magnetic.x = outputData.x;
        mPendingEvent.magnetic.y = outputData.y;
        mPendingEvent.magnetic.z = outputData.z;
        mPendingEvent.magnetic.status = outputData.status;
        magBias[0] = outputData.x_bias;
        magBias[1] = outputData.y_bias;
        magBias[2] = outputData.z_bias;
        if (mPendingEvent.magnetic.status != lastStatus) {
            mSensorCali.saveCalibrationFloat(MAG_BIAS_SAVED_DIR, MAG_TAG_BIAS, magBias, 3);
            ALOGI("write bias: [%f, %f, %f]\n", magBias[0], magBias[1], magBias[2]);
        }
        lastStatus = mPendingEvent.magnetic.status;
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
        mSensorCali.saveCalibrationFloat(MAG_BIAS_SAVED_DIR, MAG_TAG_BIAS, magBias, 3);
        ALOGI("write bias: [%f, %f, %f]\n", magBias[0], magBias[1], magBias[2]);
    } else
        ALOGE("unknow action\n");
}
