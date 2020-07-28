# Copyright 2006 The Android Open Source Project

ifeq ($(MTK_RIL_MODE), c6m_1rild)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
  rild.c


LOCAL_SHARED_LIBRARIES := \
  libmtkrillog \
  libmtkproperty \
  libmtkcutils \
  librilfusion \
  libmtkrilutils \
  libsysenv \
  libmtkconfigutils

LOCAL_CFLAGS := -DRIL_SHLIB

# remove the below on the platform that not support sysenv
LOCAL_CFLAGS += -DMTK_SYSENV_SUPPORT
LOCAL_SHARED_LIBRARIES += liblog

ifneq ($(MTK_NUM_MODEM_PROTOCOL),1)
    LOCAL_CFLAGS += -DANDROID_MULTI_SIM
endif

ifeq ($(MTK_NUM_MODEM_PROTOCOL), 2)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

ifeq ($(MTK_NUM_MODEM_PROTOCOL), 3)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_3
endif

ifeq ($(MTK_NUM_MODEM_PROTOCOL), 4)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_4
endif

ifeq ("$(wildcard vendor/mediatek/internal/mtkrild_enable)","")
    LOCAL_CFLAGS += -D__PRODUCTION_RELEASE__
endif

LOCAL_CFLAGS += -DMTK_MUX_CHANNEL_64
LOCAL_CFLAGS += -DMTK_IMS_CHANNEL_SUPPORT
LOCAL_CFLAGS += -DMTK_USE_HIDL
#used to check if support telephonyware
LOCAL_CFLAGS += -DMTK_TELEPHONYWARE_SUPPORT

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/librilfusion \
    $(MTK_PATH_SOURCE)/external/libsysenv \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/log \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/property \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/utils \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/config \

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include

LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE:= mtkfusionrild
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := mtkrild.rc

LOCAL_MULTILIB := first

include $(MTK_EXECUTABLE)
endif
