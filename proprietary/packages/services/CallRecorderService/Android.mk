LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_JAVA_LIBRARIES += mediatek-telecom-common
LOCAL_JAVA_LIBRARIES += mediatek-framework

LOCAL_PACKAGE_NAME := CallRecorderService
LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_CERTIFICATE := platform
LOCAL_PRIVILEGED_MODULE := true

LOCAL_PROGUARD_FLAG_FILES := proguard.flags

include $(BUILD_PACKAGE)

# Build the test package.
include $(call all-makefiles-under,$(LOCAL_PATH))
