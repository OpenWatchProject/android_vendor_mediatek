LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/gpu_ext))
include $(CLEAR_VARS)
LOCAL_MODULE := libgpud
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib64
LOCAL_SHARED_LIBRARIES_64 := libcutils libutils liblog libsync libhardware libnativewindow libgralloc_extra libhidlbase android.hardware.graphics.common@1.1 android.hardware.graphics.common@1.2 android.hardware.graphics.mapper@2.0 android.hardware.graphics.mapper@2.1 android.hardware.graphics.mapper@3.0 libc++ libc libm libdl
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libgpud.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/gpu_ext))
include $(CLEAR_VARS)
LOCAL_MODULE := libgpud
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/lib
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog libsync libhardware libnativewindow libgralloc_extra libhidlbase android.hardware.graphics.common@1.1 android.hardware.graphics.common@1.2 android.hardware.graphics.mapper@2.0 android.hardware.graphics.mapper@2.1 android.hardware.graphics.mapper@3.0 libc++ libc libm libdl
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libgpud.so
include $(BUILD_PREBUILT)
endif
