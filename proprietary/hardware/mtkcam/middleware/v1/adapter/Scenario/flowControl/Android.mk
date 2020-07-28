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

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk
#-----------------------------------------------------------
LOCAL_SRC_FILES += FlowControl.cpp
LOCAL_SRC_FILES += FlowControlBase.cpp
#
LOCAL_SRC_FILES += default/DefaultFlowControl.cpp
LOCAL_SRC_FILES += default/DefaultFlowControl.pipeline.cpp
#
LOCAL_SRC_FILES += eng/EngFlowControl.cpp
LOCAL_SRC_FILES += eng/EngFlowControl.pipeline.cpp
#
ifneq ($(MTK_BASIC_PACKAGE), yes)
LOCAL_SRC_FILES += feature/FeatureFlowControl.cpp
LOCAL_SRC_FILES += feature/FeatureFlowControl.pipeline.cpp
endif

#LOCAL_SRC_FILES += stereo/shot/buffer/CallbackImageBufferManager.cpp
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
MTKCAM_HAVE_MTKSTEREO           := '1'
ifneq ("",$(PIPELINE_VER))
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/v1/adapter/Scenario/flowControl/stereo/pipelinedata/$(PIPELINE_VER)/inc
endif
LOCAL_SRC_FILES += stereo/shot/metadata/ShutterCallback.cpp
LOCAL_SRC_FILES += stereo/shot/metadata/DNGMetadataCallback.cpp
LOCAL_SRC_FILES += stereo/shot/metadata/P2DoneCallback.cpp
LOCAL_SRC_FILES += stereo/shot/metadata/CollectCallback.cpp
LOCAL_SRC_FILES += stereo/shot/image/DepthMapImageCallback.cpp
LOCAL_SRC_FILES += stereo/shot/image/ExtraDataImageCallback.cpp
LOCAL_SRC_FILES += stereo/shot/image/JpegImageCallback.cpp
LOCAL_SRC_FILES += stereo/shot/image/Raw16ImageCallback.cpp
LOCAL_SRC_FILES += stereo/shot/image/PostViewImageCallback.cpp
LOCAL_SRC_FILES += stereo/shot/image/BgYuvImageCallback.cpp
LOCAL_SRC_FILES += stereo/shot/processor/ShotCallbackProcessor.cpp
LOCAL_SRC_FILES += stereo/shot/ImageShotCallbackFactory.cpp
LOCAL_SRC_FILES += stereo/shot/MetadataShotCallbackFactory.cpp
LOCAL_SRC_FILES += stereo/shot/builder/BaseCaptureRequestBuilder.cpp
LOCAL_SRC_FILES += stereo/shot/builder/SwBokehVSDOFCaptureRequestBuilder.cpp
LOCAL_SRC_FILES += stereo/shot/builder/DepthResultCaptureRequstBuilder.cpp
LOCAL_SRC_FILES += stereo/shot/builder/SwBokehPostProcessRequestBuilder.cpp
LOCAL_SRC_FILES += stereo/shot/CaptureRequestBuilder.cpp
LOCAL_SRC_FILES += stereo/shot/processor/BokehPostProcessor.cpp
LOCAL_SRC_FILES += stereo/shot/ImagePostProcessor.cpp
LOCAL_SRC_FILES += stereo/shot/builder/ThirdPartyCaptureRequestBuilder.cpp
LOCAL_SRC_FILES += stereo/StereoFlowControl.cpp
LOCAL_SRC_FILES += stereo/StereoFlowControl.pipeline.cpp
LOCAL_SRC_FILES += stereo/StereoFlowControl.P1Pipeline.cpp
LOCAL_SRC_FILES += stereo/StereoFlowControl.P2Pipeline.cpp
LOCAL_SRC_FILES += stereo/StereoFlowControl.request.cpp
ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT), yes)
MTKCAM_STEREO_DENOISE_SUPPORT   := '1'
else
MTKCAM_STEREO_DENOISE_SUPPORT   := '0'
endif
else
MTKCAM_HAVE_MTKSTEREO           := '0'
endif

#-----------------------------------------------------------
PLATFORM_IS_MT6765           := 0

ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6765))
PLATFORM_IS_MT6765           := 1
endif

LOCAL_CFLAGS += -DPLATFORM_IS_MT6765=$(PLATFORM_IS_MT6765)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/gralloc_extra/include

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/bwc/inc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/v1/PostProc/
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include/mtkcam/middleware/v1/PostProc
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/v1/common/CbImgBufMgr/
#
MY_ADAPTER_C_INCLUDES_PATH := $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/v1/adapter
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/inc
LOCAL_C_INCLUDES += $(MY_ADAPTER_C_INCLUDES_PATH)/Scenario/Shot/inc
ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)
#LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/v1/LegacyPipeline/stereo
LOCAL_C_INCLUDES += $(TOP)/$(MTKCAM_DRV_INCLUDE)
endif
#
LOCAL_C_INCLUDES += $(MTK_PATH_COMMON)/hal/inc/
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/
LOCAL_C_INCLUDES += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning
#
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/av/include
LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/middleware/v1/common/CbImgBufMgr/

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.videorawdump
LOCAL_WHOLE_STATIC_LIBRARIES += libcam.cbimgbufmgr

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libcameracustom
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libcam.postproc

#-----------------------------------------------------------
LOCAL_MODULE := libcam.camadapter.flowControl
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
ifeq "'1'" "$(strip $(MTKCAM_HAVE_MTKSTEREO))"
LOCAL_CFLAGS += -DMTKCAM_HAVE_MTKSTEREO="$(MTKCAM_HAVE_MTKSTEREO)"
LOCAL_CFLAGS += -DMTKCAM_STEREO_DENOISE_SUPPORT="$(MTKCAM_STEREO_DENOISE_SUPPORT)"
endif
#
#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

