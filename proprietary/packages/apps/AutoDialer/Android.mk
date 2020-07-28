LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := AutoDialer
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES := \
        telephony-common \

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))

