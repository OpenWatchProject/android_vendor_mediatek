LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/external/met_utils))
include $(CLEAR_VARS)
LOCAL_MODULE := met_log_d
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/met_log_d
include $(BUILD_PREBUILT)
endif
