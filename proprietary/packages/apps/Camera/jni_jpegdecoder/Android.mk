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
#

# This makefile supplies the rules for building a library of JNI code for
# use by our example of how to bundle a shared library with an APK.

ifeq ($(MTK_CAM_NATIVE_PIP_SUPPORT), yes)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := samples

# This is the target being built.
LOCAL_MODULE:= libjni_jpegdecoder

LOCAL_MULTILIB := both

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/imagecodec/inc \

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/system/core/libion/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external

# All of the source files that we will compile.
LOCAL_SRC_FILES:= \
    jpegdecoder_jni.cpp \
    DebugHelper.cpp \

# All of the shared libraries we link against.
LOCAL_SHARED_LIBRARIES += \
    libandroid_runtime \
    libcutils \
    libutils \
    libui \
    libgui \
    libion\
    libion_mtk\
    libgralloc_extra\
    liblog\

LOCAL_SHARED_LIBRARIES += libmhalImageCodec

# No static libraries.
LOCAL_STATIC_LIBRARIES :=


# No special compiler flags.
LOCAL_CFLAGS +=

include $(BUILD_SHARED_LIBRARY)
endif
