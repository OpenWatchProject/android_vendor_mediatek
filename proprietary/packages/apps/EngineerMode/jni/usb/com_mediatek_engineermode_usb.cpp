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

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include <sys/types.h>
#include <sys/stat.h>
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/stat.h>
#include <sys/ioctl.h>

#include <nativehelper/JNIHelp.h>
#include "android_runtime/AndroidRuntime.h"
#undef LOG_NDEBUG
#undef NDEBUG

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "OTG_TEST"

using namespace android;

#define OTG_TEST_DEV "/dev/mt_otg_test"
int fd = -1;

/*macro for USB-IF for OTG driver*/
#define OTG_CMD_E_ENABLE_VBUS       0x00
#define OTG_CMD_E_ENABLE_SRP        0x01
#define OTG_CMD_E_START_DET_SRP     0x02
#define OTG_CMD_E_START_DET_VBUS    0x03
#define OTG_CMD_P_A_UUT             0x04
#define OTG_CMD_P_B_UUT             0x05
#define OTG_CMD_P_B_UUT_TD59        0x0d

/*test mode for USB host driver*/
#define HOST_CMD_TEST_SE0_NAK        0x6
#define HOST_CMD_TEST_J              0x7
#define HOST_CMD_TEST_K              0x8
#define HOST_CMD_TEST_PACKET         0x9
#define HOST_CMD_SUSPEND_RESUME      0xa
#define HOST_CMD_GET_DESCRIPTOR      0xb
#define HOST_CMD_SET_FEATURE         0xc

#define HOST_CMD_ENV_INIT             0xe
#define HOST_CMD_ENV_EXIT             0xf

#define OTG_MSG_DEV_NOT_SUPPORT     0x01
#define OTG_MSG_DEV_NOT_CONNECT     0x02
#define OTG_MSG_HUB_NOT_SUPPORT     0x03

#define OTG_STOP_CMD    0x10
#define OTG_INIT_MSG    0x20

/*test number*/
#define ENABLE_VBUS     0x01
#define ENABLE_SRP      0x02
#define DETECT_SRP      0x03
#define DETECT_VBUS     0x04
#define A_UUT                        0x05
#define B_UUT                        0x06
#define TD_5_9                        0x0e

#define TEST_SE0_NAK                0x07
#define TEST_J                        0x08
#define TEST_K                        0x09
#define TEST_PACKET                    0x0a
#define SUSPEND_RESUME                0x0b
#define GET_DESCRIPTOR                0x0c
#define SET_FEATURE                    0x0d

static jboolean usb_nativeInit(JNIEnv *, jobject) {
    fd = open(OTG_TEST_DEV, O_RDWR);
    if (fd < 0) {
        ALOGD("otg_test_init::create and open file fail!\n");
        return JNI_FALSE;
    } else {
        ALOGD("otg_test_init::create and open file OK!\n");
        unsigned int cmd = HOST_CMD_ENV_INIT;
        int ret = -1;
        int temp = 0;
        ret = ioctl(fd, cmd, &temp);

    }
    return JNI_TRUE;

}

static void usb_nativeDeInit(JNIEnv *, jobject) {
    if(fd != -1) {
        unsigned int cmd = HOST_CMD_ENV_EXIT;
        int ret = -1;
        int temp = 0;
        ret = ioctl(fd, cmd, &temp);
        close(fd);
    }
    return;
}

static jboolean usb_nativeStartTest(JNIEnv *, jobject, jint test_nr) {
    if (fd == -1) {
        ALOGD("FD == -1\n");
        return JNI_FALSE;
    }
    unsigned int cmd = 0;
    int ret = -1;
    int temp = 0;
    ALOGD("otg_test_cmd_start::test_nr=%d\n", test_nr);
    switch (test_nr) {
    case ENABLE_VBUS:
        cmd = OTG_CMD_E_ENABLE_VBUS;
        break;
    case ENABLE_SRP:
        cmd = OTG_CMD_E_ENABLE_SRP;
        break;
    case DETECT_SRP:
        cmd = OTG_CMD_E_START_DET_SRP;
        break;
    case DETECT_VBUS:
        cmd = OTG_CMD_E_START_DET_VBUS;
        break;
    case TD_5_9:
        cmd = OTG_CMD_P_B_UUT_TD59;
        break;
    case A_UUT:
        cmd = OTG_CMD_P_A_UUT;
        break;
    case B_UUT:
        cmd = OTG_CMD_P_B_UUT;
        break;
    case TEST_SE0_NAK:
        cmd = HOST_CMD_TEST_SE0_NAK;
        break;
    case TEST_J:
        cmd = HOST_CMD_TEST_J;
        break;
    case TEST_K:
        cmd = HOST_CMD_TEST_K;
        break;
    case TEST_PACKET:
        cmd = HOST_CMD_TEST_PACKET;
        break;
    case SUSPEND_RESUME:
        cmd = HOST_CMD_SUSPEND_RESUME;
        break;
    case GET_DESCRIPTOR:
        cmd = HOST_CMD_GET_DESCRIPTOR;
        break;
    case SET_FEATURE:
        cmd = HOST_CMD_SET_FEATURE;
        break;
    }
    ret = ioctl(fd, cmd, &temp);
    if (ret < 0) {
        ALOGD("otg_test_cmd_start::fail and cmd=%d\n", cmd);
        return JNI_FALSE;
    } else {
        ALOGD("otg_test_cmd_start::OK and cmd=%d\n", cmd);
        return JNI_TRUE;
    }

}

static jboolean usb_nativeStopTest(JNIEnv *, jobject, jint test_nr) {
    if (fd == -1) {
        ALOGD("FD == -1\n");
        return JNI_FALSE;
    }
    unsigned int stop_cmd;
    int ret = -1;
    ALOGD("otg_test_cmd_stop::test_nr=%d\n", test_nr);

    stop_cmd = OTG_STOP_CMD;
    if (0 == write(fd, &stop_cmd, sizeof(unsigned int))) {
        ALOGD("otg_test_cmd_stop::stop cmd OK\n");
        ret = 0;
    } else {
        ALOGD("otg_test_cmd_stop::stop cmd fail\n");
    }
    if (ret < 0) {
        return JNI_FALSE;
    } else {
        return JNI_TRUE;
    }
}

static jint usb_nativeGetMsg(JNIEnv *, jobject) {
    if (fd == -1) {
        ALOGD("FD == -1\n");
        return 0;
    }
    int msg;
    if (0 <= read(fd, (unsigned int*) &msg, sizeof(int))) {
        ALOGD("otg_test_msg_get::get msg OK,0x%x\n", msg);
    } else {
        ALOGD("otg_test_msg_get::get msg fail\n");
        msg = 0;
    }
    return msg;

}

static jboolean usb_nativeCleanMsg(JNIEnv *, jobject) {
    int ret;
    ALOGD("UsbDriver_nativeCleanMsg");
    if (fd == -1) {
        ALOGD("FD == -1\n");
        return JNI_FALSE;
    }
    unsigned int init_msg = OTG_INIT_MSG;
    ret = write(fd, &init_msg, sizeof(int));
    if (ret < 0) {
        ALOGD("UsbDriver_nativeCleanMsg fail\n");
        return JNI_FALSE;
    } else {
        ALOGD("UsbDriver_nativeCleanMsg OK\n");
        return JNI_TRUE;
    }

}

static JNINativeMethod methods[] = {
    { "nativeInit","()Z", (void *) usb_nativeInit },
    { "nativeDeInit","()V", (void *) usb_nativeDeInit },
    { "nativeStartTest","(I)Z", (void *) usb_nativeStartTest },
    { "nativeStopTest","(I)Z", (void *) usb_nativeStopTest },
    { "nativeGetMsg", "()I", (void *) usb_nativeGetMsg },
    { "nativeCleanMsg","()Z", (void *) usb_nativeCleanMsg },
};

// This function only registers the native methods
static int registerNatives(JNIEnv *env) {
    ALOGE("Register: register_com_mediatek_usb()...\n");
    return AndroidRuntime::registerNativeMethods(env,
            "com/mediatek/engineermode/usb/UsbDriver", methods, NELEM(methods));
}

jint JNI_OnLoad(JavaVM* vm, void*) {
    JNIEnv* env = NULL;
    jint result = -1;

    ALOGD("Enter JNI_OnLoad()...\n");
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        ALOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (registerNatives(env) < 0) {
        ALOGE("ERROR: Native registration failed\n");
        goto bail;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;

    ALOGD("Leave JNI_OnLoad()...\n");
    bail: return result;
}

