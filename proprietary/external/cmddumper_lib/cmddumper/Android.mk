LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/external/cmddumper))
include $(CLEAR_VARS)
LOCAL_MODULE := cmddumper
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libutils libcutils libc2kutils_sys libselinux liblog
LOCAL_INIT_RC := cmddumper.rc
LOCAL_SRC_FILES := $(call get-prebuilt-src-arch,arm arm64)_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/cmddumper
include $(BUILD_PREBUILT)
endif
