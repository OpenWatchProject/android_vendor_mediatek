# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2016. All rights reserved.
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
ifneq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk

ifeq ($(MTKCAM_IP_BASE),1)

################################################################################
LOCAL_SRC_FILES := p2sync/SyncManager.cpp
LOCAL_SRC_FILES += p2sync/IDualCamStreamingFeaturePipe.cpp
LOCAL_SRC_FILES += p2sync/DualCamStreamingFeaturePipe.cpp
LOCAL_SRC_FILES += Frame3AControl/Frame3AControl.cpp
LOCAL_SRC_FILES += policy/IDualCamPolicy.cpp

ifeq ($(strip $(MTK_CAM_DUAL_ZOOM_SUPPORT)),yes)
LOCAL_SRC_FILES += policy/DualCamZoomPolicy.cpp
LOCAL_SRC_FILES += policy/DualCamZoom/DualZoomPolicy_Common.cpp
LOCAL_SRC_FILES += policy/DualCamZoom/AAAJudgeStrategy.cpp
LOCAL_SRC_FILES += policy/DualCamZoom/CameraContext.cpp
LOCAL_SRC_FILES += policy/DualCamZoom/CameraStatus.cpp
LOCAL_SRC_FILES += policy/DualCamZoom/CameraStatusCommand.cpp
endif

ifeq ($(strip $(MTK_CAM_STEREO_DENOISE_SUPPORT)),yes)
LOCAL_SRC_FILES += policy/Denoise/ThermalDetector.cpp
LOCAL_SRC_FILES += policy/Denoise/BatteryDetector.cpp
LOCAL_SRC_FILES += policy/DualCamDenoisePolicy.cpp
LOCAL_SRC_FILES += policy/DualCamDenoisePolicy.decision.cpp
endif

ifeq ($(strip $(MTK_CAM_STEREO_CAMERA_SUPPORT)),yes)
ifeq ($(VSDOF_VERSION), 2)
LOCAL_SRC_FILES += DualCamMF/DualCamMFStrategy.cpp
endif

ifeq ($(INDEP_STEREO_PROVIDER),true)
LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif

endif

ifeq ($(strip $(MTK_CAM_STEREO_CAMERA_SUPPORT)),yes)
ifeq ($(strip $(MTK_CAM_VSDOF_SUPPORT)),yes)
LOCAL_SRC_FILES += p2sync/DualCamBokehFeaturePipe.cpp
LOCAL_SRC_FILES += policy/DualCamBokehPolicy.cpp
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware
LOCAL_SHARED_LIBRARIES += libeffecthal.base
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.bokeh
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.depthmap
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.vsdof_util
LOCAL_SHARED_LIBRARIES += libladder
LOCAL_WHOLE_STATIC_LIBRARIES += libfeature.vsdof.effecthal
endif

ifeq ($(INDEP_STEREO_PROVIDER),true)
LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif

endif

ifeq (yes, $(filter $(strip $(MTK_CAM_DUAL_ZOOM_SUPPORT)) $(strip $(MTK_CAM_STEREO_DENOISE_SUPPORT)) $(strip $(MTK_CAM_STEREO_CAMERA_SUPPORT)), yes))
LOCAL_SRC_FILES += utils/DualCameraHWHelper.cpp
endif

################################################################################

LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -Wno-unused-value
LOCAL_CFLAGS += -Wno-unused-variable
LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_CFLAGS += -Wno-reorder
LOCAL_CFLAGS += -O3

################################################################################
#LOCAL_C_INCLUDES += $(call include-path-for, camera)
################################################################################
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
# need these?
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
ifneq ($(MTKCAM_ALGO_INCLUDE),)
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libcore
LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libutility
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfdft_lib/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/libudf/libladder
#LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)/libfov
endif
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/DualCam/p2sync
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/DualCam/policy
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/DualCam/Frame3AControl
ifeq ($(strip $(MTK_CAM_STEREO_DENOISE_SUPPORT)),yes)
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
endif
# end
################################################################################

################################################################################
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)
# need these?
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/custom/common/hal/inc
# end
################################################################################
LOCAL_SHARED_LIBRARIES += libc
#LOCAL_SHARED_LIBRARIES += libcamalgo
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libgralloc_extra
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libc++
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.streaming
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libfeature_3dnr
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
ifeq ($(strip $(MTK_CAM_STEREO_DENOISE_SUPPORT)),yes)

ifeq ($(INDEP_STEREO_PROVIDER),true)
LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif
#LOCAL_SHARED_LIBRARIES += libcam.legacypipeline
endif
ifeq ($(strip $(MTK_CAM_DUAL_ZOOM_SUPPORT)),yes)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/libcamera_feature/libfov_lib/include
ifeq ($(INDEP_STEREO_PROVIDER),true)
LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif

ifeq ($(strip $(MTK_CAM_DUAL_ZOOM_MTK_FOV_SUPPORT)),yes)
LOCAL_SHARED_LIBRARIES += libcamalgo.fov
endif
LOCAL_SHARED_LIBRARIES += libmtkcam.DualCam.hal
LOCAL_SHARED_LIBRARIES += libfeatureiodrv_mem
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
endif
################################################################################
# LOCAL_STATIC_LIBRARIES += libcameracustom.camera
################################################################################
################################################################################
LOCAL_MODULE := libmtkcam.DualCam
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
################################################################################
ifneq ($(MTK_BASIC_PACKAGE),yes)
#include $(BUILD_STATIC_LIBRARY)
include $(BUILD_SHARED_LIBRARY)
endif
endif
endif
#
ifeq ($(strip $(MTK_CAM_DUAL_ZOOM_SUPPORT)),yes)
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif
