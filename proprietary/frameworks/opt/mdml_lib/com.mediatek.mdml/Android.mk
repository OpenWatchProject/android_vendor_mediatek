LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/frameworks/opt/mdml))
include $(CLEAR_VARS)
LOCAL_MODULE := com.mediatek.mdml
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .jar
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := classes.jar
include $(BUILD_PREBUILT)
endif
