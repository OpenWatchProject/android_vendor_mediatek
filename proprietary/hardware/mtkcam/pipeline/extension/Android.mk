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

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk

ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_CFLAGS += -DHWNODE_HAVE_AEE_FEATURE=1
else
LOCAL_CFLAGS += -DHWNODE_HAVE_AEE_FEATURE=0
endif

#-----------------------------------------------------------
LOCAL_SRC_FILES += VendorManagerImp.cpp
LOCAL_SRC_FILES += Vendor.cpp
LOCAL_SRC_FILES += BaseVendor.cpp
LOCAL_SRC_FILES += BaseRawVendor.cpp
LOCAL_SRC_FILES += ControllerHandler.cpp
# 3rd party features
LOCAL_SRC_FILES += 3rd/FaceBeauty.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
#
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(TOP)/system/core/include
LOCAL_C_INCLUDES += $(TOP)/system/core/libutils/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/libion_mtk/include
#
# AEE
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_CFLAGS += -DHWNODE_HAVE_AEE_FEATURE=1
else
LOCAL_CFLAGS += -DHWNODE_HAVE_AEE_FEATURE=0
endif

#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES +=
#
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.extension.utils
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.extension.nr
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.extension.mixyuv

# ----------------------------------------------------------
# HDR
ifneq ($(strip $(MTKCAM_HAVE_HDR_SUPPORT)),0)
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.extension.hdr
endif

#-----------------------------------------------------------
# MFNR
ifneq ($(strip $(MTKCAM_HAVE_MFB_SUPPORT)),0)
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.extension.mfnr
LOCAL_SHARED_LIBRARIES += libmfllcore
endif

MTKCAM_HW_NODE_USING_TUNING_UTILS := 1
# MTKCAM_HW_NODE_USING_TUNING_UTILS = disable:0 / enable:1
ifeq ($(MTKCAM_HW_NODE_USING_TUNING_UTILS), 1)
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils
endif
LOCAL_CFLAGS += -DMTKCAM_HW_NODE_USING_TUNING_UTILS=$(MTKCAM_HW_NODE_USING_TUNING_UTILS)

# ----------------------------------------------------------
# DualCam denoise
ifneq ($(strip $(MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT)),0)
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.extension.denoise

ifeq ($(INDEP_STEREO_PROVIDER), true)
	LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
	LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif

LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.bmdenoise
endif

# ----------------------------------------------------------
# DualCam Fusion
ifneq ($(strip $(MTKCAM_HAVE_DUAL_ZOOM_FUSION_SUPPORT)),0)
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.extension.fusion

ifeq ($(INDEP_STEREO_PROVIDER), true)
	LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
	LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif

LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3

# add 3rd party Fusion library here
ifneq ($(strip $(MTKCAM_HAVE_DUAL_ZOOM_FUSION_01_SUPPORT)),0)
    # N/A
endif

ifneq ($(strip $(MTKCAM_HAVE_DUAL_ZOOM_FUSION_02_SUPPORT)),0)
    LOCAL_SHARED_LIBRARIES_arm += libDualZoomFusion
    LOCAL_SHARED_LIBRARIES_arm64 += libDualZoomFusion
endif

endif

# ----------------------------------------------------------
# VSDOF vendor
ifneq ($(strip $(MTKCAM_HAVE_DUALCAM_VSDOF_VENDOR_SUPPORT)),0)
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.extension.vsdof

ifeq ($(INDEP_STEREO_PROVIDER), true)
	LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
	LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif
endif

# Stereo Cam
ifneq ($(strip $(MTKCAM_HAVE_MTKSTEREO_SUPPORT)),0)
ifeq ($(VSDOF_VERSION), 2)
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.extension.dcmf

ifeq ($(INDEP_STEREO_PROVIDER), true)
	LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
	LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif

LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.dualcam_mf
LOCAL_SHARED_LIBRARIES += libmtkcam.DualCam
endif
endif

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6775 mt6771))
LOCAL_SHARED_LIBRARIES += libmtkcam_mapping_mgr
endif

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libhardware
#
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_streamutils
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libcam.feature_utils
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libmtkcam_exif

# NR
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
#
LOCAL_SHARED_LIBRARIES += libabfadp
LOCAL_SHARED_LIBRARIES += libion_mtk
LOCAL_SHARED_LIBRARIES += libion
# AEE
ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaedv
endif

LOCAL_SHARED_LIBRARIES += libcam1_utils

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libdpframework
#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcam_extension
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)

################################################################################
#
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))
