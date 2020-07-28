#
# libimageio
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk
#-----------------------------------------------------------
LOCAL_SRC_FILES +=
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=

#-----------------------------------------------------------
#LOCAL_SHARED_LIBRARIES += libdip_drv
#LOCAL_SHARED_LIBRARIES += libdip_function
#LOCAL_SHARED_LIBRARIES += libdip_postproc

#-----------------------------------------------------------
#LOCAL_MODULE := libdip
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
#LOCAL_PRELINK_MODULE := false
#LOCAL_MODULE_TAGS := optional
#include $(MTK_SHARED_LIBRARY)

################################################################################
#
################################################################################
#include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
