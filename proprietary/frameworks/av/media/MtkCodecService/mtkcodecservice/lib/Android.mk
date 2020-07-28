LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    MtkCodecService.cpp \
    CAPEWrapper.cpp

LOCAL_C_INCLUDES := \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/media/MtkCodecService/mtkcodecservice/include \
    $(TOP)/system/libhwbinder/include \
    $(TOP)/system/core/base/include \
    $(TOP)/$(MTK_PATH_SOURCE)/external/apedec \
    $(TOP)/$(MTK_PATH_SOURCE)/external/apedec/arm_32 \
    $(TOP)/$(MTK_PATH_SOURCE)/external/apedec_arm_64 \
    $(TOP)/$(MTK_PATH_SOURCE)/external/apedec/arm_32/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/external/apedec/arm_64/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks/native/include/media/openmax \
    $(TOP)/frameworks/native/include



LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    liblog \
    libutils \
    libcutils \
    libhidlmemory \
    vendor.mediatek.hardware.mtkcodecservice@1.1 \
    android.hidl.allocator@1.0 \
    android.hidl.memory@1.0 \
    libaudioutils


LOCAL_STATIC_LIBRARIES := \
    libapedec_mtk


LOCAL_MODULE := vendor.mediatek.hardware.mtkcodecservice@1.1-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw

include $(MTK_SHARED_LIBRARY)
