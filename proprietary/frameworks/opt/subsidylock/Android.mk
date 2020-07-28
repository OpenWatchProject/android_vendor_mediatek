# Copyright 2006 The Android Open Source Project


# ===========================
# = Configuration of rild   =
# ===========================
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  subsidylock_ipc.c \
  subsidylock_adaptation.cpp \

LOCAL_SHARED_LIBRARIES := \
  libcutils \
  liblog \
  libutils \
  libc \
  android.hardware.radio@1.0 \
  vendor.mediatek.hardware.mtkradioex@1.0 \
  libhidlbase \
  libhidltransport \
  libhwbinder

LOCAL_LDLIBS:= -llog

LOCAL_CFLAGS := -DRIL_SHLIB

LOCAL_MODULE := lib_remote_simlock
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

include $(MTK_SHARED_LIBRARY)