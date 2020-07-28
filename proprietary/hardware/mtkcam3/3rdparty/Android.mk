# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2018. All rights reserved.
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
################################################################################
ifneq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
ifneq ($(strip $(BUILD_MTK_LDVT)),yes)
################################################################################
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
################################################################################
include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
################################################################################
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/include
################################################################################
LOCAL_SRC_FILES += scenario_mgr.cpp
LOCAL_SRC_FILES += sensor_control.cpp
################################################################################
LOCAL_SHARED_LIBRARIES += libmtkcam_3rdparty.core
LOCAL_SHARED_LIBRARIES += libmtkcam_3rdparty.mtk
LOCAL_SHARED_LIBRARIES += libmtkcam_3rdparty.customer
################################################################################
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)
################################################################################
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata

ifneq ($(strip $(MTKCAM_HAVE_MFB_SUPPORT)),0)
LOCAL_SHARED_LIBRARIES += libmfllcore
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
LOCAL_SHARED_LIBRARIES += libcam.feature_utils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
endif

LOCAL_SHARED_LIBRARIES += libcamalgo.dngop
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_imgbuf
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
# tuning dump
LOCAL_SHARED_LIBRARIES += libmtkcam_tuning_utils
# HDR
LOCAL_SHARED_LIBRARIES += libcam.iopipe
# SWNR
ifeq ($(MTK_CAM_NEW_NVRAM_SUPPORT),1)
LOCAL_SHARED_LIBRARIES += libmtkcam_mapping_mgr
endif
LOCAL_SHARED_LIBRARIES += libcameracustom
# FD
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
# #DP
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libmtkcam.featurepipe.vsdof_util
# Dump Buffer
LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider


################################################################################
LOCAL_MODULE := libmtkcam_3rdparty
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)
################################################################################
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))
################################################################################
endif # BUILD_MTK_LDVT
endif # MTK_EMULATOR_SUPPORT
################################################################################

