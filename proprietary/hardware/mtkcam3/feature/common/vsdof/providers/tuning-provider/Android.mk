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
# ifeq ($(MTK_CAM_STEREO_CAMERA_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)

################################################################################
#
################################################################################
include $(CLEAR_VARS)

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam3/feature/common/vsdof/vsdof_common.mk

#-----------------------------------------------------------
LOCAL_SRC_FILES += stereo_tuning_provider_kernel.cpp

STEREO_TUNINGS_FOLDER := default
ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6797))
else ifeq ($(PLATFORM), $(filter $(PLATFORM),kiboplus mt6757))
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6758 mt6763))
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6765))
STEREO_TUNINGS_FOLDER := mt6765
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6768))
STEREO_TUNINGS_FOLDER := mt6768
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6799))
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6771 mt6775))
STEREO_TUNINGS_FOLDER := mt6771
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6785))
STEREO_TUNINGS_FOLDER := mt6785
else ifeq ($(PLATFORM), $(filter $(PLATFORM),mt6779))
STEREO_TUNINGS_FOLDER := mt6779
endif

#-----------------------------------------------------------
LOCAL_C_INCLUDES += \
    $(VSDOF_COMMON_INC) \
    $(LOCAL_PATH)/$(STEREO_TUNINGS_FOLDER)/tunings \
    $(LOCAL_PATH)/sw_tuning/tunings \
    $(LOCAL_PATH)/../setting-provider/ \

ifneq ($(STEREO_TUNINGS_FOLDER),default)
	LOCAL_CFLAGS += -DHAS_STEREO_TUNING=1
else
	LOCAL_CFLAGS += -DHAS_STEREO_TUNING=0
endif
LOCAL_WHOLE_STATIC_LIBRARIES += libfeature.vsdof.hal.hw_tunings
LOCAL_WHOLE_STATIC_LIBRARIES += libfeature.vsdof.hal.sw_tunings

# Support JSON parse error handling
LOCAL_CPPFLAGS += -fexceptions
#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog

#-----------------------------------------------------------

#-----------------------------------------------------------
LOCAL_MODULE := libfeature.vsdof.hal.tuning_provider
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_STATIC_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

# endif	#MTK_CAM_STEREO_CAMERA_SUPPORT
