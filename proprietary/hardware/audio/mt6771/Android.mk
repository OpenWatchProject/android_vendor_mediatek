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

ifeq ($(MTK_PLATFORM_DIR),mt6771)

### ============================================================================
### hardware
### ============================================================================

# normal playback kernel buffer size
LOCAL_CFLAGS += -DKERNEL_BUFFER_SIZE_DL1=16*1024

# deep buffer playback kernel buffer size
LOCAL_CFLAGS += -DKERNEL_BUFFER_SIZE_DL1_DATA2=48*1024

# normal capture kernel buffer size
LOCAL_CFLAGS += -DKERNEL_BUFFER_SIZE_UL1_NORMAL=32*1024

# For New DC Trim Calibration Flow
LOCAL_CFLAGS += -DNEW_PMIC_DCTRIM_FLOW

# Record lock time out information
LOCAL_CFLAGS += -DMAX_RAW_DATA_LOCK_TIME_OUT_MS=3000
LOCAL_CFLAGS += -DMAX_PROCESS_DATA_LOCK_TIME_OUT_MS=3000

# low latency
LOCAL_CFLAGS += -DUPLINK_LOW_LATENCY
LOCAL_CFLAGS += -DDOWNLINK_LOW_LATENCY

# POWR HAL Control for low latency & low power
#LOCAL_CFLAGS += -DMTK_POWERHAL_AUDIO_LATENCY
#LOCAL_CFLAGS += -DMTK_POWERHAL_AUDIO_POWER
LOCAL_CFLAGS += -DMTK_POWERHAL_WIFI_POWRER_SAVE
LOCAL_SHARED_LIBRARIES += \
    libhidlbase \
    libhidltransport \
    libhwbinder \
    vendor.mediatek.hardware.mtkpower@1.0 \
    android.hardware.power@1.0 \
    vendor.mediatek.hardware.power@2.0 \
    android.hardware.bluetooth.a2dp@1.0

LOCAL_C_INCLUDES += \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/power/include

# HIFI audio
ifeq ($(MTK_HIFIAUDIO_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_HIFIAUDIO_SUPPORT
endif
# hifi playback kernel buffer size
LOCAL_CFLAGS += -DKERNEL_BUFFER_SIZE_IN_HIFI_MODE=64*1024

# uncomment for increase hifi playback buffer size
LOCAL_CFLAGS += -DHIFI_DEEP_BUFFER

# Temp tag for FM support WIFI-Display output
LOCAL_CFLAGS += -DMTK_FM_SUPPORT_WFD_OUTPUT

# Playback must be 24bit when using sram
LOCAL_CFLAGS += -DPLAYBACK_USE_24BITS_ONLY

# Record must be 24bit when using sram
LOCAL_CFLAGS += -DRECORD_INPUT_24BITS

# BT
LOCAL_CFLAGS += -DSW_BTCVSD_ENABLE

LOCAL_CFLAGS += -DMTK_SUPPORT_BTCVSD_ALSA

LOCAL_CFLAGS += -DSPH_BT_DELAYTIME_SUPPORT

# enable TDM
#   yes: AudioALSAPlaybackHandlerHDMI.cpp
#   no:  AudioALSAPlaybackHandlerI2SHDMI.cpp
MTK_TDM_SUPPORT = no

# APLL
LOCAL_CFLAGS += -DMTK_APLL_DEFAULT_OFF

### ============================================================================
### speech config
### ============================================================================

# MD Platform
MTK_MODEM_PLATFROM = GEN93

# MD1 and MD2 use the same MD
MTK_COMBO_MODEM_SUPPORT = yes

# ccci share memory
MTK_CCCI_SHARE_BUFFER_SUPPORT = yes

# sample rate
ifeq ($(strip $(MTK_PHONE_CALL_FIXED_48K_SAMPLERATE)),yes)
  LOCAL_CFLAGS += -DSPH_SR48K
else
  LOCAL_CFLAGS += -DSPH_SR32K
endif

# ap sidetone
LOCAL_CFLAGS += -DSPH_AP_SET_SIDETONE
LOCAL_CFLAGS += -DSPH_POSITIVE_SIDETONE_GAIN

# magic clarity
LOCAL_CFLAGS += -DMTK_SPH_MAGICLARITY_SHAPEFIR_SUPPORT

# uncomment to enable voice ultra
#LOCAL_CFLAGS += -DMTK_VOICE_ULTRA

# uncomment to disable IN_CALL_VOICE_PLAYBACK_PATH
#LOCAL_CFLAGS += -DMTK_SPEAKER_MONITOR_SPEECH_SUPPORT

# network param
#LOCAL_CFLAGS += -DMTK_AUDIO_SPH_NETWORK_PARAM

### ============================================================================
### bring up only
### ============================================================================

#LOCAL_CFLAGS += -DFORCE_ROUTING_RECEIVER
#LOCAL_CFLAGS += -DCCCI_FORCE_USE
#LOCAL_CFLAGS += -DSPEECH_PMIC_RESET
#LOCAL_CFLAGS += -DSPH_SKIP_A2M_BUFF_MSG


endif # end of MTK_PLATFORM
