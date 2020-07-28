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
LOCAL_SRC_FILES += Aee.cpp
LOCAL_SRC_FILES += Misc.cpp
LOCAL_SRC_FILES += Format.cpp
LOCAL_SRC_FILES += Profile.cpp
LOCAL_SRC_FILES += Property.cpp
#LOCAL_SRC_FILES += Sync.cpp
LOCAL_SRC_FILES += Time.cpp
LOCAL_SRC_FILES += LogTool.cpp
LOCAL_SRC_FILES += DebugScanLine.cpp
LOCAL_SRC_FILES += JobQueue.cpp
LOCAL_SRC_FILES += StlUtils.cpp
LOCAL_SRC_FILES += DebugDrawID.cpp
LOCAL_SRC_FILES += FrameController.cpp
LOCAL_SRC_FILES += FileCache.cpp

#-----------------------------------------------------------
LOCAL_HEADER_LIBRARIES += camerapostalgo_headers
#-----------------------------------------------------------
#LOCAL_CFLAGS += $(MTKCAM_CFLAGS)
#
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
LOCAL_CFLAGS += -DTARGET_BUILD_VARIANT=$(TARGET_BUILD_VARIANT)
#
# MTKCAM_HAVE_AEE_FEATURE
ifeq "yes" "$(strip $(HAVE_AEE_FEATURE))"
    MTKCAM_HAVE_AEE_FEATURE ?= 1
else
    MTKCAM_HAVE_AEE_FEATURE := 0
endif
#
# mtkcam Log Level (for compile-time loglevel control)
# WHEN MODIFY THE RULE HERE, MUST ALSO SYNC TO ../ulog/Android.mk
ifndef MTKCAM_LOG_LEVEL_DEFAULT
ifeq ($(TARGET_BUILD_VARIANT),user)
    MTKCAM_LOG_LEVEL_DEFAULT   := 2
else ifeq ($(TARGET_BUILD_VARIANT),userdebug)
    MTKCAM_LOG_LEVEL_DEFAULT   := 3
else
    MTKCAM_LOG_LEVEL_DEFAULT   := 4
endif
endif

#---- get Android log too much threshold (this only applied on user debug load)------
ifeq ($(TARGET_BUILD_VARIANT), userdebug)
    LOCAL_CFLAGS += -DMTKCAM_USER_DEBUG_LOAD="1"
else
    LOCAL_CFLAGS += -DMTKCAM_USER_DEBUG_LOAD="0"
endif
ifeq ($(TARGET_BUILD_VARIANT), eng)
    LOCAL_CFLAGS += -DMTKCAM_ENG_LOAD="1"
else
    LOCAL_CFLAGS += -DMTKCAM_ENG_LOAD="0"
endif

# eng/user debug load log threshold = 10000
LOCAL_CFLAGS += -DMTKCAM_ANDROID_LOG_MUCH_COUNT="10000"

ifeq ($(MTK_CAM_USER_DBG_LOG_OFF), yes)
    LOCAL_CFLAGS += -DMTKCAM_USER_DBG_LOG_OFF="1"
ifeq ($(TARGET_BUILD_VARIANT),userdebug)
    MTKCAM_LOG_LEVEL_DEFAULT   := 2
endif
else
    LOCAL_CFLAGS += -DMTKCAM_USER_DBG_LOG_OFF="0"
endif

LOCAL_CFLAGS += -DMTKCAM_LOG_LEVEL_DEFAULT=$(MTKCAM_LOG_LEVEL_DEFAULT)

# enable exception
LOCAL_CPP_FEATURES += exceptions
LOCAL_CPPFLAGS += -fexceptions
#
# enable exceptions
LOCAL_CPP_FEATURES := exceptions
LOCAL_CPPFLAGS := -fexceptions

#-----------------------------------------------------------
LOCAL_WHOLE_STATIC_LIBRARIES +=
#

#-----------------------------------------------------------
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += libcutils
#LOCAL_SHARED_LIBRARIES += libsync
#LOCAL_SHARED_LIBRARIES += libladder
#
ifeq "yes" "$(strip $(HAVE_AEE_FEATURE))"
    #LOCAL_SHARED_LIBRARIES += libaedv
endif
#

#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := libpostalgo_stdutils
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)


################################################################################
#
################################################################################
include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

