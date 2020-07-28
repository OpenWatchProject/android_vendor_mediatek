LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

LOCAL_SRC_FILES := \
	memtester.c \
	util.c \
	tests.c

LOCAL_MODULE := doeapp-memtester
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mediatek

LOCAL_SHARED_LIBRARIES += libaedv

include $(MTK_EXECUTABLE)
