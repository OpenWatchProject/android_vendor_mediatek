LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/external/doeapp))
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.dplanner@2.0-impl
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/vendor.mediatek.hardware.dplanner@2.0-impl.a
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/external/doeapp))
include $(CLEAR_VARS)
LOCAL_MODULE := vendor.mediatek.hardware.dplanner@2.0-impl
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/vendor.mediatek.hardware.dplanner@2.0-impl.a
include $(BUILD_PREBUILT)
endif
