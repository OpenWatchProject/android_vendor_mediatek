LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_JAVA_LIBRARIES := telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-framework mediatek-common

LOCAL_STATIC_JAVA_LIBRARIES := \
    android-support-v7-preference \
    android-support-v7-appcompat \
    android-support-v7-recyclerview \
    android-support-v4 \
    android-common \

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := UsbChecker
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

# LOCAL_PROGUARD_FLAG_FILES := proguard.flags

LOCAL_MODULE_PATH := $(TARGET_OUT)/app

include $(BUILD_PACKAGE)
