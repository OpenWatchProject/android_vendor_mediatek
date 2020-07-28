LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_AAPT_INCLUDE_ALL_RESOURCES := true

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_STATIC_JAVA_LIBRARIES += com.mediatek.lbs.em2.utils

LOCAL_SHARED_ANDROID_LIBRARIES += android-support-v4

LOCAL_PACKAGE_NAME := LocationEM2
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))

