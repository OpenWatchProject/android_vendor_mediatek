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
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/effectHal.mk
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk
endif
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
################################################################################
ifneq ($(MTKCAM_IP_BASE),0)
ifneq ($(MTK_BASIC_PACKAGE), yes)
LOCAL_SRC_FILES += ./flowControl/default/nodes/P2ANode.cpp
LOCAL_SRC_FILES += ./flowControl/default/nodes/ThirdPartyNode.cpp
LOCAL_SRC_FILES += ./flowControl/default/nodes/MDPNode.cpp
LOCAL_SRC_FILES += ./flowControl/default/DefaultIspPipeFlowControler.BufferConfig.cpp
LOCAL_SRC_FILES += ./flowControl/default/DefaultIspPipeFlowControler.cpp

ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_SRC_FILES += ./flowControl/dualCamMultiFrame/nodes/RootNode.cpp
LOCAL_SRC_FILES += ./flowControl/dualCamMultiFrame/nodes/MFNRNode.cpp
LOCAL_SRC_FILES += ./flowControl/dualCamMultiFrame/nodes/DepthNode.cpp
LOCAL_SRC_FILES += ./flowControl/dualCamMultiFrame/nodes/BokehNode.cpp
LOCAL_SRC_FILES += ./flowControl/dualCamMultiFrame/nodes/FDNode.cpp
LOCAL_SRC_FILES += ./flowControl/dualCamMultiFrame/nodes/HDRNode.cpp
LOCAL_SRC_FILES += ./flowControl/dualCamMultiFrame/DCMFIspPipeFlowControler.BufferConfig.cpp
LOCAL_SRC_FILES += ./flowControl/dualCamMultiFrame/DCMFIspPipeFlowControler.cpp
LOCAL_SRC_FILES += ./flowControl/dualCamMultiFrame/DCMFIspPipeFlowUtility.cpp
endif

LOCAL_SRC_FILES += ./IspPipe.cpp
LOCAL_SRC_FILES += ./IspPipe_Common.cpp
LOCAL_SRC_FILES += ./IspPipeNode.cpp
LOCAL_SRC_FILES += ./IspPipeRequest.cpp
LOCAL_SRC_FILES += ./IspPipeTimer.cpp
LOCAL_SRC_FILES += ./PipeBufferHandler.cpp
LOCAL_SRC_FILES += ./PipeBufferPoolMgr.cpp
LOCAL_SRC_FILES += ./IIspPipeFlowControler.cpp
LOCAL_SRC_FILES += ./StopWatch.cpp
endif

endif
################################################################################
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(call include-path-for, camera)
################################################################################
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/core
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/core/featurePipe
################################################################################
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/utils
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/
LOCAL_C_INCLUDES += $(TOP)/system/core/libsync/include/
## effectHAL
LOCAL_C_INCLUDES += $(EFFECTHAL_C_INCLUDE)
## algo/3a
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_ALGO_INCLUDE)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include
## driver
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_DRV_INCLUDE)
##
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
## ion
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external
## stereo hal
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/common/vsdof/hal/$(STEREO_HAL_VER)
endif
### exif
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc
################################################################################
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.featurepipe.core
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libnativewindow
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
ifeq ($(INDEP_STEREO_PROVIDER), true)
    LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
endif
# MFNR
LOCAL_SHARED_LIBRARIES += libmfllcore
endif
LOCAL_SHARED_LIBRARIES += libeffecthal.base
LOCAL_SHARED_LIBRARIES += libgralloc_extra
LOCAL_SHARED_LIBRARIES += libfeature.face
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.vsdof_util
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libmtkcam_exif
LOCAL_SHARED_LIBRARIES += libladder
# for fence
LOCAL_SHARED_LIBRARIES += libsync
# DpBlitStream
LOCAL_SHARED_LIBRARIES += libdpframework
# for ion
LOCAL_SHARED_LIBRARIES += libion
LOCAL_SHARED_LIBRARIES += libion_mtk
# for fd container
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libcamalgo.fdft
################################################################################
#LOCAL_CFLAGS += -DGTEST

################################################################################
ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6771))
DCMF_VER := 1.0
LOCAL_CFLAGS+=-DDCMF_VER=10
$(info "PLATFORM $(PLATFORM) support DCMF version $(DCMF_VER)")
else
DCMF_VER := 1.0
LOCAL_CFLAGS+=-DDCMF_VER=10
LOCAL_CFLAGS+=-DDCMF_TEMP=1
$(info "PLATFORM $(PLATFORM) not support DCMF officially with version $(DCMF_VER)")
endif

################################################################################
# MFLL
################################################################################
MFLL_MF_TAG_VERSION           := 9

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

LOCAL_CFLAGS += -DMFLL_MF_TAG_VERSION=$(MFLL_MF_TAG_VERSION)

################################################################################
LOCAL_MODULE := libmtkcam.featurepipe.isppipe
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)
################################################################################
#
################################################################################

endif #MTK_CAM_STEREO_CAMERA_SUPPORT
