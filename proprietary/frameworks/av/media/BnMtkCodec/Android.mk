LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    BnMtkCodec.cpp \
    CAPEWrapper.cpp

LOCAL_C_INCLUDES:= \
        $(TOP)/frameworks/native/include \
        $(TOP)/$(MTK_ROOT)/external/apedec \
        $(TOP)/$(MTK_ROOT)/external/apedec/arm_32 \
        $(TOP)/$(MTK_ROOT)/external/apedec_arm_64 \
        $(TOP)/$(MTK_ROOT)/external/apedec/arm_32/inc \
        $(TOP)/$(MTK_ROOT)/external/apedec/arm_64/inc \
        $(LOCAL_PATH)/../../../omx/osal \
        $(LOCAL_PATH)/../../../omx/inc \
        $(TOP)/$(MTK_ROOT)/frameworks/native/include/media/openmax


LOCAL_SHARED_LIBRARIES :=       \
        libbinder               \
        libutils                \
        libcutils               \
        libdl                   \
        libui                   \
        liblog



LOCAL_STATIC_LIBRARIES :=       \
        libapedec_mtk


LOCAL_PRELINK_MODULE:= false
LOCAL_MODULE := libBnMtkCodec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both

include $(MTK_SHARED_LIBRARY)
