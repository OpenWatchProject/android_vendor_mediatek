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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/effectHal.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk

#-----------------------------------------------------------
ifeq ($(BUILD_MTK_LDVT),yes)
    LOCAL_CFLAGS += -DUSING_MTK_LDVT
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ldvt/$(PLATFORM)/include
    LOCAL_WHOLE_STATIC_LIBRARIES += libuvvf
endif

#-----------------------------------------------------------
LOCAL_SRC_FILES += DummyNode.cpp
LOCAL_SRC_FILES += BaseNode.cpp

LOCAL_SRC_FILES += p1node/P1NodeImp.cpp
LOCAL_SRC_FILES += p1node/P1ConnectLMV.cpp
LOCAL_SRC_FILES += p1node/P1DeliverMgr.cpp
LOCAL_SRC_FILES += p1node/P1RegisterNotify.cpp
LOCAL_SRC_FILES += p1node/P1TaskCtrl.cpp
LOCAL_SRC_FILES += p1node/P1Utility.cpp

LOCAL_SRC_FILES += p2node/P2Node.cpp
LOCAL_SRC_FILES += p2node/FrameUtils.cpp
LOCAL_SRC_FILES += p2node/proc/P2Procedure.cpp
ifeq ($(MTKCAM_HAVE_IVENDOR_SUPPORT), 0)
LOCAL_SRC_FILES += p2node/proc/SwnrProcedure.cpp
LOCAL_SRC_FILES += p2node/proc/PluginProcedure.cpp
#LOCAL_SRC_FILES += p2node/proc/plugin/RawPostProcessing.cpp
#LOCAL_SRC_FILES += p2node/proc/plugin/YuvPostProcessing.cpp
else
LOCAL_SRC_FILES += p2node/proc/RawProcedure.cpp
LOCAL_SRC_FILES += p2node/proc/YuvProcedure.cpp
LOCAL_SRC_FILES += p2node/proc/PluginProcedure.cpp
LOCAL_SRC_FILES += p2node/proc/plugin/RawPostProcessing.cpp
endif
LOCAL_SRC_FILES += p2node/proc/MdpProcedure.cpp
LOCAL_SRC_FILES += JpegNode.cpp
LOCAL_SRC_FILES += RAW16Node.cpp
LOCAL_SRC_FILES += P2_utils.cpp

LOCAL_SRC_FILES += commonP2Node.cpp
ifneq ($(MTK_BASIC_PACKAGE),yes)
LOCAL_CFLAGS += -DMTKCAM_SUPPORT_COMMON_P2NODE
LOCAL_SRC_FILES += P2FeatureNode.cpp
LOCAL_SRC_FILES += p2/P2_BasicProcessor.cpp
LOCAL_SRC_FILES += p2/P2_CaptureProcessor.cpp
LOCAL_SRC_FILES += p2/P2_Cropper.cpp
LOCAL_SRC_FILES += p2/P2_DispatchProcessor.cpp
LOCAL_SRC_FILES += p2/P2_DumpPlugin.cpp
LOCAL_SRC_FILES += p2/P2_Info.cpp
LOCAL_SRC_FILES += p2/P2_LMVInfo.cpp
LOCAL_SRC_FILES += p2/P2_Logger.cpp
LOCAL_SRC_FILES += p2/P2_MDPProcessor.cpp
LOCAL_SRC_FILES += p2/P2_MWData.cpp
LOCAL_SRC_FILES += p2/P2_MWFrame.cpp
LOCAL_SRC_FILES += p2/P2_MWFrameRequest.cpp
LOCAL_SRC_FILES += p2/P2_Node.cpp
LOCAL_SRC_FILES += p2/P2_Param.cpp
LOCAL_SRC_FILES += p2/P2_Request.cpp
LOCAL_SRC_FILES += p2/P2_ScanlinePlugin.cpp
LOCAL_SRC_FILES += p2/P2_StreamingProcessor.cpp
LOCAL_SRC_FILES += p2/P2_Streaming_3DNR.cpp
LOCAL_SRC_FILES += p2/P2_Streaming_EIS.cpp
LOCAL_SRC_FILES += p2/P2_Util.cpp
endif

ifeq ($(MTK_CAM_HDR_SUPPORT), yes)
LOCAL_SRC_FILES += HDRNode.cpp
endif

ifneq ($(strip $(MTKCAM_HAVE_MFB_SUPPORT)),0)
LOCAL_SRC_FILES += MfllNode.cpp
endif

LOCAL_SRC_FILES += tpnode/TPNode.cpp
LOCAL_SRC_FILES += tpnode/plugin/DefaultPlugin.cpp
LOCAL_SRC_FILES += tpnode/plugin/TPBokehPlugin.cpp

LOCAL_SRC_FILES += MFCNode.cpp

ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT),yes)
LOCAL_CFLAGS += -DENABLE_STEREO_PERFSERVICE
LOCAL_SRC_FILES += StereoRootNode.cpp
LOCAL_SRC_FILES += DualImageTransformNode.cpp
LOCAL_SRC_FILES += DualYUVNode.cpp
LOCAL_SRC_FILES += Dual3rdPartyNode.cpp
LOCAL_SRC_FILES += tpnode/plugin/DCMFPlugin.cpp
#DualFeature
ifeq ($(MTK_CAM_VSDOF_SUPPORT), yes)
LOCAL_SRC_FILES += DualFeatureNode.cpp
LOCAL_SRC_FILES += BokehNode.cpp
LOCAL_SRC_FILES += DepthMapNode.cpp
endif
endif
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
#
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libdngop_lib/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libawb_core_lib/$(TARGET_BOARD_PLATFORM)/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libflash_lib/$(PLATFORM)/include
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_ALGO_INCLUDE)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_3a/libaf_core_lib/$(TARGET_BOARD_PLATFORM)/include

#
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/libion_mtk/include
#
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
#faces
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include
#For AEPlineTable.h
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/custom
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/custom/aaa
#For BuildInTypes.h
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/def
# AEE
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc

LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/libudf/libladder

ifneq ($(MTK_BASIC_PACKAGE),yes)
LOCAL_C_INCLUDES += $(EFFECTHAL_C_INCLUDE)
endif

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_CFLAGS += -DHWNODE_HAVE_AEE_FEATURE=1
else
LOCAL_CFLAGS += -DHWNODE_HAVE_AEE_FEATURE=0
endif

#-----------------------------------------------------------
#
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam_hwnode.fdNode
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_CFLAGS += -DMET_USER_EVENT_SUPPORT
LOCAL_WHOLE_STATIC_LIBRARIES += libfeature.vsdof.effecthal
endif


#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
#
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_streamutils
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libcam.feature_utils
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcameracustom.eis
#
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libfeature_eis
LOCAL_SHARED_LIBRARIES += libfeature_3dnr
LOCAL_SHARED_LIBRARIES += libfeature_rss
LOCAL_SHARED_LIBRARIES += libfeature_fsc
LOCAL_SHARED_LIBRARIES += libion_mtk
LOCAL_SHARED_LIBRARIES += libion
#For AE
LOCAL_SHARED_LIBRARIES += lib3a.ae.core
LOCAL_SHARED_LIBRARIES += lib3a.awb.core
LOCAL_SHARED_LIBRARIES += lib3a.flash
LOCAL_SHARED_LIBRARIES += libcamalgo.fdft
LOCAL_SHARED_LIBRARIES += libcamalgo.gyro
LOCAL_SHARED_LIBRARIES += libcamalgo.eis
LOCAL_SHARED_LIBRARIES += libcamalgo.lmv
# AEE
ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaedv
endif
# JpegNode
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libmtkcam_exif
# sensor
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils
ifeq ($(MTK_CAM_VHDR_SUPPORT), yes)
# vhdr
LOCAL_SHARED_LIBRARIES += libcam.vhdr
LOCAL_CFLAGS += -DMTKCAM_HW_NODE_VHDR_SUPPORT=1
else
LOCAL_CFLAGS += -DMTKCAM_HW_NODE_VHDR_SUPPORT=0
endif
# mfnr core
ifneq ($(strip $(MTKCAM_HAVE_MFB_SUPPORT)),0)
LOCAL_SHARED_LIBRARIES += libmfllcore
endif
# RAW16
LOCAL_SHARED_LIBRARIES += libcamalgo.dngop
# face feature
LOCAL_SHARED_LIBRARIES += libfeature.face
#LOCAL_SHARED_LIBRARIES += libcamalgo.ispfeature
LOCAL_SHARED_LIBRARIES += libcamalgo.fsc
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6775 mt6771))
LOCAL_SHARED_LIBRARIES += libmtkcam_mapping_mgr
endif

ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)

LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.vsdof_util

ifeq ($(INDEP_STEREO_PROVIDER), true)
	LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
	LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif

LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.isppipe
LOCAL_SHARED_LIBRARIES += libcameracustom
# dual zoom
LOCAL_SHARED_LIBRARIES += libmtkcam.DualCam
#LOCAL_SHARED_LIBRARIES += libmet-tag

ifeq ($(MTK_CAM_VSDOF_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.depthmap
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.bokeh
endif
ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.bmdenoise
endif
endif
# effecthal
ifneq ($(MTK_BASIC_PACKAGE),yes)
LOCAL_SHARED_LIBRARIES += libeffecthal.base
endif
ifneq ($(MTK_BASIC_PACKAGE),yes)
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.streaming
endif

#legacy support
MTKCAM_HW_NODE_WITH_LEGACY_SUPPORT := 0
ifeq ($(MTKCAM_DRV_ISP_VERSION), 30)
MTKCAM_HW_NODE_WITH_LEGACY_SUPPORT = 1
endif
#$(info MTKCAM_DRV_ISP_VERSION = $(MTKCAM_DRV_ISP_VERSION) ;)
#$(info MTKCAM_HW_NODE_WITH_LEGACY_SUPPORT = $(MTKCAM_HW_NODE_WITH_LEGACY_SUPPORT) ;)
LOCAL_CFLAGS += -DMTKCAM_HW_NODE_WITH_LEGACY_SUPPORT=$(MTKCAM_HW_NODE_WITH_LEGACY_SUPPORT)

#tuning utils
MTKCAM_HW_NODE_USING_TUNING_UTILS := 1
# MTKCAM_HW_NODE_USING_TUNING_UTILS = disable:0 / enable:1
ifeq ($(MTKCAM_HW_NODE_USING_TUNING_UTILS), 1)
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils
endif
LOCAL_CFLAGS += -DMTKCAM_HW_NODE_USING_TUNING_UTILS=$(MTKCAM_HW_NODE_USING_TUNING_UTILS)

#using 3A set list
MTKCAM_HW_NODE_USING_3A_LIST := 0
# MTKCAM_HW_NODE_USING_3A_LIST = disable:0 / enable:1
LOCAL_CFLAGS += -DMTKCAM_HW_NODE_USING_3A_LIST=$(MTKCAM_HW_NODE_USING_3A_LIST)

#-----------------------------------------------------------
# mtkcam hw node Log Level (for compile-time loglevel control)
ifndef MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT
ifeq ($(TARGET_BUILD_VARIANT),user)
    MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT   := 2
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
    MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT   := 3
else
    MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT   := 4
endif
endif
LOCAL_CFLAGS += -DMTKCAM_HW_NODE_LOG_LEVEL_DEFAULT=$(MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT)

#-----------------------------------------------------------
LOCAL_MODULE := libmtkcam_hwnode
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))

