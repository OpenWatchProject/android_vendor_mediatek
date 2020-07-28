LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/frameworks/opt/mdlogger))
include $(CLEAR_VARS)
LOCAL_MODULE := emdlogger3
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils libselinux libccci_util_sys libmdloggerrecycle libc++ liblog libdl
LOCAL_INIT_RC := emdlogger3.rc
LOCAL_SRC_FILES := $(call get-prebuilt-src-arch,arm arm64)_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/emdlogger3
include $(BUILD_PREBUILT)
endif
