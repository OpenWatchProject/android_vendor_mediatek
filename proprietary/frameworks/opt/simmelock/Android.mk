# Copyright 2006 The Android Open Source Project


# ===========================
# = Configuration of rild   =
# ===========================
# SIM_ME_LOCK_MODE: 1
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  simmelock_ipc.c

LOCAL_SHARED_LIBRARIES := \
  libcutils \
  libc

LOCAL_LDLIBS:= -llog

LOCAL_CFLAGS := -DRIL_SHLIB

LOCAL_MODULE := libsimmelock
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

include $(MTK_SHARED_LIBRARY)

# SMB Design: 11 => ATT, 12 => TMO, 13 => RJIO
include $(call all-makefiles-under,$(LOCAL_PATH))