LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/frameworks/neuropilot/np_utils))
include $(CLEAR_VARS)
LOCAL_MODULE := libneuropilot_hal_utils.vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib64
LOCAL_MODULE_STEM := libneuropilot_hal_utils
LOCAL_SHARED_LIBRARIES_64 := libbase libcutils libhidlbase libhidlmemory libnativewindow libui android.hardware.neuralnetworks@1.0 android.hardware.neuralnetworks@1.1 android.hardware.neuralnetworks@1.2 android.hidl.allocator@1.0 android.hidl.memory@1.0 libc++ libc libm libdl
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libneuropilot_hal_utils.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/frameworks/neuropilot/np_utils))
include $(CLEAR_VARS)
LOCAL_MODULE := libneuropilot_hal_utils.vendor
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib
LOCAL_MODULE_STEM := libneuropilot_hal_utils
LOCAL_SHARED_LIBRARIES := libbase libcutils libhidlbase libhidlmemory libnativewindow libui android.hardware.neuralnetworks@1.0 android.hardware.neuralnetworks@1.1 android.hardware.neuralnetworks@1.2 android.hidl.allocator@1.0 android.hidl.memory@1.0 libc++ libc libm libdl
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libneuropilot_hal_utils.so
include $(BUILD_PREBUILT)
endif
