## Copyright Statement:
##
## This software/firmware and related documentation ("MediaTek Software") are
## protected under relevant copyright laws. The information contained herein
## is confidential and proprietary to MediaTek Inc. and/or its licensors.
## Without the prior written permission of MediaTek inc. and/or its licensors,
## any reproduction, modification, use or disclosure of MediaTek Software,
## and information contained herein, in whole or in part, shall be strictly prohibited.
#
## MediaTek Inc. (C) 2011. All rights reserved.
##
## BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
## THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
## RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
## AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
## EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
## MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
## NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
## SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
## SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
## THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
## THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
## CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
## SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
## STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
## CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
## AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
## OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
## MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
##
## The following software/firmware and/or related documentation ("MediaTek Software")
## have been modified by MediaTek Inc. All revisions are subject to any receiver's
## applicable license agreements with MediaTek Inc.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

## This is the target being built. (Name of APK)
LOCAL_PACKAGE_NAME := Presence
LOCAL_MODULE_TAGS := optional
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES += \
    bouncycastle \
    mediatek-framework \
    com.mediatek.settings.ext \
    org.apache.http.legacy \
    ims-common \
    mediatek-common \
    mediatek-ims-common \
    telephony-common \
    mediatek-telephony-common \
    mediatek-telephony-base \
    mediatek-ims-base \
    com.mediatek.ims.rcsua \

LOCAL_STATIC_JAVA_LIBRARIES += mtk_RCSTapi16 \
    com.mediatek.ims.rcsua \
    vendor.mediatek.hardware.radio_op-V2.0-java \
    android-support-v4 \


## Only compile source java files in this apk.
LOCAL_SRC_FILES := \
    $(call all-java-files-under, src/com/mediatek)\
    $(call all-java-files-under, src/gov2)\
    $(call all-java-files-under, src/javax2)\
    $(call all-java-files-under, src/org)

## Add AIDL files (the parcelable must not be added in SRC_FILES, but included in LOCAL_AIDL_INCLUDES)
LOCAL_SRC_FILES += \
    src/com/mediatek/presence/core/ims/network/INetworkConnectivity.aidl \
    src/com/mediatek/presence/core/ims/network/INetworkConnectivityApi.aidl

## FRAMEWORKS_BASE_JAVA_SRC_DIRS comes from build/core/pathmap.mk
LOCAL_AIDL_INCLUDES += $(FRAMEWORKS_BASE_JAVA_SRC_DIRS)

##Added for JPE begin
LOCAL_JAVASSIST_ENABLED := true
LOCAL_JAVASSIST_OPTIONS := $(LOCAL_PATH)/jpe.config
##Added for JPE end

## Add classes used by reflection
LOCAL_PROGUARD_FLAG_FILES := proguard.cfg
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PRIVILEGED_MODULE := true

##Specify install path for MTK CIP solution
ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/app
endif

## Tell it to build an APK
include $(BUILD_PACKAGE)
include $(CLEAR_VARS)

include $(BUILD_MULTI_PREBUILT)

include $(call all-makefiles-under,$(LOCAL_PATH))
