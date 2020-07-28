ifneq ($(MTK_BASIC_PACKAGE), yes)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	pq_tuning_jni.cpp

LOCAL_C_INCLUDES := $(JNI_H_INCLUDE)

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/platform/$(MTK_PLATFORM_DIR)/kernel/drivers/dispsys \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/miravision/include \
    $(TOP)/frameworks/base/include \

LOCAL_SHARED_LIBRARIES := \
  libutils \
  libcutils \
  liblog

LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    vendor.mediatek.hardware.pq@2.0

LOCAL_MODULE := libPQjni
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both

include $(MTK_SHARED_LIBRARY)

endif
