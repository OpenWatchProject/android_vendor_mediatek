$(warning [FM]starting to build fm radio part...)

ifeq ($(MTK_FM_SUPPORT), yes)
LOCAL_PATH:= $(call my-dir)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
