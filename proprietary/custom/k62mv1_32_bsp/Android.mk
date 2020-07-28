LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k62mv1_32_bsp)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
