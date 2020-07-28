/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

#include "omadm_service_api.h"
#include "pal.h"
#include "pal_internal.h"
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PAL"

using namespace android;
using namespace omadm_service_api;

__BEGIN_DECLS
/*
 * User accessible interface
 */


//devDetail
int pal_system_oem_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getDeviceOEM();
    // Can return NULL on failure
    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail:oem: %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}

int pal_system_fwv_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getFWV();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail :Result of operation %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}
/**
 *
 * @param[out] allocated buffer for return value.
 * Software Version of current device
 * For Android device it is a version of android & build
 * Software Version comes from soft build data or
 * can be get by parcing result of shell command "getProp"
 * or
 * using android.os.Build
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_swv_get(data_buffer_t *Buffer) {


     if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getSWV();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
        ALOGD("DevDetail :Result of operation %s", Buffer->data);
    #endif
        return RESULT_SUCCESS;

}/**
 *
 * @param[out] allocated buffer for return value.
 * Hardware Version of current device
 * For Android device it is a version of android & build
 * Hardware Version comes from firmware build data
 * or
 * can be get by parcing result of shell command "getProp"
 * or
 * using android.os.Build
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hwv_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getHWV();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail :Result of operation %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}
/**
 *
 * @param[out] allocated buffer for return value.
 * Support of Large Object("true" or "false")
 * Hardcoded
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_support_lrgobj_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::support_lrgobj_get();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
       ALOGD("DevDetail :Result of operation %s", Buffer->data);
   #endif
       return RESULT_SUCCESS;
}

#define __USE_TIME_FROM_SYSTEM_PARTITION 1
const static char* const __s__system_part_path = "/system";

#define __UPDATE_LATEST_TIME(__PATH__, __STV__, __TMV__, __P_TIME__)       \
{                                                                          \
    int __rc = 0;                                                          \
  if ((__rc = stat((__PATH__), &(__STV__))) >= 0) {                        \
      if ((__STV__).st_mtime > *(__P_TIME__)) {                            \
          *(__P_TIME__) = (__STV__).st_mtime;                              \
          gmtime_r((__P_TIME__), &(__TMV__));                              \
          PRINT_DBG("%04d-%02d-%02d %02d:%02d:%02d %s",                     \
            (__TMV__).tm_year + 1900, (__TMV__).tm_mon + 1,                \
            (__TMV__).tm_mday,                                             \
            (__TMV__).tm_hour, (__TMV__).tm_min, (__TMV__).tm_sec,         \
            (__PATH__));                                                   \
      }                                                                    \
  } else {                                                                 \
      PRINT_WRN("stat(%s, &stv)= %d", (__PATH__), __rc);                    \
  }                                                                        \
}


/*
 * searchs FS object with the latest time of modification
 * recursively and returns this time via p_time
 */
static int __get_latest_dir_time_r(const char* path, time_t* p_time)
{
    char* full_path = NULL;
    struct stat stv;
    struct tm tmv;
    int rc = 0;
    if (p_time == NULL) {
        PRINT_WRN("p_time is null");
        return RESULT_ERROR_INVALID_ARGS;
    }
    if (*p_time == 0) {
        __UPDATE_LATEST_TIME(path, stv, tmv, p_time);
    }
    DIR* dir = opendir(path);
    if (dir == NULL) {
        PRINT_WRN("cannot open DIR:%s", path);
        return RESULT_ERROR;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        if(pal_strcat_path(path, entry->d_name, &full_path) != NULL) {
            __UPDATE_LATEST_TIME(full_path, stv, tmv, p_time);
        }
        FREE_IF(full_path);
        if (pal_is_path_dir(full_path)) {
            rc = __get_latest_dir_time_r(full_path, p_time);
        }
    }

    closedir(dir);
    FREE_IF(full_path);
    return rc;
}

/**
 *
 * @param[out] allocated buffer for return value.
 * Date the Software Update was successfully completed. The format
 shall be "MM:DD:YYYY"
 * from java.util.Date or in c using time.h (library)
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_date_get(data_buffer_t *Buffer)
{
    PRINT_DBG("(%p)", Buffer);
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getDate();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
    PRINT_DBG("(%s):OK", Buffer->data);
    return RESULT_SUCCESS;
}

/**
 *
 * @param[out] allocated buffer for return value.
 * UTC time the Software Update was successfully completed. The UTC
 format shall be "HH:MM"
 * from java.util.Date or in c using time.h (library)
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_timeutc_get(data_buffer_t *Buffer)
{
    PRINT_DBG("(%p)", Buffer);
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getTime();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
    PRINT_DBG("(%s):OK", Buffer->data);
    return RESULT_SUCCESS;
}
/**
 *
 * @param[out] allocated buffer for return value.
 * manufacturer of host device
 * can be get using shell command "lsusb" / "lsusb -v"
 * or
 * from file on host device
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_manu_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getHostDeviceManu();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail :Result of operation %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}
/**
 *
 * @param[out] allocated buffer for return value.
 * Model Name of host device
 * can be get using shell command "lsusb" / "lsusb -v"
 * or
 * from file on host device
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_model_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getHostDeviceModel();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail :Result of operation %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}
/**
 *
 * @param[out] allocated buffer for return value.
 * Software Version of host device
 *from file on host device
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_swv_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getHostSWV();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail :Result of operation %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}
/**
 *
 * @param[out] allocated buffer for return value.
 * Firmware Version of host device
 * from file on host device
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_fwv_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getHostFWV();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail :Result of operation %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}
/**
 *
 * @param[out] allocated buffer for return value.
 * Hardware Version of host device
 * from file on host device
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_hwv_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getHostHWV();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail :Result of operation %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}
/**
 *
 * @param[out] allocated buffer for return value.
 * Date of the latest successful Host Device Software Update.
 * hardcoded or
 * if host device works under linux we need to get date of "/var/log/dpkg.log
 * " file
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_datestamp_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getHostDateStamp();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail :Result of operation %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}
/**
 *
 * @param[out] allocated buffer for return value.
 * ID of host device
 * It can be IMEI, MAC address or ID of usb device
 * if hostdevice works under android we can read using shell "getprop"
 * from android.provider.Settings.Secure
 * from telephonyManager
 * using shell "service call iphonesubinfo"
 * if host device works under linux we need to parce result of
 * "ifconfig command on device"
 * " file
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_deviceid_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getHostID();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail :Result of operation %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}

/**
 *
 * @param[out] allocated buffer for return value.
 * Type of current device
 * It can be "Smart Device" or "Feature Phone"
 *
 * @return
 * if length of buffer <  length of value, then returns BUFFER_OVERFLOW
 * if buffer == NULL or data of buffer == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS
 *
 */
int pal_system_devtype_get(data_buffer_t *Buffer) {
    if ((NULL == Buffer) || (NULL == Buffer->data) || (0 == Buffer->size))
            return RESULT_BUFFER_NOT_DEFINED;
    AutoMutex _l(OmadmServiceAPI::mPalLock);

       if (!OmadmServiceAPI::isPalInitialized())
           return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getTyp();

    if (NULL == reply)
        return RESULT_ERROR;

    if ((reply->size() + 1) > Buffer->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(Buffer->data, 0, Buffer->size);
    strncpy(Buffer->data, String8(*reply).string(), Buffer->size);
    delete reply;
#ifdef DEBUG
    ALOGD("DevDetail :Result of operation %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}

__END_DECLS
