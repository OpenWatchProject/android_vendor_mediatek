#
# ImageIo_Test
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk

#
LOCAL_SRC_FILES := \
    main.cpp \
    main_ImageIo_Cam.cpp \

#
# Note: "/bionic" and "/external/stlport/stlport" is for stlport.
#LOCAL_C_INCLUDES +=  \
#    $(TOP)/bionic \
#    $(TOP)/external/stlport/stlport \



# vector
LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils

LOCAL_SHARED_LIBRARIES += libc++


# Imem/IspDrv/Isp_drv_p1/isp_drv_p2
LOCAL_SHARED_LIBRARIES +=  libimageio
LOCAL_SHARED_LIBRARIES +=  libcamdrv_isp
LOCAL_SHARED_LIBRARIES +=  libcamdrv_imem
LOCAL_SHARED_LIBRARIES += libimageio_plat_pipe
LOCAL_SHARED_LIBRARIES += libimageio_plat_drv
LOCAL_SHARED_LIBRARIES += libcamdrv_tuning_mgr

#LOCAL_SHARED_LIBRARIES +=  libfeatureiodrv

LOCAL_SHARED_LIBRARIES += \
	libmtkcam_stdutils libmtkcam_imgbuf

LOCAL_HEADER_LIBRARIES += libhardware_headers
LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := ImageIo_Test
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_PRELINK_MODULE := false

# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/drv


LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)/imageio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)/imageio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)/imageio/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)/imageio/inc/cam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)/imageio/pipe/inc




LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
# End of common part ---------------------------------------
#
include $(MTK_EXECUTABLE)


#
#include $(call all-makefiles-under,$(LOCAL_PATH))
