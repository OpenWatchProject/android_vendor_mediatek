LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), c7s)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
