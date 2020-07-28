/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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


#define LOG_TAG "USP_NATIVE"


#include "jni.h"
#include <nativehelper/JNIHelp.h>
#include "../include/usp_native.h"

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <fcntl.h>
#include <linux/disp_session.h>
#include "hardware/ccci_intf.h"

static jint freeze_frame(JNIEnv *env, jobject thiz) {
    int fd = -1;
    int enable = 1;
    ALOGI("[USP] Screen freezed \n");
    fd = open("/dev/mtk_disp_mgr", O_RDWR);
    if (fd < 0) {
        ALOGE("[USP] cannot open /dev/mtk_disp_mgr \n");
        return -1;
    }

    if (ioctl(fd, DISP_IOCTL_SCREEN_FREEZE, &enable) < 0) {
        ALOGE("[USP] failed to freeze screen \n");
        close(fd);
        return -1;
    }
    return 0;
}

static jint unfreeze_frame(JNIEnv *env, jobject thiz) {
    int fd = -1;
    int enable = 0;
    ALOGI("[USP] unfreezed screen \n");
    fd = open("/dev/mtk_disp_mgr", O_RDWR);
    if (fd < 0) {
        ALOGE("[USP] cannot open /dev/mtk_disp_mgr \n");
        return -1;
    }

    if (ioctl(fd, DISP_IOCTL_SCREEN_FREEZE, &enable) < 0) {
        ALOGE("[USP] failed to unfreeze screen \n");
        close(fd);
        return -1;
    }
    return 0;
}

static jint ccci_enter_deep_flight(JNIEnv *env, jobject thiz) {
    int res = -1;

#ifdef __USP_CCCI_DEEP_FLIGHT_SUPPORT__
    char file[64] = "/dev/ccci_ioctl4";
    int fd = open(file, O_RDWR);
    if(fd >= 0) {
        int ret = ioctl(fd, CCCI_IOC_ENTER_DEEP_FLIGHT);
        if(ret != 0) {
            ALOGE("[USP][%s]CCCI_IOC_ENTER_DEEP_FLIGHT failed %d, error=%d\n",__func__, ret, errno);
        } else {
            ALOGI("[USP][%s]CCCI_IOC_ENTER_DEEP_FLIGHT OK %d\n",__func__, ret);
            res = 0;
        }
        close(fd);
    } else {
        ALOGE("[USP][%s]ERROR open fail %d, error=%d\n",__func__, fd, errno);
    }
#else
    ALOGI("[USP][%s]CCCI_IOC_ENTER_DEEP_FLIGHT not implemented\n",__func__);
#endif

return res;
}

static jint ccci_leave_deep_flight(JNIEnv *env, jobject thiz) {
    int res = -1;

#ifdef __USP_CCCI_DEEP_FLIGHT_SUPPORT__
    char file[64] = "/dev/ccci_ioctl4";
    int fd = open(file, O_RDWR);
    if(fd >= 0) {
        int ret = ioctl(fd, CCCI_IOC_LEAVE_DEEP_FLIGHT);
        if(ret != 0) {
            ALOGE("[USP][%s]CCCI_IOC_LEAVE_DEEP_FLIGHT failed %d, error=%d\n",__func__, ret, errno);
        } else {
            ALOGI("[USP][%s]CCCI_IOC_LEAVE_DEEP_FLIGHT OK %d\n",__func__, ret);
            res = 0;
        }
        close(fd);
    } else {
        ALOGE("[USP][%s]ERROR open fail %d, error=%d\n",__func__, fd, errno);
    }
#else
    ALOGI("[USP][%s]ccci_leave_deep_flight not implemented\n",__func__);
#endif
    return res;
}

jint
Java_com_mediatek_usp_UspServiceImpl_freezeFrame(JNIEnv* env, jobject thiz) {
   return freeze_frame(env, thiz);
}

jint
Java_com_mediatek_usp_UspServiceImpl_unfreezeFrame(JNIEnv* env, jobject thiz) {
   return unfreeze_frame(env, thiz);
}

jint
Java_com_mediatek_usp_UspServiceImpl_ccciEnterDeepFlight(JNIEnv* env, jobject thiz) {
   return ccci_enter_deep_flight(env, thiz);
}

jint
Java_com_mediatek_usp_UspServiceImpl_ccciLeaveDeepFlight(JNIEnv* env, jobject thiz) {
   return ccci_leave_deep_flight(env, thiz);
}

static JNINativeMethod gMethods[] = {
    { "freezeFrame",       "()I",       (int *)Java_com_mediatek_usp_UspServiceImpl_freezeFrame},
    { "unfreezeFrame",       "()I",       (int *)Java_com_mediatek_usp_UspServiceImpl_unfreezeFrame},
    { "ccciEnterDeepFlight",       "()I",       (int *)Java_com_mediatek_usp_UspServiceImpl_ccciEnterDeepFlight},
    { "ccciLeaveDeepFlight",       "()I",       (int *)Java_com_mediatek_usp_UspServiceImpl_ccciLeaveDeepFlight},
};
