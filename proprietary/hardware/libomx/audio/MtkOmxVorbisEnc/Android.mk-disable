
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
#include frameworks/base/media/libstagefright/codecs/common/Config.mk

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

LOCAL_SRC_FILES := \
    MtkOmxVorbisEnc.cpp

LOCAL_C_INCLUDES := \
        $(TOP)/$(MTK_ROOT)/frameworks/native/include/media/openmax \
        $(TOP)/$(MTK_ROOT)/frameworks/av/media/libstagefright/include/omx_core \
        $(TOP)/$(MTK_ROOT)/hardware/omx/inc \
        $(LOCAL_PATH)/../../osal \
        $(LOCAL_PATH)/../../../omx/osal \
        $(LOCAL_PATH)/../../../omx/inc \
        $(LOCAL_PATH)/../MtkOmxAudioEncBase \
      $(TOP)/$(MTK_ROOT)/external/vorbisenc


LOCAL_MODULE := libMtkOmxVorbisEnc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES :=   \
    libMtkOmxAudioEncBase   \
    libMtkOmxOsalUtils  \
    libvorbisenc_mtk

LOCAL_SHARED_LIBRARIES :=       \
        libutils    \
        libcutils \
        liblog

include $(MTK_SHARED_LIBRARY)

