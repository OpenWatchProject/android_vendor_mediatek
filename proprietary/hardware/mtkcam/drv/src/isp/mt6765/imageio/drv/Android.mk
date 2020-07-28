
################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES +=  \
    cam/cam_dupCmdQMgr.cpp \
    cam/cam_path.cpp \
    cam/cam_path_pass1.cpp \
    cam/dip_CmdQMgr.cpp \
    cam/isp_function.cpp \
    cam/isp_function_cam.buf_ctrl.cpp \
    cam/isp_function_cam.cpp \
    cam/isp_function_cam.dmax.cpp \
    cam/isp_function_cam.pipechk.cpp \
    cam/isp_function_cam.YUV.cpp \
    cam/isp_function_cam.uni.cpp \
    cam/isp_function_cam.tuning.cpp \
    cam/isp_function_cam.twin.cpp \
    cam/isp_function_camsv.cpp \
    cam/isp_function_dip.cpp \
    cam/DipRingBuffer.cpp \
    cam/twin_mgr.cpp \
    cam/twin_mgr.buf_ctrl.cpp \
    cam/uni_Mgr.cpp \
    cam/rlb_mgr.cpp \
    cam/sec_mgr.cpp \
    cam/cam_capibility.cpp \
    cam/cam_dfs.cpp \
    cam/isp_function.TimeStamp.cpp


LOCAL_SRC_FILES += crz/crz_drv.cpp
LOCAL_SRC_FILES += mdp/mdp_mgr.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include


#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libccu_lib/mt6765/ver1/include
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_ext/ccu/mt6765/ver1/drv/include
#LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libccu_lib/mt6765/ver1/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)/imageio

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)/imageio
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)/imageio/inc
#cam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(MTKCAM_DRV_PLATFORM)/imageio/inc/cam


LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ldvt/$(TARGET_BOARD_PLATFORM)/include
#
#LOCAL_C_INCLUDES += $(TOP)/bionic
#LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libcam_p1cam_headers
#-----------------------------------------------------------
# systrace
LOCAL_C_INCLUDES += $(TOP)/system/core/include
#-----------------------------------------------------------
LOCAL_CFLAGS += -DMTK_CAMERA_IP_BASE
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)

#-----------------------------------------------------------
#LOCAL_WHOLE_STATIC_LIBRARIES +=
#LOCAL_STATIC_LIBRARIES += libcamdrv_tuning
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libcamdrv_tuning_mgr
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
#LOCAL_SHARED_LIBRARIES += libstlport
LOCAL_SHARED_LIBRARIES += libc++
LOCAL_SHARED_LIBRARIES += libdpframework
# systrace
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcamdrv_isp
LOCAL_SHARED_LIBRARIES += libcamdrv_imem
LOCAL_SHARED_LIBRARIES += libcamdrv_twin
LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
#CCU
#LOCAL_SHARED_LIBRARIES += lib3a.ccudrv
#//LOCAL_SHARED_LIBRARIES += lib3a.ccuif

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
endif


ifneq ($(BUILD_MTK_LDVT),yes)
    LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
endif

#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libimageio_plat_drv
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
#include $(MTK_STATIC_LIBRARY)
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))

