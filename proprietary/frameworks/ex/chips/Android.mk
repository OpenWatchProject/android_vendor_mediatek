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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android-common-chips
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v4
#LOCAL_SDK_VERSION := 14
#MTK_EX_CHIPS_JAVA_PATH := ../../../mediatek/frameworks-ext/ex/chips/src
LOCAL_SRC_FILES := \
     $(call all-java-files-under, src) \
     $(call all-logtags-files-under, src) 
     #$(call all-java-files-under,$(MTK_EX_CHIPS_JAVA_PATH))
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res
LOCAL_PROGUARD_ENABLED := disabled
# disable proguard because this module is jar for App to use

#LOCAL_PROGUARD_FLAG_FILES := proguard.flags
#LOCAL_PROGUARD_SOURCE := javaclassfile
#LOCAL_EXCLUDED_JAVA_CLASSES := com/android/mtkex/chips/*.class
#LOCAL_EXCLUDED_JAVA_CLASSES += com/android/mtkex/chips/recipientchip/*.class
include $(BUILD_STATIC_JAVA_LIBRARY)

##################################################
# Build all sub-directories

include $(call all-makefiles-under,$(LOCAL_PATH))
