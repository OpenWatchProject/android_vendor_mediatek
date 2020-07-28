LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := teed
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES

LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := trustkernel

LOCAL_SRC_FILES_arm := ../../../client/libs/armeabi-v7a/teed
LOCAL_SRC_FILES_arm64 := ../../../client/libs/arm64-v8a/teed

LOCAL_SHARED_LIBRARIES := \
	libkphproxy \
	libpl \
	liblog

ifneq ($(strip $(TRUSTKERNEL_TRUSTSTORE_PROTECT)), yes)
LOCAL_INIT_RC := ../../../bsp/platform/common/scripts/trustkernel.rc
endif

include $(BUILD_PREBUILT)
