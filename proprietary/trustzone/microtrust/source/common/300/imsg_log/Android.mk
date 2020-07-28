# Copyright (c) 2015-2016 MICROTRUST Incorporated
# All rights reserved
#
# This file and software is confidential and proprietary to MICROTRUST Inc.
# Unauthorized copying of this file and software is strictly prohibited.
# You MUST NOT disclose this file and software unless you get a license
# agreement from MICROTRUST Incorporated.

ifeq ($(MICROTRUST_TEE_SUPPORT),yes)

LOCAL_PATH := $(call my-dir)

#-----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_MODULE := libimsg_log
LOCAL_SRC_FILES := imsg_log.c
LOCAL_LDFLAGS := -llog
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS += -Werror
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)
#-----------------------------------------------------

endif
