# Copyright Statement:
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

################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)
################################################################################
#
################################################################################
include $(CLEAR_VARS)
#-----------------------------------------------------------
include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
#-----------------------------------------------------------
LOCAL_SRC_FILES += mtk_sensor_control.cpp
LOCAL_SRC_FILES += s_async/sample_streaming_async.cpp
LOCAL_SRC_FILES += s_async/sample_streaming_meta.cpp
LOCAL_SRC_FILES += s_disp/sample_streaming_disp.cpp
LOCAL_SRC_FILES += s_eisq/sample_streaming_eisq.cpp
LOCAL_SRC_FILES += s_fb/sample_streaming_fb.cpp
LOCAL_SRC_FILES += s_green/sample_streaming_green.cpp
LOCAL_SRC_FILES += s_semi/sample_streaming_semi.cpp

ifeq ($(MTK_CAM_VSDOF_SUPPORT), yes)
LOCAL_SRC_FILES += s_dual/sample_streaming_dual.cpp
LOCAL_SRC_FILES += s_fova/sample_streaming_fova.cpp
endif

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_HEADER_LIBRARIES += libcameracustom_headers
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/include
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/core/featurePipe/vsdof/util
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/core/featurePipe/core/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
# DpBlitStream
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/include
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/
#
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/common/vsdof/vsdof_common.mk
LOCAL_C_INCLUDES += $(VSDOF_COMMON_INC)
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore

ifneq ($(strip $(MTKCAM_HAVE_MFB_SUPPORT)),0)
LOCAL_SHARED_LIBRARIES += libmfllcore
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
LOCAL_SHARED_LIBRARIES += libcam.feature_utils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_exif
endif

ifneq ($(strip $(MTKCAM_HAVE_AINR_SUPPORT)),0)
LOCAL_SHARED_LIBRARIES += libainrcore
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
LOCAL_SHARED_LIBRARIES += libcam.feature_utils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_exif
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils
endif
LOCAL_SHARED_LIBRARIES += libcamalgo.fdft
LOCAL_SHARED_LIBRARIES += libcamalgo.dngop
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_3rdparty.core

SUPPORTED_PLATFORM := mt6757 mt6763 mt6765 mt6768
ifneq (,$(filter $(SUPPORTED_PLATFORM),$(TARGET_BOARD_PLATFORM)))
# --------------------------------------------------------------------
#adf
LOCAL_SHARED_LIBRARIES += libabfadp
endif

# ULog
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
# tuning dump
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils
# HDR
LOCAL_SHARED_LIBRARIES += libcam.iopipe
# SWNR
ifeq ($(MTK_CAM_NEW_NVRAM_SUPPORT),1)
LOCAL_SHARED_LIBRARIES += libmtkcam_mapping_mgr
endif
LOCAL_SHARED_LIBRARIES += libcameracustom
# FD
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
# #DP
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.vsdof_util
# Dump Buffer
LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
#
LOCAL_SHARED_LIBRARIES += liblpcnr
#-----------------------------------------------------------
ifneq ($(strip $(MTKCAM_HAVE_MFB_SUPPORT)),0)
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.mfnr
endif
#4 Cell
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.remosaic
# HDR
ifneq ($(strip $(MTKCAM_HAVE_HDR_SUPPORT)),0)
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.hdr
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.rawhdr
endif
# AINR
ifneq ($(strip $(MTKCAM_HAVE_AINR_SUPPORT)),0)
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.ainr
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.aihdr
endif
# dualcam
ifeq ($(strip $(MTK_CAM_STEREO_CAMERA_SUPPORT)),yes)
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.tk_bokeh
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.tk_depth
#
LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
endif
# SWNR
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.swnr
# FB
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.fb
# YHDR
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.yuvhdr

SUPPORTED_PLATFORM := mt6757 mt6763 mt6765 mt6768
ifneq (,$(filter $(SUPPORTED_PLATFORM),$(TARGET_BOARD_PLATFORM)))
# --------------------------------------------------------------------
# ABF
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.plugin.abf
endif

LOCAL_STATIC_LIBRARIES +=
# DepthMap
ifeq ($(MTK_CAM_VSDOF_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.depthmap
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.vsdof_util
endif

#-----------------------------------------------------------
LOCAL_MODULE := libmtkcam_3rdparty.mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)

################################################################################
#
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))

