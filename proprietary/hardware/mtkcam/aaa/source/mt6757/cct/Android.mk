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

#-----------------------------------------------------------
MTKCAM_3A_PATH    := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/$(TARGET_BOARD_PLATFORM)

BUILD_CAM3_ISP_CCT := yes

#-----------------------------------------------------------
# Add a define value that can be used in the code to indicate that it's using LDVT now.
# For print message function when using LDVT.
# Note: It will be cleared by "CLEAR_VARS", so if it is needed in other module, it
# has to be set in other module again.
ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ldvt/$(TARGET_BOARD_PLATFORM)/include/
endif

#-----------------------------------------------------------
LOCAL_SRC_FILES += if/cct_feature.cpp
LOCAL_SRC_FILES += if/cct_if.cpp
LOCAL_SRC_FILES += if/cct_main.cpp
LOCAL_SRC_FILES += if/cct_nvram_feature.cpp
LOCAL_SRC_FILES += if/cct_sensor_feature.cpp
LOCAL_SRC_FILES += if/cct_isp_feature.cpp

LOCAL_SRC_FILES += calibration/cct_calibration.cpp
LOCAL_SRC_FILES += calibration/cct_flash_cali.cpp
LOCAL_SRC_FILES += calibration/cct_flash_util.cpp
LOCAL_SRC_FILES += calibration/cct_imgtool.cpp
#LOCAL_SRC_FILES += calibration/lsc_calibration.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/main/acdk
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/lib3a
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/lib3a/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libaf_core_lib/$(MTK_PLATFORM_DIR)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libgma_lib/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/liblce_lib/$(TARGET_BOARD_PLATFORM)/include
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include/cct
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/main/acdk/$(TARGET_BOARD_PLATFORM)/inc/cctia
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/main/acdk/$(TARGET_BOARD_PLATFORM)/inc/acdk
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(TARGET_BOARD_PLATFORM)/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/isp/$(TARGET_BOARD_PLATFORM)/tuning/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/src/sensor/$(TARGET_BOARD_PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libispfeature_lib/$(TARGET_BOARD_PLATFORM)/include/$(MTK_CAM_SW_VERSION)
#
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_tuning
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/buf_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/flash_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/isp_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/ispdrv_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/lsc_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/nvram_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/awb_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/af_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/ae_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/sensor_mgr
LOCAL_C_INCLUDES += $(MTKCAM_3A_PATH)/ResultBufMgr
#
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/acdk/$(MTK_CAM_SW_VERSION)/

LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/cam
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/cam/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
#
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc/aaa/debug_exif/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc/aaa/debug_exif/cam
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc/isp_tuning
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/video
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
#

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

ifeq ($(BUILD_CAM3_ISP_CCT),yes)
    LOCAL_CFLAGS += -DCAM3_ISP_CCT_EN=1
else
    LOCAL_CFLAGS += -DCAM3_ISP_CCT_EN=0
endif

#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES :=

ifeq ($(BUILD_MTK_LDVT),true)
   LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

#-----------------------------------------------------------
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcct
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)
#include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
RELATIVE_PATH_MTKCAM_3A_COMMON  := ../../common

#-----------------------------------------------------------
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/cct/cctia/AcdkCctBase.cpp
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/cct/cctia/AcdkCctMain.cpp
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/cct/cctia/AcdkCctMhalBase.cpp
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/cct/cctia/AcdkCctMhalEng.cpp
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/cct/cctia/AcdkCctMhalEng2.cpp
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/cct/cctia/AcdkCctMhalPure.cpp
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/cct/cctia/AcdkUtility.cpp
#
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/cct/surfaceview/AcdkSurfaceView.cpp
LOCAL_SRC_FILES += $(RELATIVE_PATH_MTKCAM_3A_COMMON)/cct/surfaceview/surfaceView.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include/cct

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)/drv

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/drivers/video
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/m4u/$(TARGET_BOARD_PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/jpeg/include

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

#LOCAL_CFLAGS += -DACDK_CAMERA_3A
#LOCAL_CFLAGS += -DACDK_FAKE_SENSOR
#LOCAL_CFLAGS += -DACDK_BYPASS_P2

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libcct

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libstdc++
#
LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_SHARED_LIBRARIES += libbinder
#
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_fwkutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
#
LOCAL_SHARED_LIBRARIES += libcam.camshot
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libcam3a_imem
LOCAL_SHARED_LIBRARIES += libm4u
#
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.nvram
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3.lsctbl
LOCAL_SHARED_LIBRARIES += libcamalgo lib3a.gma lib3a.lce
#
LOCAL_SHARED_LIBRARIES += libccap

#-----------------------------------------------------------
# Camera Device

MTKCAM_HAVE_SENSOR_HAL          := '1'
MTKCAM_HAVE_3A_HAL              := '1'
MTKCAM_HAVE_CAM_MANAGER         := '1'
ifeq ($(strip $(MTK_HEART_RATE_MONITOR_SUPPORT)),yes)
    MTKCAM_HR_MONITOR_SUPPORT                 := '1'
else
    MTKCAM_HR_MONITOR_SUPPORT                 := '0'
endif
#
LOCAL_CFLAGS += -DMTKCAM_HAVE_SENSOR_HAL="$(MTKCAM_HAVE_SENSOR_HAL)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_3A_HAL="$(MTKCAM_HAVE_3A_HAL)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_CAM_MANAGER="$(MTKCAM_HAVE_CAM_MANAGER)"
LOCAL_CFLAGS += -DMTKCAM_HR_MONITOR_SUPPORT="$(MTKCAM_HR_MONITOR_SUPPORT)"

#
LOCAL_C_INCLUDES += $(TOP)/vendor/mediatek/proprietary/hardware/mtkcam/main/hal/device/1.x
LOCAL_SHARED_LIBRARIES += libmtkcam_device1

LOCAL_C_INCLUDES += $(TOP)/frameworks/native/libs/arect/include



#
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include
LOCAL_C_INCLUDES += $(TOP)/hardware/interfaces/camera/common/1.0/default/include
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/media/openmax
LOCAL_C_INCLUDES += $(TOP)/system/core/include/utils/


LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam_device1_base
LOCAL_STATIC_LIBRARIES += libmtkcam_device1_hidlcommon
LOCAL_STATIC_LIBRARIES += android.hardware.camera.common@1.0-helper

LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidlmemory
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += android.hardware.camera.device@1.0
LOCAL_SHARED_LIBRARIES += android.hardware.camera.common@1.0

LOCAL_SHARED_LIBRARIES += android.hardware.graphics.mapper@2.0
LOCAL_SHARED_LIBRARIES += android.hidl.allocator@1.0
LOCAL_SHARED_LIBRARIES += android.hidl.memory@1.0

LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.device@1.1

# from DefaultCameraDevice
LOCAL_SHARED_LIBRARIES += libmtkcam_fwkutils
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils

LOCAL_SHARED_LIBRARIES += libcam1_utils

LOCAL_SHARED_LIBRARIES += libcam.paramsmgr
LOCAL_SHARED_LIBRARIES += libcam.client
LOCAL_SHARED_LIBRARIES += libcam.camadapter

#
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/main/hal/device/1.x
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include

LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc
endif
# Stereo
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
endif

#
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
#LOCAL_C_INCLUDES += $(TOP)/hardware/interfaces/camera/common/1.0/default/include

LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/media/openmax

LOCAL_C_INCLUDES += $(TOP)/system/core/include
#

# Stereo
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif


#-----------------------------------------------------------
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libmtkcam_cct
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
