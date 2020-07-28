ifeq ($(TRUSTONIC_TEE_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)

ifneq (,$(TRUSTONIC_TEE_VERSION))
ifeq ($(KEYMASTER_VERSION),4.0)
$(info KeyboxInject - including $(LOCAL_PATH)/$(KEYMASTER_VERSION)/Android.mk)
include $(LOCAL_PATH)/$(KEYMASTER_VERSION)/Android.mk
endif
ifeq ($(KEYMASTER_VERSION),3.0)
$(info KeyboxInject - including $(LOCAL_PATH)/$(KEYMASTER_VERSION)/Android.mk)
include $(LOCAL_PATH)/$(KEYMASTER_VERSION)/Android.mk
endif
else
$(error please define the TRUSTONIC_TEE_VERSION to be used by $(MTK_PLATFORM))
endif

endif
