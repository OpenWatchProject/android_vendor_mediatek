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



LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


LOCAL_SRC_FILES := \
    main.cpp \
    Em_hidl_service.cpp \
    ModuleMcfGetFileList.cpp \


LOCAL_C_INCLUDES :=  system/core/include/cutils/ \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libbase \
    libhardware \
    libbinder \
    libhidlbase \
    libhidltransport \
    libutils \
    libcutils \
    vendor.mediatek.hardware.engineermode@1.0 \
    vendor.mediatek.hardware.engineermode@1.1 \
    vendor.mediatek.hardware.engineermode@1.2 \

ifeq ($(MTK_BT_SUPPORT), yes)
ifeq ($(filter MTK_MT76%, $(MTK_BT_CHIP)), )

    LOCAL_SRC_FILES += \
    BluetoothTest.cpp \

    BT_DRV_HEADER_DIR := $(MTK_ROOT)/hardware/connectivity/bluetooth/driver/pure/inc
    ifeq ($(wildcard $(BT_DRV_HEADER_DIR)),)
    BT_DRV_HEADER_DIR := $(MTK_ROOT)/hardware/connectivity/bluetooth/driver/mt66xx/pure/inc
    endif

    LOCAL_C_INCLUDES += \
    $(BT_DRV_HEADER_DIR) \

    LOCAL_CFLAGS += -DMTK_BT_EM_HIDL_SUPPORT

    ifeq ($(MTK_COMBO_SUPPORT), yes)
        LOCAL_CFLAGS += -DMTK_EM_BT_COMBO_SUPPORT
    endif
    LOCAL_SHARED_LIBRARIES += \
        libbluetooth_relayer \
        libbluetoothem_mtk \

endif
endif

ifeq ($(strip $(NFC_CHIP_SUPPORT)), yes)
	LOCAL_CFLAGS += -DST_NFC_SUPPORT
endif

ifeq ($(MTK_GAUGE_VERSION), 30)
    LOCAL_CFLAGS += -DMTK_GAUGE_VERSION
endif

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE := em_hidl
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true


ifeq ($(TARGET_BUILD_VARIANT), eng)
LOCAL_INIT_RC := em_hidl_eng.rc
else
LOCAL_INIT_RC := em_hidl_user.rc
endif

include $(BUILD_EXECUTABLE)





