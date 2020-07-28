# Copyright 2006 The Android Open Source Project

# XXX using libutils for simulator build only...
#

ifeq ($(MTK_RIL_MODE), c6m_1rild)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ViaHandler.cpp

LOCAL_SHARED_LIBRARIES := \
    libmtkcutils libmtkutils libmtk-ril libmtkproperty libmtkrillog libmtkrilutils

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/framework/include/core \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/framework/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/framework/include/base \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/framework/port/android/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/ \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/telcore \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/mdcomm \
    $(MTK_PATH_SOURCE)/hardware/ccci/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/property \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/log \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/utils \

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

LOCAL_CFLAGS += -DMTK_MUX_CHANNEL_64
LOCAL_CFLAGS += -DMTK_IMS_CHANNEL_SUPPORT
#used to check if support telephonyware
LOCAL_CFLAGS += -DMTK_TELEPHONYWARE_SUPPORT

#build shared library
LOCAL_CFLAGS += -DRIL_SHLIB
LOCAL_MODULE:= libvia-ril
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := first
include $(MTK_SHARED_LIBRARY)

endif
