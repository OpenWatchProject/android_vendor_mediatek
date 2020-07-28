ifeq ($(TRUSTONIC_TEE_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)

ifneq (,$(TRUSTONIC_TEE_VERSION))

ifeq (,$(filter MT6735 MT6737 MT6753 MT6755 MT6757, $(MTK_PLATFORM)))
$(info gatekeeper - including $(LOCAL_PATH)/$(TRUSTONIC_TEE_VERSION)/Android.mk)
include $(LOCAL_PATH)/$(TRUSTONIC_TEE_VERSION)/Android.mk
else
$(info gatekeeper - including $(LOCAL_PATH)/302d/Android.mk)
include $(LOCAL_PATH)/302d/Android.mk
endif

else
$(error please define the TRUSTONIC_TEE_VERSION to be used by $(MTK_PLATFORM))
endif

endif
