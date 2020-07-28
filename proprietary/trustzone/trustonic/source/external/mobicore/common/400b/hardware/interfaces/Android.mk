ifeq ($(TRUSTONIC_TEE_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)

ifneq ($(TRUSTONIC_TEE_VERSION), 311c)
include $(LOCAL_PATH)/teeregistry/1.0/default/Android.mk
endif

endif
