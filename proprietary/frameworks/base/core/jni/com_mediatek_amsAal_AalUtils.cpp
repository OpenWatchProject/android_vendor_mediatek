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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

#include "jni.h"
#include <nativehelper/JNIHelp.h>
#include <log/log.h>
#include <sys/system_properties.h>
#include <vendor/mediatek/hardware/pq/2.3/IPictureQuality.h>

using vendor::mediatek::hardware::pq::V2_3::IPictureQuality;
using vendor::mediatek::hardware::pq::V2_0::Result;

#ifdef MTK_SYSTEM_AAL
#include "AALClient.h"
#endif

namespace android {

static int gAalSupport = -1;

bool AalIsSupport(void)
{
    char value[PROP_VALUE_MAX];

    if (gAalSupport < 0) {
        if (__system_property_get("ro.vendor.mtk_aal_support", value) > 0) {
            gAalSupport = (int)strtoul(value, NULL, 0);
        }
    }

    return (gAalSupport > 0) ? true : false;
}

void com_mediatek_amsAal_AalUtils_setSmartBacklightStrength(JNIEnv* /*env*/, jobject /*thiz*/, jint level) {
    if (AalIsSupport() == false) {
        ALOGE("[AMSAAL]com_mediatek_amsAal_AalUtils_setSmartBacklightStrength(): MTK_AAL_SUPPORT disabled");
        return;
    }

#ifdef MTK_SYSTEM_AAL
    AALClient::getInstance().setSmartBacklightStrength(level);
#else
    sp<IPictureQuality> service = IPictureQuality::tryGetService();

    if (service == nullptr) {
        ALOGE("[AMSAAL] failed to get HW service");
        return;
    }

    android::hardware::Return<Result> ret = service->setSmartBacklightStrength(level);
    if (!ret.isOk() || ret != Result::OK) {
        ALOGE("[AMSAAL] IPictureQuality::setSmartBacklightStrength failed!");
    }
#endif
}

int register_com_mediatek_amsAal_AalUtils(JNIEnv *env) {
    const JNINativeMethod methods[] = {
        { "setSmartBacklightStrength", "(I)V", (void *)com_mediatek_amsAal_AalUtils_setSmartBacklightStrength },
    };

    jclass clazz = env->FindClass("com/mediatek/amsAal/AalUtils");
    if (env->RegisterNatives(clazz, methods, NELEM(methods)) < 0) {
        return JNI_ERR;
    }

    return JNI_OK;
}

}  // namespace android

