LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/external/met_utils))
include $(CLEAR_VARS)
LOCAL_MODULE := met-cmd
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/met-cmd
include $(BUILD_PREBUILT)
endif
