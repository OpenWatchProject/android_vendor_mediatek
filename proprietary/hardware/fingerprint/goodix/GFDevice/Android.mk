#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#
# Copyright (C) 2013 The Android Open Source Project
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
LOCAL_MODULE := libgfdevice
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk


LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../public \
    $(LOCAL_PATH)/../gf_hal/public \
    $(LOCAL_PATH)/../gf_hal/include \
    $(LOCAL_PATH)/../include

LOCAL_SRC_FILES := \
    GFDevice.cpp \
    GoodixFingerprintBase.cpp \
    GoodixFingerprintExt.cpp \
    GoodixFingerprintTest.cpp

LOCAL_SHARED_LIBRARIES := \
    libgf_hal \
    liblog

LOCAL_HEADER_LIBRARIES := \
    libcutils_headers \
    libutils_headers\
    libhardware_headers

LOCAL_MODULE_TAGS := optional
include $(MTK_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))

