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
# Cmdq user space test file
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

#### gcov
LOCAL_CFLAGS += -fprofile-arcs -ftest-coverage
LOCAL_LDFLAGS += -lgcov

LOCAL_SRC_FILES:= \
    BasicTask.cpp

LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)hardware/dpframework/inc \
	$(TOPDIR)system/core/include/utils

LOCAL_C_INCLUDES += \
    $(TOPDIR)frameworks/native/include/utils \
    $(TOPDIR)system/core/include \
    $(MTK_PATH_SOURCE)kernel/include \
    $(MTK_PATH_PLATFORM)kernel/drivers/dispsys

# Note: "/bionic" and "/external/stlport/stlport" is for stlport.
#LOCAL_C_INCLUDES += $(TOP)/bionic
#LOCAL_C_INCLUDES += $(TOP)/external/stlport/stlport

LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/hardware/dpframework/inc \
    $(MTK_PATH_SOURCE)/protect-bsp/platform/mt6580/hardware/dpframework/src/buffer \
    $(MTK_PATH_SOURCE)/protect-bsp/platform/mt6580/hardware/dpframework/src/core \
    $(MTK_PATH_SOURCE)/protect-bsp/platform/mt6580/hardware/dpframework/src/engine \
    $(MTK_PATH_SOURCE)/protect-bsp/platform/mt6580/hardware/dpframework/src/osal \
    $(MTK_PATH_SOURCE)/protect-bsp/platform/mt6580/hardware/dpframework/src/stream \
    $(MTK_PATH_SOURCE)/protect-bsp/platform/mt6580/hardware/dpframework/src/util \
    $(MTK_PATH_SOURCE)/protect-bsp/platform/mt6580/hardware/dpframework/src/platform/mt6580/engine \
    $(MTK_PATH_SOURCE)/protect-bsp/platform/mt6580/hardware/dpframework/src/test/unitTest \

    

LOCAL_C_INCLUDES += \
    $(MTK_PATH_PLATFORM)kernel/drivers/cmdq

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include
LOCAL_C_INCLUDES += $(MTK_PATH_PLATFORM)/hardware/include

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libdpframework

LOCAL_STATIC_LIBRARIES := \

LOCAL_WHOLE_STATIC_LIBRARIES := \

LOCAL_MODULE := cmdq_test
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#LOCAL_MODULE_STEM_64 := cmdq_test64
#LOCAL_MODULE_STEM_32 := cmdq_test32
#LOCAL_MULTILIB := both
LOCAL_32_BIT_ONLY := true

include $(MTK_EXECUTABLE)

