# Copyright (C) 2014 The Android Open Source Project
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

###
### netd daemon.
###
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
        $(MTK_PATH_SOURCE)/hardware/perfservice/perfservicenative  \
        system/libhidl/base/include   \
        system/libhidl/transport/include \
        vendor/mediatek/proprietary/system/netdagent/include

LOCAL_CLANG := true
# LOCAL_CPPFLAGS := -std=c++11 -Wall
LOCAL_MODULE := netdagent

# Bug: http://b/29823425 Disable -Wvarargs for Clang update to r271374
LOCAL_CPPFLAGS +=  -Wno-varargs

ifeq ($(TARGET_ARCH), x86)
ifneq ($(TARGET_PRODUCT), gce_x86_phone)
        LOCAL_CPPFLAGS += -D NETLINK_COMPAT32
endif
endif

ifneq ($(filter userdebug eng,$(TARGET_BUILD_VARIANT)),)
    LOCAL_CPPFLAGS += -DMTK_DEBUG
endif

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        liblog \
        libforkexecwrap \
        libutils \
        libhwbinder  \
        libhidlbase \
        libhidltransport  \
        libifcutils_mtk \
        vendor.mediatek.hardware.netdagent@1.0

LOCAL_SRC_FILES := \
        CommandListener.cpp \
        CommandService.cpp  \
        CommandController.cpp \
        CommandRespondor.cpp \
        CommandDispatch.cpp \
        FirewallController.cpp \
        ThrottleController.cpp  \
        NetworkController.cpp   \
        IptablesInterface.cpp \
        NetlinkCommands.cpp  \
        NetdagentUtils.cpp    \
        main.cpp

LOCAL_INIT_RC := netdagent.rc

LOCAL_PROPRIETARY_MODULE := true

LOCAL_MODULE_OWNER := mtk

include $(MTK_EXECUTABLE)

