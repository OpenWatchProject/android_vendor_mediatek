LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k71v1_bsp_2g)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
