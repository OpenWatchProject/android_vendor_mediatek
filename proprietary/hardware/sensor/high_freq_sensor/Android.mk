LOCAL_PATH:= $(call my-dir)

ifeq ($(MTK_HIGH_FREQ_SENSOR), yes)
include $(CLEAR_VARS)
LOCAL_MODULE := libhfmanager
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES := \
    HfManager.cpp 

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libcutils 

LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(LOCAL_PATH) 

LOCAL_MULTILIB := first 

include $(MTK_SHARED_LIBRARY)

#include $(LOCAL_PATH)/test/Android.mk
endif