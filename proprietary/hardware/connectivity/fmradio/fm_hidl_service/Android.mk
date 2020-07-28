LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_ONWER := mtk

#LOCAL_INCLUDE_MTK_GLOBAL_CONFIGS := no
LOCAL_INIT_RC := fm_hidl_service.rc

LOCAL_MODULE := fm_hidl_service
LOCAL_PROPRIETARY_MODULE := true

#LOCAL_C_INCLUDES += \
#    $(LOCAL_PATH)/mtk_socket_utils/inc \

LOCAL_SRC_FILES := \
    fm_hidl_service.cpp \
    fmr_core.cpp \
    common.cpp \
    fmr_err.cpp \
    custom.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libhardware \
    libbinder \
    libhidlbase \
    libhidltransport \
    libutils \
    vendor.mediatek.hardware.fm@1.0

#include $(BUILD_SHARED_LIBRARY)

include $(MTK_EXECUTABLE)
