LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := appluginmanager
LOCAL_STATIC_JAVA_LIBRARIES := xerces
LOCAL_SRC_FILES := $(call all-java-files-under, src)

include $(BUILD_STATIC_JAVA_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := xerces:lib/xerces.jar
include $(BUILD_MULTI_PREBUILT)
