LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := mediatek-framework \
                        telephony-common \
                        MtkSettingsLib \
                        ims-common

LOCAL_SHARED_ANDROID_LIBRARIES := \
    android-support-v4 \
    android-support-v7-appcompat \
    android-support-v7-recyclerview \
    android-support-v7-preference \
    android-support-v14-preference

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_MODULE := com.mediatek.settings.ext

LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_STATIC_JAVA_LIBRARY)


