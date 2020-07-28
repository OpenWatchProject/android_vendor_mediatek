LOCAL_PATH := $(call my-dir)

# -----------------------------------------------------------------------------
# Builds libwvhdcpext
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/WVHdcpExt.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := libwvhdcpext
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
