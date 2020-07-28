#
# tuningTest
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
#-----------------------------------------------------------
LOCAL_SRC_FILES += main.cpp main_tuningmgr.cpp 

#
# Note: "/bionic" and "/external/stlport/stlport" is for stlport.
#LOCAL_C_INCLUDES += $(TOP)/bionic
#LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport
#
#Thread Priority
LOCAL_C_INCLUDES += $(TOP)/system/core/include
#utility
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include

#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core/drv/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/mtkcam/core


#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------


LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
#
#for camera_vendor_tags.h:
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

#
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libmtkcam_stdutils libmtkcam_imgbuf \

LOCAL_SHARED_LIBRARIES += libc++

LOCAL_SHARED_LIBRARIES  += liblog
LOCAL_SHARED_LIBRARIES  += libcamdrv_isp
LOCAL_SHARED_LIBRARIES  += libcamdrv_tuning_mgr

#
LOCAL_STATIC_LIBRARIES := \

#
LOCAL_WHOLE_STATIC_LIBRARIES := \

#
LOCAL_MODULE := tuningTestIP
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk


#
LOCAL_PRELINK_MODULE := false

#
# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

MTKCAM_CFLAGS += -DL1_CACHE_BYTES=32
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)/drv

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/common/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)/inc

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/Cam

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_PLATFORM)/hardware/include

# End of common part ---------------------------------------
#
include $(MTK_EXECUTABLE)


#
include $(call all-makefiles-under,$(LOCAL_PATH))
