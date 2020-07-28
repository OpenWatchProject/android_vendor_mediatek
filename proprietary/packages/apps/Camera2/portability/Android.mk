LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_JAVA_LIBRARIES := junit

LOCAL_JAVA_LIBRARIES := mediatek-framework
LOCAL_JAVA_LIBRARIES += mediatek-common
LOCAL_STATIC_JAVA_LIBRARIES += vendor.mediatek.hardware.camera.bgservice-V1.0-java

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_MODULE := com.mediatek.camera.portability

include $(BUILD_STATIC_JAVA_LIBRARY)
