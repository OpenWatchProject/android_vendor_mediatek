LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/frameworks/opt/wapicertstore))
include $(CLEAR_VARS)
LOCAL_MODULE := WapiCertStore
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := .jar
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_SRC_FILES := classes.jar
include $(BUILD_PREBUILT)
endif
