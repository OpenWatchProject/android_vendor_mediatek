LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/connectivity/wapi-v2))
include $(CLEAR_VARS)
LOCAL_MODULE := libwapi
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES_64 := libkeystore-wifi-hidl libcrypto liblog
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libwapi.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/connectivity/wapi-v2))
include $(CLEAR_VARS)
LOCAL_MODULE := libwapi
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES := libkeystore-wifi-hidl libcrypto liblog
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libwapi.so
include $(BUILD_PREBUILT)
endif
