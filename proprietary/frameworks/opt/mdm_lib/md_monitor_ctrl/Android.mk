LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/frameworks/opt/mdm))
include $(CLEAR_VARS)
LOCAL_MODULE := md_monitor_ctrl
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc++ liblog libutils libcutils vendor.mediatek.hardware.mdmonitor@1.0 libhwbinder libhidlbase libhidltransport
LOCAL_SRC_FILES := $(call get-prebuilt-src-arch,arm arm64)_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/md_monitor_ctrl
include $(BUILD_PREBUILT)
endif
