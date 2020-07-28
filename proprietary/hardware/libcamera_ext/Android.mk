#Split build wrap
ifdef MTK_TARGET_PROJECT

ifneq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)

LOCAL_PATH:= $(call my-dir)
include $(call all-makefiles-under,$(LOCAL_PATH))

endif

else #MTK_TARGET_PROJECT

LOCAL_PATH:= $(call my-dir)
include $(LOCAL_PATH)/lib/libutility/Android.mk

endif #MTK_TARGET_PROJECT
