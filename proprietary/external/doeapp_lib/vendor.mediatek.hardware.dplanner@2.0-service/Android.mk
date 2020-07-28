LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/external/doeapp))
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.dplanner@2.0-service
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_SUFFIX := .0-service
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin/hw
LOCAL_MODULE_STEM := vendor.mediatek.hardware.dplanner@2
LOCAL_SHARED_LIBRARIES := libhidlbase libutils vendor.mediatek.hardware.dplanner@2.0 libziparchive libcutils liblog libc++ libc libm libdl
LOCAL_INIT_RC := 2.0/vendor.mediatek.hardware.dplanner@2.0-service.rc
LOCAL_VINTF_FRAGMENTS := 2.0/vendor.mediatek.hardware.dplanner@2.0-service.xml
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/vendor.mediatek.hardware.dplanner@2.0-service
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/external/doeapp))
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.dplanner@2.0-service
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_SUFFIX := .0-service
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin/bin32
LOCAL_MODULE_STEM := vendor.mediatek.hardware.dplanner@2
LOCAL_SHARED_LIBRARIES := libhidlbase libutils vendor.mediatek.hardware.dplanner@2.0 libziparchive libcutils liblog libc++ libc libm libdl
LOCAL_INIT_RC := 2.0/vendor.mediatek.hardware.dplanner@2.0-service.rc
LOCAL_VINTF_FRAGMENTS := 2.0/vendor.mediatek.hardware.dplanner@2.0-service.xml
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/vendor.mediatek.hardware.dplanner@2.0-service
include $(BUILD_PREBUILT)
endif
