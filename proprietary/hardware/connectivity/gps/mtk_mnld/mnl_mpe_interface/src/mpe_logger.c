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
#include "mpe_common.h"
#include "mpe_sensor.h"
#include "gps_dbg_log.h"

//-----------------------------------------------------------------------------
//using namespace android;
//-----------------------------------------------------------------------------

#ifdef MNLD_WRITE_STORAGE
#define MAX_DBG_LOG_W_SEC 3600
#define MAX_DBG_LOG_DIR_SIZE   300*1024*1024
#define MPE_DBG_LOG_FILE_NUM_LIMIT  200
#endif

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

#define LOG_TAG "MPE_LOG"
#include <cutils/sockets.h>
#include <log/log.h>     /*logging in logcat*/
#define LOGD(fmt, arg ...) ALOGD("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGW(fmt, arg ...) ALOGW("%s: " fmt, __FUNCTION__ , ##arg)
#define LOGE(fmt, arg ...) ALOGE("%s: " fmt, __FUNCTION__ , ##arg)

enum {
    GPS_DEBUGLOG_DISABLE       = 0x00,
    GPS_DEBUGLOG_ENABLE        = 0x01,
};

enum {
    log_in_sdcard = 0x00,
    log_in_data_misc,
};

#ifdef MNLD_WRITE_STORAGE
FILE *data_file_local = NULL;
static UINT32 current_file_w_sec = 0;
static long long current_file_sec = 0;
static INT32 current_file_index = 0;
#endif

//for mtklogger
INT16 u2Log_enable = 0;
static INT8 u1Log_location = log_in_sdcard;
static char Log_path[MPE_LOG_NAME_MAX_LEN];

extern UINT32 current_gps_sec;

#ifdef MNLD_WRITE_STORAGE
static void mpe_log_check_and_delete_file (void);
#endif

/*******************************************************************************
* Static function
********************************************************************************/

#ifdef MNLD_WRITE_STORAGE
static int GetFileSize(char *filename)
{
    char dir_filename[MPE_LOG_NAME_MAX_LEN];
    struct stat statbuff;

    memset(dir_filename, 0x00, sizeof(dir_filename));

    if (NULL == filename) {
        LOGD("File name is NULL!! %s", filename);
        return 0;
    }

    snprintf(dir_filename, sizeof(dir_filename), "%s/%s", Log_path,filename);

    if (stat(dir_filename,&statbuff) < 0) {
        LOGD("open file(%s) state  fail(%s)!!", dir_filename, strerror(errno));
        return 0;
    } else {
        return statbuff.st_size;
    }
}

//-----------------------------------------------------------------------------
static INT16 mpe_log_check_total_file(char se_filename[][MPE_LOG_NAME_MAX_LEN], int *total_file_size )
{
    DIR *dir;
    struct dirent *ptr;
    struct stat buf;
    INT16 i= 0;
    int tmp_size = 0, total_size = 0;
    char pathname[MPE_LOG_NAME_MAX_LEN];
    char *file_in_dir, name_tmp[MPE_LOG_NAME_MAX_LEN];

    dir = opendir(Log_path);
    *total_file_size = 0;

    if (dir != NULL) {
        while((ptr = readdir(dir)) != NULL && i < MPE_DBG_LOG_FILE_NUM_LIMIT) {
            memset(pathname, 0, MPE_LOG_NAME_MAX_LEN);
            memset(name_tmp, 0, MPE_LOG_NAME_MAX_LEN);
            sprintf(pathname, "%s%s", Log_path, ptr->d_name );
            if (stat(pathname, &buf) == -1) {
                LOGD("stat return failed %s%s\n", Log_path, ptr->d_name);
                continue;
            }
            if (i >= MPE_DBG_LOG_FILE_NUM_LIMIT) {
                LOGD("max file reached %d\n",i);
                break;
            }
            if (S_ISREG(buf.st_mode)) {
                file_in_dir = ptr->d_name;
                if(!strncmp(file_in_dir, "se_ut", 5)) {
                    memcpy(&name_tmp,file_in_dir, MPE_LOG_NAME_MAX_LEN);
                    memcpy((se_filename+i), &name_tmp, MPE_LOG_NAME_MAX_LEN);
                    tmp_size = GetFileSize(*(se_filename+i));
                    total_size += tmp_size;
                    i++;
                }
            }
        }
        closedir(dir);
        *total_file_size = total_size;
        LOGD("total file size = %d %d", total_size, i);
    }
    return i;
}

//-----------------------------------------------------------------------------
static void mpe_log_conv_filename_to_time( UINT16 file_cnt, char se_filename[][MPE_LOG_NAME_MAX_LEN], INT32 *file_time[],UINT16 *oldest_file_indx  )
{
    UINT16 min_index = 0, j;
    INT32 min_time = 0, min_subtag = 0;
    INT32 local_file_time[MPE_DBG_LOG_FILE_NUM_LIMIT];
    INT32 local_file_subtag[MPE_DBG_LOG_FILE_NUM_LIMIT] = { 0 };
    char tmp_file_time[12];
    char tmp_file_subtag[12];
    char tmp_file_name[MPE_LOG_NAME_MAX_LEN];

    if (file_cnt < 1) {
        return;
    }

    for (j = 0; j < file_cnt; j++) {
        memset(tmp_file_time, 0, sizeof(tmp_file_time));
        memset(tmp_file_subtag, 0, sizeof(tmp_file_subtag));
        memset(tmp_file_name, 0, sizeof(tmp_file_name));
        strncpy(tmp_file_name, se_filename[j], strlen(se_filename[j]));
        tmp_file_name[strlen(tmp_file_name)-4] = 0;
        if (tmp_file_name[16] == '_') {
            strncpy(tmp_file_subtag, tmp_file_name+17, strlen(tmp_file_name+17));
            local_file_subtag[j] = atoi(tmp_file_subtag);
        } else {
            local_file_subtag[j] = 0;
        }

        tmp_file_name[16] = 0;
        strncpy(tmp_file_time, tmp_file_name+5, strlen(tmp_file_name+5));
        local_file_time[j] = atoi(tmp_file_time);

        if ((j == 0) || (local_file_time[j] < min_time) ||
            (local_file_time[j] == min_time && local_file_subtag[j] < min_subtag))
        {
            min_time = local_file_time[j];
            min_subtag = local_file_subtag[j];
            min_index = j;
        }
    }
    memcpy(file_time,local_file_time, file_cnt*sizeof(INT32));

    *oldest_file_indx = min_index;
    LOGD("oldest_file = %u, %s, %d %d\n", *oldest_file_indx, se_filename[min_index], min_time, min_subtag);

    return;
}


//-----------------------------------------------------------------------------
static void mpe_log_check_and_delete_file (void)
{
    //DIR * dir;
    //struct dirent * ptr;
    int total_file_size;
    char se_filename[MPE_DBG_LOG_FILE_NUM_LIMIT][MPE_LOG_NAME_MAX_LEN];
    INT16 total_file;
    UINT16 oldest_indx;
    INT32 *file_time[MPE_DBG_LOG_FILE_NUM_LIMIT];
    char del_filename[MPE_LOG_NAME_MAX_LEN];

    total_file = mpe_log_check_total_file(se_filename, &total_file_size);

    if (total_file > 0) {
        mpe_log_conv_filename_to_time(total_file, se_filename, file_time, &oldest_indx);
        while (total_file_size > MAX_DBG_LOG_DIR_SIZE || total_file >= MPE_DBG_LOG_FILE_NUM_LIMIT) {
            LOGD("total size = %d, total file = %d", total_file_size, total_file);

            //remove oldest file
            memset(del_filename, 0, MPE_LOG_NAME_MAX_LEN);
            sprintf(del_filename, "%s%s", Log_path,*(se_filename + oldest_indx));
            LOGD("delete filename %s %s", del_filename, *(se_filename+oldest_indx));

            if (remove(del_filename) == 0) {
                LOGD("remove %s success", del_filename);
            } else {
                LOGD("remove %s failed", del_filename);
            }

            total_file = mpe_log_check_total_file(se_filename, &total_file_size);
            mpe_log_conv_filename_to_time(total_file, se_filename, file_time, &oldest_indx);
        }
    }
}

//-----------------------------------------------------------------------------
static unsigned char mpe_log_open_new_file (void)
{
    char name[MPE_LOG_NAME_MAX_LEN];
    unsigned char ret = FALSE;
    char header_name[6] = "se_ut";

    if (data_file_local != NULL) {
        fclose(data_file_local);
    } else {
        return ret;
    }

    if (u2Log_enable) {
        current_file_index++;
        sprintf(name, "%s%s0%lld_%d.txt", Log_path, header_name, current_file_sec, current_file_index);
        LOGD("open file - time %s", name);
        data_file_local = fopen(name, "w+");
        if (data_file_local == NULL) {
            LOGD("se file open %s\n", strerror(errno));
            ret = FALSE;
        } else {
            ret = TRUE;
        }
        mpe_log_check_and_delete_file();
    }
    return ret;
}
#endif

/*******************************************************************************
* Global function
********************************************************************************/

void mpe_log_mtklogger_check ( INT16 record_mode, char *logpath, INT8 log_location)
{
    char name[MPE_LOG_NAME_MAX_LEN];
    char header_name[6] = "se_ut";


    if (record_mode == GPS_DEBUGLOG_DISABLE) {
        u2Log_enable = 0;
#ifdef MNLD_WRITE_STORAGE
        if (data_file_local != NULL) {
            LOGD("stop recording se data");
            fclose(data_file_local);
            data_file_local = NULL;
        }
#else
        mnld2logd_close_mpelog();
#endif
    } else if(record_mode == GPS_DEBUGLOG_ENABLE) {
        memcpy(&Log_path, logpath, MPE_LOG_NAME_MAX_LEN);
        u2Log_enable = 1;
        LOGD("se logger path: %s", Log_path);

#ifdef MNLD_WRITE_STORAGE
        if (data_file_local == NULL && mpe_sensor_get_listen_mode() == MPE_START_MODE) {
            sprintf(name, "%s%s0%u.txt", Log_path, header_name, current_gps_sec);
            current_file_sec = (long long)current_gps_sec;
            current_file_index = 0;

            data_file_local = fopen(name,"w+");
            LOGD("open file - mtklog %s\n", name);
            if (data_file_local == NULL) {
                LOGD("se file open %s\n", strerror(errno));
            }
            if (chmod(name, 0600) < 0) {
                LOGD("se file chmod %s\n", strerror(errno));
            }
            mpe_log_check_and_delete_file();
        }
#else
        if (mpe_sensor_get_listen_mode() == MPE_START_MODE) {
            sprintf(name, "%s%s0%u", Log_path, header_name, current_gps_sec);
            mnld2logd_open_mpelog(name);
        }
#endif
    }
    u1Log_location = log_location;
}

#ifdef MNLD_WRITE_STORAGE
UINT16 mpe_log_check_file()
{
    unsigned char file_stat;

    if (u2Log_enable) {
        current_file_w_sec++;
        if (current_file_w_sec >= MAX_DBG_LOG_W_SEC) {
            file_stat = mpe_log_open_new_file();
            if (file_stat) {
                current_file_w_sec = 0;
            }
        }
    }
    return 0;
}
#endif

void mpe_log_init()
{
    char name[MPE_LOG_NAME_MAX_LEN];
    char header_name[6] = "se_ut";

#ifdef MNLD_WRITE_STORAGE
    current_file_w_sec = 0;
#endif

    if (u2Log_enable) {
        LOGD("logpath : %s", Log_path);
#ifdef MNLD_WRITE_STORAGE
        sprintf(name, "%s%s0%u.txt",Log_path, header_name ,current_gps_sec);
        current_file_sec = (long long)current_gps_sec;
        current_file_index = 0;

        LOGD("open file - gps %s", name);
        data_file_local = fopen(name, "w+");
        if (data_file_local == NULL) {
            LOGD("se file open %s\n", strerror(errno));
        }
        if(chmod(name, 0600) < 0) {
            LOGD("se file chmod %s\n", strerror(errno));
        }
        mpe_log_check_and_delete_file();
#else
        sprintf(name, "%s%s0%u",Log_path, header_name ,current_gps_sec);
        mnld2logd_open_mpelog(name);
#endif
    }
    return;
}
void mpe_log_deinit()
{
#ifdef MNLD_WRITE_STORAGE
    if(data_file_local != NULL) {
        fclose(data_file_local);
    }
    current_file_w_sec = 0;
#else
    mnld2logd_close_mpelog();
#endif
    return;
}


