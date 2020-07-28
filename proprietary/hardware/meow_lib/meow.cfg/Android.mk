LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/meow))
include $(CLEAR_VARS)
LOCAL_MODULE := meow.cfg
LOCAL_MODULE_CLASS := ETC
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/meow.cfg
include $(BUILD_PREBUILT)
endif
