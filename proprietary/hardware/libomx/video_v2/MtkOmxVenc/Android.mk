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
# Copyright (C) 2010 The Android Open Source Project
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


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    MtkOmxMVAMgr.cpp \
    MtkOmxVencDrv.cpp \
    MtkOmxVencStatic.cpp  \
    MtkOmxVenc.cpp  \
    MtkOmxVencSMVR.cpp  \
    MtkOmxVencVT.cpp  \
    MtkOmxVencWFD.cpp  \
    MtkOmxVencHeif.cpp  \
    MtkOmxVencDeque.cpp  \
    MtkOmxVencParam.cpp \
    MtkOmxVencConfig.cpp \
    MtkOmxVencBuffer.cpp \
    MtkOmxVencColor.cpp \
    MtkOmx2V4L2.cpp \
    MtkOmx2V4L2Avc.cpp \
    MtkOmx2V4L2Mpeg4.cpp \
    MtkOmx2V4L2Hevc.cpp \
    MtkOmx2V4L2Heif.cpp \
    MtkOmxVencBufQueue.cpp \
    MtkOmxVencCmdQueue.cpp \

LOCAL_CFLAGS += -DUSE_MTK_HW_VCODEC

ifeq ($(strip $(MTK_MDP_MHAL)), yes)
LOCAL_CFLAGS += -DMTK_OMXVENC_USE_MHAL
endif

ifeq ($(MTK_USE_RT_PRIORITY), yes)
LOCAL_CFLAGS += -DMTK_USE_RT_PRIORITY=1
else
LOCAL_CFLAGS += -DMTK_USE_RT_PRIORITY=0
endif

LOCAL_CFLAGS += -DV4L2
#LOCAL_CFLAGS += -fsyntax-only -H

ifeq ($(MTK_QOS_SUPPORT), yes)
LOCAL_CFLAGS += -DMTK_QOS_SUPPORT
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH) \
                    $(LOCAL_PATH)/include/

LOCAL_C_INCLUDES += $(TOP)/frameworks/av/include/media/stagefright \
                    $(TOP)/frameworks/av/include \
                    $(TOP)/frameworks/native/include/media/hardware \
                    $(TOP)/frameworks/native/include \
                    $(TOP)/frameworks/native/libs/nativewindow/include \
                    $(TOP)/$(MTK_ROOT)/frameworks/native/include/media/openmax \
                    $(TOP)/$(MTK_ROOT)/external/libion_mtk/include \
                    $(TOP)/bionic/libc/kernel/uapi/ \
                    $(TOP)/device/mediatek/common/kernel-headers/video \

LOCAL_C_INCLUDES += $(TOP)/system/core/include/system \
                    $(TOP)/system/core/libutils/include/utils

LOCAL_C_INCLUDES += $(TOP)/system/libhidl/transport/token/1.0/utils/include/ \
                    $(TOP)/system/libhidl/base/include/

LOCAL_C_INCLUDES += $(TOP)/system/libvintf/include/

LOCAL_C_INCLUDES += $(TOP)/hardware/libhardware/modules/gralloc \
                    $(TOP)/$(MTK_ROOT)/hardware/omx/osal \
                    $(TOP)/$(MTK_ROOT)/hardware/omx/inc \
                    $(TOP)/$(MTK_ROOT)/hardware/gralloc_extra/include/ \
                    $(TOP)/$(MTK_ROOT)/hardware/bwc/inc


LOCAL_EXPORT_SHARED_LIBRARY_HEADERS := \
        android.hardware.media.omx@1.0 \

LOCAL_SHARED_LIBRARIES :=       \
        libbinder               \
        libutils                \
        libcutils               \
        liblog                  \
        libdl                   \
        libui \
        libion \
        libion_mtk \
        libgralloc_extra \
        android.hardware.media@1.0 \
        android.hardware.media.omx@1.0 \
        libbwc

#added for dump test TZ
ifeq ($(strip $(MTK_IN_HOUSE_TEE_SUPPORT)),yes)
LOCAL_C_INCLUDES += \
    $(call include-path-for, trustzone-uree)
LOCAL_SHARED_LIBRARIES += libtz_uree
endif
LOCAL_STATIC_LIBRARIES := libMtkOmxOsalUtils
LOCAL_SHARED_LIBRARIES += libvcodecdrv
LOCAL_SHARED_LIBRARIES += libvcodec_utility
LOCAL_SHARED_LIBRARIES += libhardware
ifeq ($(strip $(MTK_MDP_MHAL)), yes)
LOCAL_SHARED_LIBRARIES += libmhal
else
LOCAL_SHARED_LIBRARIES += libdpframework
endif

# check exist and load external libraries configuration for video codec
LC_MTK_PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )

# for ROI
LOCAL_SRC_FILES += MtkOmxVencRoi.cpp

ifeq ($(strip $(MTK_VCFNET_SUPPORT)),yes)

SALIENCY_LIB := $(TOP)/$(MTK_ROOT)/external/aihub/saliency_lib/libsaliency.so
ifneq (,$(strip $(wildcard $(SALIENCY_LIB))))

LOCAL_CFLAGS += -DMTK_ROINET_SUPPORT
LOCAL_HEADER_LIBRARIES += libsaliency_headers
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/frameworks/ml/nn/
LOCAL_C_INCLUDES += $(TOP)/frameworks/ml/nn/runtime/include/

LOCAL_SHARED_LIBRARIES += libsaliency

endif
endif
# end ROI ---

LOCAL_MODULE := libMtkOmxVenc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32

include $(MTK_SHARED_LIBRARY)
