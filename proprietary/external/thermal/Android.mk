# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= thermal
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#bobule workaround pdk build error, needing review
LOCAL_MULTILIB := first

LOCAL_MODULE_TAGS:= optional

LOCAL_SRC_FILES:= \
    thermal.c

LOCAL_C_INCLUDES = \
 $(LOCAL_PATH)/ \
 $(TOPDIR)hardware/libhardware_legacy/include \
 $(TOPDIR)hardware/libhardware/include \
 $(MTK_PATH_SOURCE)/hardware/ccci/include \
 $(MTK_PATH_SOURCE)/system/netdagent/include


LOCAL_INIT_RC := init.thermal.rc
LOCAL_SHARED_LIBRARIES := libcutils libc libifcutils_mtk libdl liblog

include $(MTK_EXECUTABLE)
