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

LOCAL_MODULE:= atci_service
LOCAL_MULTILIB := 32
LOCAL_INIT_RC := atci_service.rc

LOCAL_SRC_FILES:= \
    src/atci_service.c \
    src/atci_generic_cmd_dispatch.c \
    src/atci_telephony_cmd.c \
    src/atci_system_cmd.c \
    src/atci_pq_cmd.cpp \
    src/atci_battery_cmd.c \
    src/atci_gm_cmd.c \
    src/atci_touch_cmd.c \
    src/atci_util.c \
    src/at_tok.c

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    liblog

ifeq ($(MTK_TC1_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libtc1part
LOCAL_CFLAGS += -DMTK_TC1_FEATURE
endif

ifeq ($(MTK_BLUEANGEL_SUPPORT),yes)
LOCAL_SHARED_LIBRARIES += libexttestmode
LOCAL_CFLAGS += -D__MTK_BT_SUPPORT__
endif

LOCAL_C_INCLUDES += \
#        $(KERNEL_HEADERS) \
        $(TOP)/frameworks/base/include

# ------------------------------------ Add Include Path for CCT AT Command -----------------------------------
# ---- check if cct support ----
ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6739 mt6757 mt6761 mt6763 mt6765 mt6768 mt6771 mt6779 mt6785 mt6885 mt8168))
$(info $(TARGET_BOARD_PLATFORM) don't support ccap hidl)
else
MTK_CCTIA_SUPPORT := yes

ifeq ($(BUILD_MTK_LDVT),yes)
MTK_CCTIA_SUPPORT := no
endif

ifeq ($(MTK_CCTIA_SUPPORT),yes)
#Add Include Path for CCT AT Command
$(warning CCTIA is built)

LOCAL_SRC_FILES += \
    src/atci_cct_cmd.cpp

LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += libhidlmemory
LOCAL_SHARED_LIBRARIES += android.hidl.allocator@1.0
LOCAL_SHARED_LIBRARIES += android.hidl.memory@1.0
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.camera.ccap@1.0
LOCAL_LDFLAGS += -ldl

LOCAL_CFLAGS += \
    -DENABLE_CCAP_AT_CMD
endif
endif

LOCAL_C_INCLUDES += ${LOCAL_PATH}/../atci/src

ifeq ($(MTK_GPS_SUPPORT),yes)

LOCAL_SRC_FILES += \
    src/atci_gps_cmd.c

LOCAL_CFLAGS += \
    -DENABLE_GPS_AT_CMD

endif

ifeq ($(strip $(MTK_OD_SUPPORT)),yes)
    LOCAL_CFLAGS += -DMTK_OD_SUPPORT
endif

ifeq (,$(filter $(strip $(MTK_PQ_SUPPORT)), no PQ_OFF))
    LOCAL_CFLAGS += -DMTK_PQ_SERVICE

    LOCAL_SHARED_LIBRARIES += \
        libhidlbase \
        vendor.mediatek.hardware.pq@2.0

    LOCAL_C_INCLUDES += \
        $(MTK_PATH_SOURCE)/hardware/pq/v2.0/include
endif

# Add Flags and source code for MMC AT Command
LOCAL_SRC_FILES += \
    src/atci_mmc_cmd.c
LOCAL_CFLAGS += \
    -DENABLE_MMC_AT_CMD

# Add Flags and source code for CODECRC AT Command
LOCAL_SRC_FILES += \
    src/atci_code_cmd.c
LOCAL_CFLAGS += \
    -DENABLE_CODECRC_AT_CMD

#Add Flags and source code for  backlight and  vibrator AT Command
LOCAL_SRC_FILES += \
    src/atci_lcdbacklight_vibrator_cmd.c
LOCAL_CFLAGS += \
    -DENABLE_BLK_VIBR_AT_CMD

#Add Flags and source code for touchpanel AT Command
LOCAL_SRC_FILES += \
    src/atci_touchpanel_cmd.c
LOCAL_CFLAGS += \
    -DENABLE_TOUCHPANEL_AT_CMD

#Add Flags and source code for touchpanel AT Command

ifneq (,$(filter $(strip $(TARGET_BOARD_PLATFORM)), mt6797))
    LOCAL_CFLAGS += -DTDSHP_2_0
endif

ifeq ($(MTK_GAUGE_VERSION), 30)
LOCAL_CFLAGS += -DMTK_GM_30
LOCAL_C_INCLUDES += $(TOP)/vendor/mediatek/proprietary/external/nvram/libnvram
LOCAL_SHARED_LIBRARIES += libnvram
endif

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(MTK_EXECUTABLE)

endif
