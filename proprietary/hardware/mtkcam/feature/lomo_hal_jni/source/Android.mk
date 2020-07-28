
LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk
#------------iopipeFrmBtest----------------------------
#-----------------------------------------------------------
LOCAL_SRC_FILES += lomo_hal_jni.cpp

# -----------extra setting: start
V2_SUPPORT_PLATFORM := mt6799 mt6759 mt6763 mt6758 mt6771 mt6775 mt6765
#V2_SUPPORT_PLATFORM := mt6757
ifneq (, $(filter $(V2_SUPPORT_PLATFORM),$(TARGET_BOARD_PLATFORM)))
  LOMO_VDIR := v2
else
  LOMO_VDIR := v1
endif
$(warning PPP_PLATFORM=$(TARGET_BOARD_PLATFORM))
$(warning LLL_LOMO_VDIR=$(LOMO_VDIR))

LOCAL_SRC_FILES += $(LOMO_VDIR)/apply_tuning_setting.cpp
# -----------extra setting: end

#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libispfeature_lib/$(PLATFORM)/include/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libawb_core_lib/$(TARGET_BOARD_PLATFORM)/include

#-----------------------------------------------------------
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/lomo
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/lomo_jni
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libflash_lib/$(PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libaf_core_lib/$(TARGET_BOARD_PLATFORM)/include

#feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/feature/lomo_hal_jni/include
#LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/lomo_hal_jni
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/custom

#for NSIspTuningv3<
# temp for using iopipe>

LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

# TODO: need to remove for ipbase
LOCAL_C_INCLUDES += $(MTKCAM_DRV_INCLUDE)

LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)
# temp for using iopipe<

#for camera_vendor_tags.h:
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

#for ipbase 5.x

ifeq ($(MTKCAM_P2_DECOUPLE), 1)
    DIP_REG_NEEDED := 1
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/dip/$(MTKCAM_DRV_PLATFORM)
else
    DIP_REG_NEEDED := 0
endif
MTKCAM_CFLAGS += -DDIP_REG_NEEDED=$(DIP_REG_NEEDED)

ifeq ($(MTKCAM_DRV_ISP_VERSION), 30)
    MTKCAM_CFLAGS += -DMTKCAM_DRV_ISP_VERSION=30
endif
#-----------------------------------------------------------
#-----------------------------------------------------------
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libcam.iopipe
#For AE
LOCAL_SHARED_LIBRARIES += lib3a.ae.core

#-----------------------------------------------------------
# for feature stream
LOCAL_SHARED_LIBRARIES += libcameracustom
# remove aft evrest // LOCAL_SHARED_LIBRARIES += libfeatureiodrv
LOCAL_SHARED_LIBRARIES += libdpframework
# for gralloc
LOCAL_SHARED_LIBRARIES += libgralloc_extra
# for v3>
# for v3<

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#-----------------------------------------------------------
LOCAL_MODULE := libcam.jni.lomohaljni
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
#include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

