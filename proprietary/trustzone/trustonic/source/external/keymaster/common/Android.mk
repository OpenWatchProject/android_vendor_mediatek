ifeq ($(TRUSTONIC_TEE_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)

ifeq (,$(TRUSTONIC_TEE_VERSION))
$(info keymaster - including $(LOCAL_PATH)/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/Android.mk)
include $(LOCAL_PATH)/$(MTK_PLATFORM)/Android.mk
else
$(info keymaster - including $(LOCAL_PATH)/$(TRUSTONIC_TEE_VERSION)/Android.mk)
include $(LOCAL_PATH)/$(TRUSTONIC_TEE_VERSION)/Android.mk
endif

endif
