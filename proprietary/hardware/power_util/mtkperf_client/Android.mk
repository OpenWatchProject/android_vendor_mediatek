
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := mtkperf_lock.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libhidlbase \
    libutils \
    vendor.mediatek.hardware.mtkpower@1.0

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/power/config/common/intf_types

LOCAL_MODULE := libmtkperf_client_vendor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := mtkperf_lock.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libhidlbase \
    libutils \
    vendor.mediatek.hardware.mtkpower@1.0

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/power/config/common/intf_types

LOCAL_MODULE := libmtkperf_client
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)

