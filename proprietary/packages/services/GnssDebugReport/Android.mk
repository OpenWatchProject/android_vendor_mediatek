LOCAL_PATH:= $(call my-dir)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_JAVA_LIBRARIES += mediatek-framework

LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := GnssDebugReport
LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_MODULE_OWNER := mtk

LOCAL_CERTIFICATE := platform
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.lbs-V1.0-java

include $(BUILD_PACKAGE)

