LOCAL_PATH := $(call my-dir)

# MTK internal solution, only for BSP load
ifeq ($(MTK_BSP_PACKAGE) , yes)
include $(CLEAR_VARS)

# Include all makefiles in subdirectories
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
