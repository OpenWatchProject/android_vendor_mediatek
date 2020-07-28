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

# decouple pipeline policy with pipeline pluign interface
################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += FeatureSettingPolicy.cpp
LOCAL_SRC_FILES += FeatureSettingPolicy_Streaming.cpp

#-----------------------------------------------------------
ifeq ($(MTK_CAM_NR3D_SUPPORT),yes)
LOCAL_CFLAGS += -DNR3D_SUPPORTED
endif
#-----------------------------------------------------------
my-public-include-dirs += $(MTK_PATH_COMMON)/hal/inc
my-public-include-dirs += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
my-public-include-dirs += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6757))
LOCAL_HEADER_LIBRARIES += libcustom_hal_headers
endif
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam3/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
#
LOCAL_C_INCLUDES += $(my-public-include-dirs)

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam_3rdparty
LOCAL_SHARED_LIBRARIES += libmtkcam_3rdparty.core
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_exif
LOCAL_SHARED_LIBRARIES += libmtkcam_sysutils
#
LOCAL_SHARED_LIBRARIES += libcameracustom

#
LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider

ifeq ($(MTK_CAM_EIS_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += libcameracustom.eis
endif
ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
endif
#

# For FeatureProfileHelper
LOCAL_SHARED_LIBRARIES += libcam.feature_utils
LOCAL_SHARED_LIBRARIES += libcam.vhdr
#-----------------------------------------------------------
LOCAL_EXPORT_C_INCLUDE_DIRS := $(my-public-include-dirs)

#-----------------------------------------------------------
LOCAL_MODULE := libmtkcam_featurepolicy
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)



################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += PipelineSettingPolicyImpl.cpp
LOCAL_SRC_FILES += ConfigSettingPolicyMediator.cpp
LOCAL_SRC_FILES += RequestSettingPolicyMediator.cpp
LOCAL_SRC_FILES += RequestSettingPolicyMediatorPDE.cpp
LOCAL_SRC_FILES += RequestSettingPolicyMediatorMulticamZoom.cpp
#
LOCAL_SRC_FILES += config/ConfigPipelineNodesNeedPolicy.cpp
LOCAL_SRC_FILES += config/ConfigPipelineNodesNeedPolicy_multicam.cpp
LOCAL_SRC_FILES += config/ConfigPipelineNodesNeedPolicy_AppRaw16Reprocess.cpp
LOCAL_SRC_FILES += config/ConfigPipelineNodesNeedPolicy_HalJpeg.cpp
LOCAL_SRC_FILES += config/ConfigPipelineTopologyPolicy.cpp
LOCAL_SRC_FILES += config/SensorSettingPolicy.cpp
LOCAL_SRC_FILES += config/P1HwSettingPolicy.cpp
LOCAL_SRC_FILES += config/P1DmaNeedPolicy.cpp
LOCAL_SRC_FILES += config/ConfigStreamInfoPolicy.cpp
LOCAL_SRC_FILES += config/ConfigStreamInfoPolicy_HalJpeg.cpp
LOCAL_SRC_FILES += config/ConfigAppImageStreamInfoMaxBufNumPolicy.cpp
LOCAL_SRC_FILES += config/SensorSettingPolicy_4cell.cpp
# multi cam
LOCAL_SRC_FILES += config/P1HwSettingPolicy_multicam.cpp
LOCAL_SRC_FILES += config/SensorSettingPolicy_vsdof.cpp
LOCAL_SRC_FILES += request/IOMapPolicy_P2Node_multicam.cpp
#
LOCAL_SRC_FILES += request/FaceDetectionIntentPolicy.cpp
LOCAL_SRC_FILES += request/P2NodeDecisionPolicy.cpp
LOCAL_SRC_FILES += request/P2NodeDecisionPolicy_AppRaw16Reprocess.cpp
LOCAL_SRC_FILES += request/TopologyPolicy.cpp
LOCAL_SRC_FILES += request/TopologyPolicy_HalJpeg.cpp
LOCAL_SRC_FILES += request/TopologyPolicy_Multicam.cpp
LOCAL_SRC_FILES += request/CaptureStreamUpdaterPolicy.cpp
LOCAL_SRC_FILES += request/IOMapPolicy_P2Node.cpp
LOCAL_SRC_FILES += request/IOMapPolicy_NonP2Node.cpp
LOCAL_SRC_FILES += request/IOMapPolicy_NonP2Node_HalJpeg.cpp
LOCAL_SRC_FILES += request/RequestMetadataPolicy.cpp
LOCAL_SRC_FILES += request/RequestMetadataPolicy_HalJpeg.cpp
LOCAL_SRC_FILES += request/RequestMetadataPolicy_AppRaw16Reprocess.cpp
# multi-cam zoom
LOCAL_SRC_FILES += request/RequestSensorControlPolicy_Multicam.cpp

#-----------------------------------------------------------
ifeq ($(MTK_CAM_NR3D_SUPPORT),yes)
LOCAL_CFLAGS += -DNR3D_SUPPORTED
endif
#-----------------------------------------------------------
my-public-include-dirs += $(MTK_PATH_COMMON)/hal/inc
my-public-include-dirs += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
my-public-include-dirs += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam3/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
#
LOCAL_C_INCLUDES += $(my-public-include-dirs)

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES += libmtkcam_pipelinepolicy-streaminfo

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam_streamutils
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_pipelinemodel_utils
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
#
LOCAL_SHARED_LIBRARIES += libcameracustom
#
LOCAL_SHARED_LIBRARIES += libfeature.stereo.provider

ifeq ($(MTK_CAM_EIS_SUPPORT), yes)
LOCAL_SHARED_LIBRARIES += libcameracustom.eis
endif
ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
endif
#

# For FeaturePolicy
LOCAL_SHARED_LIBRARIES += libmtkcam_featurepolicy
#
LOCAL_SHARED_LIBRARIES += libmtkcam_3rdparty
LOCAL_SHARED_LIBRARIES += libmtkcam_3rdparty.core
#-----------------------------------------------------------
LOCAL_EXPORT_C_INCLUDE_DIRS := $(my-public-include-dirs)
LOCAL_EXPORT_STATIC_LIBRARY_HEADERS := libmtkcam_pipelinepolicy-streaminfo

#-----------------------------------------------------------
LOCAL_MODULE := libmtkcam_pipelinepolicy
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)



################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += config/SensorSettingPolicy_SMVR.cpp
LOCAL_SRC_FILES += config/ConfigStreamInfoPolicy_SMVR.cpp
LOCAL_SRC_FILES += config/SensorSettingPolicy_SMVRBatch.cpp
LOCAL_SRC_FILES += config/ConfigStreamInfoPolicy_SMVRBatch.cpp
LOCAL_SRC_FILES += request/RequestMetadataPolicy_SMVRBatch.cpp
LOCAL_SRC_FILES += RequestSettingPolicyMediator_SMVRBatch.cpp

#-----------------------------------------------------------
my-public-include-dirs += $(MTK_PATH_COMMON)/hal/inc
my-public-include-dirs += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc
my-public-include-dirs += $(MTK_PATH_CUSTOM_PLATFORM)/hal/inc/isp_tuning

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam3/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
#
LOCAL_C_INCLUDES += $(my-public-include-dirs)

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_streamutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_pipelinepolicy
#
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog

#-----------------------------------------------------------
LOCAL_MODULE := libmtkcam_pipelinepolicy-smvr
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += config/ConfigPipelineNodesNeedPolicy_Security.cpp
LOCAL_SRC_FILES += config/P1DmaNeedPolicy_Security.cpp
LOCAL_SRC_FILES += config/SensorSettingPolicy_Security.cpp

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_HEADER_LIBRARIES += libmtkcam3_headers
LOCAL_HEADER_LIBRARIES += libcameracustom_headers
LOCAL_HEADER_LIBRARIES += $(MTKCAM_INCLUDE_HEADER_LIB)

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_hwutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libmtkcam_streamutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_pipelinepolicy
#
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
#-----------------------------------------------------------
LOCAL_MODULE := libmtkcam_pipelinepolicy-security
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)



################################################################################
#
################################################################################
include $(CLEAR_VARS)

#-----------------------------------------------------------
-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/mtkcam.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += PipelineSettingPolicyFactoryImpl.cpp

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += libmtkcam_headers
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam3/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libdl
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
#
# For FeaturePolicy
LOCAL_SHARED_LIBRARIES += libmtkcam_featurepolicy
#
LOCAL_SHARED_LIBRARIES += libmtkcam_pipelinepolicy
LOCAL_SHARED_LIBRARIES += libmtkcam_pipelinepolicy-smvr
LOCAL_SHARED_LIBRARIES += libmtkcam_pipelinepolicy-security
#
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
#-----------------------------------------------------------
LOCAL_MODULE := libmtkcam_pipelinepolicy_factory
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#LOCAL_SHARED_LIBRARIES += libmtkcam_pipelinepolicy
LOCAL_SHARED_LIBRARIES += libmtkcam_pipelinepolicy-smvr
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
################################################################################
include $(call all-makefiles-under,$(LOCAL_PATH))
