# Copyright (C) 2011 The Android Open Source Project
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

# Only trying to build it in case of telephony add on is supported
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/src/java frameworks/opt/telephony/src/java
LOCAL_SRC_FILES := $(call all-java-files-under, src/java) \
    src/java/com/mediatek/mms/appserviceproxy/IAppServiceProxy.aidl \
    src/java/com/mediatek/mms/appcallback/IMmsCallbackService.aidl

#LOCAL_AIDL_INCLUDES := src/java/com/mediatek/mms/appcallback/SmsEntry.aidl
LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/src/java


#LOCAL_STATIC_JAVA_LIBRARIES := vendor.mediatek.hardware.radio-V2.0-java-static \
#    vendor.mediatek.hardware.radio.deprecated-V1.1-java-static

res_dirs := res \
    $(phone_common_dir)/res \
    $(contacts_common_dir)/res

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform
LOCAL_MODULE_TAGS := optional
LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_JAVA_LIBRARIES := telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-common
LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += mediatek-telephony-base
LOCAL_JAVA_LIBRARIES += mediatek-telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-telecom-common
LOCAL_JAVA_LIBRARIES += ims-common
LOCAL_JAVA_LIBRARIES += mediatek-ims-common

LOCAL_STATIC_JAVA_LIBRARIES += android-support-v4 \
                               com.android.browser.provider \
                               com.mediatek.mmsappservice.ext

#LOCAL_JAVA_LIBRARIES += mediatek-telephony-common mediatek-framework

LOCAL_PRIVILEGED_MODULE := true
LOCAL_PACKAGE_NAME := MtkMmsAppService

include $(BUILD_PACKAGE)
include $(call all-makefiles-under,$(LOCAL_PATH))

endif
