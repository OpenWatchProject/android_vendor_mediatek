# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2015. All rights reserved.
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
include $(CLEAR_VARS)
################################################################################
ifneq ($(strip $(MTKCAM_HAVE_MFB_SUPPORT)),0)
ifeq ($(strip $(SUPPORT_YUV_BSS)),0)
LOCAL_CFLAGS += -DSUPPORT_MFNR
endif
endif
################################################################################
LOCAL_SRC_FILES += CaptureFeature_Common.cpp
LOCAL_SRC_FILES += CaptureFeatureRequest.cpp
LOCAL_SRC_FILES += CaptureFeatureNode.cpp
LOCAL_SRC_FILES += CaptureFeatureInference.cpp
LOCAL_SRC_FILES += CaptureFeaturePipe.cpp
LOCAL_SRC_FILES += CaptureFeaturePlugin.cpp
LOCAL_SRC_FILES += ICaptureFeaturePipe.cpp
LOCAL_SRC_FILES += nodes/RootNode.cpp
LOCAL_SRC_FILES += nodes/RAWNode.cpp
LOCAL_SRC_FILES += nodes/P2ANode.cpp
LOCAL_SRC_FILES += nodes/FDNode.cpp
LOCAL_SRC_FILES += nodes/FusionNode.cpp
LOCAL_SRC_FILES += nodes/MultiFrameNode.cpp
LOCAL_SRC_FILES += nodes/DepthNode.cpp
LOCAL_SRC_FILES += nodes/BokehNode.cpp
LOCAL_SRC_FILES += nodes/YUVNode.cpp
LOCAL_SRC_FILES += nodes/MDPNode.cpp
LOCAL_SRC_FILES += exif/ExifWriter.cpp
LOCAL_SRC_FILES += buffer/CaptureBufferPool.cpp
LOCAL_SRC_FILES += thread/CaptureTaskQueue.cpp

################################################################################
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

################################################################################
# MFLL
################################################################################
MTK_CAPTURE_ISP_VERSION         := 0

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6779))
MTK_CAPTURE_ISP_VERSION         := 6
endif

LOCAL_CFLAGS += -DMTK_CAPTURE_ISP_VERSION=$(MTK_CAPTURE_ISP_VERSION)
################################################################################
# MFLL
################################################################################
MFLL_MF_TAG_VERSION           := 0

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6757))
MFLL_MF_TAG_VERSION           := 5
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6763))
MFLL_MF_TAG_VERSION           := 7
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6758))
MFLL_MF_TAG_VERSION           := 8
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6771 mt6775))
MFLL_MF_TAG_VERSION           := 9
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6765))
MFLL_MF_TAG_VERSION           := 10
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6779))
MFLL_MF_TAG_VERSION           := 11
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6768))
MFLL_MF_TAG_VERSION           := 12
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6785))
MFLL_MF_TAG_VERSION           := 13
endif

LOCAL_CFLAGS += -DMFLL_MF_TAG_VERSION=$(MFLL_MF_TAG_VERSION)
################################################################################
# MFLL
################################################################################
MFLL_APPLY_RAW_BSS           := 0

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6771 mt6775 mt6765 mt6779 mt6768 mt6785))
MFLL_APPLY_RAW_BSS           := 1
endif

LOCAL_CFLAGS += -DMFLL_APPLY_RAW_BSS=$(MFLL_APPLY_RAW_BSS)
################################################################################
# PQ Size
################################################################################
SUPPORT_PQ_SIZE := 1
ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6885))
SUPPORT_PQ_SIZE := 2
endif
LOCAL_CFLAGS += -DSUPPORT_PQ_SIZE=$(SUPPORT_PQ_SIZE)
################################################################################
LOCAL_C_INCLUDES += $(call include-path-for, camera)
################################################################################
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/core/
################################################################################
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/libs/arect/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libdngop_lib/include
################################################################################
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libcore
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libutility
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

################################################################################
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/custom

## algorithm - mfnr
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libmfnr_lib/$(PLATFORM)/include

## algorithm - fd
LOCAL_SHARED_LIBRARIES += libcamalgo.fdft

#for libaf_core_lib
LOCAL_SHARED_LIBRARIES += lib3a.af.core

#for libflash_lib
LOCAL_SHARED_LIBRARIES += lib3a.flash
################################################################################
ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6779))
LOCAL_CFLAGS+=-DSUPPORT_DCE=1
LOCAL_CFLAGS+=-DSUPPORT_HFG=1
LOCAL_CFLAGS+=-DSUPPORT_DSDN_20=1
LOCAL_CFLAGS+=-DSUPPORT_AINR=1
endif

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6768))
LOCAL_CFLAGS+=-DSUPPORT_MDPQoS=1
endif
ifeq ($(TARGET_BUILD_VARIANT),user)
LOCAL_CFLAGS += -DMTKCAM_TARGET_BUILD_VARIANT="'u'"
endif
ifeq ($(TARGET_BUILD_VARIANT),userdebug)
LOCAL_CFLAGS += -DMTKCAM_TARGET_BUILD_VARIANT="'d'"
endif
ifeq ($(TARGET_BUILD_VARIANT),eng)
LOCAL_CFLAGS += -DMTKCAM_TARGET_BUILD_VARIANT="'e'"
endif
# enable exception
LOCAL_CPPFLAGS := -fexceptions
################################################################################
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libladder
LOCAL_SHARED_LIBRARIES += libnativewindow
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libgralloc_extra
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libcam.feature_utils

LOCAL_SHARED_LIBRARIES += libcamalgo.dngop
LOCAL_SHARED_LIBRARIES += libmtkcam_debugutils
#For AE
LOCAL_SHARED_LIBRARIES += lib3a.ae.core
LOCAL_SHARED_LIBRARIES += lib3a.af.core

# HW Util (FD Container)
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils

# Face Detection
LOCAL_SHARED_LIBRARIES += libfeature.face

# Power Hal
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.power@2.0

# MFNR
ifneq ($(strip $(MTKCAM_HAVE_MFB_SUPPORT)),0)
LOCAL_SHARED_LIBRARIES += libmfllcore
ifeq ($(strip $(SUPPORT_YUV_BSS)),0)
LOCAL_SHARED_LIBRARIES += lib_bsscore
LOCAL_SHARED_LIBRARIES += libcamalgo.mfnr
endif
endif

LOCAL_SHARED_LIBRARIES += libmtkcam_exif
# Util
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.vsdof_util
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_HEADER_LIBRARIES := libutils_headers liblog_headers libhardware_headers
ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6757))
LOCAL_HEADER_LIBRARIES += libcustom_hal_headers
endif
# Plugin
LOCAL_SHARED_LIBRARIES += libmtkcam_3rdparty
LOCAL_SHARED_LIBRARIES += libmtkcam_3rdparty.core
# Tuning
ifeq ($(MTK_CAM_NEW_NVRAM_SUPPORT),1)
LOCAL_SHARED_LIBRARIES += libmtkcam_mapping_mgr
endif

# DualCam
LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider

#for libawb_core_lib
LOCAL_SHARED_LIBRARIES += lib3a.awb.core

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
endif


################################################################################
LOCAL_STATIC_LIBRARIES += libmtkcam.featurepipe.core
################################################################################
#LOCAL_CFLAGS += -DGTEST
################################################################################
LOCAL_MODULE := libmtkcam.featurepipe.capture
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)
################################################################################
#
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))
