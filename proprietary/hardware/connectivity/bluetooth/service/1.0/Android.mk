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

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE := android.hardware.bluetooth@1.0-service-mediatek
ifneq ($(strip $(MTK_GMO_ROM_OPTIMIZE)), yes)
LOCAL_INIT_RC := android.hardware.bluetooth@1.0-service-mediatek.rc
LOCAL_SRC_FILES := \
  service.cpp
else
LOCAL_INIT_RC := android.hardware.bluetooth@1.0-service-mediatek-lazy.rc
LOCAL_SRC_FILES := \
  serviceLazy.cpp
endif


LOCAL_SHARED_LIBRARIES := \
  liblog \
  libcutils \
  libdl \
  libbase \
  libutils \
  libhardware \

LOCAL_SHARED_LIBRARIES += \
  libhidlbase \
  libhidltransport \
  android.hardware.bluetooth@1.0 \

include $(BUILD_EXECUTABLE)
