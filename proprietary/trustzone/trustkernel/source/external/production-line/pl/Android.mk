LOCAL_PATH := $(call my-dir)

ifeq ("$(TRUSTKERNEL_TEE_PLATFORM_APP_SUPPORT)", "yes")
#system lib
include $(CLEAR_VARS)

LOCAL_MODULE := libpl_system
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES

LOCAL_SRC_FILES_arm := ../../../ca/libs/armeabi-v7a/libpl.so
LOCAL_SRC_FILES_arm64 := ../../../ca/libs/arm64-v8a/libpl.so

LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both

LOCAL_SHARED_LIBRARIES := libteec_system

LOCAL_EXPORT_C_INCLUDE_DIRS := \
		$(LOCAL_PATH)/include

LOCAL_POST_INSTALL_CMD = \
    $(hide) ln -sf  $(notdir $(LOCAL_INSTALLED_MODULE)) $(dir $(LOCAL_INSTALLED_MODULE))libpl.so
include $(BUILD_PREBUILT)

endif

# vendor lib
include $(CLEAR_VARS)

LOCAL_MODULE := libpl
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := trustkernel

LOCAL_SRC_FILES_arm := ../../../ca/libs/armeabi-v7a/libpl.so
LOCAL_SRC_FILES_arm64 := ../../../ca/libs/arm64-v8a/libpl.so

LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both

LOCAL_EXPORT_C_INCLUDE_DIRS := \
		$(LOCAL_PATH)/include

include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE := pld
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES

LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := trustkernel

LOCAL_SRC_FILES_arm := ../../../ca/libs/armeabi-v7a/pld
LOCAL_SRC_FILES_arm64 := ../../../ca/libs/arm64-v8a/pld

LOCAL_SHARED_LIBRARIES := libpl

include $(BUILD_PREBUILT)
