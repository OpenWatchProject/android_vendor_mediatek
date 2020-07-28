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

/* OTP add */
#include <stdlib.h>
#include <sys/ioctl.h>
#include "fsd_platform.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>

#ifdef ENABLE_DEBUG_LOG
char  debug_level = ANDROID_LOG_WARN;

void get_debug_log_level(void)
{
    char buf[PROPERTY_VALUE_MAX];
    int val = 0;
    /*log level: warning == 5, info == 4, debug == 3*/
    property_get("persist.vendor.md.fsd.debug", buf, "none");
    if (0 != strcmp(buf, "none")) {
        val = atoi(buf);
        debug_level = val;
        LOGW("persist.vendor.md.fsd.debug %d, warn = %d, debug = %d\n", debug_level, ANDROID_LOG_WARN, ANDROID_LOG_DEBUG);
    }
}
#else
void get_debug_log_level(void)
{
}
#endif


/************************************************/
/* OTP(one time operation) function                              */
/*                                                                             */
/************************************************/
#define ENABL_OTP_API
#ifdef ENABL_OTP_API
#include <storage_otp.h>

static unsigned int    otp_offset = 0;

struct otp_ctl
{
    unsigned int QLength;
    unsigned int Offset;
    unsigned int Length;
    char *BufferPtr;
    unsigned int status;
};

#define OTP_DEVICE          "/dev/otp"
#define OTP_MAGIC           'k'
#define OTP_GET_LENGTH         _IOW(OTP_MAGIC, 1, int)
#define OTP_READ             _IOW(OTP_MAGIC, 2, int)
#define OTP_WRITE             _IOW(OTP_MAGIC, 3, int)
//#define OTP_LOCK             _IOW(OTP_MAGIC, 4, int)

#endif

int FS_OTPLock(int devtype __attribute__((unused)))
{
    int ret = FS_NO_ERROR;

#ifdef ENABL_OTP_API
    if (otp_get_libversion()) {
        struct otp * otp_dev = NULL;

        otp_dev = otp_open(USER_CCCI);
        if(!otp_dev) {
            LOGE("%s:otp_open failed, errno=%d!\n", __func__, errno);
            ret = FS_UNSUPPORTED_DRIVER_FUNCTION;
            goto Exit;
        }
        ret = otp_lock(otp_dev, WP_TEMPORARY);
        if (ret) {
            LOGE("%s:otp_lock:ret=%d, errno=%d!\n", __func__, ret, errno);
            otp_close(otp_dev);
            ret = FS_PARAM_ERROR;
            goto Exit;
        }
        otp_close(otp_dev);
    } else {
        int fd;

        fd = open(OTP_DEVICE, O_RDONLY, 0);
        if (fd < 0) {
            LOGE("%s: [error]Cann't open the /dev/otp \n", __func__);
            ret = FS_UNSUPPORTED_DRIVER_FUNCTION;
            goto Exit;
        }
        close(fd);
    }
#else
    LOGD("OTPLock: Fake !!!\n");
#endif //ENABL_OTP_API

Exit:
    LOGD("OL: %d : %d \n", devtype, ret);
    return ret;
}


int FS_OTPQueryLength(int devtype __attribute__((unused)), unsigned int * Length)
{
    int ret = FS_NO_ERROR;

    if (Length == NULL) {
        LOGE("%s:Length = NULL\n", __func__);
        ret = FS_PARAM_ERROR;
        goto Exit;
    }
#ifdef ENABL_OTP_API
    if (otp_get_libversion()) {
        struct otp * otp_dev = NULL;
        unsigned long len;

        otp_dev = otp_open(USER_CCCI);
        if(!otp_dev) {
            LOGE("%s:otp_open failed, errno=%d!\n", __func__, errno);
            ret = FS_UNSUPPORTED_DRIVER_FUNCTION;
            goto Exit;
        }
        len = otp_get_size(otp_dev);
        LOGE("%s:otp_get_size:%ld!\n", __func__, len);
        *(unsigned int *)Length = len/2; // One MD occupy 1/2
        otp_close(otp_dev);
    } else {
        int fd;
        struct otp_ctl otpctl = {.QLength=0 };

        fd = open(OTP_DEVICE, O_RDONLY, 0);
        if (fd < 0) {
            LOGE("%s:Open %s failed, errno=%d \n", __func__, OTP_DEVICE, errno);
            ret = FS_UNSUPPORTED_DRIVER_FUNCTION;
            goto Exit;
        }
        otpctl.status = 'c' + ('c'<<8) + ('c'<<16) + ('i'<<24);
        ret = ioctl(fd, (unsigned int)OTP_GET_LENGTH, (unsigned long) &otpctl);
        close(fd);
        *(unsigned int *)Length = otpctl.QLength/2; // One MD occupy 1/2
    }
#else
    LOGD("OTPQueryLength: Fake !!! \n");
    *Length = 4096;
#endif //ENABL_OTP_API

Exit:
    LOGD("OQL: %d: %d %d \n", devtype, (Length!=NULL)?*Length:0, ret);
    return ret;
}


int FS_OTPRead(int devtype __attribute__((unused)), unsigned int  Offset,
    void * BufferPtr __attribute__((unused)), unsigned int Length)
{
    int ret = FS_NO_ERROR;

#ifdef ENABL_OTP_API
    if (otp_get_libversion()) {
        struct otp * otp_dev = NULL;
        unsigned long read_start;

        otp_dev = otp_open(USER_CCCI);
        if(!otp_dev) {
            LOGE("%s:otp_open failed, errno=%d!\n", __func__, errno);
            ret = FS_UNSUPPORTED_DRIVER_FUNCTION;
            goto Exit;
        }
        read_start = md_id == 0 ? Offset : Offset + otp_offset;
        ret = otp_read(otp_dev, BufferPtr, read_start, Length);
        if (ret){
            LOGE("%s:otp_read:%d, errno=%d!\n", __func__, ret, errno);
            ret = FS_ERROR_RESERVED;
            otp_close(otp_dev);
            goto Exit;
        }
        otp_close(otp_dev);
    } else {
        int fd;
        struct otp_ctl otpctl = {.QLength=0 };

        fd = open(OTP_DEVICE, O_RDONLY, 0);
        if (fd < 0) {
            LOGE("OTPLock: Cann't open the /dev/otp \n");
            ret = FS_UNSUPPORTED_DRIVER_FUNCTION;
            goto Exit;
        }
        otpctl.BufferPtr = BufferPtr;
        otpctl.Length = Length;
        otpctl.Offset = md_id == 0 ? Offset : Offset + otp_offset;
        otpctl.status = 0;
        ioctl(fd, (unsigned int)OTP_READ, (unsigned long) &otpctl);
        close(fd);

        if (otpctl.status) {
            LOGE("OTPLock: [error]fail read operation! \n");
            ret = FS_ERROR_RESERVED;
            goto Exit;
        }
    }
#else
    unsigned int index;

    LOGD("OTPRead: Fake !!! \n");
    for(index = 0; index < Length; index++)
        *((unsigned char*)BufferPtr+index) = Length - index;
#endif //ENABL_OTP_API

Exit:
    LOGD("OR: %d %d %d: %d \n", devtype, Offset, Length, ret);
    return ret;
}


int FS_OTPWrite(int devtype __attribute__((unused)), unsigned int  Offset,
    void * BufferPtr __attribute__((unused)), unsigned int Length)
{
    int ret = FS_NO_ERROR;

#ifdef ENABL_OTP_API
    if (otp_get_libversion()) {
        struct otp * otp_dev = NULL;
        unsigned long write_start;

        otp_dev = otp_open(USER_CCCI);
        if(!otp_dev) {
            LOGE("%s:otp_open failed, errno=%d!\n", __func__, errno);
            ret = FS_UNSUPPORTED_DRIVER_FUNCTION;
            goto Exit;
        }
        write_start = md_id == 0 ? Offset : Offset + otp_offset;
        ret = otp_write(otp_dev, BufferPtr, write_start, Length);
        if (ret){
            LOGE("%s:otp_write:%d, errno=%d!\n", __func__, ret, errno);
            ret = FS_ERROR_RESERVED;
            otp_close(otp_dev);
            goto Exit;
        }
        otp_close(otp_dev);
    } else {
        int fd;
        struct otp_ctl otpctl = {.QLength=0 };

        fd = open(OTP_DEVICE, O_RDONLY, 0);
        if (fd < 0) {
            LOGE("OTPWrite: Cann't open the /dev/otp \n");
            ret = -1;
            goto Exit;
        }

        otpctl.BufferPtr = (char *)BufferPtr;
        otpctl.Length = Length;
        otpctl.Offset = md_id == 0 ? Offset: Offset + otp_offset;
        otpctl.status = 0;
        ioctl(fd, (unsigned int)OTP_WRITE, (unsigned long) &otpctl);
        close(fd);
        if (otpctl.status) {
            LOGE("OTPWrite: [error]fail write operation! \n");
            ret = FS_ERROR_RESERVED;
            goto Exit;
        }
    }
#else
    LOGD("OTPWrite: Fake !!!\n");
#endif //ENABL_OTP_API

Exit:
    LOGD("OW: %d %d %d: %d \n", devtype, Offset, Length, ret);
    return ret;
}
int FS_OTP_init(int md_id __attribute__((unused)))
{
    int ret = FS_NO_ERROR;

#ifdef ENABL_OTP_API
    //md_id = md_idx;
    if (otp_get_libversion()) {
        struct otp * otp_dev = NULL;
        unsigned long len;
        unsigned int status = 0, type = 0;

        otp_dev = otp_open(USER_CCCI);
        if(!otp_dev) {
            LOGE("%s:otp_open failed, errno=%d!\n", __func__, errno);
            ret = FS_UNSUPPORTED_DRIVER_FUNCTION;
            goto Exit;
        }
        len = otp_get_size(otp_dev);
        otp_offset = len/2; // One MD occupy 1/2
        //otp_get_status(otp_dev, &status, &type);
        otp_close(otp_dev);
        LOGE("%s:otp_get_size:%ld, status=%d, type=%d!\n", __func__, len, status, type);
    } else {
        int otp_fd;
        struct otp_ctl otpctl = {.QLength=0 };

        // Check whether OTP is support
        LOGD("Check OTP...\n");
        otp_fd = open(OTP_DEVICE, O_RDONLY, 0);
        if(otp_fd < 0) {
            LOGE("%s:open %s failed, errno=%d!\n", __func__, OTP_DEVICE, errno);
        } else {
            LOGD("OTP feature enabled\n");
            otpctl.status = 'c' + ('c'<<8) + ('c'<<16) + ('i'<<24);
            ioctl(otp_fd, (unsigned int)OTP_GET_LENGTH, (unsigned long) &otpctl);
            otp_offset = otpctl.QLength/2;// One MD occupy 1/2
            close(otp_fd);
        }
    }
#else
    LOGD("FS OTP is disabled!\n");
#endif
Exit:
    return ret;
}

int get_modem_status(void)
{
    char mdstatus[50];
    char mdstatus_val[PROPERTY_VALUE_MAX]={'\0'};
    int retpropget = 0, ret = CCCI_MD_STA_UNDEFINED;

    snprintf(mdstatus, sizeof(mdstatus), "vendor.mtk.md%d.status", md_id+1);
    retpropget = property_get(mdstatus, mdstatus_val, NULL);

    if (retpropget < 0) /* property_get return negative, it seems impossible */
        LOGI("get prop error returned:%d, errno:%d\n", retpropget, errno);
    else if (retpropget == 0) { /* has no this property, needn't try again */
        /*LOGI("get no prop:%s\n", md_status);*/
    } else if (strncmp(mdstatus_val, "bootup", 6) == 0)
        ret = CCCI_MD_STA_BOOT_UP;
    else if (strncmp(mdstatus_val, "ready", 5) == 0)
        ret = CCCI_MD_STA_BOOT_READY;
    else if (strncmp(mdstatus_val, "init", 4) == 0)
        ret = CCCI_MD_STA_INIT;
    else if (strncmp(mdstatus_val, "reset", 5) == 0)
        ret = CCCI_MD_STA_RESET;
    else if (strncmp(mdstatus_val, "stop", 4) == 0)
        ret = CCCI_MD_STA_STOP;
    else if (strncmp(mdstatus_val, "flightmode", 10) == 0)
        ret = CCCI_MD_STA_FLIGHT_MODE;
    else if (strncmp(mdstatus_val, "exception", 9) == 0)
        ret = CCCI_MD_STA_EXCEPTION;

    return ret;    
}


