LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k37tv1_64_bsp)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
