LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libdl \
    libbinder \
    libcutils \
    libutils \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    vendor.mediatek.hardware.nwk_opt@1.0
LOCAL_SRC_FILES := \
    nwk_opt_hal_wrap.cpp
LOCAL_MODULE := libnwk_opt_halwrap_vendor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
include $(MTK_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libdl \
    libbinder \
    libcutils \
    libutils \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    vendor.mediatek.hardware.nwk_opt@1.0
LOCAL_SRC_FILES := \
    nwk_opt_hal_wrap.cpp
LOCAL_MODULE := libnwk_opt_halwrap
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
include $(MTK_SHARED_LIBRARY)
