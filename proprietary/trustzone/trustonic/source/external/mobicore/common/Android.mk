ifeq ($(TRUSTONIC_TEE_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)

ifeq (,$(filter MT6582 MT6592, $(MTK_PLATFORM)))

ifeq (,$(TRUSTONIC_TEE_VERSION))
$(info mobicore - including $(LOCAL_PATH)/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/Android.mk)
include $(LOCAL_PATH)/$(MTK_PLATFORM)/Android.mk
else
$(info mobicore - including $(LOCAL_PATH)/$(TRUSTONIC_TEE_VERSION)/Android.mk)
include $(LOCAL_PATH)/$(TRUSTONIC_TEE_VERSION)/Android.mk
endif

else

# Please put mobicore source code in mobicore/platform/{platform} folder for ARMv7

endif

endif
