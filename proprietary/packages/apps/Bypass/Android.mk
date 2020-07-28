LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_SDK_VERSION := system_current
LOCAL_PACKAGE_NAME := Bypass
LOCAL_PRODUCT_MODULE := true
LOCAL_CERTIFICATE := platform

# LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

include $(CLEAR_VARS)

LOCAL_MODULE := bypass
LOCAL_PRODUCT_MODULE := true

LOCAL_SRC_FILES := bypass

LOCAL_MODULE_CLASS := EXECUTABLES

LOCAL_MODULE_TAGS := optional

include $(BUILD_PREBUILT)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
