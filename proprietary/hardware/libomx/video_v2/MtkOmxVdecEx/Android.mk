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

ifeq ($(MTK_V4L2_INTERFACE), yes)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    MtkOmxMVAMgr.cpp \
    MtkOmxVdecEx.cpp \
    MtkOmxVdecDriver.cpp \
    MtkOmxVdecSetParam.cpp \
    MtkOmxVdecGetParam.cpp \
	MtkV4L2Device.cpp

LOCAL_CFLAGS += -DUSE_MTK_HW_VCODEC

ifeq ($(strip $(USES_CACHE_PMEM)),true)
LOCAL_CFLAGS += -DUSES_CACHE_PMEM
endif


LOCAL_CFLAGS += -DDYNAMIC_PRIORITY_ADJUSTMENT

ifeq ($(MTK_USE_RT_PRIORITY), yes)
LOCAL_CFLAGS += -DMTK_USE_RT_PRIORITY=1
else
LOCAL_CFLAGS += -DMTK_USE_RT_PRIORITY=0
endif

ifeq ($(MTK_QOS_SUPPORT), yes)
LOCAL_CFLAGS += -DMTK_QOS_SUPPORT
endif

LOCAL_C_INCLUDES:= \
        $(TOP)/frameworks/av/include/media/stagefright \
        $(TOP)/$(MTK_ROOT)/frameworks/native/include/media/openmax \
        $(TOP)/$(MTK_ROOT)/frameworks/av/media/libstagefright/include/omx_core \
        $(TOP)/$(MTK_ROOT)/frameworks/av/media/libstagefright/include/ \
        $(TOP)/frameworks/av/media/libstagefright/include \
        $(TOP)/$(MTK_ROOT)/frameworks/av/media/libstagefright/include/omx_core/ \
        $(LOCAL_PATH)/../../../omx/inc \
        $(TOP)/$(MTK_ROOT)/hardware/omx/osal \
        $(MTK_PATH_SOURCE)/external/emi/inc \
        $(TOP)/$(MTK_ROOT)/kernel/include/ \
        $(TOP)/$(MTK_ROOT)/kernel/include/linux/vcodec \
        $(TOP)/frameworks/native/include/media/hardware \
        $(TOP)/frameworks/native/include/ \
        $(TOP)/frameworks/native/libs/nativewindow/include \
        $(TOP)/system/core/libutils/include/utils \
        $(TOP)/$(MTK_ROOT)/external/mhal/src/core/drv/inc \
        $(MTK_PATH_PLATFORM)/hardware/vcodec/inc \
        $(TOP)/$(MTK_ROOT)/hardware/omx/inc \
        $(TOP)/vendor/mediatek/proprietary/external/libion_mtk/include \
        $(TOP)/vendor/mediatek/proprietary/external/include \
        $(TOP)/vendor/mediatek/proprietary/hardware/gpu_ext/ged/include \
        $(TOP)/$(MTK_ROOT)/hardware/include \
        $(MTK_ROOT)/external/aee/binary/inc \
        $(MTK_PATH_SOURCE)/hardware/gralloc_extra/include \
        $(TOP)/bionic/libc/kernel/uapi/ \
        $(TOP)/device/mediatek/common/kernel-headers/video \

LOCAL_SHARED_LIBRARIES :=       \
        libutils                \
        libcutils               \
        liblog \
        libui \
        libdl \
        libion \
        libged \
        libion_mtk

LOCAL_STATIC_LIBRARIES := libMtkOmxOsalUtils
LOCAL_SHARED_LIBRARIES += libdpframework
LOCAL_SHARED_LIBRARIES += libgralloc_extra

LOCAL_SHARED_LIBRARIES += libvcodecdrv
LOCAL_SHARED_LIBRARIES += libvcodec_utility
LOCAL_SHARED_LIBRARIES += libhardware

LOCAL_MODULE := libMtkOmxVdecEx
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := 32

include $(MTK_SHARED_LIBRARY)
endif
