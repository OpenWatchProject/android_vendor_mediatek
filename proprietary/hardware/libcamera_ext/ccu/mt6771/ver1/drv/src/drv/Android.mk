################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
#sinclude $(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------CCU options-------------------------
LOCAL_SRC_FILES += ccu_udrv.cpp
LOCAL_SRC_FILES += ccu_udrv_ipc.cpp
LOCAL_SRC_FILES += ccu_twin_drv.cpp
LOCAL_SRC_FILES += ../util/ccu_aee_mgr.cpp
LOCAL_SRC_FILES += ../util/ccu_drvutil.cpp

#Feature options
FEATURE_CFLAGS :=
FEATURE_CFLAGS += -DCCU_AF_ENABLE

ifeq ($(TARGET_BUILD_VARIANT), user)
FEATURE_CFLAGS += -DCCU_IS_USER_LOAD
endif

ifeq ($(TARGET_BUILD_VARIANT), userdebug)
FEATURE_CFLAGS += -DCCU_IS_USER_DEBUG_LOAD
endif

LIBCCU_DRV_SRC_PATH := $(MTK_PATH_SOURCE)/hardware/libcamera_ext/ccu/$(TARGET_BOARD_PLATFORM)/ver1

LOCAL_CFLAGS += $(FEATURE_CFLAGS)

# check if using trustonic TEE
ifeq ($(strip $(TRUSTONIC_TEE_SUPPORT)), yes)
LOCAL_CFLAGS += -DUSING_TRUSTONIC_TEE
endif
#-----------------------------------------------------------

LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/common/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/ext/include

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/isp_50
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/isp_50/acdk

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/isp_50/drv
LOCAL_C_INCLUDES += $(LIBCCU_DRV_SRC_PATH)/drv/include

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/isp_50/imageio
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include

#LOCAL_C_INCLUDES += $(LOCAL_PATH)/../inc
LOCAL_C_INCLUDES += $(LIBCCU_DRV_SRC_PATH)/drv/src/inc
LOCAL_C_INCLUDES += $(LIBCCU_DRV_SRC_PATH)/drv/src/util

#LOCAL_C_INCLUDES += bionic
#LOCAL_C_INCLUDES += external/stlport/stlport
#
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/kernel/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ldvt/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/aee/binary/inc

#//LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/m4u/$(MTK_PLATFORM)            <=capital
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/m4u/$(TARGET_BOARD_PLATFORM)
LOCAL_C_INCLUDES += device/mediatek/$(TARGET_BOARD_PLATFORM)/kernel-headers

LOCAL_C_INCLUDES += system/core/include

#for ESensorDev_T
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/
#for IHalSensor
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/

#for systrace
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_ext/lib/libutility/utilSystem

# ion/ion.h : standard ion api
#LOCAL_C_INCLUDES += system/core/libion/include

# libion_mtk/include/ion.h : path for mtk ion api
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external

# linux/ion.h : ioctl define for ion
LOCAL_C_INCLUDES += system/core/libion/kernel-headers

# linux/ion_drv.h :define for ion_mm_data_t
LOCAL_C_INCLUDES += device/mediatek/common/kernel-headers

#$(warning $(LOCAL_C_INCLUDES))
#
#-----------------------------------------------------------
ifeq ($(TARGET_BUILD_VARIANT), user)
MTKCAM_LOGENABLE_DEFAULT   := 0
else
MTKCAM_LOGENABLE_DEFAULT   := 1
endif

#-----------------------------------------------------------
LOCAL_CFLAGS += -DMTKCAM_LOGENABLE_DEFAULT=$(MTKCAM_LOGENABLE_DEFAULT)
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if this line needed in other module, it
# have to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif
ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=
LOCAL_STATIC_LIBRARIES += lib3a.ccu.headers

ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

LOCAL_STATIC_LIBRARIES += 

LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libcamalgo.utility

ifeq ($(MTK_ION_SUPPORT),yes)
	LOCAL_SHARED_LIBRARIES += libion libion_mtk
endif

LOCAL_SHARED_LIBRARIES += libdl

ifneq ($(BUILD_MTK_LDVT),yes)
    LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
endif
#-----------------------------------------------------------
LOCAL_MODULE := lib3a.ccudrv
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_SHARED_LIBRARY)
#-----------------------------------------------------------


