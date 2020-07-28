LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_RESOURCE_DIR := vendor/mediatek/proprietary/packages/apps/DeskClock/res

LOCAL_MODULE_TAGS := optional
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PRIVILEGED_MODULE := true
LOCAL_PACKAGE_NAME := MtkDeskClock
LOCAL_OVERRIDES_PACKAGES := AlarmClock DeskClock
LOCAL_CERTIFICATE := platform

# M: [ALPS03416716] Added to compile ScreensaverSettingsActivity requierd by Settings app @{
LOCAL_PROGUARD_ENABLED := disabled
# @}

LOCAL_SRC_FILES := $(call all-java-files-under, src gen)

LOCAL_PROGUARD_FLAG_FILES := ../../../frameworks/support/design/proguard-rules.pro
LOCAL_PROGUARD_FLAG_FILES += ../../../frameworks/support/v7/preference/proguard-rules.pro
LOCAL_PROGUARD_FLAG_FILES += ../../../frameworks/support/v7/recyclerview/proguard-rules.pro

LOCAL_STATIC_ANDROID_LIBRARIES := \
        android-support-design \
        android-support-percent \
        android-support-transition \
        android-support-compat \
        android-support-media-compat \
        android-support-v13 \
        android-support-v14-preference \
        android-support-v7-appcompat \
        android-support-v7-gridlayout \
        android-support-v7-preference \
        android-support-v7-recyclerview

LOCAL_STATIC_JAVA_LIBRARIES := vendor.mediatek.hardware.pplagent-V1.0-java

LOCAL_USE_AAPT2 := true

include $(BUILD_PACKAGE)
