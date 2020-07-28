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
-include $(TOP)/$(MTK_PATH_CUSTOM)/hal/mtkcam/mtkcam.mk
MTKCAM_MTK_PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
#MTKCAM_TARGET_BOARD_PLATFORM := $(TARGET_BOARD_PLATFORM)

#-----------------------------------------------------------
# LTM module on/off option
#         0: disabled
# otherwise: enabled
#-----------------------------------------------------------
MTK_CAM_LTM_SUPPORT := 1

ifeq ($(TARGET_BOARD_PLATFORM), mt6757)
ifneq ($(strip $(MTK_CAM_SW_VERSION)),ver2)
    MTK_CAM_LTM_SUPPORT := 0
endif
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6761))
    MTK_CAM_LTM_SUPPORT := 0
endif


#-----------------------------------------------------------
# Camera ip base design version control
#-----------------------------------------------------------
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6797 mt6757 mt6799 mt6759 mt6763 mt6758 mt6771 mt6775 mt6765 mt3967 mt6779 mt6768 mt6785 mt6885))
    MTKCAM_IP_BASE := 1
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), kiboplus))
    MTKCAM_IP_BASE := 1
else
    MTKCAM_IP_BASE := 0
endif

# ISP3 HAL3 use ipbase
MTK_PATH_CUSTOM_PLATFORM_HAL1 := $(MTK_PATH_CUSTOM_PLATFORM)
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739))
    MTKCAM_IP_BASE := 1
ifeq ($(strip $(MTK_CAM_LEGACY_HAL_SUPPORT)),yes)
ifeq ($(strip $(MTK_CAM_HAL_VERSION)),1)
    MTKCAM_IP_BASE := 0
    MTK_PATH_CUSTOM_PLATFORM_HAL1 := $(MTK_PATH_CUSTOM_PLATFORM)/hal1
endif
endif
endif

# for 6761 platform-based/ip-based
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6761))
ifeq ($(strip $(MTK_CAM_HAL_VERSION)),3)
    MTKCAM_IP_BASE := 1
else
    MTKCAM_IP_BASE := 0
    MTK_PATH_CUSTOM_PLATFORM_HAL1 := $(MTK_PATH_CUSTOM_PLATFORM)/hal1
endif
endif

#-----------------------------------------------------------
# Camera ip base design include path
#-----------------------------------------------------------
ifneq ($(MTK_CAM_HAL_VERSION),)
    CAMERA_HAL_VERSION := $(MTK_CAM_HAL_VERSION)
else
    CAMERA_HAL_VERSION := 3
    $(warning MTK_CAM_HAL_VERSION is not set, please check FO, set to HAL$(CAMERA_HAL_VERSION))
endif
MTKCAM_CFLAGS += -DMTKCAM_HAL_VERSION=$(CAMERA_HAL_VERSION)

# Temp solution for mt6761 projects' MTK_CAM_HAL_VERSION is not set
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6761))
ifeq ($(MTK_CAM_HAL_VERSION),)
    $(warning mt6761 project MTK_CAM_HAL_VERSION is not set, please check FO, set to HAL$(CAMERA_HAL_VERSION))
    CAMERA_HAL_VERSION := 1
endif
endif

MTKCAM_INCLUDE_HEADER_LIB :=

ifeq ($(MTKCAM_IP_BASE),1)
#### ip-based chips ########################################
IS_LEGACY := 0

MTKCAM_ALGO_INCLUDE := $(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(TARGET_BOARD_PLATFORM)

ifeq ($(CAMERA_HAL_VERSION), 3)
MTKCAM_PUBLIC_INCLUDE := \
    $(MTK_PATH_SOURCE)/hardware/mtkcam3/include \
    $(MTK_PATH_SOURCE)/hardware/mtkcam/include
MTKCAM_INCLUDE_HEADER_LIB += libmtkcam3_headers
MTKCAM_INCLUDE_HEADER_LIB += libmtkcam_headers
else
MTKCAM_PUBLIC_INCLUDE := $(MTK_PATH_SOURCE)/hardware/mtkcam/include
MTKCAM_INCLUDE_HEADER_LIB += libmtkcam_headers
endif

MTKCAM_C_INCLUDES := $(MTKCAM_PUBLIC_INCLUDE)
MTKCAM_C_INCLUDES += $(MTK_PATH_SOURCE)/external/aee/binary/inc
ifeq ($(BUILD_MTK_LDVT),yes)
MTKCAM_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ldvt/$(MTKCAM_MTK_PLATFORM)/include
endif

else
#### platform-based (legacy) chips #########################
IS_LEGACY := 1

#MTKCAM_C_INCLUDES := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam
MTKCAM_C_INCLUDES := $(MTK_PATH_SOURCE)/external/aee/binary/inc
MTKCAM_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
MTKCAM_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
MTKCAM_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include/mtkcam
MTKCAM_INCLUDE_HEADER_LIB += libmtkcam_headers
MTKCAM_INCLUDE_HEADER_LIB += libmtkcam_legacy_headers

# path for legacy platform
MTK_MTKCAM_PLATFORM    := $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735m mt6737m))
MTK_MTKCAM_PLATFORM    := $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735m
endif
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6737t))
MTK_MTKCAM_PLATFORM    := $(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/platform/mt6735
endif
endif ####################################


# MTKCAM_LOG_LEVEL_DEFAULT for compile-time loglevel control
# {{{ MTKCAM_LOG_LEVEL_DEFAULT
MTKCAM_LOG_LEVEL_DEFAULT   := 4
LOG_EXCEPTION_LIST :=
ifneq ($(filter $(TARGET_BOARD_PLATFORM),$(LOG_EXCEPTION_LIST)),$(TARGET_BOARD_PLATFORM))
  ifeq ($(TARGET_BUILD_VARIANT), user)
    MTKCAM_LOG_LEVEL_DEFAULT   := 2
  else ifeq ($(TARGET_BUILD_VARIANT), userdebug)
    # for user debug load & MTKCAM_USER_DBG_LOG_OFF (depends on special customer's request)
    # change default log level to ALOGI
    ifeq ($(MTK_CAM_USER_DBG_LOG_OFF), yes)
      MTKCAM_LOG_LEVEL_DEFAULT   := 2
    else
      MTKCAM_LOG_LEVEL_DEFAULT   := 3
    endif
  endif
endif
MTKCAM_CFLAGS += -DMTKCAM_LOG_LEVEL_DEFAULT=$(MTKCAM_LOG_LEVEL_DEFAULT)
ifeq ($(TARGET_BUILD_VARIANT), eng)
  MTKCAM_CFLAGS += -fno-omit-frame-pointer
endif
# }}} MTKCAM_LOG_LEVEL_DEFAULT


#-----------------------------------------------------------
# MFLL default ON, makes MFLL could be triggered in normal scene mode
# 0: do not enable buildin MFLL
# 1: enable buildin MFLL as MFLL mode
# 2: enable buuldin MFLL as AIS
#-----------------------------------------------------------
MTK_CAM_MFB_BUILDIN_SUPPORT ?= 0

#-----------------------------------------------------------
# iVendor Mechanism Enable Define
# 0: disabled
# 1: enabled
#-----------------------------------------------------------
MTK_CAM_IVENDOR_SUPPORT ?= 1

#-----------------------------------------------------------
# CommonCapturePipeline Mechanism Enable Define
# 0: disabled
# 1: enabled
#-----------------------------------------------------------
ifneq ($(strip $(MTK_CAM_MFB_SUPPORT)),0)
MTK_CAM_COMMON_CAPTURE_PIPELINE_SUPPORT ?= 0
else
MTK_CAM_COMMON_CAPTURE_PIPELINE_SUPPORT ?= 0
endif

#ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6779))
#MTKCAM_CFLAGS += -DMTKCAM_DISABLE_SENSOR=1
#endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6779 mt6785 mt6885))
MTKCAM_CFLAGS += -DMTKCAM_FORCE_VIDEO_MODE=1
endif

#-----------------------------------------------------------
# 3rd party preview fov
#-----------------------------------------------------------
MTK_CAM_DUAL_ZOOM_VENDOR_FOV_SUPPORT := no

#-----------------------------------------------------------
# MTKCAM_CFLAGS define
# In Android.mk, add followed words to use it:
# LOCAL_CFLAGS + = MTKCAM_CFLAGS
#-----------------------------------------------------------
# MTKCAM_HAVE_AEE_FEATURE
ifeq "yes" "$(strip $(HAVE_AEE_FEATURE))"
    MTKCAM_HAVE_AEE_FEATURE ?= 1
else
    MTKCAM_HAVE_AEE_FEATURE := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_AEE_FEATURE=$(MTKCAM_HAVE_AEE_FEATURE)
#-----------------------------------------------------------
# MTK_BASIC_PACKAGE
ifneq ($(MTK_BASIC_PACKAGE), yes)
    MTKCAM_BASIC_PACKAGE := 0
else
    MTKCAM_BASIC_PACKAGE := 1
endif
MTKCAM_CFLAGS += -DMTKCAM_BASIC_PACKAGE=$(MTKCAM_BASIC_PACKAGE)
#-----------------------------------------------------------
# MTK_BSP_PACKAGE
ifneq ($(MTK_BSP_PACKAGE), yes)
    MTKCAM_BSP_PACKAGE := 0
else
    MTKCAM_BSP_PACKAGE := 1
endif
MTKCAM_CFLAGS += -DMTKCAM_BSP_PACKAGE=$(MTKCAM_BSP_PACKAGE)
#-----------------------------------------------------------
# Hal3 MTK_ADV_CAM_SUPPORT
ifneq ($(MTK_CAM_ADV_CAM_SUPPORT), yes)
    MTKCAM_ADV_CAM_SUPPORT := 0
else
    MTKCAM_ADV_CAM_SUPPORT := 1
endif
MTKCAM_CFLAGS += -DMTKCAM_ADV_CAM_SUPPORT=$(MTKCAM_ADV_CAM_SUPPORT)
#-----------------------------------------------------------
# MTK_CAM_LOMO_SUPPORT
ifeq ($(MTK_CAM_LOMO_SUPPORT), yes)
    MTKCAM_LOMO_SUPPORT := 1
else
    MTKCAM_LOMO_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_LOMO_SUPPORT=$(MTKCAM_LOMO_SUPPORT)
#-----------------------------------------------------------
# MTKCAM_MMSDK_SUPPORT
ifneq ($(MTK_CAM_MMSDK_SUPPORT), yes)
    MTKCAM_MMSDK_SUPPORT := 0
else
    MTKCAM_MMSDK_SUPPORT := 1
endif
MTKCAM_CFLAGS += -DMTKCAM_MMSDK_SUPPORT=$(MTKCAM_MMSDK_SUPPORT)
#-----------------------------------------------------------
ifeq ($(strip $(MTK_ISP_SUPPORT_COLOR_SPACE)),2)
    MTKCAM_ISP_SUPPORT_COLOR_SPACE := 2
else ifeq ($(strip $(MTK_ISP_SUPPORT_COLOR_SPACE)),1)
    MTKCAM_ISP_SUPPORT_COLOR_SPACE := 1
else
    MTKCAM_ISP_SUPPORT_COLOR_SPACE := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_ISP_SUPPORT_COLOR_SPACE=$(MTKCAM_ISP_SUPPORT_COLOR_SPACE)
#-----------------------------------------------------------
# MFLL option
ifndef MTK_CAM_MFB_SUPPORT
    MTK_CAM_MFB_SUPPORT := 0
endif
ifneq ($(strip $(MTK_CAM_MFB_SUPPORT)),0)
    MTKCAM_HAVE_MFB_SUPPORT := $(MTK_CAM_MFB_SUPPORT)
else
    MTKCAM_HAVE_MFB_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_MFB_SUPPORT=$(MTKCAM_HAVE_MFB_SUPPORT)
#-----------------------------------------------------------
# AINR option
ifndef MTK_CAM_AINR_SUPPORT
    MTK_CAM_AINR_SUPPORT := 0
endif
ifneq ($(strip $(MTK_CAM_AINR_SUPPORT)),0)
    MTKCAM_HAVE_AINR_SUPPORT := $(MTK_CAM_AINR_SUPPORT)
else
    MTKCAM_HAVE_AINR_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_AINR_SUPPORT=$(MTKCAM_HAVE_AINR_SUPPORT)
#-----------------------------------------------------------
# AIHDR option
ifndef MTK_CAM_AIHDR_SUPPORT
    MTK_CAM_AIHDR_SUPPORT := 0
endif
ifneq ($(strip $(MTK_CAM_AIHDR_SUPPORT)),0)
    MTKCAM_HAVE_AIHDR_SUPPORT := $(MTK_CAM_AIHDR_SUPPORT)
else
    MTKCAM_HAVE_AIHDR_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_AIHDR_SUPPORT=$(MTKCAM_HAVE_AIHDR_SUPPORT)
#-----------------------------------------------------------
# p1 unpack raw support or not
# ISP3.0 is unsupported
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6885 mt6779))
    MTKCAM_PASS1_UNPACK_RAW_SUPPORT := 1
else
    MTKCAM_PASS1_UNPACK_RAW_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_PASS1_UNPACK_RAW_SUPPORT=$(MTKCAM_PASS1_UNPACK_RAW_SUPPORT)
#-----------------------------------------------------------
# ABF option
ifndef MTK_CAM_ABF_SUPPORT
    MTK_CAM_ABF_SUPPORT := 0
endif
ifneq ($(strip $(MTK_CAM_ABF_SUPPORT)),0)
    MTKCAM_HAVE_ABF_SUPPORT := $(MTK_CAM_ABF_SUPPORT)
else
    MTKCAM_HAVE_ABF_SUPPORT := 0
endif
#-----------------------------------------------------------
# ZSD+MFLL
ifeq "yes" "$(strip $(MTK_CAM_ZSDMFB_SUPPORT))"
    MTKCAM_HAVE_ZSDMFB_SUPPORT := 1
else
    MTKCAM_HAVE_ZSDMFB_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_ZSDMFB_SUPPORT=$(MTKCAM_HAVE_ZSDMFB_SUPPORT)
#-----------------------------------------------------------
# build in MFLL option, which means MFLL may be triggered even in normal scene mode
ifneq ($(strip $(MTK_CAM_MFB_BUILDIN_SUPPORT)),0)
    MTKCAM_HAVE_MFB_BUILDIN_SUPPORT := $(MTK_CAM_MFB_BUILDIN_SUPPORT)
else
    MTKCAM_HAVE_MFB_BUILDIN_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_MFB_BUILDIN_SUPPORT=$(MTKCAM_HAVE_MFB_BUILDIN_SUPPORT)
#-----------------------------------------------------------
# ZSD+HDR
ifeq "yes" "$(strip $(MTK_CAM_ZSDHDR_SUPPORT))"
    MTK_CAM_HAVE_ZSDHDR_SUPPORT := 1
else
    MTK_CAM_HAVE_ZSDHDR_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTK_CAM_HAVE_ZSDHDR_SUPPORT=$(MTK_CAM_HAVE_ZSDHDR_SUPPORT)
#-----------------------------------------------------------
# VHDR
ifeq ($(MTK_CAM_VHDR_SUPPORT),yes)
    MTKCAM_HAVE_VHDR_SUPPORT := 1
else
    MTKCAM_HAVE_VHDR_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_VHDR_SUPPORT=$(MTKCAM_HAVE_VHDR_SUPPORT)
#-----------------------------------------------------------
# 3DNR
ifeq ($(MTK_CAM_NR3D_SUPPORT),yes)
    MTKCAM_HAVE_NR3D_SUPPORT := 1
else
    MTKCAM_HAVE_NR3D_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_NR3D_SUPPORT=$(MTKCAM_HAVE_NR3D_SUPPORT)
#-----------------------------------------------------------
# EIS
ifeq ($(MTK_CAM_EIS_SUPPORT),yes)
    MTKCAM_HAVE_EIS_SUPPORT := 1
else
    MTKCAM_HAVE_EIS_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_EIS_SUPPORT=$(MTKCAM_HAVE_EIS_SUPPORT)
#-----------------------------------------------------------
# HDR
ifeq ($(MTK_CAM_HDR_SUPPORT),yes)
    MTKCAM_HAVE_HDR_SUPPORT := 1
else
    MTKCAM_HAVE_HDR_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_HDR_SUPPORT=$(MTKCAM_HAVE_HDR_SUPPORT)
#-----------------------------------------------------------
# HDR Detection
MTKCAM_CFLAGS += -DMTKCAM_HDR_DETECTION_MODE=$(MTK_CAM_HDR_DETECTION_MODE)

#-----------------------------------------------------------
# V4L2
ifeq ($(MTK_V4L2_DIP_MDP),yes)
    MTKCAM_V4L2_DIP_MDP := 1
else
    MTKCAM_V4L2_DIP_MDP := 0
endif
# $(warning " MTKCAM_V4L2_DIP_MDP:$(MTKCAM_V4L2_DIP_MDP) ")
MTKCAM_CFLAGS += -DMTKCAM_V4L2_DIP_MDP=$(MTKCAM_V4L2_DIP_MDP)
#-----------------------------------------------------------
# iVendor Mechanism Support
ifeq ($(MTK_CAM_IVENDOR_SUPPORT),1)
    MTKCAM_HAVE_IVENDOR_SUPPORT := 1
else
    MTKCAM_HAVE_IVENDOR_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_IVENDOR_SUPPORT=$(MTKCAM_HAVE_IVENDOR_SUPPORT)

#-----------------------------------------------------------
# Common Capture Pipeline Mechanism Support
ifeq ($(MTK_CAM_COMMON_CAPTURE_PIPELINE_SUPPORT),1)
    MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT := 1
else
    MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT=$(MTKCAM_HAVE_COMMON_CAPTURE_PIPELINE_SUPPORT)

#-----------------------------------------------------------
# 3rd party Vendor FOV
ifeq ($(MTK_CAM_DUAL_ZOOM_VENDOR_FOV_SUPPORT),yes)
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_VENDOR_FOV_SUPPORT=1
else
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_VENDOR_FOV_SUPPORT=0
endif
#-----------------
# lowmemory
ifeq ($(MTK_GMO_RAM_OPTIMIZE), yes)
    MTKCAM_CFLAGS += -DMTKCAM_LOW_MEM=1
else
    MTKCAM_CFLAGS += -DMTKCAM_LOW_MEM=0
endif
#-----------------
# min memory
ifeq ($(strip $(CUSTOM_CONFIG_MAX_DRAM_SIZE)),0x20000000)
    MTKCAM_CFLAGS += -DMTKCAM_MIN_LOW_MEM=1
else
    MTKCAM_CFLAGS += -DMTKCAM_MIN_LOW_MEM=0
endif
#-----------------
# FD
ifeq ($(MTK_CAM_FD_SUPPORT),yes)
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_FD_SUPPORT=1
else
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_FD_SUPPORT=0
endif

ifeq ($(MTK_CAM_SECURE_FD_SUPPORT),yes)
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_SECURE_FD_SUPPORT=1
else
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_SECURE_FD_SUPPORT=0
endif

#-----------------
# ASD
ifeq ($(MTK_CAM_ASD_SUPPORT),yes)
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_ASD_SUPPORT=1
else
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_ASD_SUPPORT=0
endif

#-----------------------------------------------------------
# Dual Zoom
ifeq ($(MTK_CAM_DUAL_ZOOM_SUPPORT),yes)
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_SUPPORT=1
    ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6799 mt6771 mt6785))
    MTKCAM_CFLAGS += -DMTKCAM_FOV_USE_WPE=1
    else
    MTKCAM_CFLAGS += -DMTKCAM_FOV_USE_WPE=0
    endif

    ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6785))
    MTKCAM_CFLAGS += -DMTKCAM_ISP_SUPPORT_IQ=1
    else
    MTKCAM_CFLAGS += -DMTKCAM_ISP_SUPPORT_IQ=0
    endif

else
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_SUPPORT=0
    MTKCAM_CFLAGS += -DMTKCAM_FOV_USE_WPE=0
    MTKCAM_CFLAGS += -DMTKCAM_ISP_SUPPORT_IQ=0
endif

ifeq ($(MTK_CAM_DUAL_ZOOM_MTK_FOV_SUPPORT),yes)
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_MTK_FOV_SUPPORT=1
else
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_MTK_FOV_SUPPORT=0
endif

ifeq ($(MTK_CAM_DUAL_ZOOM_MTK_FOV_SUPPORT),yes)
    ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6775 mt6785))
    MTKCAM_CFLAGS += -DFOV_HAL_VER=20
    endif
endif

#-----------------------------------------------------------
# Dual Zoom Fusion
#-----------------------------------------------------------
ifeq ($(MTK_CAM_DUAL_ZOOM_FUSION_SUPPORT),yes)
    MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT := 1
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT=1

#-----------------
ifeq ($(MTK_CAM_DUAL_ZOOM_FUSION_01_SUPPORT),yes)
    MTKCAM_HAVE_DUAL_ZOOM_FUSION_01_SUPPORT := 1
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_FUSION_01_SUPPORT=1
else
    MTKCAM_HAVE_DUAL_ZOOM_FUSION_01_SUPPORT := 0
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_FUSION_01_SUPPORT=0
endif

#-----------------
ifeq ($(MTK_CAM_DUAL_ZOOM_FUSION_02_SUPPORT),yes)
    MTKCAM_HAVE_DUAL_ZOOM_FUSION_02_SUPPORT := 1
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_FUSION_02_SUPPORT=1
else
    MTKCAM_HAVE_DUAL_ZOOM_FUSION_02_SUPPORT := 0
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_FUSION_02_SUPPORT=0
endif

#-----------------
ifeq ($(MTK_CAM_DUAL_ZOOM_FUSION_03_SUPPORT),yes)
    MTKCAM_HAVE_DUAL_ZOOM_FUSION_03_SUPPORT := 1
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_FUSION_03_SUPPORT=1
else
    MTKCAM_HAVE_DUAL_ZOOM_FUSION_03_SUPPORT := 0
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_FUSION_03_SUPPORT=0
endif

#----------------------------------
else
    MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT := 0
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT=0
endif
# Dual Zoom Fusion
#-----------------------------------------------------------


#-----------------------------------------------------------
# DualCam denoise
ifeq "yes" "$(strip $(MTK_CAM_STEREO_DENOISE_SUPPORT))"
    MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT := 1
else
    MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT=$(MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT)

#-----------------------------------------------------------
# MTKCAM_HAVE_RR_PRIORITY
MTKCAM_HAVE_RR_PRIORITY      ?= 0  # built-in if '1' ; otherwise not built-in
MTKCAM_CFLAGS += -DMTKCAM_HAVE_RR_PRIORITY=$(MTKCAM_HAVE_RR_PRIORITY)
#-----------------------------------------------------------
# L1_CACHE_BYTES for 32-byte cache line
MTKCAM_CFLAGS += -DL1_CACHE_BYTES=32
#-----------------------------------------------------------
ifeq ($(strip $(MTK_CAM_MAX_NUMBER_OF_CAMERA)), 4)
    MTKCAM_CFLAGS += -DMTK_SUB2_IMGSENSOR
endif
#----------------------------------------------------------
# LTM enable define
MTKCAM_CFLAGS += -DMTKCAM_LTM_SUPPORT=$(MTK_CAM_LTM_SUPPORT)

#-----------------------------------------------------------
# Stereo Cam
ifeq "yes" "$(strip $(MTK_CAM_STEREO_CAMERA_SUPPORT))"
    MTKCAM_HAVE_MTKSTEREO_SUPPORT := 1
else
    MTKCAM_HAVE_MTKSTEREO_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_MTKSTEREO_SUPPORT=$(MTKCAM_HAVE_MTKSTEREO_SUPPORT)

#-----------------------------------------------------------
# VSDOF 3rd party support
ifeq "yes" "$(strip $(MTK_CAM_VSDOF_VENDOR_SUPPORT))"
    MTKCAM_HAVE_DUALCAM_VSDOF_VENDOR_SUPPORT := 1
else
    MTKCAM_HAVE_DUALCAM_VSDOF_VENDOR_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_HAVE_DUALCAM_VSDOF_VENDOR_SUPPORT=$(MTKCAM_HAVE_DUALCAM_VSDOF_VENDOR_SUPPORT)


#-----------------------------------------------------------
# vsdof
ifeq ($(MTK_CAM_VSDOF_SUPPORT),yes)
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_VSDOF_SUPPORT=1
else
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_VSDOF_SUPPORT=0
endif

#-----------------------------------------------------------
# Legacy mtkcam with ip based sensor interface
#-----------------------------------------------------------
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739))
    LOCAL_CFLAGS += -DMTKCAM_LEGACY_INCLUDE_NEW_IHAL_SENSOR
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6761))
    LOCAL_CFLAGS += -DMTKCAM_LEGACY_INCLUDE_NEW_IHAL_SENSOR
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt8173))
    LOCAL_CFLAGS += -DMTKCAM_LEGACY_INCLUDE_NEW_IHAL_SENSOR
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt8167))
    LOCAL_CFLAGS += -DMTKCAM_LEGACY_INCLUDE_NEW_IHAL_SENSOR
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt8163))
    LOCAL_CFLAGS += -DMTKCAM_LEGACY_INCLUDE_NEW_IHAL_SENSOR
endif


# Preview with YUVO
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6755 mt6753 mt6761 mt6762 mt6765 mt6799 mt6771 mt6779 mt6785))
    MTK_CAM_PREFER_RRZO_USE_YUVFMT := 0
else
    MTK_CAM_PREFER_RRZO_USE_YUVFMT := 1
endif
MTKCAM_CFLAGS += -DMTK_CAM_PREFER_RRZO_USE_YUVFMT=$(MTK_CAM_PREFER_RRZO_USE_YUVFMT)

# Use P2C output process raw
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6885))
    MTK_CAM_MUST_USE_P2_PROCESS_RAW := 1
else
    MTK_CAM_MUST_USE_P2_PROCESS_RAW := 0
endif
MTKCAM_CFLAGS += -DMTK_CAM_MUST_USE_P2_PROCESS_RAW=$(MTK_CAM_MUST_USE_P2_PROCESS_RAW)

# Capture use YUV420
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6779 mt6785 mt6885))
    MTK_CAM_YUV420_JPEG_ENCODE_SUPPORT := 1
else
    MTK_CAM_YUV420_JPEG_ENCODE_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTK_CAM_YUV420_JPEG_ENCODE_SUPPORT=$(MTK_CAM_YUV420_JPEG_ENCODE_SUPPORT)

#-----------------------------------------------------------
# Use new precapture flow or legacy precapture flow
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6763 mt6771 mt6765 mt6785))
    MTK_CAM_FAST_PRECAPTURE_FLOW_ON := 1
else
    MTK_CAM_FAST_PRECAPTURE_FLOW_ON := 0
endif
MTKCAM_CFLAGS += -DMTK_CAM_FAST_PRECAPTURE_FLOW_ON=$(MTK_CAM_FAST_PRECAPTURE_FLOW_ON)

#-----------------------------------------------------------
# DisplayClient auto decide to use YUV format or RGB888 format buffer by consumer usage
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6775 mt6785))
    MTK_CAM_DISPLAY_BUFFER_RGB_FORMAT_SUPPORT := 1
else
    MTK_CAM_DISPLAY_BUFFER_RGB_FORMAT_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTK_CAM_DISPLAY_BUFFER_RGB_FORMAT_SUPPORT=$(MTK_CAM_DISPLAY_BUFFER_RGB_FORMAT_SUPPORT)

#-----------------------------------------------------------
# Display don't hide init 4 request for lunch time performance better, but may cause quality issue
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6785))
    MTK_CAM_DISPLAY_INIT_REQUEST_FRAME_SUPPORT := 0
else
    MTK_CAM_DISPLAY_INIT_REQUEST_FRAME_SUPPORT := 0
endif
MTKCAM_CFLAGS += -DMTK_CAM_DISPLAY_INIT_REQUEST_FRAME_SUPPORT=$(MTK_CAM_DISPLAY_INIT_REQUEST_FRAME_SUPPORT)
MTKCAM_CFLAGS += -DMTK_CAM_DISPLAY_INIT_REQUEST_FRAME_NUM=4

#-----------------------------------------------------------
# Default Display Frame control on
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6797 mt6799 mt6757 mt6758))
    MTK_CAM_DISPAY_FRAME_CONTROL_ON := 0
else
    MTK_CAM_DISPAY_FRAME_CONTROL_ON := 1
endif
MTKCAM_CFLAGS += -DMTK_CAM_DISPAY_FRAME_CONTROL_ON=$(MTK_CAM_DISPAY_FRAME_CONTROL_ON)

#-----------------------------------------------------------
# Default PDE node off, only need on ISP3.0 platform
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739))
    MTK_CAM_PDE_NODE_ON := 0
else
    MTK_CAM_PDE_NODE_ON := 0
endif
MTKCAM_CFLAGS += -DMTK_CAM_PDE_NODE_ON=$(MTK_CAM_PDE_NODE_ON)

#-----------------------------------------------------------
# stereo support
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT),yes)
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_STEREO_CAMERA_SUPPORT=1
else
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_STEREO_CAMERA_SUPPORT=0
endif

#------------------------------------------------------
# VSDOF flow Ver1 target:
# MT6797 MT6799 MT6757 MT6758 MT6763
#------------------------------------------------------
PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
VER1_PLATFORMS := mt6797 mt6799 mt6757 mt6758 mt6763 mt6765
VSDOF_VERSION := 1
ifneq (,$(filter $(VER1_PLATFORMS),$(PLATFORM)))
    VSDOF_VERSION := 1
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_VSDOF_MODE=1
else
    VSDOF_VERSION := 2
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_VSDOF_MODE=2
endif

# New Nvram Architecture
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6771 mt6775 mt6779 mt6785 mt6885))
    MTK_CAM_NEW_NVRAM_SUPPORT := 1
endif

ifeq ($(MTK_CAM_NEW_NVRAM_SUPPORT),1)
    MTKCAM_CFLAGS += -DMTK_CAM_NEW_NVRAM_SUPPORT=$(MTK_CAM_NEW_NVRAM_SUPPORT)
endif
#-----------------------------------------------------------
# IP-based Low power VSS
ifeq ($(MTKCAM_IP_BASE),1)
    ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6797 mt6799 mt6757 mt6758 mt6763))
        MTKCAM_LOW_POWER_VSS_DEFAULT := 0
    else
        MTKCAM_LOW_POWER_VSS_DEFAULT := 1
    endif
else
    MTKCAM_LOW_POWER_VSS_DEFAULT := 0
endif
MTKCAM_CFLAGS += -DMTKCAM_LOW_POWER_VSS_DEFAULT=$(MTKCAM_LOW_POWER_VSS_DEFAULT)

#-----------------------------------------------------------
# Legacy IPlugProcessing sopport
ifeq ($(IS_LEGACY),1)
    ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6761))
        MTKCAM_PLUG_PROCESSING_SUPPORT := 1
    else
        MTKCAM_PLUG_PROCESSING_SUPPORT := 0
    endif
else
    MTKCAM_PLUG_PROCESSING_SUPPORT := 1
endif
MTKCAM_CFLAGS += -DMTKCAM_PLUG_PROCESSING_SUPPORT=$(MTKCAM_PLUG_PROCESSING_SUPPORT)

#-----------------------------------------------------------
# 4 Cell Remosaic Provider
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6765 mt6771 mt6779 mt6785))
    MTKCAM_CFLAGS += -DMTKCAM_REMOSAIC_UNPACK_INOUTPUT_SUPPORT=1
else
    MTKCAM_CFLAGS += -DMTKCAM_REMOSAIC_UNPACK_INOUTPUT_SUPPORT=0
endif

#-----------------------------------------------------------
# systrace level
MTKCAM_SYSTRACE_LEVEL_DEFAULT := 1

ifneq (,$(filter $(strip $(CUSTOM_CONFIG_MAX_DRAM_SIZE)), 0x20000000, 0x40000000))
    ifneq ($(filter yes,$(BUILD_AGO_GMS) $(MTK_GMO_RAM_OPTIMIZE)),)
        ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739))
            MTKCAM_SYSTRACE_LEVEL_DEFAULT := 0
        endif
    endif
endif

MTKCAM_CFLAGS += -DMTKCAM_SYSTRACE_LEVEL_DEFAULT=$(MTKCAM_SYSTRACE_LEVEL_DEFAULT)

#-----------------------------------------------------------
# bgservice support
ifeq ($(MTK_CAM_BGSERVICE_SUPPORT), yes)
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_BGSERVICE_SUPPORT=1
else
    MTKCAM_CFLAGS += -DMTKCAM_HAVE_BGSERVICE_SUPPORT=0
endif

#-----------------------------------------------------------
# remove libmtkjpeg
MTKCAM_CFLAGS += -DMTKCAM_REMOVE_LIBMTKJPEG=1
#-----------------------------------------------------------
# Secure camera
ifeq ($(strip $(MTK_CAM_SECURITY_SUPPORT)), yes)
    MTKCAM_CFLAGS += -DMTKCAM_SECURITY_SUPPORT
endif # MTK_CAM_SECURITY_SUPPORT


# for 6761 platform-based/ip-based
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6761 mt6768 mt6771 mt6779 mt6785 mt6762 mt6765 mt6739 mt6885 mt6757))
    MTKCAM_CFLAGS += -DMTKCAM_CUSTOM_METADATA_COMMON=1
else
    MTKCAM_CFLAGS += -DMTKCAM_CUSTOM_METADATA_COMMON=0
endif

#------------------------------------------------------
# ISP Profile Mapper
PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
SUPPORT_ISPMAAPPER_PLATFORMS := mt6779
MTKCAM_ENABLE_ISPPROFILE_MAPPER := 1
ifneq (,$(filter $(SUPPORT_ISPMAAPPER_PLATFORMS),$(PLATFORM)))
    MTKCAM_CFLAGS += -DMTKCAM_ENABLE_ISPPROFILE_MAPPER=1
    MTKCAM_ENABLE_ISPPROFILE_MAPPER := 1
else
    MTKCAM_CFLAGS += -DMTKCAM_ENABLE_ISPPROFILE_MAPPER=0
    MTKCAM_ENABLE_ISPPROFILE_MAPPER := 0
endif

#------------------------------------------------------
# Flip in P2A before 3rd party plugin on P2C
MTKCAM_CFLAGS += -DMTKCAM_EARLY_FLIP_SUPPORT=0
