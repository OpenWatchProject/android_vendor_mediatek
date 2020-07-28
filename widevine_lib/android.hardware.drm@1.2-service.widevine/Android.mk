LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/widevine))
include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.drm@1.2-service.widevine
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := widevine
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/bin/hw
LOCAL_SHARED_LIBRARIES := android.hardware.drm@1.0 android.hardware.drm@1.1 android.hardware.drm@1.2 libbase libhidlbase libhidltransport libhwbinder liblog libutils libwvhidl libbinder
LOCAL_INIT_RC := src_hidl/android.hardware.drm@1.2-service.widevine.rc
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/android.hardware.drm@1.2-service.widevine
include $(BUILD_PREBUILT)
endif
