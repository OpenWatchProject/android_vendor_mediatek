# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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
LOCAL_SRC_FILES += FileReadRule.cpp
LOCAL_SRC_FILES += FileDumpNamingHint.cpp
LOCAL_SRC_FILES += CommonRule.cpp
LOCAL_SRC_FILES += genFileName_HW_AAO.cpp
LOCAL_SRC_FILES += genFileName_JPG.cpp
LOCAL_SRC_FILES += genFileName_LCSO.cpp
LOCAL_SRC_FILES += genFileName_LSC2.cpp
LOCAL_SRC_FILES += genFileName_LSC.cpp
LOCAL_SRC_FILES += genFileName_TSF.cpp
LOCAL_SRC_FILES += genFileName_RAW.cpp
LOCAL_SRC_FILES += genFileName_Reg.cpp
LOCAL_SRC_FILES += genFileName_TUNING.cpp
LOCAL_SRC_FILES += genFileName_YUV.cpp
LOCAL_SRC_FILES += genFileName_VSDOF_BUFFER.cpp
LOCAL_SRC_FILES += genFileName_YNR_FD_TBL.cpp
LOCAL_SRC_FILES += genFileName_YNR_INT_TBL.cpp
LOCAL_SRC_FILES += genFileName_YNR_ALPHA_MAP.cpp
LOCAL_SRC_FILES += genFileName_DCESO.cpp
LOCAL_SRC_FILES += genFileName_LTMSO.cpp
LOCAL_SRC_FILES += genFileName_LTM_CURVE.cpp
LOCAL_SRC_FILES += genFileName_BPC_TBL.cpp
LOCAL_SRC_FILES += genFileName_face_info.cpp
LOCAL_SRC_FILES += genFileName_LCESHO.cpp
#------------------------------------------------------------------------------
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735 mt6737t))
    ISP_PROFILE_HEADER_PATH    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735m mt6737 mt6737m))
    ISP_PROFILE_HEADER_PATH    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc
else
    ISP_PROFILE_HEADER_PATH    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
endif
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(ISP_PROFILE_HEADER_PATH)
#------------------------------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
MTK_DP_VERSION := 0
ifeq ($(TARGET_BOARD_PLATFORM), mt6771)
    MTK_DP_VERSION := 5
endif
ifeq ($(TARGET_BOARD_PLATFORM), mt6775)
    MTK_DP_VERSION := 6
endif
ifeq ($(TARGET_BOARD_PLATFORM), mt6765)
    MTK_DP_VERSION := 4
endif
ifeq ($(TARGET_BOARD_PLATFORM), mt6768)
    MTK_DP_VERSION := 4
endif
ifeq ($(TARGET_BOARD_PLATFORM), mt6779)
    MTK_DP_VERSION := 9
endif
ifeq ($(TARGET_BOARD_PLATFORM), mt6763)
    MTK_DP_VERSION := 3
endif

LOCAL_CFLAGS += -DMTK_DP_VERSION=$(MTK_DP_VERSION)

#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libmtkcam_tuning_utils
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libhardware

#-----------------------------------------------------------
LOCAL_INTERMEDIATES         := $(call local-intermediates-dir)
LOCAL_GEN_TOOL              := $(LOCAL_PATH)/enum2str
LOCAL_ISP_PROFILE_STR_ARRAY := $(LOCAL_INTERMEDIATES)/EIspProfile_string.h
ifeq ($(MTK_CAM_NEW_NVRAM_SUPPORT),1)
    LOCAL_ISP_PROFILE_HEADER    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/tuning_mapping/cam_idx_struct_ext.h
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735 mt6737t))
    LOCAL_ISP_PROFILE_HEADER    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/D1/inc/isp_tuning/isp_tuning.h
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735m mt6737 mt6737m))
    LOCAL_ISP_PROFILE_HEADER    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/D2/inc/isp_tuning/isp_tuning.h
else
    LOCAL_ISP_PROFILE_HEADER    := $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/isp_tuning.h
endif

$(LOCAL_ISP_PROFILE_STR_ARRAY): $(LOCAL_ISP_PROFILE_HEADER) $(LOCAL_GEN_TOOL)
	$(LOCAL_GEN_TOOL) $(LOCAL_ISP_PROFILE_HEADER) EIspProfile_ strEIspProfile > $@ || { rm $@; exit -1; }

LOCAL_GENERATED_SOURCES += $(LOCAL_ISP_PROFILE_STR_ARRAY)
LOCAL_C_INCLUDES += $(LOCAL_INTERMEDIATES)

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)

