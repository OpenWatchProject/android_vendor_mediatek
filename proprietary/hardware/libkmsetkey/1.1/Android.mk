LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.keymaster_attestation@1.1-impl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
    KeymasterDevice.cpp \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libhidlbase \
    libhidltransport \
    libutils \
    libhardware \
    vendor.mediatek.hardware.keymaster_attestation@1.0 \
    vendor.mediatek.hardware.keymaster_attestation@1.1

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.keymaster_attestation@1.1-service
LOCAL_INIT_RC := vendor.mediatek.hardware.keymaster_attestation@1.1-service.rc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
    service.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libhardware \
    libhidlbase \
    libhidltransport \
    vendor.mediatek.hardware.keymaster_attestation@1.1

include $(BUILD_EXECUTABLE)
