LOCAL_PATH:= $(call my-dir)
ifeq ($(MTK_PROJECT), k68v1_2g_hdp)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
