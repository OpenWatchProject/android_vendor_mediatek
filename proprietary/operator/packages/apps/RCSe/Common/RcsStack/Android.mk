# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2011. All rights reserved.
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

# Software Name : RCS IMS Stack
#
# Copyright (C) 2010 France Telecom S.A.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

###########################################################################
## Build the RCS Sdk : mtk_RCSTapi16.jar
###########################################################################

include $(CLEAR_VARS)

## This is the target being built. (Name of Jar)
LOCAL_MODULE := mtk_RCSTapi16

## Only compile source java files in this jar.
LOCAL_SRC_FILES := \
    $(call all-java-files-under, src/com/gsma)

## Add AIDL files (the parcelable must not be added in SRC_FILES, but included in LOCAL_AIDL_INCLUDES)

LOCAL_SRC_FILES += \
    src/com/gsma/services/rcs/IJoynServiceRegistrationListener.aidl\
    src/com/gsma/services/rcs/capability/ICapabilitiesListener.aidl\
    src/com/gsma/services/rcs/capability/ICapabilityService.aidl\
    src/com/gsma/services/rcs/chat/IOneToOneChat.aidl\
    src/com/gsma/services/rcs/chat/IOneToOneChatListener.aidl\
    src/com/gsma/services/rcs/chat/IExtendChat.aidl\
    src/com/gsma/services/rcs/chat/IExtendChatListener.aidl\
    src/com/gsma/services/rcs/chat/IChatMessage.aidl\
    src/com/gsma/services/rcs/chat/IChatService.aidl\
    src/com/gsma/services/rcs/chat/IGroupChatListener.aidl\
    src/com/gsma/services/rcs/chat/INewChatListener.aidl\
    src/com/gsma/services/rcs/chat/IGroupChat.aidl\
    src/com/gsma/services/rcs/chat/IGroupChatSyncingListener.aidl\
    src/com/gsma/services/rcs/chat/IChatServiceConfiguration.aidl\
    src/com/gsma/services/rcs/ft/IFileTransfer.aidl\
    src/com/gsma/services/rcs/ft/IFileTransferService.aidl\
    src/com/gsma/services/rcs/ft/IOneToOneFileTransferListener.aidl\
    src/com/gsma/services/rcs/ft/IGroupFileTransferListener.aidl\
    src/com/gsma/services/rcs/ft/INewFileTransferListener.aidl\
    src/com/gsma/services/rcs/ft/IFileTransferServiceConfiguration.aidl\
    src/com/gsma/services/rcs/contact/IContactService.aidl\
    src/com/gsma/services/rcs/ICoreServiceWrapper.aidl \
    src/com/gsma/services/rcs/extension/IMultimediaSessionServiceConfiguration.aidl\
    src/com/gsma/services/rcs/extension/IMultimediaMessagingSession.aidl\
    src/com/gsma/services/rcs/extension/IMultimediaStreamingSessionListener.aidl\
    src/com/gsma/services/rcs/extension/IMultimediaMessagingSessionListener.aidl\
    src/com/gsma/services/rcs/extension/IMultimediaStreamingSession.aidl\
    src/com/gsma/services/rcs/extension/IMultimediaSessionService.aidl\
    src/com/gsma/services/rcs/ICommonServiceConfiguration.aidl\
    src/com/gsma/services/rcs/IRcsServiceRegistrationListener.aidl\
    src/com/gsma/services/rcs/upload/IFileUpload.aidl\
    src/com/gsma/services/rcs/upload/IFileUploadListener.aidl\
    src/com/gsma/services/rcs/upload/IFileUploadService.aidl\
    src/com/gsma/services/rcs/upload/IFileUploadServiceConfiguration.aidl\
    src/com/gsma/services/rcs/history/IHistoryService.aidl\
    src/com/gsma/services/rcs/sharing/geoloc/IGeolocSharingListener.aidl\
    src/com/gsma/services/rcs/sharing/geoloc/IGeolocSharing.aidl\
    src/com/gsma/services/rcs/sharing/geoloc/IGeolocSharingService.aidl\
    src/com/gsma/services/rcs/sharing/image/IImageSharing.aidl\
    src/com/gsma/services/rcs/sharing/image/IImageSharingListener.aidl\
    src/com/gsma/services/rcs/sharing/image/IImageSharingServiceConfiguration.aidl\
    src/com/gsma/services/rcs/sharing/image/IImageSharingService.aidl\
    src/com/gsma/services/rcs/sharing/video/IVideoPlayer.aidl\
    src/com/gsma/services/rcs/sharing/video/IVideoSharing.aidl\
    src/com/gsma/services/rcs/sharing/video/IVideoSharingListener.aidl\
    src/com/gsma/services/rcs/sharing/video/IVideoSharingService.aidl\
    src/com/gsma/services/rcs/sharing/video/IVideoSharingServiceConfiguration.aidl\

##Specify install path for MTK CIP solution
ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/framework
endif

LOCAL_PROGUARD_ENABLED := disabled

## Tell it to build an Jar
include $(BUILD_JAVA_LIBRARY)

##For make files
include $(CLEAR_VARS)
#
###########################################################################
## Build the RCS Sdk : RcsStack.apk
###########################################################################
OPTR:= $(word 1,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))

ifneq ($(OPTR), OP01)

include $(CLEAR_VARS)

## This is the target being built. (Name of APK)
LOCAL_PACKAGE_NAME := RcsStack
LOCAL_MODULE_TAGS := optional
LOCAL_PRIVATE_PLATFORM_APIS := true


LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES += \
        mediatek-framework\
        ims-common\
        mtk_RCSTapi16\
        com.mediatek.settings.ext\
        mediatek-common \
        telephony-common \
        org.apache.http.legacy \
        telephony-common \
        mediatek-telephony-base \
        com.mediatek.ims.rcsua

LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.rcs-V1.0-java \
                               mtk_RCSTapi16 \
                               vendor.mediatek.hardware.radio_op-V2.0-java \
                               bouncycastle-unbundled

## Only compile source java files in this apk.
LOCAL_SRC_FILES := \
    $(call all-java-files-under, src/com/orangelabs)\
    $(call all-java-files-under, src/gov2)\
    $(call all-java-files-under, src/javax2)\
    $(call all-java-files-under, src/org)

## Add AIDL files (the parcelable must not be added in SRC_FILES, but included in LOCAL_AIDL_INCLUDES)
LOCAL_SRC_FILES += \
    src/com/orangelabs/rcs/core/ims/network/INetworkConnectivity.aidl \
    src/com/orangelabs/rcs/core/ims/network/INetworkConnectivityApi.aidl \
    src/com/orangelabs/rcs/service/api/client/terms/ITermsApi.aidl \
    src/com/gsma/services/rcs/ICoreServiceWrapper.aidl \

## FRAMEWORKS_BASE_JAVA_SRC_DIRS comes from build/core/pathmap.mk
LOCAL_AIDL_INCLUDES += $(FRAMEWORKS_BASE_JAVA_SRC_DIRS)
LOCAL_AIDL_INCLUDES += \
    com/gsma/services/rcs/capability/Capabilities.aidl \
    com/gsma/services/rcs/chat/ConferenceUser.aidl\

## Add classes used by reflection
## LOCAL_PROGUARD_FLAG_FILES := proguard.cfg
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

endif
