# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
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


#
# data path framework test file - engine
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

include $(TOP)/$(MTK_PATH_SOURCE)/hardware/dpframework/mtkcam_inc.mk

MDP_VERSION:=2
ifeq ($(strip $(TARGET_BOARD_PLATFORM)), mt6799)
    ifeq ($(MDP_VERSION), 2)
        LOCAL_CFLAGS += -DMDP_VERSION2
        MDP_PLATFORM_VERSION := mt6799p
    endif
endif

#set platform folder name
ifeq ($(MDP_PLATFORM_VERSION), mt6799p)
    MDP_PLATFORM_FOLDER := mt6799p
    MDP_PATH_SWITCH :=
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735 mt6753 mt6737t))
    MDP_PLATFORM_FOLDER := mt6735
    MDP_PATH_SWITCH := /D1
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6735m mt6737m))
    MDP_PLATFORM_FOLDER := mt6735
    MDP_PATH_SWITCH := /D2
else ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6755 mt6750))
    MDP_PLATFORM_FOLDER := mt6755
    MDP_PATH_SWITCH :=
else
    MDP_PLATFORM_FOLDER := $(TARGET_BOARD_PLATFORM)
    MDP_PATH_SWITCH :=
endif

# Platform dependent part
MTK_MDP_PLATFORM_SRC := platform/$(MDP_PLATFORM_FOLDER)$(MDP_PATH_SWITCH)

LOCAL_SRC_FILES:= \
    ../../../common/util/DpReadBMP.cpp \

LOCAL_SRC_FILES += \
    BlitStream.cpp \
    IspStream.cpp \
    FragStream.cpp \
    AsyncBlitStream.cpp \
    VEncStream.cpp \

#SVP
SVP_TEST:=no
ifeq ($(SVP_TEST), yes)
    LOCAL_CFLAGS += -DCONFIG_FOR_SEC_VIDEO_PATH
endif

LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/hardware/dpframework/inc \
    $(TOPDIR)system/core/include/utils

LOCAL_C_INCLUDES += \
    $(TOPDIR)frameworks/native/include/utils \
    $(TOPDIR)system/core/include \
    $(TOPDIR)system/core/libion/include \
    $(TOP)/system/core/include/utils \
    $(TOP)/system/core/libsync/include/sync \
    $(TOP)/system/core/libsync \
    $(TOP)/system/core/include \
    $(MTK_PATH_SOURCE)/kernel/include \
    $(MTK_PATH_SOURCE)/external/include

LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/hardware/pq/v2.0/include \
    $(MTK_PATH_SOURCE)/hardware/m4u/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z ) \
    $(TOP)/vendor/mediatek/proprietary/external/libion_mtk/include \
    $(TOP)/vendor/mediatek/proprietary/external/include

LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/hardware/dpframework/common/buffer \
    $(MTK_PATH_SOURCE)/hardware/dpframework/common/core \
    $(MTK_PATH_SOURCE)/hardware/dpframework/MDP2.0/engine \
    $(MTK_PATH_SOURCE)/hardware/dpframework/common/osal \
    $(MTK_PATH_SOURCE)/hardware/dpframework/common/stream \
    $(MTK_PATH_SOURCE)/hardware/dpframework/common/util

LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/hardware/dpframework/MDP2.0/$(MTK_MDP_PLATFORM_SRC)/engine \
    $(MTK_PATH_SOURCE)/hardware/dpframework/MDP2.0/$(MTK_MDP_PLATFORM_SRC)

LOCAL_C_INCLUDES += \
    $(MTK_PATH_PLATFORM)/kernel/drivers/cmdq \


LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(MTK_PATH_PLATFORM)/hardware/include/$(MDP_PATH_SWITCH)

ifeq ($(IS_LEGACY), 0)
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/$(TARGET_BOARD_PLATFORM)/drv
else
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/include
LOCAL_C_INCLUDES += $(MTK_MTKCAM_PLATFORM)/include/mtkcam/drv
endif

LOCAL_C_INCLUDES += $(TOP)/external/gtest/include

#SVP
ifeq ($(SVP_TEST), yes)
    LOCAL_C_INCLUDES += \
            $(MTK_PATH_SOURCE)/trustzone/trustonic/source/bsp/platform/$(TRUSTONIC_FOLDER)/t-sdk/TlcSdk/Out/Public/ \
            $(MTK_PATH_SOURCE)/trustzone/trustonic/source/external/mobicore/common/302a/common/MobiCore/inc/ \
            $(MTK_PATH_SOURCE)/trustzone/trustonic/source/external/secmem/platform/$(TRUSTONIC_FOLDER)/public/
endif

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libion \
    liblog \
    libsync \
    libdpframework \
    libion_mtk

#SVP
ifeq ($(SVP_TEST), yes)
LOCAL_SHARED_LIBRARIES += \
    libsec_mem
endif

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := dpframework_test
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_STEM_32 := dpframework_GTest
LOCAL_MODULE_STEM_64 := dpframework_GTest64
LOCAL_MULTILIB := both

include $(BUILD_NATIVE_TEST)
