#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libgf_ca
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

ifeq ($(TARGET_BUILD_VARIANT),eng)
LOCAL_SRC_FILES_64 := debug/arm64-v8a/libgf_ca.so
LOCAL_SRC_FILES_32 := debug/armeabi-v7a/libgf_ca.so
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
LOCAL_SRC_FILES_64 := userdebug/arm64-v8a/libgf_ca.so
LOCAL_SRC_FILES_32 := userdebug/armeabi-v7a/libgf_ca.so
else
LOCAL_SRC_FILES_64 := release/arm64-v8a/libgf_ca.so
LOCAL_SRC_FILES_32 := release/armeabi-v7a/libgf_ca.so
endif

LOCAL_SHARED_LIBRARIES := \
    libcutils

LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
include $(BUILD_PREBUILT)

