#
# build SamplePinpad
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PACKAGE_NAME := SamplePinpad
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
#LOCAL_PROGUARD_FLAGS := -include $(LOCAL_PATH)/proguard-project.txt
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_SHARED_JAVA_LIBRARIES := TeeClient
LOCAL_JNI_SHARED_LIBRARIES := libTlcPinpad

include $(BUILD_PACKAGE)
