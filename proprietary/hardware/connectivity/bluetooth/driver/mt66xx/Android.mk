ifeq ($(MTK_BT_SUPPORT), yes)
ifeq ($(filter MTK_MT76%, $(MTK_BT_CHIP)),)
include $(call all-subdir-makefiles)
endif
endif
