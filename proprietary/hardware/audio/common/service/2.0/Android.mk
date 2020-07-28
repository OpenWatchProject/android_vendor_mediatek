#
# Copyright (C) 2016 The Android Open Source Project
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
#LOCAL_MODULE := vendor.mediatek.hardware.audio@2.1-impl
LOCAL_MODULE := android.hardware.audio@2.0-impl-mediatek
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
    Conversions.cpp \
    Device.cpp \
    DevicesFactory.cpp \
    ParametersUtil.cpp \
    PrimaryDevice.cpp \
    Stream.cpp \
    StreamIn.cpp \
    StreamOut.cpp \
    MTKPrimaryDevice.cpp

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libcutils \
    libfmq \
    libhardware \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    android.hardware.audio@2.0 \
    android.hardware.audio.common-util \
    android.hardware.audio.common@2.0 \
    android.hardware.audio.common@2.0-util \
    vendor.mediatek.hardware.audio@2.1 \
    libmedia_helper

LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/hardware/audio/common/include

LOCAL_HEADER_LIBRARIES := \
    libaudioclient_headers \
    libaudio_system_headers \
    libhardware_headers \
    libmedia_headers \

include $(BUILD_SHARED_LIBRARY)

#
# Service
#

include $(CLEAR_VARS)
#LOCAL_MODULE := vendor.mediatek.hardware.audio@2.1-service
#LOCAL_INIT_RC := vendor.mediatek.hardware.audio@2.1-service.rc
LOCAL_MODULE := android.hardware.audio@2.0-service-mediatek
LOCAL_INIT_RC := android.hardware.audio@2.0-service-mediatek.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
    service.cpp

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libcutils \
    libhardware \
    android.hardware.audio@2.0 \
    android.hardware.audio.common@2.0 \
    android.hardware.audio.effect@2.0 \
    android.hardware.soundtrigger@2.1 \
    android.hardware.broadcastradio@1.0 \
    android.hardware.broadcastradio@1.1 \
    vendor.mediatek.hardware.audio@2.1

LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/hardware/audio/common/include

ifeq ($(strip $(AUDIOSERVER_MULTILIB)),)
LOCAL_MULTILIB := 32
else
LOCAL_MULTILIB := $(AUDIOSERVER_MULTILIB)
endif

ifneq ($(TARGET_BUILD_VARIANT), user)
LOCAL_CFLAGS += -DFORCE_DIRECTCOREDUMP
endif

include $(BUILD_EXECUTABLE)
