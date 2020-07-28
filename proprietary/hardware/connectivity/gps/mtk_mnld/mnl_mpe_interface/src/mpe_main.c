/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
***************************************************************/
#ifndef MPE_MAIN_C
#define MPE_MAIN_C

#ifdef __cplusplus
  extern "C" {
#endif

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <utils/Log.h> // For Debug
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <math.h>
#include <cutils/properties.h>
#include "mpe_common.h"
#include "mpe_DR.h"
#include "mpe_sensor.h"
#include "mpe.h"
#include "data_coder.h"
#include "mtk_lbs_utility.h"
#include "mnld.h"
#include "gps_dbg_log.h"

/*****************************************************************************
 * Define
 *****************************************************************************/
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

#define LOG_TAG "MPE_MA"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)

#ifndef UNUSED
#define UNUSED(x) (x)=(x)
#endif

/*****************************************************************************
 * GLobal Variable
 *****************************************************************************/
static const char pMPEVersion[] = {MPE_VER_INFO,0x00};
unsigned char isUninit_SE = FALSE;
static MNL2MPE_AIDING_DATA mnl_latest_in;
static MNL2MPE_AIDING_DATA mnl_glo_in;
static MPE2MNL_AIDING_DATA mnl_glo_out;
static UINT32 gMPEConfFlag = 0;
static UINT32 gMPERawFlag = 0;

//Mtklogger function
char mpe_debuglog_file_name[MPE_LOG_NAME_MAX_LEN];

//check for baro validity
extern unsigned char isfirst_baro;

// se log
#ifdef MNLD_WRITE_STORAGE
extern FILE *data_file_local;
#else
extern INT16 u2Log_enable;
#define MPE_LOG_BUFF 1024
static char log_buff[MPE_LOG_BUFF] = {0};
#endif

char mpe_log_buf_pool[MPE_LOG_BUF_POOL_LEN] = {0};
int buf_pool_idx = 0;

char accelerometer_name[MPE_SENSOR_NAME_LEN] = {0};
char gyroscope_name[MPE_SENSOR_NAME_LEN] = {0};
char magnetic_name[MPE_SENSOR_NAME_LEN] = {0};

pthread_mutex_t mpe_sensor_freq_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t mpe_sensor_freq_cond = PTHREAD_COND_INITIALIZER;
int mpe_sensor_freq_flag = MTK_MPE_SENSOR_INIT;

extern float gyr_temperature;
extern float gyr_scp_calib_done;

unsigned char gMpeThreadExist = 0;

static void mpe_sensor_stop_notify(void);

UINT32 mpe_sys_get_mpe_conf_flag(void) {
    return gMPEConfFlag;
}

void mpe_sys_read_mpe_conf_flag(void) {
    const char mpe_prop_path[] = "/system/vendor/etc/mpe.conf";
    char propbuf[512];
    UINT32 flag = 0;
    FILE *fp = fopen(mpe_prop_path, "rb");
    LOGD("%s\n", pMPEVersion);
    if(!fp) {
        LOGD("mpe config flag - can't open");
        gMPEConfFlag = 0;
        return;
    }
    while(fgets(propbuf, sizeof(propbuf), fp)) {
        if(strstr(propbuf, "mpe_enable=1")) {
            flag |= MPE_CONF_MPE_ENABLE;
        } else if(strstr(propbuf, "print_rawdata=1")) {
            flag |= MPE_CONF_PRT_RAWDATA;
        } else if(strstr(propbuf, "auto_calib=1")) {
            flag |= MPE_CONF_AUTO_CALIB;
        } else if(strstr(propbuf, "indoor_enable=1")) {
            flag |= MPE_CONF_INDOOR_ENABLE;
        }
    }
    LOGD("mpe config flag %u", flag);
    fclose(fp);
    gMPEConfFlag = flag;

    mpe_check_sensor_name();
    LOGD("Get sensor name with acc(%s), gyro(%s), mag(%s)", accelerometer_name, gyroscope_name, magnetic_name);
    return;
}

INT32 mpe_sys_gps_to_sys_time( UINT32 gps_sec ) {
    return (INT32) ( gps_sec + DIFF_GPS_C_TIME );// difference between GPS and
}

void mpe_sys_get_time_stamp(double* timetag, UINT32 leap_sec) {
    struct tm tm_pt;
    struct timeval tv;
    //get second and usec
    gettimeofday(&tv, NULL);
    //convert to local time
    localtime_r(&tv.tv_sec, &tm_pt);
    (*timetag) = mktime(&tm_pt);
    (*timetag)= (*timetag) + leap_sec +((float)tv.tv_usec)/1000000;
}

void mpe_sys_get_encrypt_string (char *s_out, double f_in, int s_size) {
    unsigned int i = 0;
    snprintf(s_out, s_size, "%.8lf", f_in);
    for (i = 0; i < strlen(s_out); i++)
        s_out[i] ^= 'm';
}

void mnl2mpe_hdlr_init(void) {
    memset(&mnl_latest_in, 0 , sizeof(MNL2MPE_AIDING_DATA));
    memset(&mnl_glo_in, 0 , sizeof(MNL2MPE_AIDING_DATA));
    memset(&mnl_glo_out, 0 , sizeof(MPE2MNL_AIDING_DATA));
}

int mnl2mpe_hdlr(int fd) {
    char mnl2mpe_buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    char mpe2mnl_buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int mnl2mpe_offset = 0;
    int mpe2mnl_offset = 0;
    int read_len;
    int log_rec = 0, rec_loc =0;
    UINT32 gps_sec = 0;
    UINT32 leap_sec = 18;
    MPE2MNL_AIDING_DATA mnl_out;
    INT8 accuracy[SENSOR_TYPE_MAX] = {-1};
    UINT32 type, length;

    read_len = safe_recvfrom(fd, mnl2mpe_buff, sizeof(mnl2mpe_buff));
    if (read_len <= 0) {
        LOGE("safe_recvfrom() failed read_len=%d", read_len);
        return -1;
    }

    type = get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff));
    length = get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff));
    LOGD("mpe recv, type = %u, len = %u", type, length);

    switch (type) {
        case CMD_START_MPE_REQ: {
            LOGD("%s\n", pMPEVersion);
            get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff)); //no_used
            get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff)); //no_used
            gps_sec = get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff));
            leap_sec = get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff));
            LOGD("gps_sec =%u, leap_sec=%u\n", gps_sec, leap_sec);
            put_int(mpe2mnl_buff, &mpe2mnl_offset, CMD_START_MPE_RES);
            put_int(mpe2mnl_buff, &mpe2mnl_offset, 0);
            mpe2mnl_hdlr((char*)&mpe2mnl_buff);
            mpe_sensor_update_mnl_sec(gps_sec, leap_sec);

            if(mpe_sensor_get_listen_mode() == MPE_IDLE_MODE) {
                mpe_sensor_run();
                LOGD("SE run = %d", mpe_sensor_get_listen_mode());
            } else {
                LOGD("SE already run");
            }
            break;
        }
        case CMD_STOP_MPE_REQ: {
            LOGD("trigger uninit se");
            memset(&mnl_glo_in, 0 , sizeof(MNL2MPE_AIDING_DATA));
            memset(&mnl_latest_in, 0 , sizeof(MNL2MPE_AIDING_DATA));
            mpe_sensor_set_listen_mode(MPE_IDLE_MODE);
            if(!isUninit_SE) {
                LOGD("uninit se type, source");
                put_int(mpe2mnl_buff, &mpe2mnl_offset, CMD_STOP_MPE_RES);
                put_int(mpe2mnl_buff, &mpe2mnl_offset, 0);
                mpe2mnl_hdlr((char*)&mpe2mnl_buff);
            } else {
                isUninit_SE = FALSE;
                LOGD("uninit se type automatically \n");
            }
            break;
        }
        case CMD_GET_ADR_STATUS_REQ: {
            if(mpe_sensor_get_listen_mode() == MPE_START_MODE) {
                mpe_sensor_get_accuracy(accuracy);
                memcpy(&mnl_out, &mnl_glo_out, sizeof(MPE2MNL_AIDING_DATA));
                LOGD("mpe_flag, %d, %d, %d, %d, %d, %d, %d\n",
                    mnl_out.valid_flag[0], mnl_out.valid_flag[1], mnl_out.valid_flag[2], mnl_out.valid_flag[3], accuracy[0], accuracy[1], accuracy[2]);
                if(gMPERawFlag == 1) {
                    LOGD("mnl_out, %lf, %lf, %f, %f, %d, %f, %f, %f, %f, %f\n",
                        mnl_out.latitude, mnl_out.longitude, mnl_out.velocity[0],
                        mnl_out.bearing, mnl_out.staticIndex, mnl_out.barometerHeight,
                        mnl_out.HACC, mnl_out.confidenceIndex[0],
                        gyr_temperature, gyr_scp_calib_done);
                    put_int(mpe2mnl_buff, &mpe2mnl_offset, CMD_SEND_ADR_STATUS_RES);
                    put_binary(mpe2mnl_buff, &mpe2mnl_offset, (const char*)&mnl_out, sizeof(MPE2MNL_AIDING_DATA));
                    mpe2mnl_hdlr((char*)&mpe2mnl_buff);
                }
                if(gMPERawFlag == 0) {
                    LOGD("MPE_DBG: No sensor raw data report. Init MPE kernel\n");
                    mpe_kernel_initialize();
                }
                gMPERawFlag = 0;
            }
            break;
        }
        case CMD_SET_GPS_AIDING_REQ: {
            if(length != sizeof(MNL2MPE_AIDING_DATA)) {
                LOGD("MPE_DBG: mnl_in except len = %d, receive len = %d", sizeof(MNL2MPE_AIDING_DATA), length);
                CRASH_TO_DEBUG();
            }
            memcpy(&mnl_latest_in,((INT8*)mnl2mpe_buff)+sizeof(MPE_MSG), length);
            LOGD("mnl_in, %lf, %lf, %lf, %.3lf, %u, %lld\n", mnl_latest_in.latitude[0], mnl_latest_in.longitude[0], mnl_latest_in.altitude[0],
                mnl_latest_in.gps_sec, mnl_latest_in.leap_sec, mnl_latest_in.gps_kernel_time);

            gps_sec = (UINT32)mnl_latest_in.gps_sec;
            leap_sec = mnl_latest_in.leap_sec;

            char s_lat[2][50];
            char s_lon[2][50];
            char s_alt[2][50];

            memset(s_lat, 0, sizeof(s_lat));
            memset(s_lon, 0, sizeof(s_lon));
            memset(s_alt, 0, sizeof(s_alt));
            mpe_sys_get_encrypt_string(s_lat[0], mnl_latest_in.latitude[0], 50);
            mpe_sys_get_encrypt_string(s_lat[1], mnl_latest_in.latitude[1], 50);
            mpe_sys_get_encrypt_string(s_lon[0], mnl_latest_in.longitude[0], 50);
            mpe_sys_get_encrypt_string(s_lon[1], mnl_latest_in.longitude[1], 50);
            mpe_sys_get_encrypt_string(s_alt[0], mnl_latest_in.altitude[0], 50);
            mpe_sys_get_encrypt_string(s_alt[1], mnl_latest_in.altitude[1], 50);

#ifdef MNLD_WRITE_STORAGE
            if(data_file_local != NULL) {
                fprintf(data_file_local,"& %s %s %s %s %s %s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %.3lf %u %f %f %f %f %f %lld\n",
                    s_lat[0], s_lon[0], s_alt[0],
                    s_lat[1], s_lon[1], s_alt[1],
                    mnl_latest_in.LS_velocity[0], mnl_latest_in.LS_velocity[1], mnl_latest_in.LS_velocity[2],
                    mnl_latest_in.KF_velocity[0], mnl_latest_in.KF_velocity[1], mnl_latest_in.KF_velocity[2],
                    mnl_latest_in.velocityInfo[0], mnl_latest_in.velocityInfo[1], mnl_latest_in.velocityInfo[2],
                    mnl_latest_in.KF_velocitySigma[0], mnl_latest_in.KF_velocitySigma[1], mnl_latest_in.KF_velocitySigma[2],
                    mnl_latest_in.HACC, mnl_latest_in.VACC, mnl_latest_in.HDOP,
                    mnl_latest_in.confidenceIndex[0], mnl_latest_in.confidenceIndex[1], mnl_latest_in.confidenceIndex[2],
                    mnl_latest_in.gps_sec, mnl_latest_in.leap_sec,
                    mnl_latest_in.GNSS_extra_info[0], mnl_latest_in.GNSS_extra_info[1], mnl_latest_in.GNSS_extra_info[2],
                    mnl_latest_in.GNSS_extra_info[3], mnl_latest_in.GNSS_extra_info[4], mnl_latest_in.gps_kernel_time);
            }
#else
            if (u2Log_enable) {
                snprintf(log_buff, MPE_LOG_BUFF, "& %s %s %s %s %s %s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %.3lf %u %f %f %f %f %f %lld\n",
                    s_lat[0], s_lon[0], s_alt[0],
                    s_lat[1], s_lon[1], s_alt[1],
                    mnl_latest_in.LS_velocity[0], mnl_latest_in.LS_velocity[1], mnl_latest_in.LS_velocity[2],
                    mnl_latest_in.KF_velocity[0], mnl_latest_in.KF_velocity[1], mnl_latest_in.KF_velocity[2],
                    mnl_latest_in.velocityInfo[0], mnl_latest_in.velocityInfo[1], mnl_latest_in.velocityInfo[2],
                    mnl_latest_in.KF_velocitySigma[0], mnl_latest_in.KF_velocitySigma[1], mnl_latest_in.KF_velocitySigma[2],
                    mnl_latest_in.HACC, mnl_latest_in.VACC, mnl_latest_in.HDOP,
                    mnl_latest_in.confidenceIndex[0], mnl_latest_in.confidenceIndex[1], mnl_latest_in.confidenceIndex[2],
                    mnl_latest_in.gps_sec, mnl_latest_in.leap_sec,
                    mnl_latest_in.GNSS_extra_info[0], mnl_latest_in.GNSS_extra_info[1], mnl_latest_in.GNSS_extra_info[2],
                    mnl_latest_in.GNSS_extra_info[3], mnl_latest_in.GNSS_extra_info[4], mnl_latest_in.gps_kernel_time);
                mpe_publish_to_log_buf_pool(log_buff);
            }
#endif
            if(mpe_sensor_get_listen_mode() == MPE_START_MODE) {
                mpe_sensor_update_mnl_sec(gps_sec, leap_sec);
                mpe_sensor_check_time();
                //mpe_log_check_file();
                put_int(mpe2mnl_buff, &mpe2mnl_offset, CMD_SEND_GPS_AIDING_RES);
                put_int(mpe2mnl_buff, &mpe2mnl_offset, 0);
                mpe2mnl_hdlr((char*)&mpe2mnl_buff);
            } else {
                mpe_sensor_update_mnl_sec(gps_sec,leap_sec);
                mpe_sensor_run();
                LOGD("SE run = %d", mpe_sensor_get_listen_mode());
            }
            break;
        }
        case CMD_SEND_FROM_MNLD: {
            log_rec = get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff));
            rec_loc = get_int(mnl2mpe_buff, &mnl2mpe_offset, sizeof(mnl2mpe_buff));
            get_binary(mnl2mpe_buff, &mnl2mpe_offset, mpe_debuglog_file_name, sizeof(mnl2mpe_buff), sizeof(mpe_debuglog_file_name));
            LOGD("log_rec =%d, rec_loc=%d, mpelog_path:%s", log_rec, rec_loc,mpe_debuglog_file_name );
            mpe_log_mtklogger_check((INT16)log_rec, mpe_debuglog_file_name, (INT8)rec_loc);
            break;
        }
        default: {
            LOGD("MPE_DBG: invalid msg type = %d", type);
        }
    }
    return 0;
}

void *mpe_calib_thread(void * arg) {
    int timeout_cnt = 0;
    INT8 accuracy[SENSOR_TYPE_MAX] = {0};

    UNUSED(arg);
    pthread_detach(pthread_self());
    LOGD("MPE_calib_thread, create\n");

    sleep(10);
    mpe_sensor_init(SENSOR_USER_ID_CALIB);
    mpe_sensor_start(SENSOR_USER_ID_CALIB, SENSOR_TYPE_GYR, 5);
    while (1) {
        mpe_sensor_get_calib_accuracy(accuracy);
        LOGD("MPE_calib_thread, %d %d\n", timeout_cnt, accuracy[1]);
        if (accuracy[SENSOR_TYPE_GYR] == 3) {
            LOGD("MPE_calib_thread, acc ok\n");
            break;
        } else if (timeout_cnt > 60) {
            LOGD("MPE_calib_thread, timeout\n");
            break;
        } else if (mpe_sensor_get_calib_gyr_data() == false && timeout_cnt > 15) {
            LOGD("MPE_calib_thread, no gyro data report\n");
            break;
        }
        sleep(1);
        timeout_cnt++;
    }
    mpe_sensor_stop(SENSOR_USER_ID_CALIB, SENSOR_TYPE_GYR);
    mpe_sensor_deinit(SENSOR_USER_ID_CALIB);
    LOGD("MPE_calib_thread, exit\n");

    pthread_exit(NULL);
    return NULL;
}

void *mpe_sensor_thread(void * arg) {
    UNUSED(arg);
    gMpeThreadExist = 1;
    pthread_detach(pthread_self());
    LOGD("MPE_sensor_thread, create\n");

    mpe_sensor_init(SENSOR_USER_ID_MPE);
    mpe_sensor_start(SENSOR_USER_ID_MPE, SENSOR_TYPE_UNCAL_GYR, 1000 / MPE_GYRO_DEFAULT_RATE);
    mpe_sensor_start(SENSOR_USER_ID_MPE, SENSOR_TYPE_ACC, 1000 / MPE_ACC_DEFAULT_RATE);
    mpe_sensor_start(SENSOR_USER_ID_MPE, SENSOR_TYPE_MAG, 1000 / MPE_MAG_DEFAULT_RATE);
    mpe_sensor_start(SENSOR_USER_ID_MPE, SENSOR_TYPE_BAR, 1000 / MPE_BARO_DEFAULT_RATE);
    mpe_sensor_start(SENSOR_USER_ID_MPE, SENSOR_TYPE_GYR_TMP, 1000 / MPE_GYRO_DEFAULT_RATE);
    pthread_mutex_lock(&mpe_sensor_freq_mutex);
    mpe_sensor_freq_flag = MTK_MPE_SENSOR_AT_50HZ;
    pthread_mutex_unlock(&mpe_sensor_freq_mutex);

    while (1) {
        if (!mpe_sensor_check_mnl_response()) {
            break;
        }
        mpe_sensor_sync_kernel_utc_time();

        if (mpe_sensor_freq_flag == MTK_MPE_SENSOR_CHANGE_TO_200HZ) {
            LOGD("Change sensor to 200Hz by registering again\n");
            mpe_sensor_start(SENSOR_USER_ID_MPE, SENSOR_TYPE_ACC, 1000 / 200);
            mpe_sensor_start(SENSOR_USER_ID_MPE, SENSOR_TYPE_UNCAL_GYR, 1000 / 200);
            mpe_sensor_start(SENSOR_USER_ID_MPE, SENSOR_TYPE_GYR_TMP, 1000 / 200);
            pthread_mutex_lock(&mpe_sensor_freq_mutex);
            mpe_sensor_freq_flag = MTK_MPE_SENSOR_AT_200HZ;
            pthread_mutex_unlock(&mpe_sensor_freq_mutex);
        }

        // Wait for signal up to 1s
        struct timeval now;
        struct timespec timeout;
        int retcode;
        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec + 1;
        timeout.tv_nsec = now.tv_usec * 1000;
        retcode = 0;
        pthread_mutex_lock(&mpe_sensor_freq_mutex);
        while (mpe_sensor_freq_flag != MTK_MPE_SENSOR_CHANGE_TO_200HZ && retcode != ETIMEDOUT) {
            retcode = pthread_cond_timedwait(&mpe_sensor_freq_cond, &mpe_sensor_freq_mutex, &timeout);
        }
        pthread_mutex_unlock(&mpe_sensor_freq_mutex);

        if (retcode != ETIMEDOUT) {
            LOGD("Get the signal from mpe_sensor_freq_cond\n");
        }
    }

    pthread_mutex_lock(&mpe_sensor_freq_mutex);
    mpe_sensor_freq_flag = MTK_MPE_SENSOR_INIT;
    pthread_mutex_unlock(&mpe_sensor_freq_mutex);
    mpe_sensor_stop(SENSOR_USER_ID_MPE, SENSOR_TYPE_ACC);
    mpe_sensor_stop(SENSOR_USER_ID_MPE, SENSOR_TYPE_UNCAL_GYR);
    mpe_sensor_stop(SENSOR_USER_ID_MPE, SENSOR_TYPE_MAG);
    mpe_sensor_stop(SENSOR_USER_ID_MPE, SENSOR_TYPE_BAR);
    mpe_sensor_stop(SENSOR_USER_ID_MPE, SENSOR_TYPE_GYR_TMP);
    mpe_sensor_deinit(SENSOR_USER_ID_MPE);
    mpe_dr_deinitialize();
    mpe_sensor_stop_notify();
    LOGD("MPE_sensor_thread, exit\n");
    gMpeThreadExist = 0;
    pthread_exit(NULL);
    return NULL;
}


int mpe_kernel_inject(IMU *data, uint64_t *data_utc, UINT16 len, MPE2MNL_AIDING_DATA *mpe3_out, MPE2MNL_AIDING_DATA *mpe4_out) {
    int i=0;

    if(data == NULL) {
        LOGD("allocate sensor cb error\n");
        return MTK_GPS_ERROR;
    }

    for(i = 0; i < len; i++ ) {
        if(data + i != NULL) {
            if (mpe_sys_get_mpe_conf_flag() & MPE_CONF_PRT_RAWDATA) {
                LOGD("[%d] MPErpy %lld %llu %f %f %f %f %f %f %f %f %f %f %lf %lf %lf %lf %lf %lf %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %.3lf %u %f %f %f %f %f %f",
                    i, (data+i)->input_time_gyro, data_utc[i],
                    (data+i)->acceleration[0], (data+i)->acceleration[1], (data+i)->acceleration[2],
                    (data+i)->angularVelocity[0], (data+i)->angularVelocity[1], (data+i)->angularVelocity[2],
                    (data+i)->magnetic[0], (data+i)->magnetic[1], (data+i)->magnetic[2],
                    (data+i)->pressure,
                    mnl_glo_in.latitude[0], mnl_glo_in.longitude[0], mnl_glo_in.altitude[0],
                    mnl_glo_in.latitude[1], mnl_glo_in.longitude[1], mnl_glo_in.altitude[1],
                    mnl_glo_in.LS_velocity[0], mnl_glo_in.LS_velocity[1], mnl_glo_in.LS_velocity[2],
                    mnl_glo_in.KF_velocity[0], mnl_glo_in.KF_velocity[1], mnl_glo_in.KF_velocity[2],
                    mnl_glo_in.velocityInfo[0], mnl_glo_in.velocityInfo[1], mnl_glo_in.velocityInfo[2],
                    mnl_glo_in.KF_velocitySigma[0], mnl_glo_in.KF_velocitySigma[1], mnl_glo_in.KF_velocitySigma[2],
                    mnl_glo_in.HACC, mnl_glo_in.VACC, mnl_glo_in.HDOP,
                    mnl_glo_in.confidenceIndex[0], mnl_glo_in.confidenceIndex[1], mnl_glo_in.confidenceIndex[2],
                    mnl_glo_in.gps_sec, mnl_glo_in.leap_sec, (data+i)->thermometer,
                    mnl_glo_in.GNSS_extra_info[0], mnl_glo_in.GNSS_extra_info[1], mnl_glo_in.GNSS_extra_info[2],
                    mnl_glo_in.GNSS_extra_info[3], mnl_glo_in.GNSS_extra_info[4]);
            }

            if (mpe_get_dr_entry(data+i, &mnl_glo_in, mpe3_out, mpe4_out, data_utc[i])) {
                if (mpe_sys_get_mpe_conf_flag() & MPE_CONF_PRT_RAWDATA) {
                    LOGD("MPE_DBG: MPE mpe_update_posture return false");
                }
                mpe3_out->staticIndex = 99;
                mpe3_out->confidenceIndex[0] = 3;
                mpe3_out->barometerHeight = -16000;
            }
        } else {
            LOGD("accept null data\n");
        }
    }
    return MTK_GPS_SUCCESS;
}

void mpe_merge_output_struct(MPE2MNL_AIDING_DATA *mpe3_out, MPE2MNL_AIDING_DATA *mpe4_out, MPE2MNL_AIDING_DATA *mpe_out) {
    int i = 0;
    /* Common part */
    if (mpe_sensor_get_listen_mode() == MPE_START_MODE) {
        mpe_out->valid_flag[MTK_MPE_SYS_FLAG] = 1;
    } else {
        mpe_out->valid_flag[MTK_MPE_SYS_FLAG] = 0;
    }
    gMPERawFlag = 1;

    /* MPE 3.0 baroHeight & staticIndex */
    if(isfirst_baro == 1) {
        mpe_out->barometerHeight = -16000;
    } else {
        mpe_out->barometerHeight = mpe3_out->barometerHeight;
    }
    mpe_out->staticIndex = mpe3_out->staticIndex;

    /* MPE 4.0 */
    mpe_out->valid_flag[MTK_MPE_KER_FLAG] = mpe4_out->valid_flag[MTK_MPE_KER_FLAG];
    mpe_out->valid_flag[MTK_MPE_INFO_FLAG] = mpe4_out->valid_flag[MTK_MPE_INFO_FLAG];
    mpe_out->valid_flag[MTK_MPE_VER_FLAG] = 4;
    mpe_out->latitude = mpe4_out->latitude;
    mpe_out->longitude = mpe4_out->longitude;
    mpe_out->altitude = mpe4_out->altitude;
    mpe_out->HACC = mpe4_out->HACC;
    mpe_out->bearing = mpe4_out->bearing;
    mpe_out->confidenceIndex[0] = mpe4_out->confidenceIndex[0];
    mpe_out->confidenceIndex[1] = mpe4_out->confidenceIndex[1];
    for (i = 0; i < 3; i++) {
        mpe_out->velocity[i] = mpe4_out->velocity[i];
        mpe_out->velocitySigma[i] = mpe4_out->velocitySigma[i];
    }

    /* Tunnel Config */
    if (!(mpe_sys_get_mpe_conf_flag() & MPE_CONF_INDOOR_ENABLE)/* || gyr_scp_calib_done == 0 */) {
        mpe_out->HACC= 20000;
    }
}

void mpe_sensor_stop_notify( void ) {
    char mpe2mpe_buff[MNL_MPE_MAX_BUFF_SIZE] = {0};
    int offset = 0;
    put_int(mpe2mpe_buff, &offset, CMD_STOP_MPE_REQ);
    put_int(mpe2mpe_buff, &offset, sizeof(INT32));
    put_int(mpe2mpe_buff, &offset, 2);
    if(safe_sendto(MNLD_MPE_SOCKET, mpe2mpe_buff, MNL_MPE_MAX_BUFF_SIZE) == -1) {
        LOGE("safe_sendto fail:[%s]%d", strerror(errno), errno);
    }
    isUninit_SE = TRUE;
    LOGD("send uninit request from listener \n");
}

void mpe_run_algo( void ) {
    int data_ret = 0;
    IMU SE_data[MAX_NUM_SAMPLES];
    UINT16 data_cnt = 0;
    uint64_t SE_data_utc[MAX_NUM_SAMPLES] = {0};
    MPE2MNL_AIDING_DATA mpe_out;
    MPE2MNL_AIDING_DATA mpe3_out;
    MPE2MNL_AIDING_DATA mpe4_out;

    memset(SE_data, 0 ,MAX_NUM_SAMPLES*sizeof(IMU));
    memset(&mpe_out, 0, sizeof(MPE2MNL_AIDING_DATA));
    memset(&mpe3_out, 0, sizeof(MPE2MNL_AIDING_DATA));
    memset(&mpe4_out, 0, sizeof(MPE2MNL_AIDING_DATA));

    data_cnt = mpe_sensor_acquire_Data(SE_data, SE_data_utc);
    if(data_cnt) {
        data_ret = mpe_kernel_inject(SE_data, SE_data_utc, data_cnt, &mpe3_out, &mpe4_out);
        mpe_merge_output_struct(&mpe3_out, &mpe4_out, &mpe_out);
        memcpy(&mnl_glo_out, &mpe_out, sizeof(MPE2MNL_AIDING_DATA));
    }
    memcpy(&mnl_glo_in, &mnl_latest_in, sizeof(MNL2MPE_AIDING_DATA));
}

void mpe_kernel_initialize(void) {
    MPE_SENSOR_NAME pSensor = {0};
    pSensor.acc_freq = MPE_ACC_DEFAULT_RATE;
    pSensor.gyro_freq = MPE_GYRO_DEFAULT_RATE;
    pSensor.mag_freq = MPE_MAG_DEFAULT_RATE;
    memcpy(pSensor.acc_name, accelerometer_name, sizeof(pSensor.acc_name));
    memcpy(pSensor.gyro_name, gyroscope_name, sizeof(pSensor.gyro_name));
    memcpy(pSensor.mag_name, magnetic_name, sizeof(pSensor.mag_name));

    mpe_dr_re_initialize(pSensor);
}

#ifdef __cplusplus
  extern "C" }
#endif

#endif //#ifndef MPE_FUSION_MAIN_C
