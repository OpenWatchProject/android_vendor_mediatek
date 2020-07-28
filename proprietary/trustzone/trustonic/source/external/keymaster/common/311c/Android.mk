ifeq ($(TRUSTONIC_TEE_SUPPORT), yes)

ifneq (,$(TRUSTONIC_TEE_VERSION))

LOCAL_PATH := $(call my-dir)

ifneq (,$(KEYMASTER_VERSION))

$(info keymaster - including $(LOCAL_PATH)/$(KEYMASTER_VERSION)/Android.mk)
include $(LOCAL_PATH)/$(KEYMASTER_VERSION)/Android.mk

endif

endif

endif
