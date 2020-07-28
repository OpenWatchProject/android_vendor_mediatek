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

-include $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/feature/common/vsdof/vsdof_common.mk

LOCAL_MODULE := TestStereoSizeProvider
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#TARGET_PREFER_32_BIT := true

#-----------------------------------------------------------
LOCAL_SRC_FILES += main.cpp \
                   ../stereo_size_provider.cpp \
                   ../../setting-provider/stereo_setting_provider.cpp \

#*************** Rear ***************
ifeq ($(MTK_CAM_STEREO_DENOISE_SUPPORT), yes)
	LOCAL_CFLAGS += -DSTEREO_DENOISE_SUPPORTED=1

	##### 4:3 #####
	LOCAL_SRC_FILES += Capture_4_3_Rear_Rotate0_BM.cpp
	LOCAL_SRC_FILES += Capture_4_3_Rear_Rotate90_BM.cpp

	##### 16:9 #####
	LOCAL_SRC_FILES += Capture_16_9_Rear_Rotate0_BM.cpp
	LOCAL_SRC_FILES += Capture_16_9_Rear_Rotate90_BM.cpp

	# B+M VSDoF
	ifeq ($(MTK_CAM_VSDOF_SUPPORT), yes)
		LOCAL_SRC_FILES += Capture_16_9_Rear_Rotate90_BM_VSDoF.cpp
		LOCAL_SRC_FILES += Capture_4_3_Rear_Rotate90_BM_VSDoF.cpp
		LOCAL_SRC_FILES += Preview_16_9_Rear_Rotate90_BM_VSDoF.cpp
		LOCAL_SRC_FILES += Record_16_9_Rear_Rotate90_BM_VSDoF.cpp
		LOCAL_SRC_FILES += Preview_4_3_Rear_Rotate90_BM_VSDoF.cpp
		LOCAL_SRC_FILES += Record_4_3_Rear_Rotate90_BM_VSDoF.cpp

		LOCAL_SRC_FILES += Capture_16_9_Rear_Rotate0_BM_VSDoF.cpp
		LOCAL_SRC_FILES += Capture_4_3_Rear_Rotate0_BM_VSDoF.cpp
		LOCAL_SRC_FILES += Preview_16_9_Rear_Rotate0_BM_VSDoF.cpp
		LOCAL_SRC_FILES += Record_16_9_Rear_Rotate0_BM_VSDoF.cpp
		LOCAL_SRC_FILES += Preview_4_3_Rear_Rotate0_BM_VSDoF.cpp
		LOCAL_SRC_FILES += Record_4_3_Rear_Rotate0_BM_VSDoF.cpp
	endif
else
	##### 4:3 #####
	LOCAL_SRC_FILES += Capture_4_3_Rear_Rotate0_BB.cpp
	#LOCAL_SRC_FILES += Capture_4_3_Rear_Rotate90_BB.cpp

	LOCAL_SRC_FILES += Preview_4_3_Rear_Rotate0_BB.cpp
	#LOCAL_SRC_FILES += Preview_4_3_Rear_Rotate90_BB.cpp

	LOCAL_SRC_FILES += Record_4_3_Rear_Rotate0_BB.cpp
	#LOCAL_SRC_FILES += Record_4_3_Rear_Rotate90_BB.cpp

	##### 16:9 #####
	LOCAL_SRC_FILES += Capture_16_9_Rear_Rotate0_BB.cpp
	#LOCAL_SRC_FILES += Capture_16_9_Rear_Rotate90_BB.cpp

	LOCAL_SRC_FILES += Preview_16_9_Rear_Rotate0_BB.cpp

	LOCAL_SRC_FILES += Record_16_9_Rear_Rotate0_BB.cpp
	#LOCAL_SRC_FILES += Record_16_9_Rear_Rotate90_BM.cpp
endif

####################  4:3 ####################
LOCAL_SRC_FILES += Capture_4_3_Front_Rotate90_BB.cpp
LOCAL_SRC_FILES += Preview_4_3_Front_Rotate90_BB.cpp
LOCAL_SRC_FILES += Record_4_3_Front_Rotate90_BB.cpp

#################### 16:9 ####################
LOCAL_SRC_FILES += Capture_16_9_Front_Rotate90_BB.cpp
LOCAL_SRC_FILES += Preview_16_9_Front_Rotate90_BB.cpp
LOCAL_SRC_FILES += Record_16_9_Front_Rotate90_BB.cpp


#-----------------------------------------------------------
LOCAL_C_INCLUDES += \
    $(VSDOF_COMMON_INC) \
    ../pass2 \

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#
LOCAL_STATIC_LIBRARIES +=

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_stdutils
#
LOCAL_SHARED_LIBRARIES += libmtkcam_metadata
LOCAL_SHARED_LIBRARIES += libmtkcam_metastore
LOCAL_SHARED_LIBRARIES += libmtkcam_modulehelper
LOCAL_SHARED_LIBRARIES += libcameracustom
#

LOCAL_CFLAGS += -DGTEST

include $(BUILD_NATIVE_TEST)
################################################################################
#
################################################################################
#include $(CLEAR_VARS)
#include $(call all-makefiles-under,$(LOCAL_PATH))

