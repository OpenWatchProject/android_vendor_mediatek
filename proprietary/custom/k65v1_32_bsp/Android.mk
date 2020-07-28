LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k65v1_32_bsp)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
