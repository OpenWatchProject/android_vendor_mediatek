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
ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT), yes)
ifeq ($(BMDN_VER), 1.5)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
# MET
MTKCAM_HAVE_MET          		:= '0'      # built-in if '1' ; otherwise not built-in
################################################################################
ifneq ($(MTKCAM_IP_BASE),0)
ifneq ($(MTK_BASIC_PACKAGE), yes)
ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT), yes)
LOCAL_SRC_FILES += BMDeNoisePipe_Common.cpp
LOCAL_SRC_FILES += BMDeNoisePipeNode.cpp
LOCAL_SRC_FILES += BMDeNoisePipeFactory.cpp

# common nodes
LOCAL_SRC_FILES += ./nodes/P2AFMNode.cpp
LOCAL_SRC_FILES += ./nodes/BMN3DNode.cpp
LOCAL_SRC_FILES += ./nodes/BMHelperNode.cpp
LOCAL_SRC_FILES += ./nodes/SWNRNode.cpp
# LOCAL_SRC_FILES += ./nodes/DPENode.cpp

# BMDN
# LOCAL_SRC_FILES += BMDeNoisePipe.cpp
# LOCAL_SRC_FILES += ./nodes/RootNode.cpp
# LOCAL_SRC_FILES += ./nodes/PreProcessNode.cpp
# LOCAL_SRC_FILES += ./nodes/DeNoiseNode.cpp
# LOCAL_SRC_FILES += ./nodes/PostProcessNode.cpp

# MFHR
LOCAL_SRC_FILES += BMDeNoiseHRPipe.cpp
LOCAL_SRC_FILES += ./nodes/RootHRNode.cpp
LOCAL_SRC_FILES += ./nodes/BayerPreProcessNode.cpp
LOCAL_SRC_FILES += ./nodes/MonoPreProcessNode.cpp
LOCAL_SRC_FILES += ./nodes/BSSNode.cpp
LOCAL_SRC_FILES += ./nodes/FusionNode.cpp
LOCAL_SRC_FILES += ./nodes/MFNRNode.cpp

# sub-modules
LOCAL_SRC_FILES += ./bufferPool/BMBufferPool.cpp
LOCAL_SRC_FILES += ./exif/ExifWriter.cpp
endif
endif
endif
################################################################################
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(call include-path-for, camera)
LOCAL_C_INCLUDES += $(VSDOF_COMMON_INC)
################################################################################
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/core
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/core/featurePipe
################################################################################
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/common
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/core/featurePipe/core/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/core/featurePipe/vsdof/util
################################################################################
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/pd_buf_mgr
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/mtkcam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/cam
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/cam/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a

## effectHAL
LOCAL_C_INCLUDES += $(EFFECTHAL_C_INCLUDE)
## algo/3a
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_ALGO_INCLUDE)
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/lib3a/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include
##
# LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
## ion
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/kernel/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/
################################################################################
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam.featurepipe.core
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libnativewindow
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libEGL
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libgralloc_extra
LOCAL_SHARED_LIBRARIES += libfeature.face
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.vsdof_util
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif
# DpBlitStream
LOCAL_SHARED_LIBRARIES += libdpframework
# for ion
LOCAL_SHARED_LIBRARIES += libion
LOCAL_SHARED_LIBRARIES += libion_mtk
# BSS
LOCAL_SHARED_LIBRARIES += libcamalgo
# MET
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MET))"
LOCAL_C_INCLUDES += $(TOP)/vendor/mediatek/proprietary/external/met/met-tag
LOCAL_SHARED_LIBRARIES += libmet-tag
LOCAL_CFLAGS += -DMET_USER_EVENT_SUPPORT
endif
# MFNR
LOCAL_SHARED_LIBRARIES += libmfllcore

LOCAL_SHARED_LIBRARIES += libmtkcam_exif

################################################################################
LOCAL_MODULE := libmtkcam.featurepipe.bmdenoise
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)
################################################################################
#
################################################################################
endif	#check BMDN_VER
endif #MTK_CAM_STEREO_DENOISE_SUPPORT
