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

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

LOCAL_CFLAGS := $(MTKCAM_CFLAGS)

SUPPORTED_PLATFORM := mt6757 mt6763 mt6765 mt6768
ifneq (,$(filter $(SUPPORTED_PLATFORM),$(TARGET_BOARD_PLATFORM)))
# --------------------------------------------------------------------

LOCAL_SRC_FILES := \
    AbfAdapter.cpp \
    AbfAdapterUT.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libcutils \
    libcamalgo.abf \
    libmtkcam_modulehelper \
    libmtkcam_stdutils \
    libmtkcam_imgbuf \
    libmtkcam_sysutils \
    libcameracustom \
    libdpframework\
    libcam.feature_utils\
    libmtkcam_tuning_utils

# Perf service is not present in basic package
ifneq ($(MTK_BASIC_PACKAGE),yes)
LOCAL_SHARED_LIBRARIES += libhidlbase \
    libhidltransport \
    libhwbinder \
    android.hardware.power@1.0 \
    vendor.mediatek.hardware.power@1.1
LOCAL_CFLAGS += -DSWABF_USE_PERF
endif

# --------------------------------------------------------------------
else # not in SUPPORTED_PLATFORM

# Build a dummy version to avoid compile error
LOCAL_SRC_FILES := \
    AbfAdapterDummy.cpp

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils

endif
# --------------------------------------------------------------------

LOCAL_EXPORT_C_INCLUDE_DIRS:=$(LOCAL_PATH)

LOCAL_HEADER_LIBRARIES := libhardware_headers libmtkcam_headers libcameracustom_headers libutils_headers liblog_headers libmtkcam3_headers

LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE := libabfadp
ifeq ($(MTK_CAM_HAL_VERSION), 3)
LOCAL_MULTILIB := first
endif
include $(BUILD_SHARED_LIBRARY)
