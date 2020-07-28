LOCAL_PATH := $(call my-dir)
#
# libfpspolicy_fw
#
include $(CLEAR_VARS)

include $(LOCAL_PATH)/BuildFpsPolicy.mk
LOCAL_MODULE := libfpspolicy_fw
LOCAL_SHARED_LIBRARIES += \
        vendor.mediatek.hardware.dfps@1.0

include $(BUILD_SHARED_LIBRARY)
