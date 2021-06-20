LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), c10)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
