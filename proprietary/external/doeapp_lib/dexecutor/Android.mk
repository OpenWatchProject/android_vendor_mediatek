LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/external/doeapp))
include $(CLEAR_VARS)
LOCAL_MODULE := dexecutor
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := mediatek
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call get-prebuilt-src-arch,arm arm64)_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/dexecutor
include $(BUILD_PREBUILT)
endif
