
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_SDK_VERSION := current

LOCAL_PACKAGE_NAME := InstallOmaDrmFile

LOCAL_JAVA_LIBRARIES += mediatek-framework

# Add *.so files to apk
LOCAL_JNI_SHARED_LIBRARIES := librecoveryomadrm_jni

LOCAL_STATIC_JAVA_LIBRARIES := com.mediatek.omadrm.common

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))