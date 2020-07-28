/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
//-----------------------------------------------------------------------------

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <string>
#include <vector>
#include "mpe_common.h"
#include "mpe_sensor.h"
#include "gps_dbg_log.h"

/* HIDL */
#include <sys/prctl.h>
#include <android/sensor.h>
//#include <android-base/macros.h>
#include <android/frameworks/sensorservice/1.0/IEventQueue.h>
#include <android/frameworks/sensorservice/1.0/IEventQueueCallback.h>
#include <android/frameworks/sensorservice/1.0/ISensorManager.h>
#include <sensors/convert.h>

#include <utils/Mutex.h>

using namespace android;
using android::frameworks::sensorservice::V1_0::ISensorManager;
using android::frameworks::sensorservice::V1_0::Result;
using android::hardware::sensors::V1_0::SensorType;
using android::frameworks::sensorservice::V1_0::IEventQueueCallback;
using android::hardware::Return;
using android::hardware::sensors::V1_0::Event;
using android::frameworks::sensorservice::V1_0::IEventQueue;

#define SENSOR_TYPE_DEVICE_PRIVATE_BASE       65536
#define SENSOR_TYPE_GYRO_TEMPERATURE          (71 + SENSOR_TYPE_DEVICE_PRIVATE_BASE)

#ifdef LOGD
#undef LOGD
#endif
#ifdef LOGW
#undef LOGW
#endif
#ifdef LOGE
#undef LOGE
#endif
#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define SENSOR_LOG_SCALE 1e6

#define LOG_TAG "MPE_SE"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)

unsigned char isfirst_baro = 1;
UINT32 current_gps_sec = 0;

static unsigned char phone_time_reset = 0;
static UINT32 current_leap_sec = 18;
static double mnl_inject_sys_time = 0.0;
static unsigned char SE_listener_mode = MPE_IDLE_MODE;
static UINT16 gyr_log_cnt = 0;
static INT64 gyr_log_time = 0;

//chre data sync
static UINT16 acc_cnt = 0;
static UINT16 gyr_cnt = 0;
static unsigned char prev_data_is_gyro = false;
static EVENT_DATA acc_samples[MAX_NUM_SAMPLES];
static EVENT_DATA gyr_samples[MAX_NUM_SAMPLES];
static EVENT_DATA mag_samples;
static float bar_samples = 0;
float gyr_temperature = 0.0f;
float gyr_scp_calib_done = 0.0f;

static INT8 raw_accuracy[SENSOR_TYPE_MAX] = {0};
static INT8 calib_accuracy[SENSOR_TYPE_MAX] = {0};
static unsigned char calib_get_gyro_data = false;

static int64_t gyr_latest_kernel_time = 0;
static int64_t sys_latest_kernel_time = 0;
static uint64_t sys_latest_utc_time = 0;
static uint64_t gyr_latest_utc_time = 0;
static UINT32 sys_sync_time_cnt = 0;

#ifdef MNLD_WRITE_STORAGE
extern FILE *data_file_local;
#else
#define MPE_LOG_BUFF 1024
static char log_buff[MPE_LOG_BUFF] = {0};
#endif
extern INT16 u2Log_enable;

static struct tm gyro_tm;
static struct timeval temp_timeval;
static struct timeval gyro_timeval;
static struct timeval sys_latest_utc_timeval;

static pthread_mutex_t mpe_log_mutex = PTHREAD_MUTEX_INITIALIZER;

static std::vector<std::string> acc_support_name = {"lsm6dsm_acc, bmi160_acc"};
static std::vector<std::string> gyro_support_name = {"lsm6dsm_gyro, bmi160_gyro"};

static int64_t last_acc_timestamp = 0;
static int64_t last_uncli_gyro_timestamp = 0;
static int acc_exceed_50Hz_count = 0;
static int uncli_gyro_exceed_50Hz_count = 0;

typedef enum {
    SENSOR_ERR   = -1,
    SENSOR_INIT   = 0,
    SENSOR_UNINIT = 1,
} SENSOR_STATUS;

class SensorDeathRecipient : public android::hardware::hidl_death_recipient
{
public:
    // hidl_death_recipient interface
    virtual void serviceDied(uint64_t cookie,
            const ::android::wp<::android::hidl::base::V1_0::IBase>& who) override;
};

typedef struct SENSOR_CONTEXT {
    Mutex   mDataLock;
    int32_t sensorHandle[SENSOR_TYPE_MAX];
    int     mSensorStatus[SENSOR_TYPE_MAX];
    sp<IEventQueueCallback> mpListener;
    sp<IEventQueue> queue;
    sp<SensorDeathRecipient> mpDeathRecipient;
} SENSOR_CONTEXT;

static SENSOR_CONTEXT sensorContext[SENSOR_USER_ID_MAX];

//-----------------------------------------------------------------------------
unsigned char mpe_sensor_get_listen_mode(void)
{
    return SE_listener_mode;
}

void mpe_sensor_set_listen_mode(unsigned char mode)
{
    LOGD("Set listen mode from %u to %u", SE_listener_mode, mode);
    SE_listener_mode = mode;
}


UINT16 mpe_sensor_check_time()
{
    INT32 mnl_inject_gps_time;
    double time_diff;

    mnl_inject_gps_time = mpe_sys_gps_to_sys_time(current_gps_sec);
    time_diff = abs((int)(mnl_inject_sys_time - mnl_inject_gps_time));

    if (time_diff > 3600) {
        LOGD("Detect change in global time, global: %lf, gps: %d, diff: %lf \n",mnl_inject_sys_time, mnl_inject_gps_time, time_diff);
        phone_time_reset = 1;
    } else {
        phone_time_reset = 0;
    }
    return 0;
}

//-----------------------------------------------------------------------------

void mpe_sensor_get_accuracy(INT8 *accuracy)
{
    memcpy(accuracy, raw_accuracy, sizeof(raw_accuracy));
}

//-----------------------------------------------------------------------------

UINT16 mpe_sensor_acquire_Data (IMU* data, uint64_t* data_utc)
{
    INT16 i = 0, j = 0;
    UINT16 size = 0;

    //LOGD("SE_Acquire_Data acc_cnt:%u gyr:%u",acc_cnt, gyr_cnt);
    if (gyr_cnt >= MAX_NUM_SAMPLES || gyr_cnt == 0) {
        LOGD("MPE_DBG: gyr data abnormal, acc_cnt:%d, gyr_cnt:%d",acc_cnt, gyr_cnt);
        size = 0;
    } else if (acc_cnt >= MAX_NUM_SAMPLES || acc_cnt == 0) {
        LOGD("MPE_DBG: acc data abnormal, acc_cnt:%d, gyr_cnt:%d",acc_cnt, gyr_cnt);
        size = 0;
    } else {
        for(i = 0; i < gyr_cnt; i++) {
            // acc
            if(i >= acc_cnt) {
                j = acc_cnt - 1;
            } else {
                j = i;
            }
            data[i].acceleration[0] = acc_samples[j].x;
            data[i].acceleration[1] = acc_samples[j].y;
            data[i].acceleration[2] = acc_samples[j].z;
            data[i].acceleration_raw[0] = acc_samples[j].x;
            data[i].acceleration_raw[1] = acc_samples[j].y;
            data[i].acceleration_raw[2] = acc_samples[j].z;
            data[i].input_time_acc = acc_samples[j].timestamp;

            // gyro
            data[i].angularVelocity[0] = gyr_samples[i].x;
            data[i].angularVelocity[1] = gyr_samples[i].y;
            data[i].angularVelocity[2] = gyr_samples[i].z;
            data[i].angularVelocity_raw[0] = gyr_samples[i].x;
            data[i].angularVelocity_raw[1] = gyr_samples[i].y;
            data[i].angularVelocity_raw[2] = gyr_samples[i].z;
            data[i].input_time_gyro = gyr_samples[i].timestamp;
            data[i].thermometer = gyr_temperature;
            data_utc[i] = gyr_samples[i].utc_timestamp;

            // mag
            data[i].magnetic[0] = mag_samples.x;
            data[i].magnetic[1] = mag_samples.y;
            data[i].magnetic[2] = mag_samples.z;
            data[i].magnetic_raw[0] = mag_samples.x;
            data[i].magnetic_raw[1] = mag_samples.y;
            data[i].magnetic_raw[2] = mag_samples.z;
            data[i].input_time_mag = mag_samples.timestamp;

            // pres
            data[i].pressure = bar_samples;
        }
        size = gyr_cnt;
    }
    acc_cnt = 0;
    gyr_cnt = 0;
    memset(&acc_samples, 0 ,MAX_NUM_SAMPLES*sizeof(EVENT_DATA));
    memset(&gyr_samples, 0 ,MAX_NUM_SAMPLES*sizeof(EVENT_DATA));
    return size;
}

// Update mnl_inject_sys_time, current_gps_sec, current_leap_sec
void mpe_sensor_update_mnl_sec(UINT32 gps_sec, UINT32 leap_sec)
{
    current_gps_sec = gps_sec;
    current_leap_sec = leap_sec;
    mpe_sys_get_time_stamp(&mnl_inject_sys_time, current_leap_sec);
    return;
}

//-----------------------------------------------------------------------------
void mpe_sensor_run(void)
{
    //unsigned char ret = FALSE;
    INT32 mnl_inject_gps_time;

    LOGD("SE sensor run source\n");
    mnl_inject_gps_time = mpe_sys_gps_to_sys_time(current_gps_sec);
    LOGD("mnl_inject_gps_time: %d, mnl_inject_sys_time: %lf",mnl_inject_gps_time, mnl_inject_sys_time);
    LOGD("global_gps_diff =%lf\n", (fabs(mnl_inject_sys_time - mnl_inject_gps_time)));

    if(fabs(mnl_inject_sys_time - mnl_inject_gps_time) <= 3600.0)
    {
        if(!mpe_sys_sensor_threads_create())
        {
            return;
        }
        mpe_sensor_set_listen_mode(MPE_START_MODE);
        mpe_log_init();

        //register sensor listener
        mpe_kernel_initialize();
        LOGD("SE_sensor_run success");
    }
    return;
}

class CalibListenrCallback : public IEventQueueCallback {
  public:
    Return<void> onEvent(const Event &e) {
        sensors_event_t sensorEvent;
        android::hardware::sensors::V1_0::implementation::convertToSensorEvent(e, &sensorEvent);
        INT32 type = (INT32)e.sensorType;

        switch(type)
        {
            case (INT32)SensorType::GYROSCOPE:
                calib_accuracy[SENSOR_TYPE_GYR] = sensorEvent.gyro.status;
                calib_get_gyro_data = TRUE;
                break;
            default:
                LOGD("unknown type(%d)", sensorEvent.type);
        }
        return android::hardware::Void();
    }
};

class MpeListenrCallback : public IEventQueueCallback {
  public:
    Return<void> onEvent(const Event &e) {
        sensors_event_t sensorEvent;
        android::hardware::sensors::V1_0::implementation::convertToSensorEvent(e, &sensorEvent);
        INT32 type = (INT32)e.sensorType;

        switch(type)
        {
            case (INT32)SensorType::ACCELEROMETER:
                mpe_sensor_detect_changing_freq(sensorEvent.timestamp, &last_acc_timestamp, &acc_exceed_50Hz_count);

                if(prev_data_is_gyro) {
                    prev_data_is_gyro = false;
                    mpe_run_algo();
                }
                if(acc_cnt < MAX_NUM_SAMPLES) {
                    acc_samples[acc_cnt].x = sensorEvent.acceleration.x;
                    acc_samples[acc_cnt].y = sensorEvent.acceleration.y;
                    acc_samples[acc_cnt].z = sensorEvent.acceleration.z;
                    acc_samples[acc_cnt].timestamp = sensorEvent.timestamp;
                    acc_cnt++;
                    raw_accuracy[SENSOR_TYPE_ACC] = sensorEvent.acceleration.status;

#ifdef MNLD_WRITE_STORAGE
                    if(u2Log_enable && (data_file_local != NULL)) {
                        fprintf(data_file_local,"$%c%X%c%X%c%X\n",(sensorEvent.acceleration.x<0)?'-':' ',abs((int)(sensorEvent.acceleration.x*SENSOR_LOG_SCALE)),
                            (sensorEvent.acceleration.y<0)?'-':' ',abs((int)(sensorEvent.acceleration.y*SENSOR_LOG_SCALE)),
                            (sensorEvent.acceleration.z<0)?'-':' ',abs((int)(sensorEvent.acceleration.z*SENSOR_LOG_SCALE)));
                    }
#else
                    if(u2Log_enable) {
                        snprintf(log_buff, MPE_LOG_BUFF, "$%c%X%c%X%c%X\n",(sensorEvent.acceleration.x<0)?'-':' ',abs((int)(sensorEvent.acceleration.x*SENSOR_LOG_SCALE)),
                            (sensorEvent.acceleration.y<0)?'-':' ',abs((int)(sensorEvent.acceleration.y*SENSOR_LOG_SCALE)),
                            (sensorEvent.acceleration.z<0)?'-':' ',abs((int)(sensorEvent.acceleration.z*SENSOR_LOG_SCALE)));
                        mpe_publish_to_log_buf_pool(log_buff);
                    }
#endif
                }
                break;
            case (INT32)SensorType::GYROSCOPE:
                if(gyr_cnt < MAX_NUM_SAMPLES) {
                    gyr_samples[gyr_cnt].x = sensorEvent.gyro.x;
                    gyr_samples[gyr_cnt].y = sensorEvent.gyro.y;
                    gyr_samples[gyr_cnt].z = sensorEvent.gyro.z;
                    gyr_samples[gyr_cnt].timestamp = sensorEvent.timestamp;
                    gyr_latest_kernel_time = sensorEvent.timestamp;
                    gyr_cnt++;
                    raw_accuracy[SENSOR_TYPE_GYR] = sensorEvent.gyro.status;
                }
                prev_data_is_gyro = true;
#ifdef MNLD_WRITE_STORAGE
                if(u2Log_enable && (data_file_local != NULL)) {
                    if(gyr_log_cnt >= 200 || gyr_log_cnt == 0) {
                        fprintf(data_file_local,"^ %llX\n",(sensorEvent.timestamp));
                        gyr_log_cnt = 0;
                        gyr_log_time = sensorEvent.timestamp;
                    }
                    gyr_log_cnt++;
                    fprintf(data_file_local, "! %X%c%X%c%X%c%X\n",(int)((sensorEvent.timestamp-gyr_log_time)/1000.0),
                        (sensorEvent.gyro.x<0)?'-':' ',abs((int)(sensorEvent.gyro.x*SENSOR_LOG_SCALE)),
                        (sensorEvent.gyro.y<0)?'-':' ',abs((int)(sensorEvent.gyro.y*SENSOR_LOG_SCALE)),
                        (sensorEvent.gyro.z<0)?'-':' ',abs((int)(sensorEvent.gyro.z*SENSOR_LOG_SCALE)));
                }
#else
                if(u2Log_enable) {
                    if(gyr_log_cnt >= 200 || gyr_log_cnt == 0) {
                        snprintf(log_buff, MPE_LOG_BUFF, "^ %llX\n", (sensorEvent.timestamp));
                        mpe_publish_to_log_buf_pool(log_buff);
                        gyr_log_cnt = 0;
                        gyr_log_time = sensorEvent.timestamp;
                    }
                    gyr_log_cnt++;
                    snprintf(log_buff, MPE_LOG_BUFF, "! %X%c%X%c%X%c%X\n",(int)((sensorEvent.timestamp-gyr_log_time)/1000.0),
                        (sensorEvent.gyro.x<0)?'-':' ',abs((int)(sensorEvent.gyro.x*SENSOR_LOG_SCALE)),
                        (sensorEvent.gyro.y<0)?'-':' ',abs((int)(sensorEvent.gyro.y*SENSOR_LOG_SCALE)),
                        (sensorEvent.gyro.z<0)?'-':' ',abs((int)(sensorEvent.gyro.z*SENSOR_LOG_SCALE)));
                    mpe_publish_to_log_buf_pool(log_buff);
                }
#endif
                break;
            case (INT32)SensorType::GYROSCOPE_UNCALIBRATED:
                mpe_sensor_detect_changing_freq(sensorEvent.timestamp, &last_uncli_gyro_timestamp, &uncli_gyro_exceed_50Hz_count);

                if(gyr_cnt < MAX_NUM_SAMPLES) {
                    gyr_samples[gyr_cnt].x = sensorEvent.uncalibrated_gyro.x_uncalib;
                    gyr_samples[gyr_cnt].y = sensorEvent.uncalibrated_gyro.y_uncalib;
                    gyr_samples[gyr_cnt].z = sensorEvent.uncalibrated_gyro.z_uncalib;
                    gyr_samples[gyr_cnt].timestamp = sensorEvent.timestamp;
                    gyr_latest_kernel_time = sensorEvent.timestamp;
                    if (sys_latest_utc_time == 0) {
                        gyr_samples[gyr_cnt].utc_timestamp = 0;
                    } else {
                        //gyr_samples[gyr_cnt].utc_timestamp = sys_latest_utc_time + ((gyr_samples[gyr_cnt].timestamp - sys_latest_kernel_time)/1000000);
                        temp_timeval.tv_sec = (gyr_samples[gyr_cnt].timestamp - sys_latest_kernel_time) / 1000000000; //ns to s
                        temp_timeval.tv_usec = ((gyr_samples[gyr_cnt].timestamp - sys_latest_kernel_time) % 1000000000) / 1000; //ns to us
                        timeradd(&temp_timeval, &sys_latest_utc_timeval, &gyro_timeval);
                        gmtime_r(&gyro_timeval.tv_sec, &gyro_tm);
                        gyr_samples[gyr_cnt].utc_timestamp =
                                              (uint64_t)(gyro_tm.tm_year + 1900) * 10000000000000 + (uint64_t)(gyro_tm.tm_mon + 1) * 100000000000
                                            + (uint64_t)gyro_tm.tm_mday * 1000000000 + (uint64_t)gyro_tm.tm_hour * 10000000
                                            + (uint64_t)gyro_tm.tm_min * 100000 + (uint64_t)gyro_tm.tm_sec * 1000 + gyro_timeval.tv_usec / 1000;
                    }
                    gyr_latest_utc_time = gyr_samples[gyr_cnt].utc_timestamp;
                    gyr_cnt++;
                    raw_accuracy[SENSOR_TYPE_GYR] = 3;
                }
                prev_data_is_gyro = true;
#ifdef MNLD_WRITE_STORAGE
                if(u2Log_enable && (data_file_local != NULL)) {
                    if(gyr_log_cnt >= 200 || gyr_log_cnt == 0) {
                        fprintf(data_file_local,"^ %llX\n",(sensorEvent.timestamp));
                        gyr_log_cnt = 0;
                        gyr_log_time = sensorEvent.timestamp;
                    }
                    gyr_log_cnt++;
                    fprintf(data_file_local, "! %X%c%X%c%X%c%X%c%X %llu\n",(int)((sensorEvent.timestamp-gyr_log_time)/1000.0),
                        (sensorEvent.gyro.x<0)?'-':' ',abs((int)(sensorEvent.gyro.x*SENSOR_LOG_SCALE)),
                        (sensorEvent.gyro.y<0)?'-':' ',abs((int)(sensorEvent.gyro.y*SENSOR_LOG_SCALE)),
                        (sensorEvent.gyro.z<0)?'-':' ',abs((int)(sensorEvent.gyro.z*SENSOR_LOG_SCALE)),
                        (gyr_temperature<0)?'-':' ',abs((int)(gyr_temperature*SENSOR_LOG_SCALE)), gyr_latest_utc_time);
                }
#else
                if(u2Log_enable) {
                    if(gyr_log_cnt >= 200 || gyr_log_cnt == 0) {
                        snprintf(log_buff, MPE_LOG_BUFF, "^ %llX\n", (sensorEvent.timestamp));
                        mpe_publish_to_log_buf_pool(log_buff);
                        gyr_log_cnt = 0;
                        gyr_log_time = sensorEvent.timestamp;
                    }
                    gyr_log_cnt++;
                    snprintf(log_buff, MPE_LOG_BUFF, "! %X%c%X%c%X%c%X%c%X %llu\n",(int)((sensorEvent.timestamp-gyr_log_time)/1000.0),
                        (sensorEvent.gyro.x<0)?'-':' ',abs((int)(sensorEvent.gyro.x*SENSOR_LOG_SCALE)),
                        (sensorEvent.gyro.y<0)?'-':' ',abs((int)(sensorEvent.gyro.y*SENSOR_LOG_SCALE)),
                        (sensorEvent.gyro.z<0)?'-':' ',abs((int)(sensorEvent.gyro.z*SENSOR_LOG_SCALE)),
                        (gyr_temperature<0)?'-':' ',abs((int)(gyr_temperature*SENSOR_LOG_SCALE)), gyr_latest_utc_time);
                    mpe_publish_to_log_buf_pool(log_buff);
                }
#endif
                break;
            case (INT32)SensorType::MAGNETIC_FIELD:
                mag_samples.x = sensorEvent.magnetic.x;
                mag_samples.y = sensorEvent.magnetic.y;
                mag_samples.z = sensorEvent.magnetic.z;
                mag_samples.timestamp = sensorEvent.timestamp;
                raw_accuracy[SENSOR_TYPE_MAG] = sensorEvent.magnetic.status;
#ifdef MNLD_WRITE_STORAGE
                if(u2Log_enable && (data_file_local != NULL)) {
                    fprintf(data_file_local,"@%c%X%c%X%c%X\n",(sensorEvent.magnetic.x<0)?'-':' ',abs((int)(sensorEvent.magnetic.x*SENSOR_LOG_SCALE)),
                        (sensorEvent.magnetic.y<0)?'-':' ',abs((int)(sensorEvent.magnetic.y*SENSOR_LOG_SCALE)),
                        (sensorEvent.magnetic.z<0)?'-':' ',abs((int)(sensorEvent.magnetic.z*SENSOR_LOG_SCALE)));
                }
#else
                if(u2Log_enable) {
                    snprintf(log_buff, MPE_LOG_BUFF, "@%c%X%c%X%c%X\n",(sensorEvent.magnetic.x<0)?'-':' ',abs((int)(sensorEvent.magnetic.x*SENSOR_LOG_SCALE)),
                        (sensorEvent.magnetic.y<0)?'-':' ',abs((int)(sensorEvent.magnetic.y*SENSOR_LOG_SCALE)),
                        (sensorEvent.magnetic.z<0)?'-':' ',abs((int)(sensorEvent.magnetic.z*SENSOR_LOG_SCALE)));
                    mpe_publish_to_log_buf_pool(log_buff);
                }
#endif
                break;
            case (INT32)SensorType::PRESSURE:
                bar_samples = sensorEvent.pressure;
                isfirst_baro = 0;
#ifdef MNLD_WRITE_STORAGE
                if(u2Log_enable && (data_file_local != NULL)) {
                    fprintf(data_file_local,"~%c%X\n",(bar_samples<0)?'-':' ',abs((int)(bar_samples*SENSOR_LOG_SCALE)));
                }
#else
                if(u2Log_enable) {
                    snprintf(log_buff, MPE_LOG_BUFF, "~%c%X\n",(bar_samples<0)?'-':' ',abs((int)(bar_samples*SENSOR_LOG_SCALE)));
                    mpe_publish_to_log_buf_pool(log_buff);
                }
#endif
                break;
            case SENSOR_TYPE_GYRO_TEMPERATURE:
                gyr_temperature = sensorEvent.data[0];
                gyr_scp_calib_done = sensorEvent.data[1];
                break;
            default:
                LOGD("unknown type(%d)", sensorEvent.type);
        }
        return android::hardware::Void();
    }
};

void SensorDeathRecipient::serviceDied(uint64_t, const wp<::android::hidl::base::V1_0::IBase>&)
{

    Mutex::Autolock lock(sensorContext[SENSOR_USER_ID_MPE].mDataLock);
    LOGD("Sensor service died. Cleanup sensor manager instance!");
    sensorContext[SENSOR_USER_ID_MPE].mpListener = NULL;
    sensorContext[SENSOR_USER_ID_MPE].queue = NULL;
    sensorContext[SENSOR_USER_ID_MPE].mpDeathRecipient = NULL;
}


unsigned char mpe_sensor_init(SENSOR_USER_ID id)
{
    if (id >= SENSOR_USER_ID_MAX) {
        LOGD("mpe_sensor_init wrong id=%d", id);
        return FALSE;
    }
    if (id == SENSOR_USER_ID_CALIB) {
        ::prctl(PR_SET_NAME,"CalibThread", 0, 0, 0);
    } else if (id == SENSOR_USER_ID_MPE) {
        ::prctl(PR_SET_NAME,"MpeThread", 0, 0, 0);
    }
    sensorContext[id].mpListener = NULL;
    sensorContext[id].queue = NULL;
    sensorContext[id].mpDeathRecipient = NULL;
    return TRUE;
}

unsigned char mpe_sensor_deinit(SENSOR_USER_ID id)
{
    if (id >= SENSOR_USER_ID_MAX) {
        LOGD("mpe_sensor_deinit wrong id=%d", id);
        return FALSE;
    }
    sensorContext[id].mpListener = NULL;
    sensorContext[id].queue = NULL;
    sensorContext[id].mpDeathRecipient = NULL;

    if (id == SENSOR_USER_ID_MPE) {
        mpe_log_deinit();
        mpe_sensor_set_listen_mode(MPE_IDLE_MODE);
        phone_time_reset = 0;
        isfirst_baro = 1;
        acc_cnt = 0;
        gyr_cnt = 0;
        gyr_log_cnt = 0;
        prev_data_is_gyro = false;
        gyr_temperature = 0;
        gyr_scp_calib_done = 0;
        gyr_latest_kernel_time = 0;
        sys_latest_kernel_time = 0;
        sys_latest_utc_time = 0;
        gyr_latest_utc_time = 0;
        sys_sync_time_cnt = 0;
        last_acc_timestamp = 0;
        last_uncli_gyro_timestamp = 0;
        acc_exceed_50Hz_count = 0;
        uncli_gyro_exceed_50Hz_count = 0;
    }
    return TRUE;
}

unsigned char mpe_sensor_start(SENSOR_USER_ID id, SENSOR_TYPE mpe_type, UINT32 periodInMs)
{
    INT32 type;

    LOGD("mpe_sensor_start id=%d type=%d periodMs=%u",id, mpe_type, periodInMs);
    if (mpe_type == SENSOR_TYPE_ACC) {
        type = (INT32)SensorType::ACCELEROMETER;
    } else if (mpe_type == SENSOR_TYPE_GYR) {
        type = (INT32)SensorType::GYROSCOPE;
    } else if (mpe_type == SENSOR_TYPE_UNCAL_GYR) {
        type = (INT32)SensorType::GYROSCOPE_UNCALIBRATED;
    } else if (mpe_type == SENSOR_TYPE_MAG) {
        type = (INT32)SensorType::MAGNETIC_FIELD;
    } else if (mpe_type == SENSOR_TYPE_BAR) {
        type = (INT32)SensorType::PRESSURE;
    } else if (mpe_type == SENSOR_TYPE_GYR_TMP) {
        type = SENSOR_TYPE_GYRO_TEMPERATURE;
    } else {
        LOGD("unknown type");
        return FALSE;
    }
    Mutex::Autolock lock(sensorContext[id].mDataLock);
    sensorContext[id].mSensorStatus[mpe_type] = SENSOR_ERR;
    // sensor HIDL interface
    sp<ISensorManager> manager = ISensorManager::getService();
    if (manager == NULL) {
        LOGD("getService() failed");
        return FALSE;
    }
    if (sensorContext[id].mpDeathRecipient == NULL) {
        sensorContext[id].mpDeathRecipient = new SensorDeathRecipient();
        ::android::hardware::Return<bool> linked = manager->linkToDeath(sensorContext[id].mpDeathRecipient, /*cookie*/ 0);
        if (!linked || !linked.isOk()) {
            LOGE("Unable to link to sensor service death notifications");
            return FALSE;
        }
    }
    if (sensorContext[id].sensorHandle[mpe_type] == 0) {
        manager->getDefaultSensor((android::hardware::sensors::V1_0::SensorType)type,
            [&](const auto &sensor, Result result) {
                if (result == Result::OK) {
                    sensorContext[id].sensorHandle[mpe_type] = sensor.sensorHandle;
                }
            });
        if (sensorContext[id].sensorHandle[mpe_type] == 0) {
            LOGD("getDefaultSensor FAIL!");
            return FALSE;
        }
    }
    //Result res;
    if (sensorContext[id].mpListener == NULL) {
        if (id == SENSOR_USER_ID_CALIB) {
            sensorContext[id].mpListener = new CalibListenrCallback();
        } else if (id == SENSOR_USER_ID_MPE) {
            sensorContext[id].mpListener = new MpeListenrCallback();
        }
        manager->createEventQueue(sensorContext[id].mpListener,
            [&] (const auto &q, Result result) {
                if (result == Result::OK) {
                    sensorContext[id].queue = q;
                }
            });
        if (sensorContext[id].queue == NULL) {
            LOGD("createEventQueue FAIL!");
            return FALSE;
        }
    }

    // Enable the selected sensor with a specified sampling period and max batch report latency.
    // If enableSensor is called multiple times on the same sensor, the previous calls must be overridden by the last call.
    ::android::hardware::Return<Result> _ret =
        sensorContext[id].queue->enableSensor(
        sensorContext[id].sensorHandle[mpe_type], periodInMs*1000, 0);
    if (!_ret.isOk()) {
            LOGD("enableSensor FAIL!");
            return FALSE;
    }

    sensorContext[id].mSensorStatus[mpe_type] = SENSOR_INIT;
    return TRUE;
}

unsigned char mpe_sensor_stop(SENSOR_USER_ID id, SENSOR_TYPE mpe_type)
{
    Mutex::Autolock lock(sensorContext[id].mDataLock);

    LOGD("mpe_sensor_stop id=%d type=%d",id, mpe_type);

    if (sensorContext[id].mSensorStatus[mpe_type] != SENSOR_INIT) {
        LOGD("mpe_sensor_stop no need");
        return FALSE;
    }
    if (sensorContext[id].queue == NULL) {
        LOGD("mpe_sensor_stop bypass: serviceDied");
        return FALSE;
    }
    ::android::hardware::Return<Result> _ret =
        sensorContext[id].queue->disableSensor(sensorContext[id].sensorHandle[mpe_type]);
    if (!_ret.isOk()) {
        LOGD("disableSensor FAIL!");
        return FALSE;
    }
    sensorContext[id].mSensorStatus[mpe_type] = SENSOR_UNINIT;
    sensorContext[id].sensorHandle[mpe_type] = 0;
    return TRUE;
}

void mpe_sensor_detect_changing_freq(int64_t curr_time, int64_t *last_time, int *count)
{
    if (mpe_sensor_freq_flag == MTK_MPE_SENSOR_AT_50HZ) {
        if (curr_time - *last_time > 0 && curr_time - *last_time < MPE_200HZ_TIME_THRESHOLD * 1000 * 1000) {
            (*count)++;

            if (*count >= MPE_200HZ_COUNT_THRESHOLD) {
                LOGD("Sensor is not 50Hz anymore with interval %lld ns, trigger the signal\n", curr_time - *last_time);

                pthread_mutex_lock(&mpe_sensor_freq_mutex);
                mpe_sensor_freq_flag = MTK_MPE_SENSOR_CHANGE_TO_200HZ;
                pthread_cond_signal(&mpe_sensor_freq_cond);
                pthread_mutex_unlock(&mpe_sensor_freq_mutex);
            }
        } else {
            *count = 0;
        }
    }
    *last_time = curr_time;
}

void mpe_sensor_get_calib_accuracy(INT8 *accuracy)
{
    memcpy(accuracy, calib_accuracy, sizeof(calib_accuracy));
}

unsigned char mpe_sensor_get_calib_gyr_data(void)
{
    return calib_get_gyro_data;
}

unsigned char mpe_sensor_check_mnl_response(void)
{
    double current_sys_time, delta_input_time;

    mpe_sys_get_time_stamp(&current_sys_time, current_leap_sec);
    delta_input_time = fabs(current_sys_time - mnl_inject_sys_time);

    LOGD("mpe_sensor_check_mnl_response %lf, %lf %lf %d %d\n",delta_input_time, current_sys_time, mnl_inject_sys_time, phone_time_reset, mpe_sensor_get_listen_mode());

    if (delta_input_time > 2.f || phone_time_reset)
    {
        mpe_sensor_set_listen_mode(MPE_IDLE_MODE);
        LOGD("close SE, dt = %lf, %lf, ,%lf, %u\n",delta_input_time,current_sys_time, mnl_inject_sys_time, phone_time_reset);
        return FALSE;
    }
    return TRUE;
}

unsigned char mpe_sensor_sync_kernel_utc_time()
{
    struct tm tm_pt;
    struct timeval tv;

    if (gyr_latest_kernel_time != 0) {
        if (sys_sync_time_cnt % 100 == 0) {
            LOGD("cnt: %u\n", sys_sync_time_cnt);
        }
        if (sys_sync_time_cnt == 0 || sys_sync_time_cnt > 3600) {
            sys_sync_time_cnt = 0;
            gettimeofday(&tv, NULL);
            gmtime_r(&tv.tv_sec, &tm_pt);
            sys_latest_kernel_time = gyr_latest_kernel_time;
            sys_latest_utc_time = (uint64_t)(tm_pt.tm_year + 1900) * 10000000000000 + (uint64_t)(tm_pt.tm_mon + 1) * 100000000000
                                + (uint64_t)tm_pt.tm_mday * 1000000000 + (uint64_t)tm_pt.tm_hour * 10000000
                                + (uint64_t)tm_pt.tm_min * 100000 + (uint64_t)tm_pt.tm_sec * 1000 + tv.tv_usec / 1000;
            memcpy(&sys_latest_utc_timeval, &tv, sizeof(struct timeval));
            LOGD("%llu %lld %lld %d %d %d %d\n",
                sys_latest_utc_time, sys_latest_kernel_time, gyr_latest_kernel_time, tm_pt.tm_mday, tm_pt.tm_hour, tm_pt.tm_min, tm_pt.tm_sec);
        }
        sys_sync_time_cnt++;
    }
    return TRUE;
}

void mpe_publish_to_log_buf_pool(char *buf) {
    pthread_mutex_lock(&mpe_log_mutex);
    if (buf_pool_idx > MPE_LOG_BUF_POOL_LEN) buf_pool_idx = MPE_LOG_BUF_POOL_LEN;
    int num_of_char = snprintf(mpe_log_buf_pool + buf_pool_idx, MPE_LOG_BUF_POOL_LEN - buf_pool_idx, "%s", buf);
    if (num_of_char >= 0) {
        buf_pool_idx += num_of_char;
    } else {
        LOGE("Error in snprintf with error code %d\n", num_of_char);
        memset(mpe_log_buf_pool, 0, sizeof(mpe_log_buf_pool));
        buf_pool_idx = 0;
    }

    if (buf_pool_idx >= MPE_LOG_BUF_POOL_PROC_LEN) {
        mnld2logd_write_mpelog(mpe_log_buf_pool, strlen(mpe_log_buf_pool));
        memset(mpe_log_buf_pool, 0, sizeof(mpe_log_buf_pool));
        buf_pool_idx = 0;
    }
    pthread_mutex_unlock(&mpe_log_mutex);
}

unsigned char mpe_check_sensor_name() {
    unsigned char acc_pass = false, gyro_pass = false;

    sp<ISensorManager> manager = ISensorManager::getService();
    if (manager == NULL) {
        LOGD("getService() failed");
        return FALSE;
    }

    manager->getSensorList(
        [&](const auto &list, Result result) {
            if (result == Result::OK) {
                for (const auto &n : list) {
                    if (n.type == android::hardware::sensors::V1_0::SensorType::ACCELEROMETER) {
                        acc_pass = (std::find(acc_support_name.begin(), acc_support_name.end(), n.vendor) != acc_support_name.end());
                        strncpy(accelerometer_name, n.vendor.c_str(), sizeof(accelerometer_name) - 1);
                        LOGD("ACCELEROMETER name: %s, %s, %s", n.name.c_str(), n.vendor.c_str(), n.typeAsString.c_str());
                    } else if (n.type == android::hardware::sensors::V1_0::SensorType::GYROSCOPE) {
                        gyro_pass = (std::find(gyro_support_name.begin(), gyro_support_name.end(), n.vendor) != gyro_support_name.end());
                        strncpy(gyroscope_name, n.vendor.c_str(), sizeof(gyroscope_name) - 1);
                        LOGD("GYROSCOPE name: %s, %s, %s", n.name.c_str(), n.vendor.c_str(), n.typeAsString.c_str());
                    } else if (n.type == android::hardware::sensors::V1_0::SensorType::MAGNETIC_FIELD) {
                        strncpy(magnetic_name, n.vendor.c_str(), sizeof(magnetic_name) - 1);
                        LOGD("MAGNETIC_FIELD name: %s, %s, %s", n.name.c_str(), n.vendor.c_str(), n.typeAsString.c_str());
                    }
                }
            }
        });

   return (acc_pass && gyro_pass);
}
