# Copyright 2006 The Android Open Source Project


# ===========================
# = Configuration of rild   =
# ===========================
# SIM_ME_LOCK_MODE: 4
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  simlock11_ipc.c

LOCAL_SHARED_LIBRARIES := \
  libcutils \
  libc

LOCAL_LDLIBS:= -llog

LOCAL_MODULE := libsimlock
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

include $(MTK_SHARED_LIBRARY)