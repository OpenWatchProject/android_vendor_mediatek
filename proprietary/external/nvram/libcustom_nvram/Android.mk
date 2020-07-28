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


LOCAL_PATH:=$(call my-dir)
include $(CLEAR_VARS)
#LOCAL_ARM_MODE:=arm

# DMNR 3.0
ifeq ($(strip $(MTK_HANDSFREE_DMNR_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_HANDSFREE_DMNR_SUPPORT
endif
# DMNR 3.0

# Dual Mic Support
ifeq ($(MTK_DUAL_MIC_SUPPORT),yes)
LOCAL_CFLAGS += -D__MTK_DUAL_MIC_SUPPORT__
endif
# Dual Mic Support

# VoIP normal mode DMNR Support
ifeq ($(MTK_VOIP_NORMAL_DMNR),yes)
LOCAL_CFLAGS += -D__MTK_VOIP_NORMAL_DMNR__
endif
# VoIP normal mode DMNR Support

# VoIP handsfree mode DMNR Support
ifeq ($(MTK_VOIP_HANDSFREE_DMNR),yes)
LOCAL_CFLAGS += -D__MTK_VOIP_HANDSFREE_DMNR__
endif
# VoIP handsfree mode DMNR Support

# Merge Interface Support
ifeq ($(MTK_MERGE_INTERFACE_SUPPORT),yes)
LOCAL_CFLAGS += -D__MTK_MERGE_INTERFACE_SUPPORT__
endif
# Merge Interface Support

# BT I2S interface Support
ifeq ($(MTK_BT_I2S_SUPPORT),yes)
LOCAL_CFLAGS += -D__MTK_BT_I2S_SUPPORT__
endif
# BT I2S interface Support

ifeq ($(MTK_CONSYS_ADIE), MT6631)
LOCAL_CFLAGS += -DMTK_CONSYS_ADIE_6631
endif

# SmartPA Support
ifeq ($(findstring smartpa, $(MTK_AUDIO_SPEAKER_PATH)), smartpa)
LOCAL_CFLAGS += -D__NXP_SMARTPA_SUPPORT__
endif

ifeq ($(findstring mt6660, $(MTK_AUDIO_SPEAKER_PATH)), mt6660)
LOCAL_CFLAGS += -DMTK_SCP_SMARTPA_SUPPORT
endif
# SmartPA Support

ifeq ($(MTK_ECCCI_C2K),yes)
LOCAL_CFLAGS += -DMTK_ECCCI_C2K
endif

ifeq ($(MTK_ECCCI_C2K),yes)
LOCAL_CFLAGS += -DMTK_ECCCI_C2K
endif

ifeq ($(MTK_FACTORY_GAMMA_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_FACTORY_GAMMA_SUPPORT
endif

ifeq ($(MTK_EMMC_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_EMMC_SUPPORT
endif

ifeq ($(MTK_UFS_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_UFS_SUPPORT
endif

LOCAL_SHARED_LIBRARIES:= libc libcutils liblog
LOCAL_SRC_FILES:= \
      CFG_file_info.c
LOCAL_C_INCLUDES:= \
      $(MTK_PATH_SOURCE)/external/nvram/libnvram \
      $(MTK_PATH_SOURCE)/external/nvram/libfile_op \
      $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc \
      $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen

LOCAL_MODULE:=libcustom_nvram
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
include $(MTK_SHARED_LIBRARY)

