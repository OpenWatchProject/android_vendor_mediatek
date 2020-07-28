LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/gift))
include $(CLEAR_VARS)
LOCAL_MODULE := libMEOW_gift
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib64/egl
LOCAL_SHARED_LIBRARIES_64 := libFrameRecord libDefaultFpsActor libNoFpsActor libnativewindow libutilscallstack libcutils libutils liblog libc++ libc libm libdl
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libMEOW_gift.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/gift))
include $(CLEAR_VARS)
LOCAL_MODULE := libMEOW_gift
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib/egl
LOCAL_SHARED_LIBRARIES := libFrameRecord libDefaultFpsActor libNoFpsActor libnativewindow libutilscallstack libcutils libutils liblog libc++ libc libm libdl
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libMEOW_gift.so
include $(BUILD_PREBUILT)
endif
