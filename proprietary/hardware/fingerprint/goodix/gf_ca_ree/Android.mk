#
# Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

LOCAL_PATH := $(call my-dir)

#auto modify it in script(release_for_xxxx.sh)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libgf_ca_ree
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

ifeq ($(TARGET_BUILD_VARIANT),eng)
		LOCAL_SRC_FILES_64 := debug/arm64-v8a/libgf_ca_ree.so
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
	LOCAL_SRC_FILES_64 := userdebug/arm64-v8a/libgf_ca_ree.so
else
	LOCAL_SRC_FILES_64 := release/arm64-v8a/libgf_ca_ree.so
endif

LOCAL_SHARED_LIBRARIES :=  libgf_ta_ree

LOCAL_MULTILIB := 64
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
include $(BUILD_PREBUILT)

