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

#
# Impl shared lib
#
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.nvram@1.0-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SRC_FILES := \
    Nvram.cpp

LOCAL_C_INCLUDES += \
	$(MTK_PATH_SOURCE)/external/nvram/libnvram \
        $(MTK_PATH_SOURCE)/external/nvram/nvramagentclient

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libcutils \
    libhardware \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libnvram \
    libfile_op \
    vendor.mediatek.hardware.nvram@1.0

include $(BUILD_SHARED_LIBRARY)

#
# register the hidl service
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    service.cpp

LOCAL_C_INCLUDES += \
	$(MTK_PATH_SOURCE)/external/nvram/libnvram \
        $(MTK_PATH_SOURCE)/external/nvram/nvramagentclient

ifeq ($(strip $(MTK_INTERNAL_LOG_ENABLE)),yes)
    LOCAL_CFLAGS += -DMTK_INTERNAL_LOG_ENABLE
endif

LOCAL_SHARED_LIBRARIES := \
  libdl \
  libutils \
  libcutils \
  libhardware \
  libhidlbase \
  libhidltransport \
  libbinder \
  libnvram \
  liblog \
  vendor.mediatek.hardware.nvram@1.0 \

LOCAL_MODULE:= vendor.mediatek.hardware.nvram@1.0-service
LOCAL_INIT_RC := vendor.mediatek.hardware.nvram@1.0-sevice.rc
LOCAL_MODULE_TAGS := optional
LOCAL_CPPFLAGS += -fexceptions
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
include $(MTK_EXECUTABLE)