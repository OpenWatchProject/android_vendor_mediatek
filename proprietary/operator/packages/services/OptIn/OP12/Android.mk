LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_JAVA_LIBRARIES += telephony-common

LOCAL_MODULE := OP12OptIn

include $(BUILD_STATIC_JAVA_LIBRARY)


