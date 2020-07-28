LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.sensors@2.0-service-mediatek
LOCAL_INIT_RC := android.hardware.sensors@2.0-service-mediatek.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
    Sensors.cpp \
    service.cpp 

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libhardware \
    libhwbinder \
    libbase \
    libutils \
    libhidlbase \
    libhidltransport \
    android.hardware.sensors@1.0 \
    android.hardware.sensors@2.0 \
    libfmq \
    libpower 

LOCAL_STATIC_LIBRARIES := \
    android.hardware.sensors@1.0-convert \
    multihal 

include $(BUILD_EXECUTABLE)
