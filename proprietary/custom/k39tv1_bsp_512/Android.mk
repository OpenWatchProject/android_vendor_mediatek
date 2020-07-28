LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k39tv1_bsp_512)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
