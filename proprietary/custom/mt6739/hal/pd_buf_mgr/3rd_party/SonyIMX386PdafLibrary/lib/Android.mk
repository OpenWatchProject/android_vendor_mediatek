# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.

# MediaTek Inc. (C) 2017. All rights reserved.
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

include $(CLEAR_VARS)

ifeq ($(wildcard $(TOP)/$(MTK_PATH_SOURCE)/hardware/libstackpdaf/libimx386pdaf),)
$(warning using dummy library)
#-----------------------------------------------------------
LOCAL_SRC_FILES := SonyIMX386PdafLibrary.c
#-----------------------------------------------------------
LOCAL_C_INCLUDES += $(wildcard $(LOCAL_PATH)/../inc)
#-----------------------------------------------------------
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libSonyIMX386PdafLibrary
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#-----------------------------------------------------------
include $(MTK_SHARED_LIBRARY)
else
$(warning using 3rd party library)
LOCAL_IS_HOST_MODULE :=
LOCAL_MODULE := libSonyIMX386PdafLibrary
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TAGS := optional
#OVERRIDE_BUILT_MODULE_PATH :=$(PRODUCT_OUT)/obj/lib
LOCAL_UNINSTALLABLE_MODULE :=
LOCAL_SRC_FILES_32 := ../../../../../../../../../../$(MTK_PATH_SOURCE)/hardware/libstackpdaf/libimx386pdaf/32/libSonyIMX386PdafLibrary.so
LOCAL_SRC_FILES_64 := ../../../../../../../../../../$(MTK_PATH_SOURCE)/hardware/libstackpdaf/libimx386pdaf/64/libSonyIMX386PdafLibrary.so
#LOCAL_BUILT_MODULE_STEM := libSonyIMX386PdafLibrary.so
LOCAL_STRIP_MODULE :=
#LOCAL_MODULE_STEM := libSonyIMX386PdafLibrary.so
LOCAL_CERTIFICATE :=
#LOCAL_MODULE_PATH := $(PRODUCT_OUT)/system/lib
LOCAL_REQUIRED_MODULE :=
LOCAL_SHARED_LIBRARIES :=
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both
include $(BUILD_PREBUILT)
endif

include $(CLEAR_VARS)


