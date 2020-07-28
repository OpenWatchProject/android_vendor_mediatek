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
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
ifeq ($(MTK_CAM_VSDOF_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk
#-----------------------------------------------------------
LOCAL_SRC_FILES += n3d_hal_imp.cpp
LOCAL_SRC_FILES += n3d_hal_imp.log.cpp

ifeq ($(MTK_CAM_DEPTH_AF_SUPPORT),yes)
	LOCAL_SRC_FILES += ./DepthAF/stereodepth_hal.cpp
    LOCAL_CFLAGS += -DDEPTH_AF_SUPPORTED=1
endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += \
    $(VSDOF_COMMON_INC) \
    $(TOP)/vendor/mediatek/proprietary/hardware/libcamera_feature/libstereocam_lib/$(PLATFORM)/include \
    $(TOP)/vendor/mediatek/proprietary/hardware/mtkcam/include/algorithm/$(PLATFORM)/libutility \
	$(TOP)/vendor/mediatek/proprietary/hardware/libcamera_ext/lib/libutility/ \
    $(TOP)/$(MTK_PATH_SOURCE)/custom/$(PLATFORM)/hal/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/custom/$(PLATFORM)/hal/inc/$(MTK_CAM_SW_VERSION) \
    $(TOP)/$(MTK_PATH_SOURCE)/custom/$(PLATFORM)/hal/inc/$(AAA_VERSION) \
    $(TOP)/$(MTK_PATH_SOURCE)/custom/$(PLATFORM)/hal/inc/isp_tuning \
    $(TOP)/$(MTK_PATH_SOURCE)/custom/$(PLATFORM)/hal/inc/aaa/$(AAA_VERSION) \

LOCAL_SHARED_LIBRARIES += libcamalgo.stereocam
LOCAL_SHARED_LIBRARIES += lib3a.af.core

ifeq ($(HAL_MET_PROFILE), true)
LOCAL_C_INCLUDES += $(TOP)/vendor/mediatek/proprietary/external/met/met-tag
LOCAL_CFLAGS += -DMET_USER_EVENT_SUPPORT
endif

ifeq ($(TARGET_BUILD_VARIANT), user)
LOCAL_CFLAGS += -DIS_USER_LOAD=1
endif

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libfeatureiodrv_mem
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libdpframework

#-----------------------------------------------------------
LOCAL_MODULE := libfeature.vsdof.hal.n3d
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)


################################################################################
#
################################################################################
#include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))
endif	#MTK_CAM_VSDOF_SUPPORT
endif	#MTK_CAM_STEREO_CAMERA_SUPPORT
