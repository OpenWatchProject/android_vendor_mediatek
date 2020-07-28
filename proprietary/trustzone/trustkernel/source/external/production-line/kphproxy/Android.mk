LOCAL_PATH := $(call my-dir)

ifeq ("$(TRUSTKERNEL_TEE_PLATFORM_APP_SUPPORT)", "yes")
# system lib
include $(CLEAR_VARS)
LOCAL_MODULE := libkphhelper_system
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_SRC_FILES_arm := ../../../ca/libs/armeabi-v7a/libkphhelper.so
LOCAL_SRC_FILES_arm64 := ../../../ca/libs/arm64-v8a/libkphhelper.so

LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both

LOCAL_POST_INSTALL_CMD = \
    $(hide) ln -sf  $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))libkphhelper.so
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE := libkphproxy_system
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_SRC_FILES_arm := ../../../ca/libs/armeabi-v7a/libkphproxy.so
LOCAL_SRC_FILES_arm64 := ../../../ca/libs/arm64-v8a/libkphproxy.so

LOCAL_SHARED_LIBRARIES := libkphhelper_system libteec_system

LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both

LOCAL_EXPORT_C_INCLUDE_DIRS := \
        $(LOCAL_PATH)/include

LOCAL_POST_INSTALL_CMD = \
    $(hide) ln -sf  $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))libkphproxy.so
include $(BUILD_PREBUILT)

endif

# vendor lib

include $(CLEAR_VARS)
LOCAL_MODULE := libkphhelper
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := trustkernel

LOCAL_SRC_FILES_arm := ../../../ca/libs/armeabi-v7a/libkphhelper.so
LOCAL_SRC_FILES_arm64 := ../../../ca/libs/arm64-v8a/libkphhelper.so

LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE := libkphproxy
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := trustkernel

LOCAL_SRC_FILES_arm := ../../../ca/libs/armeabi-v7a/libkphproxy.so
LOCAL_SRC_FILES_arm64 := ../../../ca/libs/arm64-v8a/libkphproxy.so

LOCAL_SHARED_LIBRARIES := libkphhelper

LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both

LOCAL_EXPORT_C_INCLUDE_DIRS := \
		$(LOCAL_PATH)/include

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE := kph
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES

LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := trustkernel

LOCAL_SRC_FILES_arm := ../../../ca/libs/armeabi-v7a/kph
LOCAL_SRC_FILES_arm64 := ../../../ca/libs/arm64-v8a/kph

LOCAL_SHARED_LIBRARIES := libkphproxy libkphhelper

include $(BUILD_PREBUILT)
