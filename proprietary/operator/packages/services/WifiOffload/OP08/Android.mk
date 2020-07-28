LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_CERTIFICATE := platform

LOCAL_JAVA_LIBRARIES += mediatek-common
LOCAL_APK_LIBRARIES := ImsService

LOCAL_MODULE_TAGS := optional
LOCAL_PACKAGE_NAME := OP08Wos
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk
LOCAL_PROGUARD_ENABLED := disabled

# Put operator customization apk together to specific folder
# Specify install path for MTK CIP solution
ifeq ($(strip $(MTK_CIP_SUPPORT)),yes)
    LOCAL_MODULE_PATH := $(TARGET_CUSTOM_OUT)/app
else
    LOCAL_MODULE_PATH := $(TARGET_OUT)/app
endif

include $(BUILD_PACKAGE)
