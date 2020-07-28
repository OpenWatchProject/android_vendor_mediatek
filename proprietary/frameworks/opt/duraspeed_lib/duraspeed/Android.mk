LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/frameworks/opt/duraspeed))
include $(CLEAR_VARS)
LOCAL_MODULE := duraspeed
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := .jar
LOCAL_SRC_FILES := javalib.jar
include $(BUILD_PREBUILT)
endif
