LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.vibrator@1.0-impl-mediatek
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
    Vibrator.cpp \

LOCAL_SHARED_LIBRARIES := \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    libutils \
    liblog \
    libcutils \
    libhardware \
    libbase \
    libcutils \
    android.hardware.vibrator@1.0 \

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
ifneq ($(strip $(MTK_GMO_ROM_OPTIMIZE)), yes)
LOCAL_MODULE := android.hardware.vibrator@1.0-service-mediatek
LOCAL_INIT_RC := android.hardware.vibrator@1.0-service-mediatek.rc
else
LOCAL_MODULE := android.hardware.vibrator@1.0-service-mediatek-lazy
LOCAL_INIT_RC := android.hardware.vibrator@1.0-service-mediatek-lazy.rc
endif

LOCAL_SRC_FILES := service.cpp

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libdl \
    libbase \
    libutils \
    libhardware_legacy \
    libhardware \

LOCAL_SHARED_LIBRARIES += \
    libhwbinder \
    libhidlbase \
    libhidltransport \
    android.hardware.vibrator@1.0 \

include $(BUILD_EXECUTABLE)
