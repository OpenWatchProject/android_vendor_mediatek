LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libkmsetkey_ca.gz_trusty

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	kmsetkey.cpp \
	kmsetkey_ipc.c

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
	libtrusty \
	liblog \
	libcutils

LOCAL_CLFAGS = -fvisibility=hidden -Wall -Werror

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := kmsetkey_ca.gz_trusty

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	main.cpp

LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
	libtrusty \
	liblog \
	libcutils \
	libkmsetkey_ca.gz_trusty

LOCAL_CLFAGS = -fvisibility=hidden -Wall -Werror

include $(BUILD_EXECUTABLE)
