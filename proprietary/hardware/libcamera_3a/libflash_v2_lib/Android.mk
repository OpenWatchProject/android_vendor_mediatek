# Auto-generated content. DO NOT EDIT.

LOCAL_PATH := $(call my-dir)

ifneq (,$(filter $(strip $(MTK_PLATFORM)), MT6771 MT6779 MT6785 MT6883 MT6885))
    $(info flash_v2)
    include $(call all-makefiles-under,$(LOCAL_PATH))
endif
