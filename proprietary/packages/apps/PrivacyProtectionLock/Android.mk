ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_PRIVILEGED_MODULE := true
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PRELINK_MODULE := false

LOCAL_PROGUARD_ENABLED := full
LOCAL_PROGUARD_FLAG_FILES:= proguard.flags

LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += mediatek-common
LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-telephony-common
LOCAL_JAVA_LIBRARIES += com.mediatek.settings.ext

LOCAL_STATIC_JAVA_LIBRARIES := vendor.mediatek.hardware.pplagent-V1.0-java

LOCAL_STATIC_ANDROID_LIBRARIES := \
    androidx.preference_preference \
    androidx.appcompat_appcompat \
    androidx.recyclerview_recyclerview \
    androidx.legacy_legacy-support-v4

LOCAL_PACKAGE_NAME := PrivacyProtectionLock
LOCAL_PRODUCT_MODULE := true
LOCAL_PRIVILEGED_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

endif