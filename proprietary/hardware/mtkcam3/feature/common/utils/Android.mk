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

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#
# Start of common part ------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

# == Only build if not legacy ==
ifeq ($(MTKCAM_IP_BASE),1)

PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
PROFILE_V1 := mt6797
PROFILE_V2 := mt6757 kiboplus

ifneq (,$(filter $(PROFILE_V1),$(PLATFORM)))
PROFILE_HELPER_FOLDER := v1
else ifneq (,$(filter $(PROFILE_V2),$(PLATFORM)))
PROFILE_HELPER_FOLDER := v2
else
#TODO  currently default use v2 folder
PROFILE_HELPER_FOLDER := v2
endif

LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6761 mt6739))
    SUPPORT_DEFAULT_PQ := 1
else
    SUPPORT_DEFAULT_PQ := 0
endif
LOCAL_CFLAGS += -DSUPPORT_DEFAULT_PQ=$(SUPPORT_DEFAULT_PQ)

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6885))
LOCAL_CFLAGS += -DSUPPORT_MSTREAM
endif

LOCAL_SRC_FILES += ImageBufferUtils.cpp
LOCAL_SRC_FILES += FeatureProfile/$(PROFILE_HELPER_FOLDER)/FeatureProfileHelper.cpp
LOCAL_SRC_FILES += log/ILogger.cpp
LOCAL_SRC_FILES += p2/P2PlatInfo.cpp
LOCAL_SRC_FILES += p2/P2Data.cpp
LOCAL_SRC_FILES += p2/P2Pack.cpp
LOCAL_SRC_FILES += p2/P2Util.cpp
LOCAL_SRC_FILES += p2/P2IOClassfier.cpp
LOCAL_SRC_FILES += SecureBufferControlUtils.cpp
LOCAL_SRC_FILES += p2/DIPStream.cpp
LOCAL_SRC_FILES += p2/DIPStream_V4L2Stream.cpp
LOCAL_SRC_FILES += p2/DIPStream_NormalStream.cpp
LOCAL_SRC_FILES += ISPProfileMapper.cpp

# ------- INCLUDE  PATHES -----------------------------

LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTKCAM_FEATURE_INCLUDE)
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/custom

#For isp_tuning.h
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
#for gralloc_extra include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam3/include $(MTK_PATH_SOURCE)/hardware/mtkcam/include
#for af_core_lib
LOCAL_SHARED_LIBRARIES += lib3a.af.core
#for libflash_lib
LOCAL_SHARED_LIBRARIES += lib3a.flash

# Test
#$(warning !!!LOCAL C PATH = $(LOCAL_C_INCLUDES))

LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libcutils
#LOCAL_SHARED_LIBRARIES += libstdc++
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libhardware
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
LOCAL_SHARED_LIBRARIES += lib3a.awb.core
# For ImageBuffer
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils libmtkcam_imgbuf
# For Metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
# For p2 utils
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libmtkcam_exif
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils
# For p2 v4l2
# LOCAL_SHARED_LIBRARIES += libmtkcam_v4l2utils
#For AE
LOCAL_SHARED_LIBRARIES += lib3a.ae.core
LOCAL_SHARED_LIBRARIES += lib3a.af.core
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
LOCAL_SHARED_LIBRARIES += libcameracustom

ifeq ($(MTK_CAM_NEW_NVRAM_SUPPORT),1)
LOCAL_SHARED_LIBRARIES += libmtkcam_mapping_mgr
# For p2 utils
ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
endif
LOCAL_STATIC_LIBRARIES += libdip.iopipe.postproc
LOCAL_SHARED_LIBRARIES += libdip_postproc
LOCAL_SHARED_LIBRARIES += libcam.halsensor
endif

#-----------------------------------------------------------
LOCAL_MODULE := libcam.feature_utils
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)

endif
# if LEGACY == 0 end if
