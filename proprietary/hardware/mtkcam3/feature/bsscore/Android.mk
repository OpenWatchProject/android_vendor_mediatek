## Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2018. All rights reserved.
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
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

################################################################################

# Default version
MFLL_MF_TAG_VERSION           := 0
BSSCORE_MODULE_VERSION        := default

################################################################################
# MFLL
################################################################################

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6757))
MFLL_MF_TAG_VERSION           := 5
BSSCORE_MODULE_VERSION        := 1.0
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6763))
MFLL_MF_TAG_VERSION           := 7
BSSCORE_MODULE_VERSION        := 1.0
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6758))
MFLL_MF_TAG_VERSION           := 8
BSSCORE_MODULE_VERSION        := 1.0
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6771 mt6775))
MFLL_MF_TAG_VERSION           := 9
BSSCORE_MODULE_VERSION        := 1.0
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6765))
MFLL_MF_TAG_VERSION           := 10
BSSCORE_MODULE_VERSION        := 1.0
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6779))
MFLL_MF_TAG_VERSION           := 11
BSSCORE_MODULE_VERSION        := 1.1
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6768))
MFLL_MF_TAG_VERSION           := 12
BSSCORE_MODULE_VERSION        := 1.0
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6785))
MFLL_MF_TAG_VERSION           := 13
BSSCORE_MODULE_VERSION        := 1.0
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6885))
MFLL_MF_TAG_VERSION           := 14
BSSCORE_MODULE_VERSION        := 1.2
endif

LOCAL_CFLAGS += -DMFLL_MF_TAG_VERSION=$(MFLL_MF_TAG_VERSION)
################################################################################
MFLL_APPLY_RAW_BSS           := 0

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6771 mt6775 mt6765 mt6779 mt6768 mt6785))
MFLL_APPLY_RAW_BSS           := 1
endif

LOCAL_CFLAGS += -DMFLL_APPLY_RAW_BSS=$(MFLL_APPLY_RAW_BSS)
################################################################################

BSSCORE_INCLUDE_PATH := $(MTK_PATH_SOURCE)/hardware/mtkcam3/include/mtkcam3/feature/bsscore
################################################################################
BSSCORE_INCLUDE_PATH += $(call include-path-for, camera)
################################################################################
BSSCORE_INCLUDE_PATH += $(TOP)/$(MTK_PATH_SOURCE)/hardware
BSSCORE_INCLUDE_PATH += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/core/
BSSCORE_INCLUDE_PATH += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/core/featurePipe/capture
################################################################################
BSSCORE_INCLUDE_PATH += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
BSSCORE_INCLUDE_PATH += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/include
BSSCORE_INCLUDE_PATH += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include
BSSCORE_INCLUDE_PATH += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
BSSCORE_INCLUDE_PATH += $(TOP)/frameworks/native/libs/arect/include
BSSCORE_INCLUDE_PATH += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libdngop_lib/include
################################################################################
BSSCORE_INCLUDE_PATH += $(MTKCAM_ALGO_INCLUDE)/libcore
BSSCORE_INCLUDE_PATH += $(MTKCAM_ALGO_INCLUDE)/libutility
BSSCORE_INCLUDE_PATH += $(MTKCAM_C_INCLUDES)
################################################################################
BSSCORE_INCLUDE_PATH += $(MTK_PATH_COMMON)/hal/inc
BSSCORE_INCLUDE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
BSSCORE_INCLUDE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
BSSCORE_INCLUDE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
BSSCORE_INCLUDE_PATH += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
BSSCORE_INCLUDE_PATH += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)
BSSCORE_INCLUDE_PATH += $(MTK_PATH_SOURCE)/custom

## algorithm - mfnr
BSSCORE_INCLUDE_PATH += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libmfnr_lib/$(PLATFORM)/include

## algorithm - fd
BSSCORE_INCLUDE_PATH += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include

#for libawb_core_lib
BSSCORE_INCLUDE_PATH += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libawb_core_lib/$(TARGET_BOARD_PLATFORM)/include

#for libae_core_lib
BSSCORE_INCLUDE_PATH += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libae_core_lib/$(TARGET_BOARD_PLATFORM)/include
BSSCORE_INCLUDE_PATH += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libae_core_lib/$(TARGET_BOARD_PLATFORM)/include/aaa

#for libaf_core_lib
BSSCORE_INCLUDE_PATH += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libaf_core_lib/$(TARGET_BOARD_PLATFORM)/include

#for libflash_lib
BSSCORE_INCLUDE_PATH += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libflash_lib/$(TARGET_BOARD_PLATFORM)/include

BSSCORE_INCLUDE_HEADER := $(MTKCAM_INCLUDE_HEADER_LIB)
BSSCORE_INCLUDE_HEADER += libmtkcam_headers
BSSCORE_INCLUDE_HEADER += libmtkcam3_headers
BSSCORE_INCLUDE_HEADER += libmtkcam_legacy_headers
BSSCORE_INCLUDE_HEADER += libcameracustom_headers
BSSCORE_INCLUDE_HEADER += libmtkcam_algorithm_headers
BSSCORE_INCLUDE_HEADER += device_kernel_headers
BSSCORE_INCLUDE_HEADER += libutils_headers liblog_headers libhardware_headers

################################################################################
BSSCORE_SHARED_LIBS := liblog
BSSCORE_SHARED_LIBS += libutils
BSSCORE_SHARED_LIBS += libcutils
BSSCORE_SHARED_LIBS += libladder
BSSCORE_SHARED_LIBS += libnativewindow
BSSCORE_SHARED_LIBS += libmtkcam_modulehelper
BSSCORE_SHARED_LIBS += libmtkcam_stdutils libmtkcam_imgbuf
BSSCORE_SHARED_LIBS += libmtkcam_ulog
BSSCORE_SHARED_LIBS += libcam.iopipe
BSSCORE_SHARED_LIBS += libdpframework
BSSCORE_SHARED_LIBS += libgralloc_extra
BSSCORE_SHARED_LIBS += libcameracustom
BSSCORE_SHARED_LIBS += libmtkcam_tuning_utils
BSSCORE_SHARED_LIBS += libmtkcam_metadata
BSSCORE_SHARED_LIBS += libcam.feature_utils

BSSCORE_SHARED_LIBS += libcamalgo.dngop
BSSCORE_SHARED_LIBS += libmtkcam_debugutils

#For AE
BSSCORE_SHARED_LIBS += lib3a.ae.core

#for libaf_core_lib
BSSCORE_SHARED_LIBS += lib3a.af.core

#for libflash_lib
BSSCORE_SHARED_LIBS += lib3a.flash

# HW Util (FD Container)
BSSCORE_SHARED_LIBS += libmtkcam_hwutils

# Face Detection
BSSCORE_SHARED_LIBS += libfeature.face

# Power Hal
BSSCORE_SHARED_LIBS += libhidlbase
BSSCORE_SHARED_LIBS += vendor.mediatek.hardware.power@2.0

# MFNR
ifneq ($(strip $(MTKCAM_HAVE_MFB_SUPPORT)),0)
BSSCORE_SHARED_LIBS += libmfllcore
BSSCORE_SHARED_LIBS += libcamalgo.mfnr
endif

## algorithm - fd
BSSCORE_SHARED_LIBS += libcamalgo.fdft

BSSCORE_SHARED_LIBS += libmtkcam_exif
# Util
BSSCORE_SHARED_LIBS += libmtkcam.featurepipe.vsdof_util
BSSCORE_SHARED_LIBS += libmtkcam_metastore

# Plugin
BSSCORE_SHARED_LIBS += libmtkcam_3rdparty
BSSCORE_SHARED_LIBS += libmtkcam_3rdparty.core
# Tuning
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6775 mt6771 mt6779))
BSSCORE_SHARED_LIBS += libmtkcam_mapping_mgr
endif

# DualCam
BSSCORE_SHARED_LIBS += libfeature.stereo.provider

#for libawb_core_lib
BSSCORE_SHARED_LIBS += lib3a.awb.core

ifeq ($(HAVE_AEE_FEATURE),yes)
    BSSCORE_SHARED_LIBS += libaedv
endif


include $(LOCAL_PATH)/$(BSSCORE_MODULE_VERSION)/Android.mk
################################################################################
#include $(call all-makefiles-under, $(LOCAL_PATH))
