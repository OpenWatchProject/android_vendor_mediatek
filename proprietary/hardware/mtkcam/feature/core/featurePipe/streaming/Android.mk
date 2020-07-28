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
ifeq ($(strip $(MTK_CAM_NR3D_SUPPORT)),yes)
LOCAL_CFLAGS += -DSUPPORT_3DNR
endif
################################################################################
LOCAL_SRC_FILES += StreamingFeature_Common.cpp
LOCAL_SRC_FILES += StreamingFeatureTimer.cpp
LOCAL_SRC_FILES += StreamingFeatureData.cpp
LOCAL_SRC_FILES += StreamingFeatureNode.cpp
LOCAL_SRC_FILES += ImgBufferStore.cpp
LOCAL_SRC_FILES += P2CamContext.cpp
LOCAL_SRC_FILES += RootNode.cpp
LOCAL_SRC_FILES += P2ANode.cpp
LOCAL_SRC_FILES += P2A_VHDR.cpp
LOCAL_SRC_FILES += P2A_FEFM.cpp
LOCAL_SRC_FILES += P2BNode.cpp
LOCAL_SRC_FILES += WarpNode.cpp
LOCAL_SRC_FILES += FDNode.cpp
LOCAL_SRC_FILES += VFBNode.cpp
LOCAL_SRC_FILES += MDPNode.cpp
LOCAL_SRC_FILES += EISNode.cpp
LOCAL_SRC_FILES += HelperNode.cpp
LOCAL_SRC_FILES += VendorNode.cpp
LOCAL_SRC_FILES += VendorMDPNode.cpp
LOCAL_SRC_FILES += RSCNode.cpp
LOCAL_SRC_FILES += StreamingFeaturePipe.cpp
LOCAL_SRC_FILES += StreamingFeaturePipeUsage.cpp
LOCAL_SRC_FILES += IStreamingFeaturePipe.cpp
LOCAL_SRC_FILES += P2A_3DNR.cpp
LOCAL_SRC_FILES += FMHal.cpp
LOCAL_SRC_FILES += WarpBase.cpp
LOCAL_SRC_FILES += WarpStream.cpp
LOCAL_SRC_FILES += GPUWarp.cpp
LOCAL_SRC_FILES += MDPWrapper.cpp
LOCAL_SRC_FILES += GPUWarpStream.cpp
LOCAL_SRC_FILES += WPEWarpStream.cpp
LOCAL_SRC_FILES += Dummy_RSCStream.cpp
LOCAL_SRC_FILES += RSCTuningStream.cpp
LOCAL_SRC_FILES += Dummy_WPEWarpStream.cpp
LOCAL_SRC_FILES += EISQControl.cpp
#dual zoom
ifeq ($(strip $(MTK_CAM_DUAL_ZOOM_SUPPORT)),yes)
LOCAL_SRC_FILES += FOVNode.cpp
LOCAL_SRC_FILES += FOVWarpNode.cpp
LOCAL_SRC_FILES += P2A_FOV.cpp
endif
# n3d
ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT),yes)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk
LOCAL_C_INCLUDES += $(VSDOF_COMMON_INC)
LOCAL_SRC_FILES += N3DNode.cpp
LOCAL_SRC_FILES += N3D_P2Node.cpp
LOCAL_SRC_FILES += N3DShotModeSelect.cpp
endif
# dual Vendor example
LOCAL_SRC_FILES += IVendorCore.cpp
LOCAL_SRC_FILES += VendorFOVNode.cpp
LOCAL_SRC_FILES += VendorExample.cpp
################################################################################
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
################################################################################
LOCAL_C_INCLUDES += $(call include-path-for, camera)
################################################################################
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/core
################################################################################
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/libs/nativewindow/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/libs/arect/include
################################################################################
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libcore
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libutility
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_ext/lib/libwarp

ifeq ($(strip $(MTK_CAM_DUAL_ZOOM_SUPPORT)),yes)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfov_lib/include
ifeq ($(strip $(MTK_CAM_DUAL_ZOOM_MTK_FOV_SUPPORT)),yes)
LOCAL_SHARED_LIBRARIES += libcamalgo.fov
endif
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.vsdof_util
endif
################################################################################
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/common/fb
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/common/FaceDetection/FD4.0
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/libfdft
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/libvfb
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/libutility
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/algorithm/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z )/libfov
################################################################################
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_3a/libflash_lib/$(PLATFORM)/include
################################################################################
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libladder
LOCAL_SHARED_LIBRARIES += libnativewindow
LOCAL_SHARED_LIBRARIES += libcamalgo.eis
LOCAL_SHARED_LIBRARIES += libcamalgo.warp
LOCAL_SHARED_LIBRARIES += libcamalgo.fsc
LOCAL_SHARED_LIBRARIES += libcamalgo.fdft
LOCAL_SHARED_LIBRARIES += libcamalgo.gyro
LOCAL_SHARED_LIBRARIES += libcamalgo.lmv
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libmtkcam_rsc
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libgralloc_extra
LOCAL_SHARED_LIBRARIES += libfeature_eis
LOCAL_SHARED_LIBRARIES += libfeature.face
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcameracustom.eis
LOCAL_SHARED_LIBRARIES += libfeature_3dnr
LOCAL_SHARED_LIBRARIES += libfeature_fsc
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils

ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT),yes)
LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.vsdof_util
endif
ifeq ($(strip $(MTK_CAM_DUAL_ZOOM_SUPPORT)),yes)
LOCAL_SHARED_LIBRARIES += libmtkcam.DualCam.hal
endif
LOCAL_SHARED_LIBRARIES += libhardware
################################################################################
LOCAL_STATIC_LIBRARIES += libmtkcam.featurepipe.core
LOCAL_SHARED_LIBRARIES += libion
################################################################################
LOCAL_MODULE := libmtkcam.featurepipe.streaming
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
ifneq ($(MTK_BASIC_PACKAGE),yes)
include $(MTK_SHARED_LIBRARY)
endif
################################################################################
#
################################################################################
