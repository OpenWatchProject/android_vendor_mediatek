LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k37mv1_bsp)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
