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

# HAL Shared library the for libhardware.
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := bootctrl.$(MTK_PLATFORM_DIR)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := bootctrl.cpp
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += \
    system/core/fs_mgr/include_fstab/fstab \
    device/mediatek/common/kernel-headers \
    bootable/recovery/bootloader_message/include

LOCAL_HEADER_LIBRARIES := libhardware_headers libsystem_headers
LOCAL_SHARED_LIBRARIES += liblog libbase libcutils
LOCAL_STATIC_LIBRARIES += libfstab

include $(BUILD_SHARED_LIBRARY)

# Static library for the update_engine_sideload from recovery.
include $(CLEAR_VARS)

LOCAL_MODULE := bootctrl.$(MTK_PLATFORM_DIR)
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := bootctrl.cpp
LOCAL_MODULE_TAGS := optional

LOCAL_C_INCLUDES += \
    system/core/fs_mgr/include_fstab/fstab \
    device/mediatek/common/kernel-headers \
    bootable/recovery/bootloader_message/include

LOCAL_HEADER_LIBRARIES := libhardware_headers libsystem_headers
LOCAL_SHARED_LIBRARIES += liblog libbase libcutils
LOCAL_STATIC_LIBRARIES += libfstab

include $(BUILD_STATIC_LIBRARY)
