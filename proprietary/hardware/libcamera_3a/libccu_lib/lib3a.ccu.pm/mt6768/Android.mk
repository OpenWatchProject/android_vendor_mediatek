LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libcamera_3a/libccu))
include $(CLEAR_VARS)
LOCAL_MODULE := lib3a.ccu.pm
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin
LOCAL_SRC_FILES := arm/lib3a.ccu.pm
include $(BUILD_PREBUILT)
endif
