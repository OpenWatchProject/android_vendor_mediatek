LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/widevine))
include $(CLEAR_VARS)
LOCAL_MODULE := libwvdrmengine
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := widevine
LOCAL_MODULE_RELATIVE_PATH := mediadrm
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES_64 := libbase libdl liblog libprotobuf-cpp-lite libstagefright_foundation libutils
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libwvdrmengine.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/widevine))
include $(CLEAR_VARS)
LOCAL_MODULE := libwvdrmengine
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := widevine
LOCAL_MODULE_RELATIVE_PATH := mediadrm
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libbase libdl liblog libprotobuf-cpp-lite libstagefright_foundation libutils
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libwvdrmengine.so
include $(BUILD_PREBUILT)
endif
