# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.
#
# BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
# THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
# RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
# AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
# NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
# SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
# SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
# THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
# THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
# CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
# SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
# STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
# CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
# AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
# OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
# MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
#
# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.

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
-include $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/aaa.mk


ifneq (,$(filter $(strip $(MTKCAM_AAA_PLATFORM)), isp_30 isp_40 isp_50))
    MTKCAM_USE_ISP_50_FLOW := 1
else
    MTKCAM_USE_ISP_50_FLOW := 0
endif

#Begin
ifeq ($(MTKCAM_USE_ISP_50_FLOW),1)
$(info "MTKCAM_AAA_PLATFORM_111=$(MTKCAM_AAA_PLATFORM)")

MTKCAM_3A_COMMON_PATH := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common
MTKCAM_3A_PATH        := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(MTKCAM_AAA_PLATFORM)
MTKCAM_3A_PLATFORM_PATH := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(TARGET_BOARD_PLATFORM)

ifeq ($(TARGET_BOARD_PLATFORM), mt6757)
ifeq ($(strip $(MTK_CAM_SW_VERSION)),ver2)
MTKCAM_3A_PLATFORM_PATH := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(TARGET_BOARD_PLATFORM)p
endif
endif


ifneq (,$(filter $(strip $(MTKCAM_AAA_PLATFORM)), isp_30 isp_40))
    MTKCAM_DRV_PATH       := $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)
else
    MTKCAM_DRV_PATH       := $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_AAA_PLATFORM)
endif

$(info "========================COMMON.MK=============================")
$(info "TOP=$(TOP)")
$(info "MTKCAM_3A_PATH=$(MTKCAM_3A_PATH)")
$(info "MTKCAM_3A_PLATFORM_PATH=$(MTKCAM_3A_PLATFORM_PATH)")
$(info "MTK_PATH_SOURCE=$(MTK_PATH_SOURCE)")
$(info "MTK_PATH_COMMON=$(MTK_PATH_COMMON)")
$(info "MTK_PATH_CUSTOM=$(MTK_PATH_CUSTOM)")
$(info "MTKCAM_C_INCLUDES=$(MTKCAM_C_INCLUDES)")
$(info "MTKCAM_ALGO_INCLUDE=$(MTKCAM_ALGO_INCLUDE)")
$(info "MTK_PATH_CUSTOM_PLATFORM=$(MTK_PATH_CUSTOM_PLATFORM)")
$(info "TARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)")
$(info "RELATIVE_PATH_MTKCAM_3A_COMMON=$(RELATIVE_PATH_MTKCAM_3A_COMMON)")
$(info "========================COMMON.MK=============================")

#-----------------------------------------------------------
LOCAL_SRC_FILES += Hal3APolicy.cpp
LOCAL_SRC_FILES += Hal3ASimulator.cpp
LOCAL_SRC_FILES += Hal3AAdapter3.cpp
LOCAL_SRC_FILES += Hal3AAdapter3Meta.cpp
LOCAL_SRC_FILES += IHal3A.cpp
LOCAL_SRC_FILES += aaa_hal_if.cpp
LOCAL_SRC_FILES += Hal3AFlowCtrl.cpp
LOCAL_SRC_FILES += Hal3AFlowCtrl_SMVR.cpp
LOCAL_SRC_FILES += Thread3AImp.cpp
LOCAL_SRC_FILES += wrapper/Dft3AImp.cpp
LOCAL_SRC_FILES += wrapper/I3AWrapper.cpp
LOCAL_SRC_FILES += buf_mgr/IBufMgr.cpp
LOCAL_SRC_FILES += buf_mgr/StatisticBuf.cpp
LOCAL_SRC_FILES += EventIrq/DefaultEventIrq.cpp
LOCAL_SRC_FILES += EventIrq/HwEventIrq.cpp
LOCAL_SRC_FILES += debug/IDebugEntry.cpp
LOCAL_SRC_FILES += debug/DebugUtil.cpp
LOCAL_SRC_FILES += ResultPool/ResultPool.cpp
LOCAL_SRC_FILES += ResultPool/Entry.cpp
LOCAL_SRC_FILES += ResultPool/ThreadResultPool.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/wrapper
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/buf_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/ResultPool
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/buf_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/EventIrq
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/wrapper
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libfdft
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/pd_buf_mgr
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/mtkcam
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/cam
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a/inc

ifeq ($(MTKCAM_AAA_PLATFORM), isp_30)
    $(info This is isp_30)
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/buf_mgr/StatisticPipeWrapper/$(TARGET_BOARD_PLATFORM)
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_mgr/$(TARGET_BOARD_PLATFORM)
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/ispdrv_mgr/$(TARGET_BOARD_PLATFORM)
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/$(TARGET_BOARD_PLATFORM)
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/$(TARGET_BOARD_PLATFORM)/paramctrl/inc
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/$(TARGET_BOARD_PLATFORM)/paramctrl/pca_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/$(TARGET_BOARD_PLATFORM)/paramctrl/ccm_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/$(TARGET_BOARD_PLATFORM)/paramctrl/ggm_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/$(TARGET_BOARD_PLATFORM)/paramctrl/gma_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/$(TARGET_BOARD_PLATFORM)/paramctrl/lce_mgr
else ifeq ($(MTKCAM_AAA_PLATFORM) $(TARGET_BOARD_PLATFORM), isp_40 $(filter $(TARGET_BOARD_PLATFORM), mt6757 mt6763 mt6765))
    # if isp_40 && (mt6765 || mt6763)
    $(info This is isp_40 && mt6765)
    LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PLATFORM_PATH)/isp_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PLATFORM_PATH)/ispdrv_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PLATFORM_PATH)/isp_tuning
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PLATFORM_PATH)/isp_tuning/paramctrl/inc
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PLATFORM_PATH)/isp_tuning/paramctrl/pca_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PLATFORM_PATH)/isp_tuning/paramctrl/ccm_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PLATFORM_PATH)/isp_tuning/paramctrl/ggm_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PLATFORM_PATH)/isp_tuning/paramctrl/gma_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PLATFORM_PATH)/isp_tuning/paramctrl/lce_mgr
else
    $(info This is else)
    LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/ispdrv_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/inc
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/pca_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/ccm_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/ggm_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/gma_mgr
    LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning/paramctrl/lce_mgr
endif

LOCAL_C_INCLUDES += $(MTKCAM_DRV_PATH)/drv
LOCAL_C_INCLUDES += $(MTKCAM_DRV_INCLUDE)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/

LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include/
LOCAL_C_INCLUDES += $(TOP)/device/mediatek/common/kernel-headers
LOCAL_C_INCLUDES += $(TOP)/vendor/mediatek/proprietary/external/libudf/libladder
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera_3a/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libn3d3a_lib/$(TARGET_BOARD_PLATFORM)/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc
#capturenr
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/perfservice/perfservicenative
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libnr_lib/$(TARGET_BOARD_PLATFORM)/include

LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/cct_svr
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/utils/aaa_utils

#For AEPlineTable.h
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/custom
ifeq ($(MTKCAM_IS_ISP_40_PLATFORM_MIX_VER_1), 0)
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/custom/aaa
endif
#For BuildInTypes.h
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/def

#-----------------------------------------------------------
ifeq ($(MTKCAM_IS_CCU_EN), 1)
LOCAL_SHARED_LIBRARIES += lib3a.ccudrv
LOCAL_SHARED_LIBRARIES += lib3a.ccuif
LOCAL_STATIC_LIBRARIES += lib3a.ccu.headers
endif
LOCAL_SHARED_LIBRARIES += libfeatureiodrv_mem

$(info "MTKCAM_DRV_PATH_DRV=$(MTKCAM_DRV_PATH)")

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
LOCAL_CFLAGS += -DCAM3_3A_ISP_30_EN=$(MTKCAM_IS_ISP_30_PLATFORM)
LOCAL_CFLAGS += -DCAM3_3A_ISP_40_EN=$(MTKCAM_IS_ISP_40_PLATFORM)
LOCAL_CFLAGS += -DCAM3_3A_ISP_50_EN=$(MTKCAM_IS_ISP_50_PLATFORM)
LOCAL_CFLAGS += -DCAM3_3A_IP_BASE=$(MTKCAM_IS_IP_BASE)
LOCAL_CFLAGS += -DCAM3_3A_ISP_40_PLATFORM_MIX_VER_1=$(MTKCAM_IS_ISP_40_PLATFORM_MIX_VER_1)
#
ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif

LOCAL_CFLAGS += -DCAM3_DEFAULT_ISP=0
LOCAL_CFLAGS += -DCAM3_DISABLE_SHADING=0

ifeq ($(USE_CAM3_FAKE_SENSOR_DRV),yes)
    LOCAL_SRC_FILES += fake_sensor_drv/NormalPipe.cpp
    LOCAL_SRC_FILES += fake_sensor_drv/StatisticPipe.cpp
    LOCAL_SRC_FILES += fake_sensor_drv/CamsvStatisticPipe.cpp
    LOCAL_SRC_FILES += fake_sensor_drv/NormalPipeModule.cpp
    LOCAL_CFLAGS += -DCAM3_FAKE_SENSOR_DRV=1
    LOCAL_CFLAGS += -DCAM3_DISABLE_AF_GSENSOR=1
    LOCAL_CFLAGS += -DCAM3_DISABLE_AE_PER_FRAME=1
else
    LOCAL_CFLAGS += -DCAM3_FAKE_SENSOR_DRV=0
    LOCAL_CFLAGS += -DCAM3_DISABLE_AF_GSENSOR=0
    LOCAL_CFLAGS += -DCAM3_DISABLE_AE_PER_FRAME=0
endif

LOCAL_CFLAGS += -DUSE_AE_THD=1
LOCAL_CFLAGS += -DCAM3_3ATESTLVL=1

LOCAL_CFLAGS += -DCAM3_3AUT=0
LOCAL_CFLAGS += -DCAM3_3ASTTUT=1
LOCAL_CFLAGS += -DCAM3_3AIT=2
LOCAL_CFLAGS += -DCAM3_3AON=3

LOCAL_CFLAGS += -DCAM3_AF_FEATURE_EN=1
LOCAL_CFLAGS += -DCAM3_LSC_FEATURE_EN=1
LOCAL_CFLAGS += -DCAM3_FLASH_FEATURE_EN=1
LOCAL_CFLAGS += -DCAM3_FLICKER_FEATURE_EN=1
LOCAL_CFLAGS += -DCAM3_LSC2_USE_GAIN=0
LOCAL_CFLAGS += -DCAM3_AE_INTERRUPT_EN=1
LOCAL_CFLAGS += -DCAM3_AAO_NEON_EN=1
LOCAL_CFLAGS += -DCAM3_PSO_NEON_EN=1

ifeq ($(TARGET_BUILD_VARIANT), user)
    LOCAL_CFLAGS += -DAE_PERFRAME_EN=1
    LOCAL_CFLAGS += -DIS_BUILD_USER=1
    LOCAL_CFLAGS += -DMTK_SUPPORT_ENG=0
else ifeq ($(TARGET_BUILD_VARIANT), userdebug)
    LOCAL_CFLAGS += -DAE_PERFRAME_EN=1
    LOCAL_CFLAGS += -DIS_BUILD_USER=0
    LOCAL_CFLAGS += -DMTK_SUPPORT_ENG=0
else
    LOCAL_CFLAGS += -DAE_PERFRAME_EN=0
    LOCAL_CFLAGS += -DIS_BUILD_USER=0
    LOCAL_CFLAGS += -DMTK_SUPPORT_ENG=1
endif

ifeq ($(strip $(MTK_NATIVE_3D_SUPPORT)),yes)
    LOCAL_CFLAGS += -DMTK_NATIVE_3D_SUPPORT
endif

# for Stereo Feature
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT),yes)
    LOCAL_CFLAGS += -DCAM3_STEREO_FEATURE_EN=1
endif

ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT), yes)
MTKCAM_STEREO_DENOISE_SUPPORT   := '1'
else
MTKCAM_STEREO_DENOISE_SUPPORT   := '0'
endif
LOCAL_CFLAGS += -DMTKCAM_STEREO_DENOISE_SUPPORT="$(MTKCAM_STEREO_DENOISE_SUPPORT)"
endif

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libladder
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
LOCAL_SHARED_LIBRARIES += lib3a.awb.core
LOCAL_SHARED_LIBRARIES += lib3a.af.core
LOCAL_SHARED_LIBRARIES += libcamalgo.fdft
LOCAL_SHARED_LIBRARIES += lib3a.ae.core

ifeq ($(MTKCAM_IS_LEGACY_ALGO), 1)
    LOCAL_SHARED_LIBRARIES += lib3a
    LOCAL_SHARED_LIBRARIES += libcamalgo
else
    LOCAL_SHARED_LIBRARIES += lib3a.gma lib3a.lce

ifneq ("$(wildcard $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/liblsc_lib/libcamalgo.lsc/$(TARGET_BOARD_PLATFORM)/arm/libcamalgo.lsc.so)","")
    LOCAL_SHARED_LIBRARIES += libcamalgo.lsc
else
    LOCAL_SHARED_LIBRARIES += libcamalgo
endif
ifneq ("$(wildcard $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libflicker_lib/libcamalgo.flicker/$(TARGET_BOARD_PLATFORM)/arm/libcamalgo.flicker.so)","")
    LOCAL_SHARED_LIBRARIES += libcamalgo.flicker
else
    LOCAL_SHARED_LIBRARIES += libcamalgo
endif

    #capturenr
    LOCAL_SHARED_LIBRARIES += libcamalgo.nr
endif

LOCAL_SHARED_LIBRARIES += libcam3a_imem
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils

ifeq ($(USE_CAM3_FAKE_SENSOR_DRV),no)
LOCAL_SHARED_LIBRARIES += libcam.iopipe
endif

ifneq (,$(filter $(strip $(MTKCAM_AAA_PLATFORM)), isp_30 isp_40))
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.nvram
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.lsctbl
else
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.nvram.50
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.lsctbl.50
endif

LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libcamdrv_tuning_mgr
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += lib3a.flash

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
endif
#

else
$(info "MTKCAM_AAA_PLATFORM_222=$(MTKCAM_AAA_PLATFORM)")

#ISP 4.X
#-----------------------------------------------------------
LOCAL_SRC_FILES += Hal3ASimulator.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
endif


endif

LOCAL_HEADER_LIBRARIES := libcutils_headers libutils_headers libsystem_headers libhardware_headers libcustom_isp_headers
LOCAL_SHARED_LIBRARIES += libcutils libutils libhardware

#-----------------------------------------------------------
LOCAL_MODULE := libcam.hal3a
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

