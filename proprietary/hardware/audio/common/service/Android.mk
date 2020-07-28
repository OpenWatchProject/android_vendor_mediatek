
LOCAL_PATH := $(call my-dir)
HIDL_VERSION := 5.0

include $(CLEAR_VARS)

#######################################################################
# Call sub-folder Android.mk
#
include $(LOCAL_PATH)/$(HIDL_VERSION)/Android.mk