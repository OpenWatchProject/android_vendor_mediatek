#
# Copyright (C) 2008 The Android Open Source Project
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
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := ctm
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_IS_RUNTIME_RESOURCE_OVERLAY:=true
LOCAL_MODULE_PATH := $(TARGET_OUT)/priv-app/MtkTeleService

LOCAL_CERTIFICATE := platform
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_JAVA_LIBRARIES += mediatek-framework \
                        ims-common \
                        mediatek-telephony-common \
                        mediatek-telephony-base \
                        telephony-common \
                        mediatek-ims-common

LOCAL_STATIC_JAVA_LIBRARIES += libdmyksdk
LOCAL_STATIC_JAVA_LIBRARIES += libdmykshttpsdk
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v4
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v7-appcompat
LOCAL_AAPT_FLAGS += --extra-packages android.support.v7.appcompat

LOCAL_AAPT_FLAGS += --package-id 0x88

ifeq ($(TARGET_ARCH),arm64)
LOCAL_PREBUILT_JNI_LIBS += ../../../prebuilts/3rd-party/CTMLib/ctm_v2/jni_libs/arm64-v8a/libaes-jni.so
LOCAL_MULTILIB := 64
else
LOCAL_PREBUILT_JNI_LIBS := ../../../prebuilts/3rd-party/CTMLib/ctm_v2/jni_libs/armeabi-v7a/libaes-jni.so
LOCAL_MULTILIB := 32
endif



LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

include $(CLEAR_VARS)
ifeq ($(TARGET_BUILD_VARIANT), eng)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libdmyksdk:../../../prebuilts/3rd-party/CTMLib/ctm_v2/englibs/dm_sdk.jar
else
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libdmyksdk:../../../prebuilts/3rd-party/CTMLib/ctm_v2/libs/dm_sdk.jar
endif

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := libdmykshttpsdk:../../../prebuilts/3rd-party/CTMLib/ctm_v2/englibs/android-async-http.jar
include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)

include $(call all-makefiles-under, $(LOCAL_PATH))

