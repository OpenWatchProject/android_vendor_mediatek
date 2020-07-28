LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := lbs_hidl_service-impl
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/mtk_socket_utils/inc \

LOCAL_SRC_FILES := \
    lbs_hidl_service.cpp \
    mtk_socket_utils/src/mtk_socket_data_coder.c \
    mtk_socket_utils/src/mtk_socket_utils.c \


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
    vendor.mediatek.hardware.lbs@1.0 \

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := lbs_hidl_service
LOCAL_PROPRIETARY_MODULE := true
LOCAL_INCLUDE_MTK_GLOBAL_CONFIGS := no
LOCAL_INIT_RC := lbs_hidl_service.rc

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/mtk_socket_utils/inc \

LOCAL_SRC_FILES := \
    service.cpp \

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
    vendor.mediatek.hardware.lbs@1.0 \
    lbs_hidl_service-impl \

include $(MTK_EXECUTABLE)
