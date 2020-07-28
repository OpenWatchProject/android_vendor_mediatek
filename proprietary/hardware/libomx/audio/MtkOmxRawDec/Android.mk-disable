#
# Copyright (C) 2010 The Android Open Source Project
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

ifeq ($(strip $(TARGET_BUILD_VARIANT)), eng)
LOCAL_CFLAGS += -DCONFIG_MT_ENG_BUILD
endif

LOCAL_SRC_FILES := \
    MtkOmxRawDec.cpp

LOCAL_C_INCLUDES := \
        $(TOP)/$(MTK_ROOT)/frameworks/native/include/media/openmax \
        $(TOP)/$(MTK_ROOT)/frameworks/av/media/libstagefright/include/omx_core \
        $(TOP)/$(MTK_ROOT)/hardware/omx/inc \
        $(LOCAL_PATH)/../../osal \
        $(LOCAL_PATH)/../../../omx/osal \
        $(LOCAL_PATH)/../../../omx/inc \
        $(LOCAL_PATH)/../MtkOmxAudioDecBase

LOCAL_SHARED_LIBRARIES :=       \
        libutils                \
        libcutils               \
        libdl                   \
        liblog

LOCAL_STATIC_LIBRARIES :=   \
    libMtkOmxAudioDecBase   \
    libMtkOmxOsalUtils

LOCAL_PRELINK_MODULE:= false

LOCAL_MODULE := libMtkOmxRawDec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32

include $(MTK_SHARED_LIBRARY)

