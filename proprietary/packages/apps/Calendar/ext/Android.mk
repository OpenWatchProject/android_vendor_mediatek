LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under,src)

LOCAL_SDK_VERSION := current

LOCAL_MODULE := com.mediatek.calendar.ext

LOCAL_STATIC_JAVA_LIBRARIES := mediatek-opt-datetimepicker
LOCAL_JAVA_LIBRARIES += mediatek-common
include $(BUILD_STATIC_JAVA_LIBRARY)
