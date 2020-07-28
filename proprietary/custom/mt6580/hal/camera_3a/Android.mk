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

# Use project imgsensor first
ifeq ($(wildcard $(MTK_PATH_CUSTOM)/hal/camera_3a),)
################################################################################
#
################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
#-----------------------------------------------------------
#$(call config-custom-folder,hal:hal)

#-----------------------------------------------------------
LOCAL_SRC_FILES += \
  af_tuning_custom.cpp \
  aaa_common_custom.cpp \
  aaa_scheduling_custom_main.cpp \
  aaa_scheduling_custom_main2.cpp \
  aaa_scheduling_custom_sub.cpp \
  aaa_scheduling_param_custom.cpp \
  aaa_yuv_tuning_custom.cpp \
  ae_tuning_custom_main.cpp \
  ae_tuning_custom_main2.cpp \
  ae_tuning_custom_sub.cpp \
  awb_tuning_custom.cpp \
  awb_tuning_custom_main2.cpp \
  awb_tuning_custom_sub.cpp \
  awb_tuning_custom_main.cpp \
  camera_custom_flicker.cpp \
  isp_tuning_custom.cpp \
  isp_tuning_effect.cpp \
  isp_tuning_idx.cpp \
  isp_tuning_idx_preview.cpp \
  isp_tuning_idx_video.cpp \
  isp_tuning_idx_capture.cpp \
  isp_tuning_user.cpp \
  shading_tuning_custom.cpp \
  tsf_tuning_custom.cpp \
  flashawb_tuning_custom.cpp \
  camera_custom_msdk.cpp \
  camera_custom_if_yuv.cpp \
  camera_custom_if.cpp \
  aaa_state_flow_custom.cpp


#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc \
                    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/isp_tuning \
                    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning \
                    $(TOP)/$(MTK_PATH_CUSTOM)/hal/inc/aaa \
                    $(TOP)/$(MTK_PATH_CUSTOM)/hal/camera/inc \
                    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/cgen/cfgfileinc \
                    $(TOP)/$(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
                    $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/mtkcam \


LOCAL_C_INCLUDES += $(TOP)/$(MTK_MTKCAM_PLATFORM)/include


LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/include

#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/camera_3a/inc \
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/aaa \
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_CUSTOM_PLATFORM)/hal/inc \
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/kernel/imgsensor/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc/camera_feature
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/legacy/include
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)/..

LOCAL_HEADER_LIBRARIES += libutils_headers libcutils_headers liblog_headers
#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES +=
#
LOCAL_WHOLE_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_MODULE := libcameracustom.camera.3a
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)

################################################################################
#
################################################################################
#include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))

endif
