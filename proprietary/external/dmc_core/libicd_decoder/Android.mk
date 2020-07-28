############################################################################
# Utility library to parse modem ICD data
# Vendor dynamic library
############################################################################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libicd_decoder
LOCAL_ARM_MODE := arm
LOCAL_MODULE_OWNER := mtk

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,,$(wildcard $(LOCAL_PATH)/src/*.cpp))

LOCAL_STATIC_LIBRARIES := libmdmonitor
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libhidlbase \
    libhidltransport \
    libhwbinder

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_LDLIBS := -llog
include $(MTK_SHARED_LIBRARY)
