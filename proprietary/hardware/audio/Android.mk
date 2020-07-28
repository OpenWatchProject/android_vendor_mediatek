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

#Split build wrap
ifdef MTK_TARGET_PROJECT

ifneq ($(MTK_EMULATOR_SUPPORT), yes)
ifneq (,$(filter $(strip $(MTK_PLATFORM_DIR)), elbrus mt2601 mt3886 mt6570 mt6572 mt6580 mt6582 mt6592 mt6735 mt6752 mt6755 mt6757 mt6759 mt6795 mt6797 mt6799 mt7623 mt8127 mt8163 mt8167 mt8173 mt8168))
include $(call all-subdir-makefiles)
else

### ============================================================================
### new chips (after mt6763)
### ============================================================================

ifeq ($(strip $(BOARD_USES_MTK_AUDIO)),true)

AUDIO_COMMON_DIR := common


### ============================================================================
### platform audio HAL
### ============================================================================

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
include $(LOCAL_PATH)/$(MTK_PLATFORM_DIR)/Android.mk


LOCAL_CFLAGS += -Werror -Wno-error=undefined-bool-conversion
#LOCAL_CFLAGS += -Wall -Wextra
LOCAL_CFLAGS += -fexceptions

LOCAL_CFLAGS += -DMTK_SUPPORT_AUDIO_DEVICE_API3


### ============================================================================
### include files
### ============================================================================

LOCAL_C_INCLUDES := \
    $(call include-path-for, audio-utils) \
    $(call include-path-for, audio-effects) \
    $(call include-path-for, alsa-utils) \
    $(TOPDIR)external/tinyxml \
    $(TOPDIR)external/tinyalsa/include  \
    $(TOPDIR)external/tinycompress/include \
    $(TOPDIR)vendor/mediatek/proprietary/hardware/ccci/include \
    $(TOPDIR)vendor/mediatek/proprietary/external/AudioCompensationFilter \
    $(TOPDIR)vendor/mediatek/proprietary/external/AudioComponentEngine \
    $(TOPDIR)vendor/mediatek/proprietary/external/audiocustparam \
    $(TOPDIR)vendor/mediatek/proprietary/external/AudioSpeechEnhancement/V3/inc \
    $(TOPDIR)vendor/mediatek/proprietary/external/libudf/libladder \
    $(TOPDIR)vendor/mediatek/proprietary/hardware/power/config/common/intf_types \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_utils/AudioToolkit \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/audiopolicy_parameters \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/custom_volume \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/gain_table \
    $(MTK_PATH_CUSTOM)/hal/audioflinger/audio \
    $(LOCAL_PATH)/$(MTK_PLATFORM_DIR)/include \
    $(LOCAL_PATH)/$(AUDIO_COMMON_DIR)/V3/include \
    $(LOCAL_PATH)/$(AUDIO_COMMON_DIR)/include \
    $(LOCAL_PATH)/$(AUDIO_COMMON_DIR)/utility \
    $(LOCAL_PATH)/$(AUDIO_COMMON_DIR)/utility/uthash \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/customization \
    $(TOPDIR)vendor/mediatek/proprietary/hardware/power/include


### ============================================================================
### library
### ============================================================================

LOCAL_SHARED_LIBRARIES += \
    libc \
    liblog \
    libcutils \
    libutils \
    libalsautils \
    libhardware_legacy \
    libhardware \
    libdl \
    libaudioutils \
    libtinyalsa \
    libtinycompress \
    libtinyxml \
    libaudiotoolkit_vendor \
    libmedia_helper \
    libladder

LOCAL_HEADER_LIBRARIES += libaudioclient_headers libaudio_system_headers libmedia_headers

### ============================================================================
### project config depends
### ============================================================================

ifeq ($(MTK_BSP_PACKAGE),yes)
 LOCAL_CFLAGS += -DMTK_BSP_PACKAGE
endif

ifeq ($(strip $(MTK_USE_ANDROID_MM_DEFAULT_CODE)),yes)
  LOCAL_CFLAGS += -DANDROID_DEFAULT_CODE
endif

ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
  LOCAL_CFLAGS += -DCONFIG_MT_ENG_BUILD
endif



### ============================================================================
### hardware project config
### ============================================================================

ifeq ($(MTK_DIGITAL_MIC_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_DIGITAL_MIC_SUPPORT
endif

ifeq ($(strip $(MTK_AUDIO_MIC_INVERSE)),yes)
  LOCAL_CFLAGS += -DMTK_AUDIO_MIC_INVERSE
endif

ifeq ($(strip $(MTK_2IN1_SPK_SUPPORT)),yes)
  LOCAL_CFLAGS += -DUSING_2IN1_SPEAKER
endif

ifeq ($(MTK_VIBSPK_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_VIBSPK_SUPPORT
endif
#ifeq ($(MTK_VIBSPK_SUPPORT),yes)
  LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/aud_drv/AudioVIBSPKControl.cpp
#endif

ifeq ($(strip $(MTK_HEADSET_ACTIVE_NOISE_CANCELLATION)),yes)
    LOCAL_CFLAGS += -DMTK_ANC_SUPPORT # new anc local define
endif

# usb phone call
ifneq ($(MTK_USB_PHONECALL),)
  ifneq ($(strip $(MTK_USB_PHONECALL)),NONE)
    LOCAL_CFLAGS += -DMTK_USB_PHONECALL
  endif
endif

ifeq ($(MTK_AUDIODSP_SUPPORT),yes)
  MTK_AUDIO_DSP_RECOVERY_SUPPORT = yes
  LOCAL_CFLAGS += -DMTK_AUDIODSP_SUPPORT

  LOCAL_SRC_FILES += \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerDsp.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderDsp.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerDsp.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataClientDsp.cpp \
    $(AUDIO_COMMON_DIR)/audiodsp/AudioDspStreamManager.cpp \
    $(AUDIO_COMMON_DIR)/audiodsp/AudioALSAHandlerKtv.cpp \
    $(AUDIO_COMMON_DIR)/audiodsp/audio_dsp_service.c
  LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/$(AUDIO_COMMON_DIR)/audiodsp
endif

# a2dp offload
ifeq ($(MTK_A2DP_OFFLOAD_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_A2DP_OFFLOAD_SUPPORT
#    LOCAL_CFLAGS += -DTEST_A2DP_TASK
endif

### ============================================================================
### speaker customization
### ============================================================================

ifeq ($(strip $(MTK_AUDIO_SPEAKER_PATH)),int_spk_amp)

else ifeq ($(strip $(MTK_AUDIO_SPEAKER_PATH)),int_lo_buf)
  LOCAL_CFLAGS += -DUSING_EXTAMP_LO
else ifeq ($(strip $(MTK_AUDIO_SPEAKER_PATH)),int_hp_buf)
  LOCAL_CFLAGS += -DUSING_EXTAMP_HP
else ifeq ($(strip $(MTK_AUDIO_SPEAKER_PATH)),2_in_1_spk)
  LOCAL_CFLAGS += -DMTK_AUDIO_SPEAKER_PATH_2_IN_1
else ifeq ($(strip $(MTK_AUDIO_SPEAKER_PATH)),3_in_1_spk)
  LOCAL_CFLAGS += -DMTK_AUDIO_SPEAKER_PATH_3_IN_1
else ifeq ($(findstring smartpa_dynamic_detect, $(MTK_AUDIO_SPEAKER_PATH)),smartpa_dynamic_detect)
  LOCAL_CFLAGS += -DSMARTPA_DYNAMIC_DETECT
else
  ifeq ($(findstring smartpa, $(MTK_AUDIO_SPEAKER_PATH)), smartpa)
    USE_SMART_PA := 1
    #LOCAL_CFLAGS += -DSMARTPA_AUTO_CALIBRATE
    LOCAL_CFLAGS += -DEXT_SPK_SUPPORT # for old volume control
    ifeq ($(findstring maxim_98926, $(MTK_AUDIO_SPEAKER_PATH)), maxim_98926)
      LOCAL_CFLAGS += -DMTK_MAXIM_SPEAKER_SUPPORT
      LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerSpeakerProtection.cpp \
        $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPcmDataCaptureIn.cpp
    endif
  endif
endif

# Smart Pa
LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioSmartPaController.cpp
LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/AudioParamParser
LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/AudioParamParser/include
LOCAL_C_INCLUDES += $(TOPDIR)external/libxml2/include
LOCAL_C_INCLUDES += $(TOPDIR)external/icu/icu4c/source/common

ifeq ($(MTK_SCP_SMARTPA_SUPPORT),yes)
  ifeq ($(USE_SMART_PA),1)
    MTK_AUDIO_SCP_SUPPORT = yes
    MTK_AUDIO_DSP_RECOVERY_SUPPORT = yes
    MTK_AUDIO_SCP_RECOVERY_SUPPORT = yes
    LOCAL_CFLAGS += -DMTK_AUDIO_SMARTPASCP_SUPPORT
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioSmartPaParam.cpp \
                       $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerSpeakerProtectionDsp.cpp \
                       $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioSCPPhoneCallController.cpp
  endif
endif

ifeq ($(MTK_AUDIO_SCP_RECOVERY_SUPPORT),yes)
  LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/audiodsp/audio_scp_service.c
endif

### ============================================================================
### software project config
### ============================================================================

ifeq ($(strip $(MTK_HIGH_RESOLUTION_AUDIO_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_HD_AUDIO_ARCHITECTURE
endif

ifeq ($(strip $(MTK_AUDENH_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_AUDENH_SUPPORT
endif

ifeq ($(strip $(MTK_BESLOUDNESS_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_BESLOUDNESS_SUPPORT
endif

ifeq ($(strip $(MTK_BESLOUDNESS_RUN_WITH_HAL)),yes)
  LOCAL_CFLAGS += -DMTK_BESLOUDNESS_RUN_WITH_HAL
endif

ifeq ($(strip $(MTK_BESSURROUND_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_BESSURROUND_SUPPORT
endif

ifeq ($(MTK_AUDIO_HYBRID_NLE_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_HYBRID_NLE_SUPPORT
#else
#  LOCAL_CFLAGS += -DMTK_AUDIO_SW_DRE
endif

# SRS Processing
ifeq ($(strip $(HAVE_SRSAUDIOEFFECT_FEATURE)),yes)
    LOCAL_CFLAGS += -DHAVE_SRSAUDIOEFFECT
endif

# Audio HD Record
ifeq ($(MTK_AUDIO_HD_REC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_AUDIO_HD_REC_SUPPORT
endif

# MTK VoIP
ifeq ($(MTK_VOIP_ENHANCEMENT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_VOIP_ENHANCEMENT_SUPPORT
endif

# DMNR 3.0
ifeq ($(strip $(MTK_HANDSFREE_DMNR_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_HANDSFREE_DMNR_SUPPORT
endif

# Native Audio Preprocess
ifeq ($(strip $(NATIVE_AUDIO_PREPROCESS_ENABLE)),yes)
    LOCAL_CFLAGS += -DNATIVE_AUDIO_PREPROCESS_ENABLE
endif

# HIFI audio
ifeq ($(MTK_HIFIAUDIO_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_HIFIAUDIO_SUPPORT
endif

# Phonecall record voice only
LOCAL_CFLAGS += -DMTK_PHONE_CALL_RECORD_VOICE_ONLY

# Check shared input device priority
# LOCAL_CFLAGS += -DMTK_CHECK_INPUT_DEVICE_PRIORITY

# Remove phonecall record
# LOCAL_CFLAGS += -DMTK_PHONE_CALL_RECORD_DISABLE

# Remove audio dump
# LOCAL_CFLAGS += -DMTK_AUDIO_HAL_DUMP_DISABLE

### ============================================================================
### vow config
### ============================================================================

ifeq ($(MTK_VOW_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_VOW_SUPPORT
  LOCAL_SRC_FILES += \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerVOW.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderVOW.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAVoiceWakeUpController.cpp

  ifeq ($(MTK_VOW_DUAL_MIC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_VOW_DUAL_MIC_SUPPORT
  endif

  # VOW Command
  ifeq ($(strip $(MTK_VOW_COMMAND)),keyword01)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND=\"keyword01\"
  else ifeq ($(strip $(MTK_VOW_COMMAND)),keyword02)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND=\"keyword02\"
  else ifeq ($(strip $(MTK_VOW_COMMAND)),keyword03)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND=\"keyword03\"
  else ifeq ($(strip $(MTK_VOW_COMMAND)),keyword04)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND=\"keyword04\"
  else ifeq ($(strip $(MTK_VOW_COMMAND)),keyword05)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND=\"keyword05\"
  else ifeq ($(strip $(MTK_VOW_COMMAND)),keyword06)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND=\"keyword06\"
  else ifeq ($(strip $(MTK_VOW_COMMAND)),keyword07)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND=\"keyword07\"
  endif

  ifeq ($(strip $(MTK_VOW_COMMAND_2ND)),keyword01)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_2ND=\"keyword01\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_2ND)),keyword02)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_2ND=\"keyword02\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_2ND)),keyword03)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_2ND=\"keyword03\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_2ND)),keyword04)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_2ND=\"keyword04\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_2ND)),keyword05)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_2ND=\"keyword05\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_2ND)),keyword06)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_2ND=\"keyword06\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_2ND)),keyword07)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_2ND=\"keyword07\"
  endif

  ifeq ($(strip $(MTK_VOW_COMMAND_3RD)),keyword01)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_3RD=\"keyword01\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_3RD)),keyword02)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_3RD=\"keyword02\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_3RD)),keyword03)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_3RD=\"keyword03\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_3RD)),keyword04)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_3RD=\"keyword04\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_3RD)),keyword05)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_3RD=\"keyword05\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_3RD)),keyword06)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_3RD=\"keyword06\"
  else ifeq ($(strip $(MTK_VOW_COMMAND_3RD)),keyword07)
    LOCAL_CFLAGS += -DMTK_VOW_COMMAND_3RD=\"keyword07\"
  endif

  # 2E2K
  ifeq ($(MTK_VOW_2E2K_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_VOW_2E2K_SUPPORT
  endif

  # VOW Barge-In
  ifeq ($(MTK_VOW_BARGE_IN_SUPPORT),yes)
      LOCAL_CFLAGS += -DMTK_VOW_BARGE_IN_SUPPORT
  endif

endif

### ============================================================================
### speech config
### ============================================================================

# C2K
ifneq ($(MTK_COMBO_MODEM_SUPPORT),yes)
ifeq ($(MTK_ECCCI_C2K),yes)
  LOCAL_CFLAGS += -DMTK_ECCCI_C2K
endif

RAT_CONFIG = $(strip $(MTK_PROTOCOL1_RAT_CONFIG))
ifneq (, $(RAT_CONFIG))
  ifneq (,$(findstring C,$(RAT_CONFIG)))
    LOCAL_CFLAGS += -DAUDIO_C2K_SUPPORT
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/speech_driver/SpeechMessengerEVDO.cpp
  endif
endif
endif

# refactor speech driver after 93 modem
ifeq ($(MTK_COMBO_MODEM_SUPPORT),yes)
LOCAL_CFLAGS += -DMTK_COMBO_MODEM_SUPPORT
LOCAL_SRC_FILES += \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechDriverNormal.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechMessageQueue.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechMessengerNormal.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechPcmMixerTelephonyTx.cpp
else
LOCAL_SRC_FILES += \
    $(AUDIO_COMMON_DIR)/V3/speech_driver/SpeechDriverLAD.cpp \
    $(AUDIO_COMMON_DIR)/V3/speech_driver/SpeechMessengerECCCI.cpp
endif

# for CCCI share memory EMI (after 92 modem)
ifeq ($(MTK_CCCI_SHARE_BUFFER_SUPPORT),yes)
    ifeq ($(MTK_COMBO_MODEM_SUPPORT),yes)
        LOCAL_CFLAGS += -DMTK_CCCI_SHARE_BUFFER_SUPPORT # for 93 modem and later
    else
        LOCAL_CFLAGS += -DUSE_CCCI_SHARE_BUFFER # for 92 modem
    endif
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/hardware/ccci/include
    LOCAL_SHARED_LIBRARIES += libccci_util
endif

# wb speech
ifeq ($(MTK_WB_SPEECH_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_WB_SPEECH_SUPPORT
endif

# incall handfree DMNR
ifeq ($(MTK_INCALL_HANDSFREE_DMNR),yes)
  LOCAL_CFLAGS += -DMTK_INCALL_HANDSFREE_DMNR
endif

# magic conference
ifeq ($(MTK_MAGICONFERENCE_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_MAGICONFERENCE_SUPPORT
endif

# HAC
ifeq ($(MTK_HAC_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_HAC_SUPPORT
endif

# TTY
ifeq ($(strip $(MTK_TTY_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_TTY_SUPPORT
  ifeq ($(MTK_TC9_FEATURE),yes)
    # TTY Speech Param support
    LOCAL_CFLAGS += -DMTK_AUDIO_SPH_TTY_PARAM
  endif
endif

# RTT
ifeq ($(strip $(MTK_RTT_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_RTT_SUPPORT
endif

# MTK Speech Encryption Support
ifeq ($(MTK_SPEECH_ENCRYPTION_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_SPEECH_ENCRYPTION_SUPPORT
    LOCAL_SRC_FILES += \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechDataEncrypter.cpp \
    $(AUDIO_COMMON_DIR)/aud_drv/AudioCustEncryptClient.cpp
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/AudioCustEncrypt
endif

# tuning at modem side
ifeq ($(strip $(DMNR_TUNNING_AT_MODEMSIDE)),yes)
  LOCAL_CFLAGS += -DDMNR_TUNNING_AT_MODEMSIDE
endif

# Speech Loopback Tunning
ifeq ($(MTK_TC1_FEATURE),yes)
  LOCAL_CFLAGS += -DMTK_AUDIO_SPH_LPBK_PARAM
else ifeq ($(MTK_TC10_FEATURE),yes)
  LOCAL_CFLAGS += -DMTK_AUDIO_SPH_LPBK_PARAM
else ifeq ($(MTK_AUDIO_SPH_LPBK_PARAM),yes)
  LOCAL_CFLAGS += -DMTK_AUDIO_SPH_LPBK_PARAM
endif

# Gain Tunning
ifeq ($(MTK_TC1_FEATURE),yes)
  LOCAL_CFLAGS += -DMTK_AUDIO_GAIN_TABLE_SUPPORT_CDMA
endif

# Dummy Speech Driver
ifeq ($(strip $(MTK_PROTOCOL1_RAT_CONFIG)),no)
  LOCAL_CFLAGS += -DMTK_SPEECH_DUMMY
endif

# Speech Parser
ifeq ($(MTK_COMBO_MODEM_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_SPEECH_PARSER_SUPPORT
  LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/speech_driver/SpeechParserBase.cpp
  LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/speech_driver/SpeechShareMemBase.cpp
  LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/speech_driver/SpeechExtMemCCCI.cpp

  ##depend on MD platform
  ifeq ($(MTK_MODEM_PLATFROM),GEN93)
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/speech_driver/SpeechParserGen93.cpp
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/speech_driver/SpeechShareMemGen93.cpp
  else
    LOCAL_CFLAGS += -DMTK_SPEECH_USIP_EMI_SUPPORT
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/speech_driver/SpeechParserGen95.cpp
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/speech_driver/SpeechShareMemGen95.cpp
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/speech_driver/SpeechExtMemUSIP.cpp
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/AudioSpeechEnhancement/ENH_Parser
  endif
endif

# Speech BT_SPK Dual Path Device (93 after)
ifeq ($(MTK_COMBO_MODEM_SUPPORT),yes)
  ifeq ($(MTK_BT_SPK_DUAL_PATH_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_BT_SPK_DUAL_PATH_SUPPORT
  endif
endif

### ============================================================================
### MTK Audio Tuning Tool
### ============================================================================

ifneq ($(MTK_AUDIO_TUNING_TOOL_VERSION),)
  ifneq ($(strip $(MTK_AUDIO_TUNING_TOOL_VERSION)),V1)
    MTK_AUDIO_TUNING_TOOL_V2_PHASE := $(shell echo $(MTK_AUDIO_TUNING_TOOL_VERSION) | sed 's/V2.//g')
    LOCAL_CFLAGS += -DMTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
    LOCAL_CFLAGS += -DMTK_AUDIO_TUNING_TOOL_V2_PHASE=$(MTK_AUDIO_TUNING_TOOL_V2_PHASE)
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/AudioParamParser
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/AudioParamParser/include
    LOCAL_C_INCLUDES += $(TOPDIR)external/libxml2/include
    LOCAL_C_INCLUDES += $(TOPDIR)external/icu/icu4c/source/common

    ifneq ($(MTK_AUDIO_TUNING_TOOL_V2_PHASE),1)
      LOCAL_CFLAGS += -DMTK_AUDIO_GAIN_TABLE
      LOCAL_CFLAGS += -DMTK_NEW_VOL_CONTROL

      LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAGainController.cpp
      LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioGainTableParamParser.cpp
      LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/speech_driver/SpeechConfig.cpp
      ifneq ($(MTK_COMBO_MODEM_SUPPORT),yes)
        LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/speech_driver/SpeechParamParser.cpp
      endif
    endif
  endif
endif



### ============================================================================
### FM
### ============================================================================
ifeq ($(strip $(MTK_FM_SUPPORT)),yes)
    ifeq ($(strip $(MTK_FM_TX_SUPPORT)),yes)
        ifeq ($(strip $(MTK_FM_TX_AUDIO)),FM_DIGITAL_OUTPUT)
            LOCAL_CFLAGS += -DFM_DIGITAL_OUT_SUPPORT
        endif
        ifeq ($(strip $(MTK_FM_TX_AUDIO)),FM_ANALOG_OUTPUT)
            LOCAL_CFLAGS += -DFM_ANALOG_OUT_SUPPORT
        endif
    endif
    ifeq ($(strip $(MTK_FM_RX_SUPPORT)),yes)
        ifeq ($(strip $(MTK_FM_RX_AUDIO)),FM_DIGITAL_INPUT)
            LOCAL_CFLAGS += -DFM_DIGITAL_IN_SUPPORT
        endif
        ifeq ($(strip $(MTK_FM_RX_AUDIO)),FM_ANALOG_INPUT)
            LOCAL_CFLAGS += -DFM_ANALOG_IN_SUPPORT
        endif
    endif
endif

# support fm audio path by control of setparameters API (default disable)
ifeq ($(MTK_FM_AUDIO_SUPPORT_PARAMETER_CONTROL),yes)
LOCAL_CFLAGS += -DSUPPORT_FM_AUDIO_BY_PROPRIETARY_PARAMETER_CONTROL
endif
### ============================================================================
### BT
### ============================================================================

ifeq ($(MTK_BT_SUPPORT),yes)
  ifeq ($(MTK_BT_PROFILE_A2DP),yes)
  LOCAL_CFLAGS += -DWITH_A2DP
  endif
else
  ifeq ($(strip $(BOARD_HAVE_BLUETOOTH)),yes)
    LOCAL_CFLAGS += -DWITH_A2DP
  endif
endif



### ============================================================================
### Aurisys Framework
### ============================================================================

ifeq ($(strip $(MTK_AURISYS_FRAMEWORK_SUPPORT)),yes)
    LOCAL_CFLAGS += -DMTK_AURISYS_FRAMEWORK_SUPPORT
#    LOCAL_CFLAGS += -DAURISYS_BYPASS_ALL_LIBRARY
#    LOCAL_CFLAGS += -DAURISYS_DUMP_LOG_V
#    LOCAL_CFLAGS += -DAURISYS_DUMP_PCM
#    LOCAL_CFLAGS += -DAURISYS_ENABLE_LATENCY_DEBUG
#    LOCAL_CFLAGS += -DAUDIO_UTIL_PULSE_LEVEL=16000
#    LOCAL_CFLAGS += -DUPLINK_DROP_POP_MS=100
    LOCAL_CFLAGS += -DUPLINK_DROP_POP_MS_FOR_UNPROCESSED=120

    LOCAL_C_INCLUDES += \
        $(TOPDIR)external/libxml2/include/libxml \
        $(TOPDIR)vendor/mediatek/proprietary/external/aurisys/utility \
        $(TOPDIR)vendor/mediatek/proprietary/external/aurisys/interface \
        $(LOCAL_PATH)/$(AUDIO_COMMON_DIR)/aurisys/utility \
        $(LOCAL_PATH)/$(AUDIO_COMMON_DIR)/aurisys/framework

    LOCAL_SHARED_LIBRARIES += libxml2

    LOCAL_SRC_FILES += \
        $(AUDIO_COMMON_DIR)/aurisys/utility/aurisys_utility.c \
        $(AUDIO_COMMON_DIR)/aurisys/utility/aurisys_adb_command.c \
        $(AUDIO_COMMON_DIR)/aurisys/utility/audio_pool_buf_handler.c \
        $(AUDIO_COMMON_DIR)/aurisys/utility/AudioAurisysPcmDump.c \
        $(AUDIO_COMMON_DIR)/aurisys/framework/aurisys_config_parser.c \
        $(AUDIO_COMMON_DIR)/aurisys/framework/aurisys_controller.c \
        $(AUDIO_COMMON_DIR)/aurisys/framework/aurisys_lib_manager.c \
        $(AUDIO_COMMON_DIR)/aurisys/framework/aurisys_lib_handler.c \
        $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataClientAurisysNormal.cpp
else
    LOCAL_SRC_FILES += \
        $(AUDIO_COMMON_DIR)/aud_drv/AudioMTKFilter.cpp
endif

# SCP ipi message
ifeq ($(MTK_AURISYS_PHONE_CALL_SUPPORT),yes)
    MTK_AUDIO_SCP_SUPPORT = yes
endif

ifeq ($(MTK_AUDIO_TUNNELING_SUPPORT),yes)
    MTK_AUDIO_SCP_SUPPORT = yes
endif

ifeq ($(MTK_VOW_SUPPORT),yes)
    MTK_AUDIO_SCP_SUPPORT = yes
endif

ifeq ($(MTK_AUDIODSP_SUPPORT),yes)
    MTK_AUDIO_SCP_SUPPORT = yes
endif
ifeq ($(MTK_AUDIO_SCP_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_AUDIO_SCP_SUPPORT
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/aurisys/interface
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(AUDIO_COMMON_DIR)/audiodsp
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/aud_drv/audio_messenger_ipi.c
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioMessengerIPI.cpp
endif

ifeq ($(MTK_TINYSYS_SCP_SUPPORT),yes)
ifneq ($(filter yes,$(MTK_VOW_SUPPORT) $(MTK_SCP_SMARTPA_SUPPORT)),)
    LOCAL_CFLAGS += -DMTK_AUDIO_CM4_SUPPORT
endif
endif

ifeq ($(MTK_SCP_SMARTPA_SUPPORT),yes)
    MTK_AUDIO_CM4_DMA_SUPPORT = yes
    LOCAL_CFLAGS += -DMTK_AUDIO_CM4_DMA_SUPPORT
endif

ifneq ($(filter yes,$(MTK_AUDIODSP_SUPPORT) $(MTK_AUDIO_CM4_DMA_SUPPORT)),)
    LOCAL_CFLAGS += -DMTK_AUDIO_IPI_DMA_SUPPORT
endif

ifeq ($(MTK_AUDIO_DSP_RECOVERY_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_AUDIO_DSP_RECOVERY_SUPPORT
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(AUDIO_COMMON_DIR)/audiodsp
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/audiodsp/audio_dsp_controller.c
endif


# OpenDSP
ifeq ($(MTK_AURISYS_PHONE_CALL_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_AURISYS_PHONE_CALL_SUPPORT
    LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/speech_driver/SpeechDriverOpenDSP.cpp
endif

# Voice Mixer
ifeq ($(MTK_SPEECH_VOICE_MIXER_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_SPEECH_VOICE_MIXER_SUPPORT
    LOCAL_SRC_FILES += \
        $(AUDIO_COMMON_DIR)/speech_driver/SpeechPcmMixerVoipRx.cpp
endif

### ============================================================================
### offload mp3
### ============================================================================

ifeq ($(findstring MTK_AOSP_ENHANCEMENT, $(MTK_GLOBAL_CFLAGS)),)
    LOCAL_CFLAGS += -DMTK_BASIC_PACKAGE
else
    LOCAL_SRC_FILES += \
        $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerOffload.cpp
endif



### ============================================================================
### HDMI
### ============================================================================

ifeq ($(strip $(MTK_HDMI_MULTI_CHANNEL_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_HDMI_MULTI_CHANNEL_SUPPORT
endif

ifeq ($(MTK_TDM_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_TDM_SUPPORT
  LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerHDMI.cpp
else
  LOCAL_SRC_FILES += $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerI2SHDMI.cpp
endif



### ============================================================================
### debug
### ============================================================================

# AEE
ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/aee/binary/inc
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

# Audio Lock 2.0
ifneq ($(filter $(TARGET_BUILD_VARIANT),eng userdebug),)
    LOCAL_CFLAGS += -DMTK_AUDIO_LOCK_ENABLE_TRACE
    #LOCAL_CFLAGS += -DMTK_AUDIO_LOCK_ENABLE_LOG
endif

# debug dump
ifeq ($(AUDIO_POLICY_TEST),true)
  ENABLE_AUDIO_DUMP := true
endif
ifeq ($(ENABLE_AUDIO_DUMP),true)
  LOCAL_SRC_FILES += AudioDumpInterface.cpp
  LOCAL_CFLAGS += -DENABLE_AUDIO_DUMP
endif

# non-aee audio lock timeout
ifneq ($(filter $(TARGET_BUILD_VARIANT),eng user),)
LOCAL_CFLAGS += -DUL_NON_AEE_LOCK_TIMEOUT
endif

ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
  LOCAL_CFLAGS += -DDEBUG_AUDIO_PCM
  LOCAL_CFLAGS += -DAUDIO_HAL_PROFILE_ENTRY_FUNCTION
endif


# detect pulse
ifneq ($(filter $(TARGET_BUILD_VARIANT),eng userdebug),)
    LOCAL_CFLAGS += -DMTK_LATENCY_DETECT_PULSE
    LOCAL_SHARED_LIBRARIES += libmtkaudio_utils_vendor
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/audio_utils/common_utils/AudioDetectPulse/
endif


### ============================================================================
### regular files
### ============================================================================

LOCAL_SRC_FILES += \
    $(AUDIO_COMMON_DIR)/utility/audio_lock.c \
    $(AUDIO_COMMON_DIR)/utility/audio_time.c \
    $(AUDIO_COMMON_DIR)/utility/audio_memory_control.c \
    $(AUDIO_COMMON_DIR)/utility/audio_ringbuf.c \
    $(AUDIO_COMMON_DIR)/utility/audio_sample_rate.c \
    $(AUDIO_COMMON_DIR)/aud_drv/audio_hw_hal.cpp \
    $(AUDIO_COMMON_DIR)/aud_drv/AudioMTKHeadsetMessager.cpp \
    $(AUDIO_COMMON_DIR)/aud_drv/AudioUtility.cpp \
    $(AUDIO_COMMON_DIR)/aud_drv/AudioFtmBase.cpp \
    $(AUDIO_COMMON_DIR)/aud_drv/WCNChipController.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/AudioALSASpeechPhoneCallController.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechDriverFactory.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechDriverDummy.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechEnhancementController.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechPcmMixerBase.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechPcmMixerBGSPlayer.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechPcm2way.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechUtility.cpp \
    $(AUDIO_COMMON_DIR)/speech_driver/SpeechMessageID.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAFMController.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioSpeechEnhanceInfo.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioSpeechEnhLayer.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioPreProcess.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSADriverUtility.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSASampleRateController.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAHardware.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSADataProcessor.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerBase.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerMixer.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerNormal.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerFast.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerVoice.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerFMTransmitter.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerBTSCO.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerBTCVSD.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAPlaybackHandlerUsb.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerBase.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerNormal.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerSyncIO.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerVoice.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerFMRadio.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerANC.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerBT.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerAEC.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerTDM.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataClient.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataClientSyncIO.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderBase.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderNormal.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderVoice.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderVoiceUL.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderVoiceDL.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderVoiceMix.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderFMRadio.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderANC.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderBTSCO.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderBTCVSD.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderEchoRef.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderEchoRefBTCVSD.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderEchoRefBTSCO.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderEchoRefExt.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderTDM.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderUsb.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderEchoRefUsb.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioExternWrapper.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACodecDeviceBase.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACodecDeviceOutReceiverPMIC.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACodecDeviceOutEarphonePMIC.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACodecDeviceOutSpeakerPMIC.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACodecDeviceOutSpeakerEarphonePMIC.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACodecDeviceOutExtSpeakerAmp.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSADeviceConfigManager.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACodecDeviceOutReceiverSpeakerSwitch.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAParamTuner.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/LoopbackManager.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSALoopbackController.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSADeviceParser.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioBTCVSDControl.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioVolumeFactory.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/SpeechDataProcessingHandler.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAANCController.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureDataProviderModemDai.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSACaptureHandlerModemDai.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSANLEController.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioUSBPhoneCallController.cpp \
    $(AUDIO_COMMON_DIR)/V3/speech_driver/SpeechVMRecorder.cpp \
    $(AUDIO_COMMON_DIR)/V3/speech_driver/AudioALSASpeechLoopbackController.cpp \
    $(AUDIO_COMMON_DIR)/V3/speech_driver/AudioALSASpeechStreamController.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAStreamOut.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioMixerOut.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAStreamIn.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAHardwareResourceManager.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioALSAStreamManager.cpp \
    $(AUDIO_COMMON_DIR)/V3/aud_drv/AudioBitTrueTest.cpp \
    $(AUDIO_COMMON_DIR)/aud_drv/AudioCustParamClient.cpp \
    $(AUDIO_COMMON_DIR)/aud_drv/AudioEventThreadManager.cpp \
    $(MTK_PLATFORM_DIR)/aud_drv/AudioFtm.cpp

LOCAL_ARM_MODE := arm
LOCAL_MODULE := audio.primary.$(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both

include $(MTK_SHARED_LIBRARY)

### ============================================================================
### common folder Android.mk (aud_policy/client/service/...)
### ============================================================================

include $(CLEAR_VARS)
include $(LOCAL_PATH)/$(AUDIO_COMMON_DIR)/Android.mk


### ============================================================================

endif # end of BOARD_USES_MTK_AUDIO
endif # end of old/new chips
endif # end of MTK_EMULATOR_SUPPORT

endif #MTK_TARGET_PROJECT
