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
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/driver.mk

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
MTKCAM_USE_LEGACY_IMAGEIO := '1'
MTKCAM_USE_LEGACY_SIMAGER := '1'
MTKCAM_USE_LEGACY_JPEGHW := '1'
#---
# FIXME, ResDrv in CamDrv
#---
MTKCAM_USE_LEGACY_CAMDRV := '1'
ifeq ($(BUILD_MTK_LDVT),yes)
    MTKCAM_USE_LEGACY_SIMAGER := '0'
    MTKCAM_USE_LEGACY_JPEGHW := '0'
endif
#-----------------------------------------------------------
IOPIPE_ENABLE_MET_PROFILE := no

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_P1_PLATFORM)/drv
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(MTKCAM_DRV_P1_PLATFORM)
LOCAL_C_INCLUDES += $(MTKCAM_ISP_INCLUDE)
#this include is temp solution, should be removed after ispio_utility.h is removed!!
LOCAL_C_INCLUDES += $(MTKCAM_ISP_INCLUDE)/imageio
# cam_capibility
LOCAL_C_INCLUDES += $(MTKCAM_ISP_INCLUDE)/imageio/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include
#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libhardware_headers
#-----------------------------------------------------------
LOCAL_SRC_FILES += Cam_cap.cpp
LOCAL_SRC_FILES += IoPipeEvent.cpp

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
LOCAL_CFLAGS += -DMTKCAM_USE_LEGACY_IMAGEIO="$(MTKCAM_USE_LEGACY_IMAGEIO)"
LOCAL_CFLAGS += -DMTKCAM_USE_LEGACY_JPEGHW="$(MTKCAM_USE_LEGACY_JPEGHW)"

#-----------------------------------------------------------
ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
endif
ifeq ($(MTKCAM_P2_DECOUPLE),1)
LOCAL_WHOLE_STATIC_LIBRARIES += libdip.iopipe.postproc
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6739 mt6761))
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.iopipe.dipwrapper
else
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.iopipe.postproc
endif
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.iopipe.camio
ifneq ($(BUILD_MTK_LDVT),yes)
ifeq "'1'" "$(strip $(MTKCAM_USE_LEGACY_SIMAGER))"
    LOCAL_WHOLE_STATIC_LIBRARIES += libcam.iopipe.simager
endif
endif
ifeq ($(BUILD_MTK_LDVT),yes)
#    LOCAL_SHARED_LIBRARIES += libaedv
endif
#
#LOCAL_STATIC_LIBRARIES := libcamdrv_isp

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
#LOCAL_SHARED_LIBRARIES += libstdc++

LOCAL_SHARED_LIBRARIES += libimageio_plat_drv
LOCAL_SHARED_LIBRARIES += libimageio_plat_pipe
ifeq ($(MTKCAM_P2_DECOUPLE),1)
LOCAL_SHARED_LIBRARIES += libdip_postproc
endif
#bit true dump library
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils
#
#ifneq ($(BUILD_MTK_LDVT),yes)
    LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
    LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
    LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
#endif
LOCAL_SHARED_LIBRARIES += libcam.halsensor
#
ifeq "'1'" "$(strip $(MTKCAM_USE_LEGACY_IMAGEIO))"
    LOCAL_SHARED_LIBRARIES += libimageio
#    LOCAL_SHARED_LIBRARIES += libcam.campipe
endif
#

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6761))
LOCAL_SHARED_LIBRARIES  += libcamdrv
LOCAL_SHARED_LIBRARIES  += libmtkcam_hwutils
else
LOCAL_SHARED_LIBRARIES  += libcamdrv_isp
LOCAL_SHARED_LIBRARIES  += libcamdrv_imem
LOCAL_SHARED_LIBRARIES  += libhardware
endif

# Performance Profiling
ifeq ($(IOPIPE_ENABLE_MET_PROFILE),yes)
    LOCAL_SHARED_LIBRARIES += libmet-tag
endif

# SImager
ifneq ($(BUILD_MTK_LDVT),yes)
ifeq "'1'" "$(strip $(MTKCAM_USE_LEGACY_SIMAGER))"
	LOCAL_SHARED_LIBRARIES += libdpframework
endif
endif
#

#
ifeq "'1'" "$(strip $(MTKCAM_USE_LEGACY_JPEGHW))"
    LOCAL_SHARED_LIBRARIES += libJpgEncPipe
endif
#

LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcam.iopipe
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))

