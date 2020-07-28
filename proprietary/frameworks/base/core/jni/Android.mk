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
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# App-based AAL @{
LOCAL_SHARED_LIBRARIES += \
    libutils \
    libhidlbase \
    vendor.mediatek.hardware.pq@2.3


# @}

LOCAL_SHARED_LIBRARIES += libcutils libnativehelper
LOCAL_LDLIBS := -llog
LOCAL_SRC_FILES := \
  com_mediatek_powerhalwrapper_PowerHalWrapper.cpp \
  com_mediatek_amsAal_AalUtils.cpp
LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \
    frameworks/base/core/jni \
    $(LOCAL_PATH)/inc
LOCAL_MODULE := libpowerhalwrap_jni
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
include $(MTK_SHARED_LIBRARY)
