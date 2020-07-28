LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# This application should be part of platform. As it has shared uid with system android.uid.system.
LOCAL_CERTIFICATE := platform

# Only compile source java files in this apk.
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := AlwaysOnDisplay
LOCAL_MODULE_OWNER := mtk

include $(BUILD_PACKAGE)

# Use the following include to make our test apk.
#include $(call all-makefiles-under,$(LOCAL_PATH))
