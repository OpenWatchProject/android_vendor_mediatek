LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := tests
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_JAVA_LIBRARIES := android.test.runner

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_PACKAGE_NAME := BypassTest
LOCAL_INSTRUMENTATION_FOR := Bypass

include $(BUILD_PACKAGE)
