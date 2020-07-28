LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := merged_hal_service
LOCAL_INIT_RC := merged_hal_service.rc

LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/hardware/connectivity/gps/lbs_hidl_service \
    $(MTK_PATH_SOURCE)/hardware/connectivity/gps/lbs_hidl_service/mtk_socket_utils/inc

LOCAL_SRC_FILES := \
    service.cpp \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libhardware \

LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    libhidltransport \
    android.hardware.vibrator@1.0 \
    android.hardware.thermal@1.0 \
    android.hardware.memtrack@1.0 \
    android.hardware.light@2.0 \
    android.hardware.graphics.allocator@2.0 \
    vendor.mediatek.hardware.gpu@1.0 \

ifeq ($(strip $(MTK_GPS_SUPPORT)), yes)
    LOCAL_CFLAGS:= -DMTK_GPS_SUPPORT
    LOCAL_SHARED_LIBRARIES += \
    android.hardware.gnss@1.0 \
    android.hardware.gnss@1.1 \
    android.hardware.gnss@2.0 \
    vendor.mediatek.hardware.lbs@1.0 \
    lbs_hidl_service-impl \

    LOCAL_REQUIRED_MODULES := \
        android.hardware.gnss@2.0-impl-mediatek
endif

LOCAL_HEADER_LIBRARIES += media_plugin_headers
include $(BUILD_EXECUTABLE)
