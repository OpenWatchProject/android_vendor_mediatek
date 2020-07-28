LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	miravision_jni.cpp

LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)

LOCAL_C_INCLUDES += \
        $(TOP)/bionic \
        $(TOP)/frameworks/base/include \
        $(TOP)/system/core/base/include

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    liblog \
    libhidlbase \
    vendor.mediatek.hardware.pq@2.3


LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := libMiraVision_jni
LOCAL_MULTILIB := both

include $(MTK_SHARED_LIBRARY)
