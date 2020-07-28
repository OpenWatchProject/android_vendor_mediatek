# Copyright 2006 The Android Open Source Project
###############################################################################
LOCAL_PATH:= $(call my-dir)
###############################################################################
include $(CLEAR_VARS)
LOCAL_SRC_FILES := libbiosensor.c
LOCAL_MODULE := libbiosensor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES += libnvram liblog libfile_op
#bobule workaround pdk build error, needing review
#LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)
LOCAL_C_INCLUDES:= \
	$(MTK_PATH_SOURCE)/external/nvram/libnvram \
	$(MTK_PATH_SOURCE)/external/nvram/libfile_op \
  $(MTK_PATH_SOURCE)/external/biosensord/libbiosensor/include

LOCAL_EXPORT_C_INCLUDE_DIRS := $(MTK_PATH_SOURCE)/external/biosensord/libbiosensor/include
LOCAL_PRELINK_MODULE := false
include $(MTK_SHARED_LIBRARY)
###############################################################################
