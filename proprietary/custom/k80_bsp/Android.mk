LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k80_bsp)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
