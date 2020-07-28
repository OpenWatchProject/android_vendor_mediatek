#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

################################################################################
#
################################################################################
include $(LOCAL_PATH)/$(MTKCAM_DRV_DIP_VERSION_FOLDER)/Android.mk

