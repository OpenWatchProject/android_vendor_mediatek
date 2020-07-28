ifeq ($(MTK_AUTO_TEST) , yes)

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform

LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res
LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += src/com/mediatek/camera/tests/helper/ILoggerService.aidl
LOCAL_SRC_FILES += src/com/mediatek/camera/tests/helper/ILoggerCallback.aidl

LOCAL_PACKAGE_NAME := CameraTestsCat
LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_PACKAGE)

endif
