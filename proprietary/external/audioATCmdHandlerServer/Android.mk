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


#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

ifeq ($(strip $(BOARD_USES_MTK_AUDIO)),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# Fixme: never tested with system_proc
#ifeq ($(strip $(MTK_AUDIO_TUNING_TOOL_VERSION)),V1)
#    SYSTEM_PROC = true
#    LOCAL_CFLAGS += -DSYSTEM_PROC
#endif

LOCAL_SRC_FILES := \
    AudioCmdHandlerService.cpp \
    Parameter.cpp \
    AudioCmdHandler.cpp \
    AudioToolkit.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libbinder \
    liblog

ifeq ($(strip $(SYSTEM_PROC)),true)
LOCAL_SHARED_LIBRARIES += \
    libmedia \
    libaudioclient \
    libaudiocomponentengine \
    libaudiocompensationfilter
else
LOCAL_SHARED_LIBRARIES += \
    libaudiocompensationfilter_vendor \
    libaudiocomponentengine_vendor \
    libaudioprimarydevicehalifclient
endif

LOCAL_HEADER_LIBRARIES += libaudio_system_headers

LOCAL_C_INCLUDES := \
    $(TOP)/frameworks/av/include \
    $(MTK_PATH_SOURCE)/external/bessound_HD \
    $(MTK_PATH_SOURCE)/hardware/audio/common/include \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc \
    $(MTK_ROOT)/external

ifeq ($(strip $(BOARD_USES_GENERIC_AUDIO)),true)
    LOCAL_CFLAGS += -DGENERIC_AUDIO
else
    LOCAL_CFLAGS += -DMTK_AUDIO
endif

# WB Speech Support
ifeq ($(MTK_WB_SPEECH_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_WB_SPEECH_SUPPORT
endif
#WB Speech Support

# Audio HD Record
ifeq ($(MTK_AUDIO_HD_REC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_AUDIO_HD_REC_SUPPORT
endif
# Audio HD Record

# DMNR3.0 Support
ifeq ($(MTK_HANDSFREE_DMNR_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_HANDSFREE_DMNR_SUPPORT
endif
# DMNR3.0 Support

# VOIP enhance Support
ifeq ($(MTK_VOIP_ENHANCEMENT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_VOIP_ENHANCEMENT_SUPPORT
endif
# VOIP enhance Support

# DMNR tuning at modem side
ifeq ($(DMNR_TUNNING_AT_MODEMSIDE),yes)
    LOCAL_CFLAGS += -DDMNR_TUNNING_AT_MODEMSIDE
endif
# DMNR tuning at modem side

# wifi only
ifeq ($(MTK_TB_WIFI_3G_MODE),WIFI_ONLY)
    LOCAL_CFLAGS += -DMTK_WIFI_ONLY_SUPPORT
endif
# wifi only

# 3g data
ifeq ($(MTK_TB_WIFI_3G_MODE),3GDATA_SMS)
    LOCAL_CFLAGS += -DMTK_3G_DATA_SUPPORT
endif
# 3g data

# ASR
ifeq ($(MTK_ASR_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_ASR_SUPPORT
endif
# ASR

# voip normal dmnr
ifeq ($(MTK_VOIP_NORMAL_DMNR),yes)
    LOCAL_CFLAGS += -DMTK_VOIP_NORMAL_DMNR
endif
# voip normal dmnr

# handsfree voip dmnr
ifeq ($(MTK_HANDSFREE_VOIP_DMNR),yes)
    LOCAL_CFLAGS += -DMTK_HANDSFREE_VOIP_DMNR
endif
# handsfree voip dmnr

# check if there is receiver
ifeq ($(DISABLE_EARPIECE),yes)
    LOCAL_CFLAGS += -DMTK_DISABLE_EARPIECE
endif
# check if there is receiver

ifeq ($(MTK_SUPPORT_TC1_TUNNING),yes)
    LOCAL_CFLAGS += -DMTK_AUDIO_GAIN_TABLE
endif

ifeq ($(strip $(MTK_AURISYS_FRAMEWORK_SUPPORT)),yes)
    LOCAL_CFLAGS += -DMTK_AURISYS_FRAMEWORK_SUPPORT
endif

# MTK Audio Tuning Tool Version
ifneq ($(MTK_AUDIO_TUNING_TOOL_VERSION),)
  ifneq ($(strip $(MTK_AUDIO_TUNING_TOOL_VERSION)),V1)
    MTK_AUDIO_TUNING_TOOL_V2_PHASE:=$(shell echo $(MTK_AUDIO_TUNING_TOOL_VERSION) | sed 's/V2.//g')
    LOCAL_CFLAGS += -DMTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
    LOCAL_CFLAGS += -DMTK_AUDIO_TUNING_TOOL_V2_PHASE=$(MTK_AUDIO_TUNING_TOOL_V2_PHASE)
  endif
endif

ifeq ($(MTK_AURISYS_FRAMEWORK_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_AURISYS_FRAMEWORK_SUPPORT
endif

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE := audiocmdservice_atci
ifeq ($(strip $(SYSTEM_PROC)),true)
    LOCAL_PROPRIETARY_MODULE := false
else
    LOCAL_PROPRIETARY_MODULE := true
endif
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32
LOCAL_INIT_RC := audiocmdservice_atci.rc

ifeq ($(TARGET_SIMULATOR),TRUE)
    LOCAL_LDLIBS += -lpthread
endif

include $(MTK_EXECUTABLE)


endif #BOARD_USES_MTK_AUDIO
