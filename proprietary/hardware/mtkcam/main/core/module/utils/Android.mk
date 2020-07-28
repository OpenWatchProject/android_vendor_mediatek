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
MY_LOCAL_SHARED_LIBRARIES :=
MY_LOCAL_SRC_FILES :=
MY_LOCAL_C_INCLUDES :=
MY_LOCAL_CFLAGS :=

#-----------------------------------------------------------
-include $(LOCAL_PATH)/config.mk

#-----------------------------------------------------------
$(info mtkcam utils MY_LOCAL_SHARED_LIBRARIES="$(sort $(MY_LOCAL_SHARED_LIBRARIES))")
$(info mtkcam utils MY_LOCAL_SRC_FILES="$(MY_LOCAL_SRC_FILES)")
$(info mtkcam utils MY_LOCAL_C_INCLUDES="$(MY_LOCAL_C_INCLUDES)")
$(info mtkcam utils MY_LOCAL_CFLAGS="$(MY_LOCAL_CFLAGS)")

#-----------------------------------------------------------
LOCAL_SRC_FILES += ../store.cpp
#
LOCAL_SRC_FILES += $(MY_LOCAL_SRC_FILES)

#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(MTKCAM_C_INCLUDES)
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/mtkcam/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/..
#
LOCAL_C_INCLUDES += $(MY_LOCAL_C_INCLUDES)

#-----------------------------------------------------------
LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
LOCAL_CFLAGS += $(MY_LOCAL_CFLAGS)
#
LOCAL_CFLAGS += -DMTKCAM_MODULE_GROUP_ID="MTKCAM_MODULE_GROUP_ID_UTILS"
LOCAL_CFLAGS += -DMTKCAM_MODULE_GROUP_ID_START="MTKCAM_MODULE_ID_UTILS_START"
LOCAL_CFLAGS += -DMTKCAM_MODULE_GROUP_ID_END="MTKCAM_MODULE_ID_UTILS_END"
LOCAL_CFLAGS += -DMTKCAM_GET_MODULE_FACTORY="MtkCam_getModuleFactory_utils"

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog libutils libhardware
LOCAL_SHARED_LIBRARIES += libmtkcam_ulog
#
LOCAL_SHARED_LIBRARIES += $(sort $(MY_LOCAL_SHARED_LIBRARIES))

#-----------------------------------------------------------
LOCAL_MODULE := libmtkcam_modulefactory_utils
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
ifeq ($(MTK_CAM_HAL_VERSION), 3)
LOCAL_MULTILIB := first
endif
#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))
