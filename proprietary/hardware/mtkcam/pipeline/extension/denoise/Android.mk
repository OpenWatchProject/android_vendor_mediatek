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
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# -----------------------------------------------------------------------------
# Global include file
# -----------------------------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

# -----------------------------------------------------------------------------
# To build libcam.extension.denoise if only if (MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT > 0)
# -----------------------------------------------------------------------------
ifneq ($(strip $(MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT)),0)

# -----------------------------------------------------------------------------
# Source code
# -----------------------------------------------------------------------------
LOCAL_SRC_FILES += controller/BaseDNCtrler.cpp
LOCAL_SRC_FILES += controller/BMDNCtrler.cpp
LOCAL_SRC_FILES += controller/MFHRCtrler.cpp
LOCAL_SRC_FILES += BMDNImp.cpp

# -----------------------------------------------------------------------------
# Include path(es)
# -----------------------------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
# for mtkcam/pipeline/extension/IVendorManager need
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
# for stereo setting provider
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc

# -----------------------------------------------------------------------------
# denoise implementation
# -----------------------------------------------------------------------------
LOCAL_STATIC_LIBRARIES += libcam.extension.utils


# -----------------------------------------------------------------------------
# C Flags
# -----------------------------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_CFLAGS += -D__SCOPE_TIMER # define to enable scope timer log


# -----------------------------------------------------------------------------
# Describes local module
# -----------------------------------------------------------------------------
LOCAL_MODULE := libcam.extension.denoise
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
include $(MTK_STATIC_LIBRARY)

endif # MTKCAM_HAVE_DUALCAM_DENOISE_SUPPORT
