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
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
MTKCAM_HAVE_HAL_SENSOR_ADAPTER                  := '1'
MTKCAM_HAVE_HAL_3A_ADAPTER                      := '0'
MTKCAM_HAVE_HAL_3A_ADAPTER_POWER_NOTIFICATION   := '0'

#-----------------------------------------------------------
LOCAL_SRC_FILES += adapter/Hal3AAdapter.cpp
LOCAL_SRC_FILES += adapter/HalSensorAdapter.cpp
LOCAL_SRC_FILES += adapter/PipelineModelManagerBase.cpp
LOCAL_SRC_FILES += adapter/PipelineModelManagerImpl.cpp
#
LOCAL_SRC_FILES += InFlightRequest.cpp
LOCAL_SRC_FILES += PipelineModelFactory.cpp
LOCAL_SRC_FILES += PipelineModel_Default.cpp
LOCAL_SRC_FILES += ScenarioControl.cpp

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
#
LOCAL_C_INCLUDES += system/media/camera/include
#
# bwc
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/bwc/inc

LOCAL_HEADER_LIBRARIES := libhardware_headers
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
LOCAL_CFLAGS += -DMTKCAM_HAVE_HAL_SENSOR_ADAPTER="$(MTKCAM_HAVE_HAL_SENSOR_ADAPTER)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_HAL_3A_ADAPTER="$(MTKCAM_HAVE_HAL_3A_ADAPTER)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_HAL_3A_ADAPTER_POWER_NOTIFICATION="$(MTKCAM_HAVE_HAL_3A_ADAPTER_POWER_NOTIFICATION)"
#
# fix build warnings from ispio_utility.h
LOCAL_CFLAGS += -DDONT_USE_QUERYRAWSTRIDE
LOCAL_CFLAGS += -DDONT_USE_QUERYRAWBITPERPIXEL
LOCAL_CFLAGS += -DDONT_USE_QUERYSUGGBURSTQNUM

#-----------------------------------------------------------
LOCAL_STATIC_LIBRARIES += libcam3_pipeline_DefaultPipelineModel
#
LOCAL_WHOLE_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidltransport
LOCAL_SHARED_LIBRARIES += libhwbinder
LOCAL_SHARED_LIBRARIES += android.hardware.power@1.0
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.power@1.1
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libcam3_utils
LOCAL_SHARED_LIBRARIES += libcam3_pipeline
LOCAL_SHARED_LIBRARIES += libcam3_hwnode
LOCAL_SHARED_LIBRARIES += libcam.halsensor
LOCAL_SHARED_LIBRARIES += libcam.hal3a.v3
LOCAL_SHARED_LIBRARIES += libbwc
# for PIP
LOCAL_SHARED_LIBRARIES += libcam_hwutils
LOCAL_SHARED_LIBRARIES += libfeatureio
#-----------------------------------------------------------
LOCAL_MODULE := libcam3_hwpipeline
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))

