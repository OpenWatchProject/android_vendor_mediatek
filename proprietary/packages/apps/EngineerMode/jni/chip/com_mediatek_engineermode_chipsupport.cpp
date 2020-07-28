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

#define MTK_LOG_ENABLE 1
#include "jni.h"
#include <nativehelper/JNIHelp.h>
#include "android_runtime/AndroidRuntime.h"
#undef LOG_NDEBUG
#undef NDEBUG

#include <cutils/properties.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EM_CHIP_SUPPORT"

#include <binder/IBinder.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <hwcomposer_defs.h>
using namespace android;

#define EM_CHIP_NAME_LENGTH (16)

#define MTK_UNKNOWN_SUPPORT 0
#define MTK_6735_SUPPORT 1
#define MTK_8163_SUPPORT 2
#define MTK_6570_SUPPORT 3
#define MTK_6580_SUPPORT 4
#define MTK_6755_SUPPORT 5
#define MTK_6750_SUPPORT 6
#define MTK_6797_SUPPORT 7
#define MTK_6757_SUPPORT 8
#define MTK_6757CH_SUPPORT 9
#define MTK_6759_SUPPORT 10
#define MTK_6799_SUPPORT 11
#define MTK_COMMON_CHIP_SUPPORT 20

typedef struct {
    const char *chip_name;
     int chip_id;
} chip_mapping;

static const chip_mapping s_chip_mappings[] = {
    {"67+35", MTK_6735_SUPPORT},
    {"67+35m", MTK_6735_SUPPORT},
    {"67+53", MTK_6735_SUPPORT},
    {"67+37t", MTK_6735_SUPPORT},
    {"67+37m", MTK_6735_SUPPORT},
    {"67+37", MTK_6735_SUPPORT},
    {"65+70", MTK_6570_SUPPORT},
    {"65+80", MTK_6580_SUPPORT},
    {"81+63", MTK_8163_SUPPORT},
    {"67+55", MTK_6755_SUPPORT},
    {"67+50", MTK_6750_SUPPORT},
    {"67+97", MTK_6797_SUPPORT},
    {"67+57", MTK_6757_SUPPORT},
    {"67+59", MTK_6759_SUPPORT},
    {"67+99", MTK_6799_SUPPORT}
};

int get_6757_chip_id() {

    typedef struct {
        unsigned int entry_num;
        unsigned int data[100];
    } DEVINFO_S;
    int fd = 0;
    int ret = 0;
    DEVINFO_S devinfo;
    fd = open("/sys/bus/platform/drivers/dev_info/dev_info", O_RDONLY);
    if (fd < 0) {
        fd = open("/proc/device-tree/chosen/atag,devinfo", O_RDONLY);
        if (fd < 0) {
            ret = -1;
            return MTK_UNKNOWN_SUPPORT;
        }
    }
    ret = read(fd, (void *)&devinfo, sizeof(DEVINFO_S));
    close(fd);
    if (ret < 0) {
        return MTK_UNKNOWN_SUPPORT;
    }
    ALOGD("Data:0x%x\n", devinfo.data[30]);

    unsigned int segment = (devinfo.data[30] & 0x000000E0) >> 5;
    if ((segment == 0x0) || (segment == 0x1)) {
        return MTK_6757_SUPPORT;
    } else if ((segment == 0x3) || (segment == 0x7)) {
        return MTK_6757CH_SUPPORT;
    }
    return MTK_UNKNOWN_SUPPORT;
}

int em_jni_get_chip_id() {
    char buff[PROPERTY_VALUE_MAX] = {0}; // PROPERTY_VALUE_MAX  = 92
    int count, i, j, len, k;
    const chip_mapping *mapping;
    char chipName[EM_CHIP_NAME_LENGTH]={0};

    property_get("ro.board.platform", buff, NULL);
    ALOGD("Chip Hardware: [%s]", buff);
    len = strlen(buff);
    for (k = 0; k < len; k++) {
        if (isupper(buff[k])) {
            buff[k] = tolower(buff[k]);
        }
    }
    if ((strcmp("mt6757", buff) == 0) || (strcmp("6757", buff) == 0)) {
        return get_6757_chip_id();
    }
    count = sizeof(s_chip_mappings) / sizeof(chip_mapping);
    for (i = 0; i < count; i++) {
        mapping = s_chip_mappings + i;
        len = strlen(mapping->chip_name);
        j = 0;
        for (k = 0; k < len; k++) {
            if (mapping->chip_name[k] != '+') { /* skip '+'*/
                chipName[j++] = mapping->chip_name[k];
            }
        }
        chipName[j] = 0;
        /* skip 'mt' prefix */
        if (strstr(buff, "mt") != NULL) {
            if (strcmp(chipName, buff + 2) == 0) {
                return mapping->chip_id;
            }
        } else {
            if (strcmp(chipName, buff) == 0) {
                return mapping->chip_id;
            }
        }
    }
    return MTK_COMMON_CHIP_SUPPORT;
}


static jint support_getChip(JNIEnv *, jobject) {
    return em_jni_get_chip_id();
}

enum {
    MTK_INIERNAL_LOAD_STUB = 0,
    MTK_TC1_COMMON_SERVICE_STUB,
};

static int support_items[] = {
#ifdef  MTK_INIERNAL_LOAD
        1,
#else
        0,
#endif
#ifdef MTK_TC1_COMMON_SERVICE
    1,
#else
     0,
#endif
        999
};

static jint support_lcmOn(JNIEnv *, jobject) {
    ALOGD("Enter support_lcmOn\n");
    const sp<IBinder> display = SurfaceComposerClient::getInternalDisplayToken();
    SurfaceComposerClient::setDisplayPowerMode(display, HWC_POWER_MODE_NORMAL);
    ALOGD("Enter support_lcmOn end\n");
    return 0;
}

static jint support_lcmOff(JNIEnv *, jobject) {
    ALOGD("Enter support_lcmOff\n");
    const sp<IBinder> display = SurfaceComposerClient::getInternalDisplayToken();
    SurfaceComposerClient::setDisplayPowerMode(display, HWC_POWER_MODE_OFF);
    ALOGD("Enter support_lcmOff end\n");
    return 0;
}

static jboolean support_isFeatureSupported(
        JNIEnv *, jobject, jint feature_id) {

    if (feature_id > MTK_TC1_COMMON_SERVICE_STUB || feature_id < 0)
        return JNI_FALSE;
    else
        return support_items[feature_id] == 1 ? JNI_TRUE : JNI_FALSE;
}

static jstring support_getAudioTuningVersion(
        JNIEnv * env, jobject) {
    const char* version;
    #ifdef MTK_AUDIO_TUNING_TOOL_VERSION_V2_2
        version = "V2.2";
    #elif MTK_AUDIO_TUNING_TOOL_VERSION_V2_1
        version = "V2.1";
    #else
        version = "V1";
    #endif

    return env->NewStringUTF(version);
}

static JNINativeMethod methods[] = {
        { "getChip", "()I", (void *) support_getChip },
        { "isFeatureSupported", "(I)Z", (void *) support_isFeatureSupported },
        { "getAudioTuningVersion", "()Ljava/lang/String;", (void *) support_getAudioTuningVersion },
        { "lcmOn", "()I", (void *) support_lcmOn },
        { "lcmOff", "()I", (void *) support_lcmOff },
 };

// This function only registers the native methods
static int registerNatives(JNIEnv *env) {
    ALOGE("Register: register_chipsupport...\n");
    return AndroidRuntime::registerNativeMethods(env,
            "com/mediatek/engineermode/ChipSupport", methods, NELEM(methods));
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
