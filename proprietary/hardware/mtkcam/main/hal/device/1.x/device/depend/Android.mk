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

#-----------------------------------------------------------
MTKCAM_HAVE_SENSOR_HAL          := '1'
MTKCAM_HAVE_3A_HAL              := '1'
MTKCAM_HAVE_CAM_MANAGER         := '1'

#-----------------------------------------------------------
LOCAL_SRC_FILES += DefaultCameraDevice1.cpp
# Stereo
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
    LOCAL_SRC_FILES += StereoCameraDevice1.cpp
    ifeq ($(CAMERA_HAL_VERSION), 3)
        LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
    else
        LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/feature/include/
        -include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk

    	ifeq ($(INDEP_STEREO_PROVIDER), true)
    		LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
    	else
    		LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
    	endif
    endif

    ifeq ($(filter $(strip $(MTK_CAM_DUAL_ZOOM_SUPPORT)) \
                   $(strip $(MTK_CAM_STEREO_DENOISE_SUPPORT)) \
                   $(strip $(MTK_CAM_STEREO_CAMERA_SUPPORT)), yes), yes)
        LOCAL_SRC_FILES += DualCameraDevice1.cpp
        LOCAL_SHARED_LIBRARIES += libmtkcam.DualCam
        ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT),yes)
            LOCAL_SHARED_LIBRARIES += libcam.legacypipeline
            LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
        endif
    endif
endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/main/hal/device/1.x
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include

LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/external/aee/binary/inc
endif
# Stereo
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
endif

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

LOCAL_CFLAGS += -DLOG_TAG='"mtkcam-dev1"'

LOCAL_CFLAGS += -DMTKCAM_HAVE_SENSOR_HAL="$(MTKCAM_HAVE_SENSOR_HAL)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_3A_HAL="$(MTKCAM_HAVE_3A_HAL)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_CAM_MANAGER="$(MTKCAM_HAVE_CAM_MANAGER)"

# HRD Monitor
ifeq ($(strip $(MTK_HEART_RATE_MONITOR_SUPPORT)),yes)
    MTKCAM_HR_MONITOR_SUPPORT                 := '1'
else
    MTKCAM_HR_MONITOR_SUPPORT                 := '0'
endif
LOCAL_CFLAGS += -DMTKCAM_HR_MONITOR_SUPPORT="$(MTKCAM_HR_MONITOR_SUPPORT)"

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libmtkcam_device1_base
LOCAL_STATIC_LIBRARIES += libmtkcam_device1_hidlcommon
LOCAL_STATIC_LIBRARIES += android.hardware.camera.common@1.0-helper

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES +=

LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils

LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidlmemory
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += android.hardware.camera.device@1.0
LOCAL_SHARED_LIBRARIES += android.hardware.graphics.mapper@2.0
LOCAL_SHARED_LIBRARIES += android.hardware.graphics.mapper@3.0
LOCAL_SHARED_LIBRARIES += android.hidl.allocator@1.0
LOCAL_SHARED_LIBRARIES += android.hidl.memory@1.0

LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_fwkutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libcam1_utils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils

LOCAL_SHARED_LIBRARIES += libcam.paramsmgr
LOCAL_SHARED_LIBRARIES += libcam.client
LOCAL_SHARED_LIBRARIES += libcam.camadapter

LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.device@1.1

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
endif

# Stereo
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcam.postproc
endif

#[TODO] Remove this one if finish graphicbuffer remove finished
LOCAL_SHARED_LIBRARIES += libui

#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libmtkcam_device1
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
ifeq ($(MTK_CAM_HAL_VERSION), 3)
LOCAL_MULTILIB := first
endif
#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
