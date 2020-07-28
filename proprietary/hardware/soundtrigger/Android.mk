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

ifneq ($(MTK_EMULATOR_SUPPORT), yes)
ifneq ($(MTK_BASIC_PACKAGE), yes)


LOCAL_PATH := $(call my-dir)


# Stub sound_trigger HAL module, used for tests
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(call include-path-for, audio-utils) \
    $(call include-path-for, audio-effects) \
    $(call include-path-for, alsa-utils) \
    $(TOPDIR)external/tinyxml \
    $(TOPDIR)vendor/mediatek/proprietary/external \
    $(TOPDIR)external/tinyalsa/include  \
    $(TOPDIR)external/tinycompress/include \
    $(TOPDIR)vendor/mediatek/proprietary/hardware/ccci/include \
    $(TOPDIR)vendor/mediatek/proprietary/external/AudioCompensationFilter \
    $(TOPDIR)vendor/mediatek/proprietary/external/AudioComponentEngine \
    $(TOPDIR)vendor/mediatek/proprietary/external/audiocustparam \
    $(TOPDIR)vendor/mediatek/proprietary/external/AudioSpeechEnhancement/V3/inc \
    $(MTK_PATH_CUSTOM)/hal/audioflinger/audio \
    $(TOPDIR)vendor/mediatek/proprietary/hardware/audio/common/V3/include \
    $(TOPDIR)vendor/mediatek/proprietary/hardware/audio/common/include \
    $(TOPDIR)vendor/mediatek/proprietary/hardware/audio/common/utility \
    $(TOPDIR)vendor/mediatek/proprietary/hardware/audio/common/utility/uthash \
    $(TOPDIR)vendor/mediatek/proprietary/hardware/power/include \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc

LOCAL_HEADER_LIBRARIES := \
    libaudioclient_headers \
    libbase_headers \
    libhardware_headers \
    libaudio_system_headers \
    libmedia_headers \
    liblog_headers

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libhardware_legacy \
    libhardware \
    libmedia_helper \
    libdl \
    android.hardware.bluetooth.a2dp@1.0

ifneq ($(wildcard vendor/mediatek/proprietary/external/voiceunlock2),)
ifeq ($(strip $(MTK_VOW_SUPPORT)),yes)
    ifeq ($(strip $(MTK_VOW_2E2K_SUPPORT)),yes)
        LOCAL_SHARED_LIBRARIES += libvow_ap_testing_alexa.vendor
        LOCAL_SHARED_LIBRARIES += libvowp2training_alexa.vendor
        LOCAL_SHARED_LIBRARIES += libvowp23_sid_training_alexa.vendor
        LOCAL_SHARED_LIBRARIES += libvowp23_sid_testing_alexa.vendor
        LOCAL_SHARED_LIBRARIES += libvowp25testing_alexa.vendor
    else
        LOCAL_SHARED_LIBRARIES += libvow_ap_testing.vendor
        LOCAL_SHARED_LIBRARIES += libvowp2training.vendor
        LOCAL_SHARED_LIBRARIES += libvowp23_sid_training.vendor
        LOCAL_SHARED_LIBRARIES += libvowp23_sid_testing.vendor
        LOCAL_SHARED_LIBRARIES += libvowp25testing.vendor
    endif
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/voiceunlock2/include
    LOCAL_CFLAGS += -DMTK_VOW_SUPPORT
    ifeq ($(strip $(MTK_VOW_DUAL_MIC_SUPPORT)),yes)
        LOCAL_CFLAGS += -DMTK_VOW_DUAL_MIC_SUPPORT
    endif
endif
endif

# Power Hal
LOCAL_SHARED_LIBRARIES += libhidlbase
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.mtkpower@1.0

# AEE
ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/aee/binary/inc
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif


LOCAL_MODULE := sound_trigger.primary.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SRC_FILES += sound_trigger_hw.cpp
LOCAL_SRC_FILES += audio_lock.c
LOCAL_SRC_FILES += audio_time.c
LOCAL_SRC_FILES += audio_memory_control.c
LOCAL_SRC_FILES += audio_ringbuffer.c
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both

include $(BUILD_SHARED_LIBRARY)

endif
endif
