LOCAL_PATH:= $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE := kmsetkey.trusty

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SRC_FILES := \
	kmsetkey_ipc.c \
	module.c \
	HalAdaptationLayer.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
	libtrusty \
	liblog \
	libcutils

LOCAL_HEADER_LIBRARIES := \
	libhardware_headers

LOCAL_CLFAGS = -Wall -Werror -g
LOCAL_MODULE_TAGS := optional

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

include $(BUILD_SHARED_LIBRARY)
