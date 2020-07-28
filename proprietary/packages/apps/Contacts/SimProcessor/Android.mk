# Only trying to build it in case of telephony add on is supported
ifeq ($(strip $(MSSI_MTK_TELEPHONY_ADD_ON_POLICY)), 0)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

ext_dir := ../ext
src_dirs := src $(ext_dir)/src
LOCAL_SRC_FILES := $(call all-java-files-under, $(src_dirs))

LOCAL_PACKAGE_NAME := MtkSimProcessor
LOCAL_CERTIFICATE := platform
LOCAL_APK_LIBRARIES += Contacts
LOCAL_PRODUCT_MODULE := true
LOCAL_PRIVILEGED_MODULE := true

LOCAL_JAVA_LIBRARIES += telephony-common
LOCAL_JAVA_LIBRARIES += mediatek-framework
LOCAL_JAVA_LIBRARIES += mediatek-telephony-common mediatek-telephony-base
LOCAL_JAVA_LIBRARIES += mediatek-common

LOCAL_PROGUARD_ENABLED := disabled

LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_PACKAGE)

endif
