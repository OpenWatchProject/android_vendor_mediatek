# Copyright 2005 The Android Open Source Project
LOCAL_PATH := $(call my-dir)

########################################
# Copy prebuilt ksensor share library
########################################
include $(CLEAR_VARS)
LOCAL_MODULE := libksensor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES := lib/libksensor.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libksensor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_SRC_FILES := lib64/libksensor.so
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := 64
include $(BUILD_PREBUILT)
