
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
#-----------------------------------------------------------
LOCAL_SRC_FILES += test_3a_main.cpp
LOCAL_SRC_FILES += test_3a_runner.cpp
LOCAL_SRC_FILES += test_3a/test_3a.cpp
LOCAL_SRC_FILES += test_3a/case_3a.cpp
LOCAL_SRC_FILES += test_af/test_af.cpp
LOCAL_SRC_FILES += test_af/case_af.cpp
# LOCAL_SRC_FILES += test_3a_statistic.cpp
MTKCAM_3A_PATH    := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(TARGET_BOARD_PLATFORM)

#------------------ip base new include path-----------------------
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include

LOCAL_C_INCLUDES += $(MTKCAM_INCLUDE)

LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libflicker
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/lib3a
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libfdft
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libsync3a

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/EventIrq
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/state_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/state_mgr_n3d
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/awb_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/af_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/pd_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/laser_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/ae_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/flash_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/flicker
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/nvram_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/buf_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/ispdrv_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/inc
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/pca_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/ccm_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/ggm_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/gma_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/lce_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/lsc_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/sensor_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/ResultBufMgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/wrapper

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/nvram/libnvram/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/nvram/nvramagentclient
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include/
LOCAL_C_INCLUDES += $(TOP)/device/mediatek/common/kernel-headers
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/pd_buf_mgr
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/mtkcam
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/cam
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera_3a/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a/inc

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc
#
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
#LOCAL_WHOLE_STATIC_LIBRARIES +=
#
#LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
# vector
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libcutils \
    libutils \
    libmtkcam_metadata \
    libmtkcam_metastore

LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libcam.halsensor
LOCAL_SHARED_LIBRARIES += libcamdrv_imem
LOCAL_SHARED_LIBRARIES += libcamdrv_isp
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils
#LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.nvram
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.lsctbl
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3

LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libcamdrv_tuning_mgr

#-----------------------------------------------------------

LOCAL_MODULE := test_3a_main
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_EXECUTABLE)
