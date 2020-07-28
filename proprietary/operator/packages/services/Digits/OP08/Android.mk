LOCAL_PATH:= $(call my-dir)

### export
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, export)

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/export

LOCAL_SRC_FILES += \
    export/com/mediatek/digits/IDigitsEventListener.aidl \
    export/com/mediatek/digits/IDigitsActionListener.aidl \
    export/com/mediatek/digits/IDigitsService.aidl

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := OP08digits-common
LOCAL_PROPRIETARY_MODULE := false

include $(BUILD_STATIC_JAVA_LIBRARY)

### apk
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_STATIC_JAVA_LIBRARIES += android-support-v4 android-support-v7-appcompat
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res frameworks/support/v7/appcompat/res

LOCAL_STATIC_JAVA_AAR_LIBRARIES := Digits-play-services-gcm Digits-play-services-iid Digits-play-services-basement Digits-play-services-base
LOCAL_AAPT_FLAGS += --extra-packages com.google.android.gms --auto-add-overlay

LOCAL_JAVA_LIBRARIES += okhttp telephony-common mediatek-telephony-base

LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.radio_op-V2.0-java OP08digits-common OpRilCommon

LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := OP08Digits
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PROPRIETARY_MODULE := false

LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PROGUARD_FLAGS := $(proguard.flags)

ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/app
else
LOCAL_MODULE_PATH := $(TARGET_OUT)/app
endif

include $(BUILD_PACKAGE)

### gcm
include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := Digits-play-services-gcm:libs/play-services-gcm-9.4.0.aar Digits-play-services-iid:libs/play-services-iid-9.4.0.aar Digits-play-services-basement:libs/play-services-basement-9.4.0.aar Digits-play-services-base:libs/play-services-base-9.4.0.aar


include $(BUILD_MULTI_PREBUILT)

include $(LOCAL_PATH)/jni/Android.mk
