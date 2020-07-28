LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    AudioStream.cpp

LOCAL_C_INCLUDES := \
    $(TOP)/frameworks/av/include \
    $(TOP)/frameworks/native/include \
    $(TOP)/frameworks/av/include/media

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libstagefright \
    libstagefright_foundation \
    libmedia

LOCAL_MULTILIB := both

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libaudiostream
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk
include $(MTK_STATIC_LIBRARY)
