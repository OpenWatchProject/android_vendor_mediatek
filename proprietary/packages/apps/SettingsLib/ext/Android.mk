LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_USE_AAPT2 := true

#LOCAL_AAPT2_ONLY := true

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_JAVA_LIBRARIES += mediatek-common
LOCAL_MODULE := com.mediatek.settingslib.ext

include $(BUILD_STATIC_JAVA_LIBRARY)