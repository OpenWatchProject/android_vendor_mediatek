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
PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/v1/adapter.mk

#-----------------------------------------------------------
MTKCAM_HAVE_MTKDEFAULT          := '1'      # built-in if '1' ; otherwise not built-in
#
MTKCAM_HAVE_SENSOR_HAL          := '1'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_3A_HAL              := '1'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_CAMPIPE             := '0'      # built-in if '1' ; otherwise not built-in
MTKCAM_HAVE_CAMSHOT             := '1'      # built-in if '1' ; otherwise not built-in
# stereo
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
MTKCAM_HAVE_MTKSTEREO           := '1'      # built-in if '1' ; otherwise not built-in
else
MTKCAM_HAVE_MTKSTEREO           := '0'      # built-in if '1' ; otherwise not built-in
endif
# dual
ifeq ($(MTK_CAM_DUAL_ZOOM_SUPPORT), yes)
MTKCAM_HAVE_MTKDUALZOOM         := '1'      # built-in if '1' ; otherwise not built-in
else
MTKCAM_HAVE_MTKDUALZOOM         := '0'      # built-in if '1' ; otherwise not built-in
endif
# denoise
ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT), yes)
MTKCAM_HAVE_MTKDENOISE          := '1'      # built-in if '1' ; otherwise not built-in
else
MTKCAM_HAVE_MTKDENOISE          := '0'      # built-in if '1' ; otherwise not built-in
endif
# hdr
ifeq ($(MTK_CAM_HDR_SUPPORT), yes)
MTKCAM_HAVE_MTKHDR              := '1'      # built-in if '1' ; otherwise not built-in
else
MTKCAM_HAVE_MTKHDR              := '0'
endif

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKSTEREO))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKSTEREO="$(MTKCAM_HAVE_MTKSTEREO)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKDEFAULT))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKDEFAULT="$(MTKCAM_HAVE_MTKDEFAULT)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_SENSOR_HAL))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_SENSOR_HAL="$(MTKCAM_HAVE_SENSOR_HAL)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_3A_HAL))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_3A_HAL="$(MTKCAM_HAVE_3A_HAL)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_CAMSHOT))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_CAMSHOT="$(MTKCAM_HAVE_CAMSHOT)"
endif
#

#-----------------------------------------------------------
LOCAL_SRC_FILES += \
    BaseCamAdapter.Instance.cpp \
    BaseCamAdapter.cpp \

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libdngop_lib/include
#
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/inc
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include

#-----------------------------------------------------------
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKDEFAULT))"
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.default
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKSTEREO))"
#
VER1_0_PLATFORMS := mt6797
PIPELINE_VER := ""
ifneq (,$(filter $(VER1_0_PLATFORMS),$(PLATFORM)))
    PIPELINE_VER := 1.0
else
    PIPELINE_VER := 1.5
endif
$(info  platform=$(PLATFORM), current pipeline version=$(PIPELINE_VER))

ifneq ("",$(PIPELINE_VER))
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/v1/adapter/Scenario/flowControl/stereo/pipelinedata/$(PIPELINE_VER)/inc
endif
#
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.stereo
endif
#
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.flowControl
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.buffer
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.scenario
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.utils.exifjpeg
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.recordbuffersnapshot

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.camadapter.state

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += libjpeg
ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
endif
#
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_fwkutils
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libcam1_utils
LOCAL_SHARED_LIBRARIES += libcam.paramsmgr
LOCAL_SHARED_LIBRARIES += libmtkcam_exif
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils
LOCAL_SHARED_LIBRARIES += libfeature.face
LOCAL_SHARED_LIBRARIES += libcam.feature_utils
LOCAL_SHARED_LIBRARIES += libcam.postproc

#
ifneq ($(strip $(MTKCAM_HAVE_MFB_SUPPORT)),0)
LOCAL_SHARED_LIBRARIES += libmfllcore
endif
#

# Plugin
LOCAL_SHARED_LIBRARIES += libcam_extension



#----for ALGO---------------
#LOCAL_SHARED_LIBRARIES += libdngop
LOCAL_SHARED_LIBRARIES += libcamalgo.dngop
LOCAL_SHARED_LIBRARIES += libcamera_metadata
#-------

#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_CAMSHOT))"
LOCAL_SHARED_LIBRARIES += libcam.camshot
endif
#-------
LOCAL_SHARED_LIBRARIES += libJpgEncPipe
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcameracustom.eis
#
LOCAL_SHARED_LIBRARIES += libcam.iopipe
#
#LOCAL_SHARED_LIBRARIES += libfeatureio.feature

#for pipeline
#LOCAL_SHARED_LIBRARIES += libcam.streambufprovider
LOCAL_SHARED_LIBRARIES += libcam.legacypipeline
LOCAL_SHARED_LIBRARIES += libmtkcam_pipeline

$(warning FIXME workaround)
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_hwnode
LOCAL_SHARED_LIBRARIES += libcam.client
#
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore

LOCAL_SHARED_LIBRARIES += libcam3_contextbuilder
# stereo
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKSTEREO))"
    -include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk
    ifeq ($(INDEP_STEREO_PROVIDER),true)
        LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
    else
        LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif

LOCAL_SHARED_LIBRARIES += libbwc
LOCAL_SHARED_LIBRARIES += libmtkcam_streamutils
# dual
ifeq "'1'" "$(filter $(strip $(MTKCAM_HAVE_MTKDUALZOOM)) $(strip $(MTKCAM_HAVE_MTKDENOISE)) $(strip $(MTKCAM_HAVE_MTKSTEREO)), '1')"
LOCAL_SHARED_LIBRARIES += libmtkcam.DualCam
endif
#dual zoom
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKDUALZOOM))"
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk

ifeq ($(INDEP_STEREO_PROVIDER), true)
    LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider
else
    LOCAL_SHARED_LIBRARIES += libfeature.vsdof.hal
endif
LOCAL_SHARED_LIBRARIES += libmtkcam.DualCam.hal
endif
endif
LOCAL_SHARED_LIBRARIES += libeffecthal.base
LOCAL_SHARED_LIBRARIES += libcam.postproc
#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcam.camadapter
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

