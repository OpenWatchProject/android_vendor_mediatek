# Copyright Statement:

# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2010. All rights reserved.

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

# The following software/firmware and/or related documentation ("MediaTek Software")
# have been modified by MediaTek Inc. All revisions are subject to any receiver's
# applicable license agreements with MediaTek Inc.

################################################################################

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
#-----------------------------------------------------------
MTKCAM_HAVE_MTKSTEREO           ?= '1'
MTKCAM_HAVE_SENSOR_HAL          ?= '1'
MTKCAM_HAVE_3A_HAL              ?= '1'
MTKCAM_HAVE_CAMSHOT             ?= '0'
MTKCAM_HAVE_VIDEO_SNAPSHOT      ?= '1'
MTKCAM_HAVE_EIS                 ?= '1'
ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT), yes)
MTKCAM_STEREO_DENOISE_SUPPORT           := '1'      # built-in if '1' ; otherwise not built-in
else
MTKCAM_STEREO_DENOISE_SUPPORT           := '0'      # built-in if '1' ; otherwise not built-in
endif

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKSTEREO))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKSTEREO="$(MTKCAM_HAVE_MTKSTEREO)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_SENSOR_HAL))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_SENSOR_HAL="$(MTKCAM_HAVE_SENSOR_HAL)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_3A_HAL))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_3A_HAL="$(MTKCAM_HAVE_3A_HAL)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_CAMSHOT))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_CAMSHOT="$(MTKCAM_HAVE_CAMSHOT)"
endif
#
ifeq "'1'" "$(strip $(MTKCAM_STEREO_DENOISE_SUPPORT))"
LOCAL_CFLAGS += -DMTKCAM_STEREO_DENOISE_SUPPORT="$(MTKCAM_STEREO_DENOISE_SUPPORT)"
endif
#------------------------------------------------------
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKSTEREO))"
LOCAL_SRC_FILES += \
    ./v3/CaptureCmdQueThread.cpp \
    ./v3/StereoAdapter.3A.cpp \
    ./v3/StereoAdapter.Capture.cpp \
    ./v3/StereoAdapter.CaptureCallback.cpp \
    ./v3/StereoAdapter.Preview.cpp \
    ./v3/StereoAdapter.Record.cpp \
    ./v3/StereoAdapter.cpp \
    ./v3/StereoAdapter.Param.cpp \
    ./v3/ZipImageCallbackThread.cpp \
    ./v3/StereoAdapter.dump.cpp \

endif


#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc

LOCAL_C_INCLUDES += $(MTKCAM_ALGO_INCLUDE)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/libcamera_feature/libdngop_lib/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/custom/$(PLATFORM)/hal/inc/isp_tuning
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/include
#
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/inc
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/Scenario/Shot/inc
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/Scenario/Shot/EngShot
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/Scenario/Shot/StereoShot
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/Scenario/flowControl/stereo
#
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc/isp_tuning
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc/aaa
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM)/hal/inc/debug_exif/aaa
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKSTEREO))"
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/$(MTKCAM_HAL_VERSION)/v1/adapter/Scenario/flowControl/stereo/
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/$(MTKCAM_HAL_VERSION)/v1/adapter/Scenario/Shot/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/$(MTKCAM_HAL_VERSION)/v1/adapter/Scenario/Shot/StereoShot
ifneq ("",$(PIPELINE_VER))
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/v1/adapter/Scenario/flowControl/stereo/pipelinedata/$(PIPELINE_VER)/inc
endif
endif
#
$(warning "[Remove Me] should not include directly")
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
#
#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=

LOCAL_STATIC_LIBRARIES +=

LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libcamalgo.dngop
#-----------------------------------------------------------
LOCAL_MODULE := libcam.camadapter.stereo
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKSTEREO))"
include $(MTK_STATIC_LIBRARY)
endif


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
