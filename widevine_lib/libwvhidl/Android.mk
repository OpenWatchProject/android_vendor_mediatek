LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/widevine))
include $(CLEAR_VARS)
LOCAL_MODULE := libwvhidl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := widevine
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES_64 := android.hardware.drm@1.0 android.hardware.drm@1.1 android.hardware.drm@1.2 android.hidl.memory@1.0 libbase libdl libhidlbase libhidlmemory libhidltransport libhwbinder liblog libprotobuf-cpp-lite libutils
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libwvhidl.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/widevine))
include $(CLEAR_VARS)
LOCAL_MODULE := libwvhidl
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := widevine
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := android.hardware.drm@1.0 android.hardware.drm@1.1 android.hardware.drm@1.2 android.hidl.memory@1.0 libbase libdl libhidlbase libhidlmemory libhidltransport libhwbinder liblog libprotobuf-cpp-lite libutils
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libwvhidl.so
include $(BUILD_PREBUILT)
endif
