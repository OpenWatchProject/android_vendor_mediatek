LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := mediatek-common

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_MODULE := com.mediatek.providers.settings.ext

LOCAL_CERTIFICATE := platform

include $(BUILD_STATIC_JAVA_LIBRARY)


