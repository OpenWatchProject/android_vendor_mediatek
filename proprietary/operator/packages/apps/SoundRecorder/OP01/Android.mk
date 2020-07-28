LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := OP01SoundRecorder
LOCAL_OVERRIDES_PACKAGES := SoundRecorder
LOCAL_CERTIFICATE := platform

ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/app
else
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/app
endif


include $(BUILD_PACKAGE)
