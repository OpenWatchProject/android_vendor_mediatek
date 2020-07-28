LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k80hd_bsp_fwv_512m)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
