#Split build wrap
ifdef MTK_TARGET_PROJECT

LOCAL_PATH:= $(call my-dir)
include $(call all-makefiles-under, $(LOCAL_PATH))

endif #MTK_TARGET_PROJECT
