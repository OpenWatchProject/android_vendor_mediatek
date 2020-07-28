ifeq ($(TRUSTONIC_TEE_SUPPORT),yes)

ifeq ($(MTK_TEE_TRUSTED_UI_SUPPORT),yes)

LOCAL_PATH:= $(call my-dir)
include $(call all-makefiles-under,$(LOCAL_PATH))

endif

endif
