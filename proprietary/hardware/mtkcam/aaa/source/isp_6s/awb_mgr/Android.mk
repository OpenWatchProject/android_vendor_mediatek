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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/aaa.mk

$(info MTKCAM_AAA_PLATFORM=$(MTKCAM_AAA_PLATFORM))

#MTKCAM_3A_PATH       := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(TARGET_BOARD_PLATFORM)
MTKCAM_3A_PATH        := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(MTKCAM_AAA_PLATFORM)
MTKCAM_3A_COMMON_PATH := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common
RELATIVE_PATH_MTKCAM_3A_COMMON  := ../../common


#-----------------------------------------------------------

LOCAL_SRC_FILES += awb_mgr.cpp
LOCAL_SRC_FILES += awb_cct_feature.cpp
LOCAL_SRC_FILES += awb_state.cpp
LOCAL_SRC_FILES += awb_mgr_if.cpp
LOCAL_SRC_FILES += awb_cb.cpp
LOCAL_SRC_FILES += isp_awb_config.cpp
LOCAL_SRC_FILES += ../sensor_mgr/aaa_sensor_mgr.cpp
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/iopipe/NormalPipeUtils.cpp

#LOCAL_SRC_FILES += buf_mgr/AAOBufMgr.cpp
#LOCAL_SRC_FILES += buf_mgr/PSOBufMgr.cpp
#LOCAL_SRC_FILES += buf_mgr/AEOBufMgr.cpp
#LOCAL_SRC_FILES += buf_mgr/AFOBufMgr.cpp
#LOCAL_SRC_FILES += buf_mgr/FLKOBufMgr.cpp
#LOCAL_SRC_FILES += buf_mgr/PDOBufMgr.cpp
#LOCAL_SRC_FILES += buf_mgr/MVHDRBufMgr.cpp
#LOCAL_SRC_FILES += buf_mgr/CamsvBufMgr.cpp
#LOCAL_SRC_FILES += buf_mgr/TSFOBufMgr.cpp
#LOCAL_SRC_FILES += buf_mgr/AAOSeparation12.cpp
#LOCAL_SRC_FILES += buf_mgr/AAOSeparation14.cpp
#LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/iopipe/NormalPipeUtils.cpp

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTKCAM_INCLUDE)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_PLATFORM)/drv
LOCAL_C_INCLUDES += $(MTKCAM_DRV_INCLUDE)

LOCAL_C_INCLUDES += $(MTKCAM_ACDK)
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libflicker
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libgma_lib/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/liblce_lib/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libispfeature_lib/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include

#for CCU
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libccu_lib/$(TARGET_BOARD_PLATFORM)/ver1/include
LOCAL_SHARED_LIBRARIES += lib3a.ccudrv
LOCAL_SHARED_LIBRARIES += lib3a.ccuif

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/EventIrq
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/ae_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/awb_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/buf_mgr

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/flash_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_config
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/sensor_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/wrapper
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/flash_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/wrapper
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/Hal3ARefBase
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/Hal3ARefBase/buf_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/ResultPool
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/hal3a/$(MTKCAM_COMMON_HAL3A_VERSION)/Hal3ARefBase/EventIrq
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include/
LOCAL_C_INCLUDES += $(TOP)/device/mediatek/common/kernel-headers
LOCAL_C_INCLUDES += $(TOP)/vendor/mediatek/proprietary/external/libudf/libladder
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/pd_buf_mgr
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/mtkcam
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/cam
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/camera_3a/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libaf_core_lib/$(MTK_PLATFORM_DIR)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libawb_core_lib/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libflash_lib/$(TARGET_BOARD_PLATFORM)/include
#For Custom AE algo
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a/customae_lib/include
#For AEPlineTable.h
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/custom
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/custom/aaa
#For BuildInTypes.h
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/def
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libn3d3a_lib/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libflicker_lib/$(TARGET_BOARD_PLATFORM)/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc
#capturenr
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/perfservice/perfservicenative
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libvpunr_lib/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libflash_lib/$(PLATFORM)/include

LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/cct_svr
LOCAL_C_INCLUDES += $(MTKCAM_3A_COMMON_PATH)/utils/aaa_utils

#### Include
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/af_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/lens
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_config
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/af_mgr/CxUAF
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

#ISP 5.0
$(info "MTKCAM_IS_ISP_30_PLATFORM=$(MTKCAM_IS_ISP_30_PLATFORM)")
$(info "MTKCAM_IS_ISP_40_PLATFORM=$(MTKCAM_IS_ISP_40_PLATFORM)")
$(info "MTKCAM_IS_ISP_50_PLATFORM=$(MTKCAM_IS_ISP_50_PLATFORM)")
$(info "MTKCAM_IS_IP_BASE=$(MTKCAM_IS_IP_BASE)")
LOCAL_CFLAGS += -DCAM3_3A_ISP_30_EN=$(MTKCAM_IS_ISP_30_PLATFORM)
LOCAL_CFLAGS += -DCAM3_3A_ISP_40_EN=$(MTKCAM_IS_ISP_40_PLATFORM)
LOCAL_CFLAGS += -DCAM3_3A_ISP_50_EN=$(MTKCAM_IS_ISP_50_PLATFORM)

LOCAL_CFLAGS += -DCAM3_3A_IP_BASE=$(MTKCAM_IS_IP_BASE)

LOCAL_CFLAGS += -DUSE_AE_THD=1
LOCAL_CFLAGS += -DCAM3_3ATESTLVL=1

LOCAL_CFLAGS += -DCAM3_3AUT=0
LOCAL_CFLAGS += -DCAM3_3ASTTUT=1
LOCAL_CFLAGS += -DCAM3_3AIT=2
LOCAL_CFLAGS += -DCAM3_3AON=3

LOCAL_CFLAGS += -DCAM3_AF_FEATURE_EN=1
LOCAL_CFLAGS += -DCAM3_LSC_FEATURE_EN=1
LOCAL_CFLAGS += -DCAM3_FLASH_FEATURE_EN=1
LOCAL_CFLAGS += -DCAM3_HIGH_BRIGHTNESS_FLOW_EN=1
LOCAL_CFLAGS += -DCAM3_FLICKER_FEATURE_EN=1
LOCAL_CFLAGS += -DCAM3_LSC2_USE_GAIN=0
LOCAL_CFLAGS += -DCAM3_AE_INTERRUPT_EN=1
LOCAL_CFLAGS += -DCAM3_AAO_NEON_EN=1
LOCAL_CFLAGS += -DCAM3_PSO_NEON_EN=1

ifeq ($(TARGET_BUILD_VARIANT), user)
    LOCAL_CFLAGS += -DAE_PERFRAME_EN=1
else ifeq ($(TARGET_BUILD_VARIANT), userdebug)
    LOCAL_CFLAGS += -DAE_PERFRAME_EN=1
else
    LOCAL_CFLAGS += -DAE_PERFRAME_EN=0
endif

ifeq ($(strip $(MTK_NATIVE_3D_SUPPORT)),yes)
    LOCAL_CFLAGS += -DMTK_NATIVE_3D_SUPPORT
endif


#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libladder
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.nvram.50

LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libmtkcam_mapping_mgr
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libcam.utils.sensorprovider

LOCAL_SHARED_LIBRARIES += lib3a.awb
LOCAL_SHARED_LIBRARIES += lib3a.awb.core
LOCAL_SHARED_LIBRARIES += lib3a.ae.core
#LOCAL_SHARED_LIBRARIES += lib3a.custom.ae
LOCAL_SHARED_LIBRARIES += lib3a.flash

LOCAL_STATIC_LIBRARIES += libcam.hal3a_utils

LOCAL_HEADER_LIBRARIES += libhardware_headers libandroid_sensor_headers
LOCAL_HEADER_LIBRARIES += libcutils_headers libutils_headers libsystem_headers
#LOCAL_HEADER_LIBRARIES += libsystem_headers

LOCAL_MODULE := libcam.hal3a.v3.awb
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#--------------------------------------------------------------------------
include $(MTK_SHARED_LIBRARY)
#########include $(MTK_STATIC_LIBRARY)

################################################################################
#
################################################################################
#include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

