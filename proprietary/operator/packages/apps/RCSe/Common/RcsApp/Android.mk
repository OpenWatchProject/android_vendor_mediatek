# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

##########################################################################
# Build the RCS Application : Rcse.apk
##########################################################################

OPTR:= $(word 1,$(subst _,$(space),$(OPTR_SPEC_SEG_DEF)))

ifneq ($(OPTR), OP01)

include $(CLEAR_VARS)

# This is the target being built. (Name of APK)
LOCAL_PACKAGE_NAME := Rcse
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
#LOCAL_INSTRUMENTATION_FOR := Dialer
LOCAL_APK_LIBRARIES := MtkContacts \
  RcsStack
LOCAL_JAVA_LIBRARIES += \
  com.mediatek.dialer.ext.vendor \
  com.mediatek.incallui.ext.vendor \
  com.mediatek.mms.ext \
  mediatek-telephony-base \
  mediatek-framework \
  mediatek-common

LOCAL_STATIC_JAVA_LIBRARIES += com.android.vcard
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v4
LOCAL_STATIC_JAVA_LIBRARIES += mtk_RCSTapi16
LOCAL_JAVA_LIBRARIES += telephony-common

# Only compile source java files in this apk.
LOCAL_SRC_FILES := \
    $(call all-java-files-under, src/com/mediatek)\
    $(call all-java-files-under, src/com/android)

# Add AIDL files (the parcelable must not be added in SRC_FILES, but included in LOCAL_AIDL_INCLUDES)
LOCAL_SRC_FILES += \
    src/com/mediatek/rcse/api/terms/ITermsApi.aidl \
    src/com/mediatek/rcse/api/IRegistrationStatusRemoteListener.aidl \
    src/com/mediatek/rcse/service/IRegistrationStatus.aidl \
    src/com/mediatek/rcse/api/ICapabilityRemoteListener.aidl \
    src/com/mediatek/rcse/service/IFlightMode.aidl \
    src/com/android/server/power/IPreShutdown.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteChatWindow.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteChatWindowManager.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteFileTransfer.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteGroupChatWindow.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteMultiChatWindow.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteOne2OneChatWindow.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteReceivedChatMessage.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteSentChatMessage.aidl \
    src/com/mediatek/rcse/service/IApiServiceWrapper.aidl \
    src/com/mediatek/rcse/plugin/message/IRemoteWindowBinder.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteChatWindowMessage.aidl \
    src/com/mediatek/rcse/service/binder/IRemoteBlockingRequest.aidl \
    src/com/mediatek/rcse/api/INetworkConnectivity.aidl \
    src/com/mediatek/rcse/api/INetworkConnectivityApi.aidl


LOCAL_AIDL_INCLUDES += \
    com/gsma/services/rcs/capability/Capabilities.aidl \
    com/mediatek/rcse/mvc/ChatMessage.aidl \
    

# Add classes used by reflection
#LOCAL_PROGUARD_FLAG_FILES := proguard.cfg
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_EMMA_COVERAGE_FILTER := +com.mediatek.*
# Exclude the java files generaged by aidl files
# Notice that the filter exclude method, so use * to cover all methods
LOCAL_EMMA_COVERAGE_FILTER += -com.mediatek.rcse.service.binder.IRemoteChatWindow* \
                              -com.mediatek.rcse.service.binder.IRemoteChatWindowManager* \
                              -com.mediatek.rcse.service.binder.IRemoteChatWindowMessage* \
                              -com.mediatek.rcse.service.binder.IRemoteFileTransfer* \
                              -com.mediatek.rcse.service.binder.IRemoteGroupChatWindow* \
                              -com.mediatek.rcse.service.binder.IRemoteMultiChatWindow* \
                              -com.mediatek.rcse.service.binder.IRemoteOne2OneChatWindow* \
                              -com.mediatek.rcse.service.binder.IRemoteReceivedChatMessage* \
                              -com.mediatek.rcse.service.binder.IRemoteSentChatMessage* \
                              -com.mediatek.rcse.plugin.message.IRemoteWindowBinder* \
                              -com.mediatek.rcse.service.binder.IRemoteBlockingRequest* \
                              -com.mediatek.rcse.api.IRegistrationStatusRemoteListener* \
                              -com.mediatek.rcse.api.ICapabilityRemoteListener* \
                              -com.mediatek.rcse.service.IRegistrationStatus* \
                              -com.mediatek.rcse.service.IFlightMode* \
                              -com.mediatek.rcse.service.IApiServiceWrapper* \
                              -com.mediatek.rcse.plugin.apn.IRcseOnlyApnStatus*
# Exclude the debug java files
LOCAL_EMMA_COVERAGE_FILTER += -com.mediatek.rcse.settings.PhoneNumberToAccountSettings* \
                              -com.mediatek.rcse.settings.ProvisionProfileSettings* \
                              -com.mediatek.rcse.activities.ConfigMessageActicity* \

#Specify install path for MTK CIP solution
ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/app
else
LOCAL_MODULE_PATH := $(TARGET_OUT)/app
endif

# Tell it to build an APK
include $(BUILD_PACKAGE)

# Include plug-in's makefile to automated generate .mpinfo
#include vendor/mediatek/proprietary/frameworks/opt/plugin/mplugin.mk

include $(CLEAR_VARS)
#For auto test

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
