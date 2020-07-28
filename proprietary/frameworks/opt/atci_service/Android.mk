ifneq ($(TARGET_BUILD_PDK),true)
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


# Copyright 2006 The Android Open Source Project

# XXX using libutils for simulator build only...
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= atci_service_sys
LOCAL_MULTILIB := 32
LOCAL_INIT_RC := atci_service_sys.rc

LOCAL_SRC_FILES:= \
    src/atci_service_adaptation.cpp \
    src/atci_service.c \
    src/atci_generic_cmd_dispatch.c \
    src/atci_util.c \
    src/at_tok.c

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    liblog \
    vendor.mediatek.hardware.atci@1.0 \
    libhidlbase \
    libhidltransport \
    libhwbinder

LOCAL_C_INCLUDES += \
        $(TOP)/frameworks/base/include

LOCAL_C_INCLUDES += ${LOCAL_PATH}/../atci/src

ifeq ($(MTK_NFC_SUPPORT),yes)

LOCAL_SRC_FILES += \
    src/atci_nfc_cmd.c

LOCAL_CFLAGS += \
    -DENABLE_NFC_AT_CMD

LOCAL_C_INCLUDES += \
    $(TOP)/vendor/mediatek/proprietary/external/mtknfc/inc \
    $(TOP)/packages/apps/Nfc/mtk-nfc/jni-dload \

LOCAL_SHARED_LIBRARIES += \
    libmtknfc_dynamic_load_jni

endif

include $(MTK_EXECUTABLE)

endif
