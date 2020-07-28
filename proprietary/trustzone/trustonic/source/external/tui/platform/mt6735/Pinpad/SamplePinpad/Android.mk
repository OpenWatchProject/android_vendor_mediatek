#
# build SamplePinpad
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_JNI_SHARED_LIBRARIES := libTlcPinpad

LOCAL_PACKAGE_NAME := SamplePinpad
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_FLAGS := -include $(LOCAL_PATH)/proguard-project.txt

include $(BUILD_PACKAGE)

#include $(LOCAL_PATH)/jni/Android.mk
