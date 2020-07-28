# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2018. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := com.mediatek.ims.rcsua.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_RELATIVE_PATH := permissions
LOCAL_SRC_FILES := api/com.mediatek.ims.rcsua.xml

include $(BUILD_PREBUILT)

###########################################################################
## Build RCS UA service library 
###########################################################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    $(call all-java-files-under, api/src)
LOCAL_SRC_FILES += \
	$(call all-java-files-under, common/src)

## Add AIDL files (the parcelable must not be added in SRC_FILES, but included in LOCAL_AIDL_INCLUDES)
LOCAL_SRC_FILES += \
	$(call all-Iaidl-files-under, common/src)

LOCAL_STATIC_JAVA_LIBRARIES += \
	android-support-annotations
LOCAL_AIDL_INCLUDES += $(LOCAL_PATH)/common/src

LOCAL_MODULE := com.mediatek.ims.rcsua
LOCAL_MODULE_TAGS := optional
LOCAL_DEX_PREOPT := false
LOCAL_PROGUARD_ENABLED := disabled

## Tell it to build an Jar
include $(BUILD_JAVA_LIBRARY)

###########################################################################
## Build the RCS UA service implementation
###########################################################################
include $(CLEAR_VARS)

## This is the target being built. (Name of APK)
LOCAL_PACKAGE_NAME := RcsUaService
LOCAL_MODULE_TAGS := optional

## Only compile source java files in this apk.
LOCAL_SRC_FILES := \
    $(call all-java-files-under, service/src)\
    $(call all-java-files-under, common/src)

## Add AIDL files (the parcelable must not be added in SRC_FILES, but included in LOCAL_AIDL_INCLUDES)
LOCAL_SRC_FILES += \
	$(call all-Iaidl-files-under, common/src)

LOCAL_AIDL_INCLUDES += $(LOCAL_PATH)/common/src

LOCAL_MANIFEST_FILE := service/AndroidManifest.xml
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/service/res

LOCAL_JAVA_LIBRARIES += \
        bouncycastle \
        mediatek-framework\
        ims-common\
        com.mediatek.settings.ext\
        mediatek-common \
        telephony-common \
        com.mediatek.ims.rcsua

LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.rcs-V1.0-java \
                               vendor.mediatek.hardware.rcs-V2.0-java \
                               vendor.mediatek.hardware.radio_op-V2.0-java \
                               vendor.mediatek.hardware.mtkradioex-V1.0-java

LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PROGUARD_ENABLED := disabled

## Tell it to build an APK
include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))

