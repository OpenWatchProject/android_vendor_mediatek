LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k69v1_64)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
