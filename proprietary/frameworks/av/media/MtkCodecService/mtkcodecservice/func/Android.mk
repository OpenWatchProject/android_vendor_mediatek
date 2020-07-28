LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    service.cpp

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libhardware \
    vendor.mediatek.hardware.mtkcodecservice@1.1 \
    

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/media/MtkCodecService/mtkcodecservice/include

LOCAL_MODULE:= vendor.mediatek.hardware.mtkcodecservice@1.1-service
LOCAL_INIT_RC := vendor.mediatek.hardware.mtkcodecservice@1.1-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_32_BIT_ONLY := true

include $(MTK_EXECUTABLE)
