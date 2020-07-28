# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2012. All rights reserved.
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
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := verizon.net.sip
LOCAL_SRC_FILES := $(call all-java-files-under, src/verizon)
LOCAL_CERTIFICATE := platform

#LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_JAVA_LIBRARIES := ims-common telephony-common framework
LOCAL_JAVA_LIBRARIES += mediatek-common mediatek-framework mediatek-telephony-base mediatek-ims-common
LOCAL_JAVA_LIBRARIES += mediatek-telecom-common voip-common mediatek-ims-base
LOCAL_STATIC_JAVA_LIBRARIES += mtk_RCSTapi16
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.clientapi-V1.0-java
LOCAL_APK_LIBRARIES += ImsService

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_MULTILIB := both

#ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
#LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/operator/libs
#else
#LOCAL_MODULE_PATH := $(TARGET_OUT)/operator/libs
#endif

include $(BUILD_JAVA_LIBRARY)
#########################################################

include $(CLEAR_VARS)
LOCAL_MODULE := verizon.net.sip.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

#########################################################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_PRIVILEGED_MODULE := true

LOCAL_MODULE := VerizonTestApk

LOCAL_CERTIFICATE := platform

LOCAL_SRC_FILES := src/1014test.apk

LOCAL_MODULE_CLASS := APPS

LOCAL_MODULE_PATH := $(TARGET_OUT)/app

LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)

include $(BUILD_PREBUILT)

#########################################################
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := OP12Ims
#LOCAL_SDK_VERSION := system_current
LOCAL_SRC_FILES := $(call all-java-files-under, src/com)
LOCAL_CERTIFICATE := platform

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MODULE_OWNER := mtk
LOCAL_JAVA_LIBRARIES := ims-common telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-common mediatek-framework mediatek-telephony-base mediatek-ims-common
LOCAL_JAVA_LIBRARIES += mediatek-ims-base
LOCAL_JAVA_LIBRARIES += mediatek-telecom-common
LOCAL_STATIC_JAVA_LIBRARIES += wfo-common

LOCAL_APK_LIBRARIES += ImsService

LOCAL_PROGUARD_ENABLED := disabled

ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/app
else
LOCAL_MODULE_PATH := $(TARGET_OUT)/app
endif

include $(BUILD_PACKAGE)

