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
MTKCAM_HAVE_HAL_3A_ADAPTER                      := '1'
MTKCAM_HAVE_HAL_3A_ADAPTER_POWER_NOTIFICATION   := '1'
#
MTKCAM_HAVE_CAM_MANAGER                         := '1'  # built-in if '1' ; otherwise not built-in

#-----------------------------------------------------------
LOCAL_SRC_FILES += adapter/Hal3AAdapter.cpp
#LOCAL_SRC_FILES += adapter/HalSensorAdapter.cpp
LOCAL_SRC_FILES += adapter/HalDeviceAdapter.cpp
LOCAL_SRC_FILES += adapter/PipelineModelManagerBase.cpp
LOCAL_SRC_FILES += adapter/PipelineModelManagerImpl.cpp
#
LOCAL_SRC_FILES += CameraSetting/CameraSettingMgr_Imp.cpp
LOCAL_SRC_FILES += CameraSetting/MyProcessedParams.cpp
#
LOCAL_SRC_FILES += PipelineModelMgr.cpp
LOCAL_SRC_FILES += PipelineModelFactory.cpp
LOCAL_SRC_FILES += PipelineDefaultImp.cpp
LOCAL_SRC_FILES += PipelineDefaultImp.ConfigHandler.cpp
LOCAL_SRC_FILES += PipelineDefaultImp.RequestHandler.cpp
#LOCAL_SRC_FILES += PipelineDefaultImp.VendorHandler.cpp
LOCAL_SRC_FILES += PipelineUtility.cpp
# hdr feature option
#ifeq ($(MTK_CAM_HDR_SUPPORT), yes)
#LOCAL_SRC_FILES += PipelineModel_Hdr.cpp
#endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/include $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/perfservice/perfservicenative
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc # custom common folder
#
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)

# if it is not basic branch, enable PerfService functions
#ifneq ($(strip $(MTK_BASIC_PACKAGE)),yes)
#LOCAL_CFLAGS += -DENABLE_PERFSERVICE
#endif

# hdr feature option
ifeq ($(MTK_CAM_HDR_SUPPORT), yes)
LOCAL_CFLAGS += -DMTKCAM_HAVE_HDR
endif

ifneq ($(MTK_BASIC_PACKAGE),yes)
LOCAL_CFLAGS += -DMTKCAM_SUPPORT_COMMON_P2NODE
endif
#
LOCAL_CFLAGS += -DMTKCAM_HAVE_HAL_SENSOR_ADAPTER="$(MTKCAM_HAVE_HAL_SENSOR_ADAPTER)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_HAL_3A_ADAPTER="$(MTKCAM_HAVE_HAL_3A_ADAPTER)"
LOCAL_CFLAGS += -DMTKCAM_HAVE_HAL_3A_ADAPTER_POWER_NOTIFICATION="$(MTKCAM_HAVE_HAL_3A_ADAPTER_POWER_NOTIFICATION)"
#
LOCAL_CFLAGS += -DMTKCAM_HAVE_CAM_MANAGER="$(MTKCAM_HAVE_CAM_MANAGER)"
#

#-----------------------------------------------------------
#
LOCAL_WHOLE_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libhardware
#
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam_streamutils
LOCAL_SHARED_LIBRARIES += libmtkcam_pipeline
LOCAL_SHARED_LIBRARIES += libmtkcam_hwnode
LOCAL_SHARED_LIBRARIES += libcam.iopipe
LOCAL_SHARED_LIBRARIES += libcam_extension
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libcameracustom.eis

ifeq "'1'" "$(strip $(MTKCAM_HAVE_CAM_MANAGER))"
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
else
$(warning "warning: FIXME: MTKCAM_HAVE_CAM_MANAGER=0")
endif

# adv cam setting option
ifneq ($(MTK_BASIC_PACKAGE),yes)
LOCAL_CFLAGS += -DMTKCAM_HAVE_ADV_SETTING=1
LOCAL_SHARED_LIBRARIES += libmtkcam.adv_setting
endif


#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libcam3_hwpipeline
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))

