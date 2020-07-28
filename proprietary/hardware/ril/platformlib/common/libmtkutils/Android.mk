#
# Copyright (C) 2014 MediaTek Inc.
# Modification based on code covered by the mentioned copyright
# and/or permission notice(s).
#
# Copyright (C) 2008 The Android Open Source Project
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

commonSources:= \
    CallStack.cpp \
    JenkinsHash.cpp \
    LinearTransform.cpp \
    NativeHandle.cpp \
    Printer.cpp \
    RefBase.cpp \
    SharedBuffer.cpp \
    Static.cpp \
    StopWatch.cpp \
    String8.cpp \
    String16.cpp \
    Threads.cpp \
    Timers.cpp \
    Unicode.cpp \
    VectorImpl.cpp \
    Parcel.cpp \
    SystemClock.cpp \

host_commonCflags := -DLIBMTKUTILS_NATIVE=1 $(TOOL_CFLAGS) -Werror

# For the device, static
# =====================================================
include $(CLEAR_VARS)


# we have the common sources, plus some device-specific stuff
LOCAL_SRC_FILES:= \
    $(commonSources) \
    BlobCache.cpp \
    Looper.cpp \
    ProcessCallStack.cpp \

ifeq ($(TARGET_ARCH),mips)
LOCAL_CFLAGS += -DALIGN_DOUBLE
endif
LOCAL_CFLAGS += -Werror -fvisibility=protected

LOCAL_SHARED_LIBRARIES := \
    libmtkcutils \
    libmtkproperty \
    libmtkrillog \
    libladder

LOCAL_MODULE := libmtkutils
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_CLANG := true
LOCAL_SANITIZE := integer
LOCAL_C_INCLUDES := $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/utils
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/log
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/property
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/iop
LOCAL_C_INCLUDES += vendor/mediatek/proprietary/external/libudf/libladder

include $(BUILD_STATIC_LIBRARY)

# For the device, shared
# =====================================================
include $(CLEAR_VARS)
LOCAL_MODULE:= libmtkutils
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_WHOLE_STATIC_LIBRARIES := libmtkutils
LOCAL_SHARED_LIBRARIES := \
        libmtkcutils \
        libmtkproperty \
        libmtkrillog \
        libladder

LOCAL_CFLAGS := -Werror
LOCAL_C_INCLUDES := $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/utils
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/log
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/property
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/iop
LOCAL_C_INCLUDES += vendor/mediatek/proprietary/external/libudf/libladder

# If not declare LOCAL_MULTILIB, it will build both 32/64 bit lib.
# In c6m_rild 64bit project, all modules that use libmtkutils are
# 64bit, it is not necessary to build 32bit libmtkrilutils.
ifeq ($(MTK_RIL_MODE), c6m_1rild)
    LOCAL_MULTILIB := first
endif


LOCAL_CLANG := true
LOCAL_SANITIZE := integer

include $(BUILD_SHARED_LIBRARY)
