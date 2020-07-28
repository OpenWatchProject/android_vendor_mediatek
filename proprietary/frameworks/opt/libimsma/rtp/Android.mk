# Copyright Statement:
#
# This software/firmware and related documentation ("MediaTek Software") are
# protected under relevant copyright laws. The information contained herein
# is confidential and proprietary to MediaTek Inc. and/or its licensors.
# Without the prior written permission of MediaTek inc. and/or its licensors,
# any reproduction, modification, use or disclosure of MediaTek Software,
# and information contained herein, in whole or in part, shall be strictly prohibited.
#
# MediaTek Inc. (C) 2014. All rights reserved.
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

############################################################

build_vilte =
ifeq ($(strip $(MSSI_MTK_VILTE_SUPPORT)),yes)
    build_vilte = yes
endif

ifeq ($(strip $(MSSI_MTK_VIWIFI_SUPPORT)),yes)
    build_vilte = yes
endif

ifdef build_vilte

############################################################

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/IRTPController.cpp \
    src/RTPController.cpp  \
    src/RTPSender.cpp      \
    src/RTPReceiver.cpp    \
    src/RTPSource.cpp      \
    src/AVCAssembler.cpp   \
    src/RTPAssembler.cpp   \
    src/HEVCAssembler.cpp  \



LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/frameworks/opt/libimsma/rtp/include \
    $(MTK_PATH_SOURCE)/frameworks/opt/libimsma/socketwrapper/inc \
    $(MTK_PATH_SOURCE)/frameworks/opt/libimsma/signal \
    frameworks/av/media/libstagefright

LOCAL_CFLAGS += -Werror

LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libmedia  \
    libutils  \
    libcutils \
    libstagefright \
    libstagefright_foundation \
    libimsma_socketwrapper \
    libvcodec_cap \
    libimsma_adapt \
    liblog \
    libsignal

LOCAL_MODULE := libimsma_rtp
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(MTK_SHARED_LIBRARY)

##############################################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/RTPControllerTest.cpp

LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/frameworks/opt/libimsma/rtp/include \
    $(MTK_PATH_SOURCE)/frameworks/opt/libimsma/socketwrapper/inc \
    $(MTK_PATH_SOURCE)/frameworks/opt/libimsma/imsma \
    frameworks/av/media/libstagefright \

LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libmedia  \
    libutils  \
    libcutils \
    libstagefright \
    libstagefright_foundation \
    libimsma_rtp \
    libimsma_socketwrapper \
    libimsma_adapt \
    liblog        

LOCAL_MODULE := RTPControllerTest
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
#LOCAL_32_BIT_ONLY := true

LOCAL_MODULE_TAGS := optional

include $(MTK_EXECUTABLE)

endif