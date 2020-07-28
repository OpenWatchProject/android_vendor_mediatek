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


LOCAL_SRC_FILES := \
    MtkOmxApeDec.cpp \
    BpMtkCodecService.cpp

LOCAL_C_INCLUDES:= \
        $(TOP)/$(MTK_ROOT)/frameworks/av/media/libstagefright/include/omx_core \
        $(TOP)/$(MTK_ROOT)/hardware/omx/osal \
        $(TOP)/frameworks/native/include \
        $(TOP)/$(MTK_ROOT)/hardware/omx/inc \
        $(LOCAL_PATH)/../../osal \
        $(LOCAL_PATH)/../MtkOmxAudioDecBase \
        $(LOCAL_PATH)/../MtkBpCodec \
        $(TOP)/frameworks/av/media/libstagefright/omx \
        $(TOP)/frameworks/native/include/media/hardware \
        $(TOP)/$(MTK_ROOT)/frameworks/native/include/media/openmax \
        $(TOP)/$(MTK_ROOT)/frameworks/av/media/libstagefright/include \
        $(TOP)/frameworks/av/media/libstagefright/include \
        $(TOP)/$(MTK_ROOT)/frameworks-ext/native/include \
        $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcodecservice/include \
        $(TOP)/system/libhwbinder/include \
        $(TOP)/system/core/base/include


LOCAL_SHARED_LIBRARIES :=       \
        libutils                \
        libcutils               \
        libdl                   \
        libui			  \
	    liblog \
        libhidlbase \
        libhidltransport \
        libhwbinder \
        libhidlmemory \
        vendor.mediatek.hardware.mtkcodecservice@1.1 \
        android.hidl.allocator@1.0 \
        android.hidl.memory@1.0


LOCAL_STATIC_LIBRARIES :=	\
	libMtkOmxAudioDecBase	\
	libMtkOmxOsalUtils
	

LOCAL_CFLAGS := -DOSCL_IMPORT_REF= -DOSCL_EXPORT_REF= -DOSCL_UNUSED_ARG=
  
LOCAL_PRELINK_MODULE:= false
LOCAL_MODULE := libMtkOmxApeDec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32

include $(MTK_SHARED_LIBRARY)

