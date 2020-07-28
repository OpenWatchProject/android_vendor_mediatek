ifneq ($(strip $(MTK_BASIC_PACKAGE)),yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)


include $(call all-makefiles-under,$(LOCAL_PATH))

endif

