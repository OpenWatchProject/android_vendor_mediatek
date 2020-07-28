#for hidl exe

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    main_mtk_mms_hal.cpp

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    liblog \
    libutils \
    libhardware \
    vendor.mediatek.hardware.mms@1.5

LOCAL_CPPFLAGS += -fexceptions
LOCAL_MODULE:= vendor.mediatek.hardware.mms@1.5-service
LOCAL_INIT_RC := vendor.mediatek.hardware.mms@1.5-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw

include $(MTK_EXECUTABLE)