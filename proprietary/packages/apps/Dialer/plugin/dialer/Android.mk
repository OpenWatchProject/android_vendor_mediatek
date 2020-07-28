LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_JAVA_LIBRARIES := mediatek-framework
LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += android-support-v7-recyclerview
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src) \
                   ../../java/com/mediatek/dialer/ext/DialpadExtensionAction.java

LOCAL_MODULE := com.mediatek.dialer.ext.vendor

LOCAL_STATIC_JAVA_LIBRARIES := guava
LOCAL_STATIC_JAVA_LIBRARIES += android-support-v13

LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_STATIC_JAVA_LIBRARY)

