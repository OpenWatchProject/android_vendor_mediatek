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

include $(CLEAR_VARS)
#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning/$(MTK_CAM_SW_VERSION)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/$(MTK_CAM_SW_VERSION)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/aaa/$(MTK_CAM_SW_VERSION)
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/cam
LOCAL_EXPORT_C_INCLUDE_DIRS += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/debug_exif/cam/$(MTK_CAM_SW_VERSION)
LOCAL_MODULE := libcustom_isp_headers
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
MTKCAM_3A_COMMON_PATH := $(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/source/common
LOCAL_EXPORT_C_INCLUDE_DIRS += $(MTKCAM_3A_COMMON_PATH)/utils/aaa_utils
LOCAL_MODULE := libmtkcam_aaa_tils_headers
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(BUILD_HEADER_LIBRARY)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += aaa_utils.cpp
LOCAL_SRC_FILES += SttBufQ.cpp
LOCAL_SRC_FILES += time_utils.cpp
LOCAL_SRC_FILES += file_utils.cpp
LOCAL_SRC_FILES += string_utils.cpp
LOCAL_SRC_FILES += property_utils.cpp
LOCAL_SRC_FILES += interp_utils.cpp
LOCAL_SRC_FILES += aaa_trace.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/aaa/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libcustom_isp_headers

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
LOCAL_SHARED_LIBRARIES += libcutils libutils liblog
#-----------------------------------------------------------
LOCAL_MODULE := libcam.hal3a_utils
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := aaa_utils_common_includes

LOCAL_EXPORT_C_INCLUDE_DIRS:=$(LOCAL_PATH)

include $(BUILD_HEADER_LIBRARY)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

