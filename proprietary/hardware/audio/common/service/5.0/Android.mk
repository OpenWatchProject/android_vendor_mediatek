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
LOCAL_MODULE := android.hardware.audio@5.0-impl-mediatek
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

LOCAL_CFLAGS += -DMAJOR_VERSION=5
LOCAL_CFLAGS += -DMINOR_VERSION=0
LOCAL_CFLAGS += -include VersionMacro.h

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libcutils \
    libfmq \
    libhardware \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    android.hardware.audio@5.0 \
    android.hardware.audio.common-util \
    android.hardware.audio.common@5.0 \
    android.hardware.audio.common@5.0-util \
    vendor.mediatek.hardware.audio@5.1 \
    libmedia_helper

LOCAL_CFLAGS += -DPLATFORM_VERSION=$(PLATFORM_VERSION)

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
LOCAL_MODULE := android.hardware.audio@5.0-service-mediatek
LOCAL_INIT_RC := android.hardware.audio@5.0-service-mediatek.rc
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES := \
    service.cpp

LOCAL_CFLAGS := -Wall -Werror
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libbinder \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libhardware \
    libhwbinder \
    android.hardware.audio@5.0 \
    android.hardware.audio.common@5.0 \
    android.hardware.audio.effect@5.0 \
    android.hardware.bluetooth.a2dp@1.0 \
    android.hardware.bluetooth.audio@2.0 \
    android.hardware.soundtrigger@2.2 \
    android.hardware.broadcastradio@1.0 \
    android.hardware.broadcastradio@1.1 \
    vendor.mediatek.hardware.audio@5.1

LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/hardware/audio/common/include

# Can not switch to Android.bp until AUDIOSERVER_MULTILIB
# is deprecated as build config variable are not supported
ifeq ($(strip $(AUDIOSERVER_MULTILIB)),)
LOCAL_MULTILIB := 32
else
LOCAL_MULTILIB := $(AUDIOSERVER_MULTILIB)
endif

ifneq ($(TARGET_BUILD_VARIANT), user)
LOCAL_CFLAGS += -DFORCE_DIRECTCOREDUMP
endif

include $(BUILD_EXECUTABLE)

#
# BluetoothAudioOffload
#

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.bluetooth.a2dp@1.0-impl-mediatek
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
    BluetoothAudioOffload.cpp

LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/hardware/audio/common/include \
    $(MTK_PATH_SOURCE)/hardware/audio/common/V3/include \
    $(MTK_PATH_CUSTOM)/custom \
    $(MTK_PATH_CUSTOM)/custom/audio \
    $(MTK_PATH_CUSTOM)/cgen/inc \
    $(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
    $(MTK_PATH_CUSTOM)/cgen/cfgdefault \
    $(MTK_PATH_CUSTOM)/../common/cgen/inc \
    $(MTK_PATH_CUSTOM)/../common/cgen/cfgfileinc \
    $(MTK_PATH_CUSTOM)/../common/cgen/cfgdefault \
    $(MTK_PATH_CUSTOM)/hal/audioflinger/audio \
    $(MTK_PATH_SOURCE)/hardware/audio/common/utility \
    $(TOPDIR)external/tinyxml \
    $(TOPDIR)external/tinyalsa/include  \
    $(TOPDIR)external/tinycompress/include \
    $(MTK_PATH_SOURCE)/hardware/ccci/include \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
    $(MTK_PATH_SOURCE)/external/AudioComponentEngine \
    $(MTK_PATH_SOURCE)/external/audiocustparam \
    $(MTK_PATH_SOURCE)/external/AudioSpeechEnhancement/V3/inc \


LOCAL_SHARED_LIBRARIES := \
    libbase \
    libcutils \
    libeffects \
    libfmq \
    libhidlbase \
    libhidlmemory \
    libhidltransport \
    liblog \
    libutils \
    android.hardware.bluetooth.a2dp@1.0 \
    android.hidl.memory@1.0 \
  # audio.primary.$(TARGET_BOARD_PLATFORM)\

LOCAL_HEADER_LIBRARIES := \
    libaudio_system_headers \
    libaudioclient_headers \
    libeffects_headers \
    libhardware_headers \
    libmedia_headers \

include $(BUILD_SHARED_LIBRARY)
