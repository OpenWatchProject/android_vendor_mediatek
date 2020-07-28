#
# Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := fingerprint.$(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/public \
    $(LOCAL_PATH)/gf_hal/include \
    $(LOCAL_PATH)/gf_hal/public \
    $(LOCAL_PATH)/GFDevice \
    $(LOCAL_PATH)/include

LOCAL_SRC_FILES := fingerprint.cpp

LOCAL_SHARED_LIBRARIES := \
    libgf_hal \
    liblog \
    libbinder \
    libhardware \
    libutils \
    libgfdevice 

LOCAL_MODULE_TAGS := optional
include $(MTK_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
