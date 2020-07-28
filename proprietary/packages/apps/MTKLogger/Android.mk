LOCAL_PATH:= $(call my-dir)

# Build DebugLoggerUI.apk
include $(CLEAR_VARS)

ifeq ($(TARGET_BUILD_VARIANT), user)
LOCAL_MANIFEST_FILE := user/AndroidManifest.xml
endif

LOCAL_MODULE_TAGS := optional
LOCAL_AAPT_INCLUDE_ALL_RESOURCES := true
#use this to distinguish android version which support or not support ActionBar and Switch
LOCAL_SUPPORT_ACTION_BAR := yes

SRC_ROOT := src/com/debug/loggerui
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := DebugLoggerUI
LOCAL_MODULE_OWNER := mtk
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES += org.apache.http.legacy

LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.log-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.lbs-V1.0-java

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_PACKAGE)

# Build log-handler.jar
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src/com/log/handler)

LOCAL_MODULE := log-handler
LOCAL_MODULE_PATH :=  $(TARGET_OUT)/framework

LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.log-V1.0-java
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.lbs-V1.0-java

include $(BUILD_JAVA_LIBRARY)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))