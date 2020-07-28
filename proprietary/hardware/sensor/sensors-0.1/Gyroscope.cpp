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

#ifdef MPE_HAL
#include "API_sensor_calibration.h"
#include "API_timestamp_calibration.h"
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "Gyroscope"
#endif

#define IGNORE_EVENT_TIME 0
#define DEVICE_PATH           "/dev/m_gyro_misc"
#define GYRO_BIAS_SAVED_DIR   "/data/misc/sensor/gyro_bias.json"
#define GYRO_CALI_SAVED_DIR   "/data/misc/sensor/gyro_cali.json"
#define GYRO_TEMP_SAVED_DIR   "/data/misc/sensor/gyro_temp.json"
#define GYRO_TAG_BIAS         "gyro_bias"
#define GYRO_TAG_CALI         "gyro_cali"
#define GYRO_TAG_TEMP         "gyro_temp"
#ifdef MPE_HAL
#define GYRO_BIAS_SAVED_DIR1   "/data/misc/sensor/gyro_saved.json"
#define GYRO_BIAS_SAVED_DIR2   "/data/misc/sensor/gyro_saved2.json"
#define GYRO_BIAS_SAVED_DIR3   "/data/misc/sensor/gyro_saved3.json"
#define GYRO_TAG_NAME          "gyro"
#define GYRO_TEMP_SLOPE        "gyro_temp_slope"
#define GYRO_TEMP_INTERCEPT    "gyro_temp_intercept"
#endif
/*****************************************************************************/
GyroscopeSensor::GyroscopeSensor()
    : SensorBase(NULL, "m_gyro_input"),//GYRO_INPUTDEV_NAME
      mEnabled(0),
      mSensorReader(BATCH_SENSOR_MAX_READ_INPUT_NUMEVENTS)
{
    mPendingEvent.version = sizeof(sensors_event_t);
    mPendingEvent.sensor = ID_GYROSCOPE;
    mPendingEvent.type = SENSOR_TYPE_GYROSCOPE;
    mPendingEvent.acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEvent.data, 0x00, sizeof(mPendingEvent.data));
    mPendingEvent.flags = 0;
    mPendingEvent.reserved0 = 0;
    mEnabledTime =0;
    mFlushCnt = 0;
    mDataDiv = 1;
    mPendingEvent.timestamp = 0;
#ifdef MPE_HAL
    mPendingEventNew.version = sizeof(sensors_event_t);
    mPendingEventNew.sensor = ID_GYROSCOPE;
    mPendingEventNew.type = SENSOR_TYPE_GYROSCOPE;
    mPendingEventNew.acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;
    memset(mPendingEventNew.data, 0x00, sizeof(mPendingEventNew.data));
    mPendingEventNew.flags = 0;
    mPendingEventNew.reserved0 = 0;
    mPendingEventNew.timestamp =0;
    data_cnt = 0;
    deltaT = 5000000;
    preGyrAcc = 0;
#endif
    input_sysfs_path_len = 0;
    memset(input_sysfs_path, 0, sizeof(input_sysfs_path));

    char datapath[64]={"/sys/class/sensor/m_gyro_misc/gyroactive"};
    int fd = -1;
    char buf[64]={0};
    int len;
    float gyroBias[3] = {0};
    int32_t gyroCali[3] = {0};
    int32_t gyroTemp[6] = {0};
    int32_t sendBias[12] = {0};

    mdata_fd = FindDataFd();
    if (mdata_fd >= 0) {
        strlcpy(input_sysfs_path, "/sys/class/sensor/m_gyro_misc/", sizeof(input_sysfs_path));
        input_sysfs_path_len = strlen(input_sysfs_path);
    } else {
        ALOGE("couldn't find input device ");
        return;
    }
    ALOGI("misc path =%s", input_sysfs_path);

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
    if (!mSensorCali.getCalibrationFloat(GYRO_BIAS_SAVED_DIR, GYRO_TAG_BIAS, gyroBias, 3))
        memset(gyroBias, 0, sizeof(gyroBias));
    ALOGI("read bias: [%f, %f, %f]\n", gyroBias[0], gyroBias[1], gyroBias[2]);
    if (!mSensorCali.getCalibrationInt32(GYRO_CALI_SAVED_DIR, GYRO_TAG_CALI, gyroCali, 3))
        memset(gyroCali, 0, sizeof(gyroCali));
    ALOGI("read cali: [%d, %d, %d]\n", gyroCali[0], gyroCali[1], gyroCali[2]);
    if (!mSensorCali.getCalibrationInt32(GYRO_TEMP_SAVED_DIR, GYRO_TAG_TEMP, gyroTemp, 6))
        memset(gyroCali, 0, sizeof(gyroCali));
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
}

GyroscopeSensor::~GyroscopeSensor() {
    if (mdata_fd >= 0)
        close(mdata_fd);
}
int GyroscopeSensor::FindDataFd() {
    int fd = -1;

    fd = TEMP_FAILURE_RETRY(open(DEVICE_PATH, O_RDONLY));
    ALOGE_IF(fd<0, "couldn't find sensor device");
    return fd;
}

int GyroscopeSensor::enableNoHALData(int en) {
    int fd = 0;
    char buf[2] = {0};
    ALOGI("enable nodata en(%d) \r\n", en);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "gyroenablenodata", sizeof(input_sysfs_path) - input_sysfs_path_len);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
          ALOGE("no enable nodata control attr\r\n");
          return -1;
    }

    buf[1] = 0;
    if (1 == en) {
        buf[0] = '1';
    }
    if (0 == en) {
        buf[0] = '0';
    }

    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
      close(fd);

    ALOGI("GYRO enable nodata done");
    return 0;
}
int GyroscopeSensor::enable(int32_t handle, int en)
{
    int fd=-1;
    int flags = en ? 1 : 0;

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
#ifdef MPE_HAL
        android::sp<android::JSONObject> bias_saved;
        android::sp<android::JSONObject> bias_saved2;
        android::sp<android::JSONObject> bias_saved3;
        float gyroBias[3] = {0};
        float gyro_Slope[3] = {0};
        float gyro_Intercept[3] = {0};
        preGyrAcc = 0;

        MPE_SENSOR_DATA mpe_data;
        MPE_SENSOR_DATA slope;
        MPE_SENSOR_DATA intercept;

        mLock.lock();
        mSensorCali.mpe_loadSensorSettings(GYRO_BIAS_SAVED_DIR1, &bias_saved);
        mSensorCali.mpe_loadSensorSettings(GYRO_BIAS_SAVED_DIR2, &bias_saved2);
        mSensorCali.mpe_loadSensorSettings(GYRO_BIAS_SAVED_DIR3, &bias_saved3);

        if (mSensorCali.mpe_getCalibrationFloat(bias_saved, GYRO_TAG_NAME, gyroBias) &&
            mSensorCali.mpe_getCalibrationFloat(bias_saved2, GYRO_TEMP_SLOPE, gyro_Slope) &&
            mSensorCali.mpe_getCalibrationFloat(bias_saved3, GYRO_TEMP_INTERCEPT, gyro_Intercept)) {

            mLock.unlock();
            ALOGE("MPE_gyr_calib_read %f %f %f,%f %f %f,%f %f %f\n", gyroBias[0], gyroBias[1], gyroBias[2], gyro_Slope[0], gyro_Slope[1], gyro_Slope[2], gyro_Intercept[0], gyro_Intercept[1], gyro_Intercept[2]);

            mpe_data.x = gyroBias[0];
            mpe_data.y = gyroBias[1];
            mpe_data.z = gyroBias[2];
            slope.x=gyro_Slope[0];
            slope.y=gyro_Slope[1];
            slope.z=gyro_Slope[2];
            intercept.x = gyro_Intercept[0];
            intercept.y = gyro_Intercept[1];
            intercept.z = gyro_Intercept[2];
            Gyro_init_calibration(mpe_data, slope, intercept);
        } else {

            mLock.unlock();
            ALOGE("MPE_gyro_read_data_fail\n");

            memset(&mpe_data, 0x00, sizeof(mpe_data));
            memset(&slope, 0x00, sizeof(slope));
            memset(&intercept, 0x00, sizeof(intercept));
            Gyro_init_calibration(mpe_data, slope, intercept);
        }
        data_cnt = 0;
#endif
    } else {
        buf[0] = '0';
#ifdef MPE_HAL
        float gyro_Bias[3] = {0};
        float gyro_Slope[3] = {0};
        float gyro_Intercept[3] = {0};
        int gyro_Accuracy;
        Gyro_get_calibration_parameter(gyro_Bias, &gyro_Accuracy, gyro_Slope, gyro_Intercept);

        if(gyro_Accuracy == 3) {
            mLock.lock();
            mSensorCali.saveSensorSettings(GYRO_BIAS_SAVED_DIR1, GYRO_TAG_NAME, gyro_Bias);
            mSensorCali.saveSensorSettings(GYRO_BIAS_SAVED_DIR2, GYRO_TEMP_SLOPE, gyro_Slope);
            mSensorCali.saveSensorSettings(GYRO_BIAS_SAVED_DIR3, GYRO_TEMP_INTERCEPT, gyro_Intercept);
            mLock.unlock();
            ALOGE("MPE_gyr_calib_write %f %f %f,%d,%f %f %f,%f %f %f,%f\n",
                gyro_Bias[0], gyro_Bias[1], gyro_Bias[2], gyro_Accuracy,
                gyro_Slope[0], gyro_Slope[1], gyro_Slope[2],
                gyro_Intercept[0], gyro_Intercept[1], gyro_Intercept[2], mPendingEvent.data[3]);
        }
#endif
    }
    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}

int GyroscopeSensor::setDelay(int32_t handle, int64_t ns)
{
    int fd=-1;
    ALOGI("setDelay: (handle=%d, ns=%" PRId64 ")", handle, ns);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "gyrodelay", sizeof(input_sysfs_path) - input_sysfs_path_len);
#ifdef MPE_HAL
    float gyro_Bias[3] = {0};
    float gyro_Slope[3] = {0};
    float gyro_Intercept[3] = {0};
    int gyro_Accuracy;
    Gyro_get_calibration_parameter(gyro_Bias, &gyro_Accuracy, gyro_Slope, gyro_Intercept);

    if(gyro_Accuracy == 3) {
        mLock.lock();
        mSensorCali.saveSensorSettings(GYRO_BIAS_SAVED_DIR1, GYRO_TAG_NAME, gyro_Bias);
        mSensorCali.saveSensorSettings(GYRO_BIAS_SAVED_DIR2, GYRO_TEMP_SLOPE, gyro_Slope);
        mSensorCali.saveSensorSettings(GYRO_BIAS_SAVED_DIR3, GYRO_TEMP_INTERCEPT, gyro_Intercept);
        mLock.unlock();
        ALOGE("MPE_gyr_calib_write %f %f %f,%d,%f %f %f,%f %f %f,%f\n",
            gyro_Bias[0], gyro_Bias[1], gyro_Bias[2], gyro_Accuracy,
            gyro_Slope[0], gyro_Slope[1], gyro_Slope[2],
            gyro_Intercept[0], gyro_Intercept[1], gyro_Intercept[2], mPendingEvent.data[3]);
    }
#endif
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no setDelay control attr \r\n" );
        return -1;
    }
#ifdef MPE_HAL
    deltaT = ns;
#endif
    char buf[80]={0};
    sprintf(buf, "%" PRId64 "", ns);
    TEMP_FAILURE_RETRY(write(fd, buf, strlen(buf)+1));
    close(fd);
    return 0;
}

int GyroscopeSensor::batch(int handle, int flags, int64_t samplingPeriodNs, int64_t maxBatchReportLatencyNs)
{
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
    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}

int GyroscopeSensor::flush(int handle)
{
    int fd = -1;
    char buf[32] = {0};

    mFlushCnt++;
    ALOGI("flush, flushCnt:%d\n", mFlushCnt);
    strlcpy(&input_sysfs_path[input_sysfs_path_len], "gyroflush", sizeof(input_sysfs_path) - input_sysfs_path_len);
    ALOGI("flush path:%s\r\n",input_sysfs_path);
    fd = TEMP_FAILURE_RETRY(open(input_sysfs_path, O_RDWR));
    if (fd < 0) {
        ALOGE("no flush control attr\r\n");
        return -1;
    }
    sprintf(buf, "%d", handle);
    TEMP_FAILURE_RETRY(write(fd, buf, sizeof(buf)));
    close(fd);
    return 0;
}

int GyroscopeSensor::readEvents(sensors_event_t* data, int count)
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
        /* we only report DATA_ACTION and FLUSH_ACTION to framework */
        if (event->flush_action <= FLUSH_ACTION) {
            /* auto cts may request flush event when sensor disable, ALPS03452281 */
            //if (mEnabled) {
                if (mPendingEvent.timestamp > mEnabledTime) {
#ifdef MPE_HAL
		if (event->flush_action == DATA_ACTION) {
                            //ALOGE("MPE_TEMPERATURE %f %f %f %f\n", mPendingEvent.gyro.x, mPendingEvent.gyro.y, mPendingEvent.gyro.z, mPendingEvent.data[3]);

                            float gyro_out[3];
                            int accuracy;
                            float gyroBias[3];
                            MPE_SENSOR_DATA mpe_data_in, mpe_data_out;

                            mpe_data_in.x = mPendingEvent.gyro.x;
                            mpe_data_in.y = mPendingEvent.gyro.y;
                            mpe_data_in.z = mPendingEvent.gyro.z;

                            //ALOGE("MPE_K: %llu %llu %f %f %f\n", prevTimestamp, mPendingEvent.timestamp, mpe_data_in.x, mpe_data_in.y,mpe_data_in.z);
                            Gyro_run_calibration(deltaT, 1, &mpe_data_in, &mpe_data_out,&accuracy, mPendingEvent.data[3]);
                            //ALOGE("MPE_RESULT_G: %f %f %f %d\n", mpe_data_out.x, mpe_data_out.y,mpe_data_out.z, accuracy);

                            gyroBias[0] = mpe_data_out.x - mPendingEvent.gyro.x;
                            gyroBias[1] = mpe_data_out.y - mPendingEvent.gyro.y;
                            gyroBias[2] = mpe_data_out.z - mPendingEvent.gyro.z;

                            mPendingEventNew.timestamp = mPendingEvent.timestamp;
                            mPendingEventNew.gyro.status = accuracy;
                            mPendingEventNew.gyro.x = mpe_data_out.x;
                            mPendingEventNew.gyro.y = mpe_data_out.y;
                            mPendingEventNew.gyro.z = mpe_data_out.z;

                            data_cnt ++;

                            float gyro_Bias[3], gyro_Slope[3], gyro_Intercept[3];
                            int gyro_Accuracy;
                            Gyro_get_calibration_parameter(gyro_Bias, &gyro_Accuracy, gyro_Slope, gyro_Intercept);

                            if (accuracy == 3) {
                                if (data_cnt > (int)(1.0e9/deltaT)) {
                                    ALOGE("MPE_gyr_calib_curr %f %f %f,%d,%f %f %f,%f %f %f,%f\n",
                                        gyroBias[0], gyroBias[1], gyroBias[2], gyro_Accuracy,
                                        gyro_Slope[0], gyro_Slope[1], gyro_Slope[2],
                                        gyro_Intercept[0], gyro_Intercept[1], gyro_Intercept[2], mPendingEvent.data[3]);

                                    data_cnt = 0;
                                }
                                if (preGyrAcc != 3) {
                                    mLock.lock();
                                    mSensorCali.saveSensorSettings(GYRO_BIAS_SAVED_DIR1, GYRO_TAG_NAME, gyro_Bias);
                                    mSensorCali.saveSensorSettings(GYRO_BIAS_SAVED_DIR2, GYRO_TEMP_SLOPE, gyro_Slope);
                                    mSensorCali.saveSensorSettings(GYRO_BIAS_SAVED_DIR3, GYRO_TEMP_INTERCEPT, gyro_Intercept);
                                    mLock.unlock();
                                    ALOGE("MPE_gyr_calib_write %f %f %f,%d,%f %f %f,%f %f %f,%f\n",
                                        gyro_Bias[0], gyro_Bias[1], gyro_Bias[2], gyro_Accuracy,
                                        gyro_Slope[0], gyro_Slope[1], gyro_Slope[2],
                                        gyro_Intercept[0], gyro_Intercept[1], gyro_Intercept[2], mPendingEvent.data[3]);
                                }
                            }
                            preGyrAcc = gyro_Accuracy;
                            *data++ = mPendingEventNew;
		} else {
			    *data++ = mPendingEvent;
		}
#else
                    *data++ = mPendingEvent;
#endif
                    numEventReceived++;
                    count--;
                }
            //}
        }
        mSensorReader.next();
    }
    return numEventReceived;
}

void GyroscopeSensor::processEvent(struct sensor_event const *event)
{
    float gyroBias[3] = {0};
    int32_t gyroCali[3] = {0};
    int32_t gyroTemp[6] = {0};
#ifdef MPE_HAL
    short int temp = 0;
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
#ifdef MPE_HAL
        temp = (short int)event->word[3];
        if (temp < 0)
            mPendingEvent.data[3] = (float)(23 - (float)(0-temp)/512);
        else
            mPendingEvent.data[3] = (float)(23 + (float)temp / 512);
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
        mFlushCnt--;
        ALOGI("flush complete, flushCnt:%d\n", mFlushCnt);
    } else if (event->flush_action == BIAS_ACTION) {
        gyroBias[0] = (float)event->word[0] / mDataDiv;
        gyroBias[1] = (float)event->word[1] / mDataDiv;
        gyroBias[2] = (float)event->word[2] / mDataDiv;
        mSensorCali.saveCalibrationFloat(GYRO_BIAS_SAVED_DIR, GYRO_TAG_BIAS, gyroBias, 3);
        ALOGI("write bias: [%f, %f, %f]\n", gyroBias[0], gyroBias[1], gyroBias[2]);
    } else if (event->flush_action == CALI_ACTION) {
        gyroCali[0] = event->word[0];
        gyroCali[1] = event->word[1];
        gyroCali[2] = event->word[2];
        mSensorCali.saveCalibrationInt32(GYRO_CALI_SAVED_DIR, GYRO_TAG_CALI, gyroCali, 3);
        ALOGI("write cali: [%d, %d, %d]\n", gyroCali[0], gyroCali[1], gyroCali[2]);
    } else if (event->flush_action == TEMP_ACTION) {
        gyroTemp[0] = event->word[0];
        gyroTemp[1] = event->word[1];
        gyroTemp[2] = event->word[2];
        gyroTemp[3] = event->word[3];
        gyroTemp[4] = event->word[4];
        gyroTemp[5] = event->word[5];
        mSensorCali.saveCalibrationInt32(GYRO_TEMP_SAVED_DIR, GYRO_TAG_TEMP, gyroTemp, 6);
        ALOGI("write temp cali: [%d, %d, %d, %d, %d, %d]\n", gyroTemp[0], gyroTemp[1], gyroTemp[2],
            gyroTemp[3], gyroTemp[4], gyroTemp[5]);
    } else
        ALOGE("unknow action\n");
}
