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

ifeq ($(MSSI_MTK_IMS_SUPPORT),yes)
build_gba := true
endif

ifeq ($(MSSI_MTK_RCS_SUPPORT),yes)
build_gba := true
endif

ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
build_gba := true
endif

# No need to build GBA if no telephony add on
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 1)
build_gba := false
endif

# No need to build XCAP on tc1
ifeq ($(strip $(MSSI_MTK_TC1_COMMON_SERVICE)), yes)
build_xcap := false
endif

ifeq ($(build_gba),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

ifneq ($(TARGET_DEVICE), gobo)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/../../../../../../device/mediatek/config/ImsSSConfig/GbaResource/res
else
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/../../../../../../device/huawei/gobo/config/ImsSSConfig/GbaResource/res
endif

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := Gba
LOCAL_PROPRIETARY_MODULE := false
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-ims-base
LOCAL_JAVA_LIBRARIES += mediatek-telephony-base
LOCAL_JAVA_LIBRARIES += mediatek-telephony-common

include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))

endif
