LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k61v1_64_bsp)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
