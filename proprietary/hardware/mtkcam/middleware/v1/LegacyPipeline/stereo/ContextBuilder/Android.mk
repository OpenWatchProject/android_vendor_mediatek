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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk

#-----------------------------------------------------------
# for single cam 3rd
#-----------------------------------------------------------
LOCAL_SRC_FILES += ./NodeData/TPNodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/JpegEnc_CleanImageNodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/JpegEnc_TP0_JpegNodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/JpegEnc_TP_JpegNodeConfigData.cpp
#-----------------------------------------------------------
# for dual cam 3rd
#-----------------------------------------------------------
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_SRC_FILES += ./NodeData/StereoRootNodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/StereoP1NodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/StereoP1NodeMain2ConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/StereoP2NodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/StereoP2NodeMain2ConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/Dual3rdPartyNodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/Raw16NodeConfigData.cpp
endif
#-----------------------------------------------------------
# for dual cam vsdof
#-----------------------------------------------------------
ifeq ($(MTK_CAM_VSDOF_SUPPORT), yes)
LOCAL_SRC_FILES += ./NodeData/BokehNodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/DepthMapNodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/DualImageTransformNodeConfigData.cpp
#LOCAL_SRC_FILES += ./NodeData/Raw16NodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/JpegEnc_JPSNodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/JpegEnc_BokehNodeConfigData.cpp
LOCAL_SRC_FILES += ./NodeData/JpegEnc_JpegNodeConfigData.cpp
endif

LOCAL_SRC_FILES += MetaStreamManager.cpp
LOCAL_SRC_FILES += ImageStreamManager.cpp
LOCAL_SRC_FILES += NodeConfigDataManager.cpp
LOCAL_SRC_FILES += ContextBuilder.cpp
LOCAL_SRC_FILES += StereoBasicParameters.cpp
#

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/perfservice/perfservicenative
#
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
# Custom
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_CFLAGS += -DMTKCAM_STEREO_SUPPORT
endif
#
ifeq ($(MTK_CAM_VSDOF_SUPPORT), yes)
LOCAL_CFLAGS += -DMTKCAM_STEREO_VSDOF_SUPPORT
endif
#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES +=
#
LOCAL_WHOLE_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam_streamutils
LOCAL_SHARED_LIBRARIES += libmtkcam_pipeline
LOCAL_SHARED_LIBRARIES += libmtkcam_hwnode
LOCAL_SHARED_LIBRARIES += libbwc
LOCAL_SHARED_LIBRARIES += libnativewindow
# stereo hal
ifeq ($(INDEP_STEREO_PROVIDER), true)
	LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
	LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif
LOCAL_SHARED_LIBRARIES += libcameracustom
# hwutil
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils

#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcam3_contextbuilder
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
#include $(call all-makefiles-under,$(LOCAL_PATH))

