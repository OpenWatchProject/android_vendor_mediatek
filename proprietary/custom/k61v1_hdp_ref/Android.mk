LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k61v1_hdp_ref)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
