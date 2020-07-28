LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.sensors@1.0-impl-mediatek
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
    Sensors.cpp 

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libhardware \
    libhwbinder \
    libbase \
    libutils \
    libhidlbase \
    libhidltransport \
    android.hardware.sensors@1.0 

LOCAL_STATIC_LIBRARIES := \
    android.hardware.sensors@1.0-convert \
    multihal 

LOCAL_MULTILIB := first 

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE := android.hardware.sensors@1.0-service-mediatek
LOCAL_INIT_RC := android.hardware.sensors@1.0-service-mediatek.rc
LOCAL_SRC_FILES := \
        service.cpp 

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
        libdl \
        libbase \
        libutils \
        libhardware_legacy \
        libhardware 

LOCAL_SHARED_LIBRARIES += \
        libhwbinder \
        libhidlbase \
        libhidltransport \
        android.hardware.sensors@1.0 

include $(BUILD_EXECUTABLE)
