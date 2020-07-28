LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/widevine))
include $(CLEAR_VARS)
LOCAL_MODULE := move_widevine_data.sh
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := widevine
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/move_widevine_data.sh
include $(BUILD_PREBUILT)
endif
