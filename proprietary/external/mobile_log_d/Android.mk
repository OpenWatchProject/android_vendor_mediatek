# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	logging.c \
	mobilelog.c \
	daemon.c \
	config.c \
	libfunc.c \
	size_control.c \
	debug_config.c \
	dump.c \
	mlog.c \
	bootmode.c \
	LogTransfer.c

LOCAL_SHARED_LIBRARIES := libcutils libutils libdl
LOCAL_STATIC_LIBRARIES := liblog

LOCAL_CFLAGS += -DMBLOG_DEBUG
LOCAL_CFLAGS += -DPLATFORM_VERSION=$(PLATFORM_VERSION)

LOCAL_C_INCLUDES = $(MTK_ROOT)/external/aee/binary/inc

LOCAL_MODULE:= mobile_log_d
LOCAL_INIT_RC := mobile_log_d.rc
# LOCAL_PROPRIETARY_MODULE := true
# LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := first

include $(MTK_EXECUTABLE)
