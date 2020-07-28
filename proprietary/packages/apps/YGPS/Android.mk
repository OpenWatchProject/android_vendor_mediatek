LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_JAVA_LIBRARIES := android-support-v4

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := YGPS
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_MODULE_OWNER := mtk
LOCAL_CERTIFICATE := platform
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.lbs-V1.0-java
include $(BUILD_PACKAGE)

