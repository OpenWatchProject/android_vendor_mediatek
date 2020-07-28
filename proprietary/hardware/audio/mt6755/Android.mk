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

#  only if use yusu audio will build this.
#ifneq ($(TARGET_BUILD_PDK),true)
ifeq ($(strip $(BOARD_USES_MTK_AUDIO)),true)

LOCAL_PATH:= $(call my-dir)
LOCAL_COMMON_PATH:=../common

include $(CLEAR_VARS)

ifeq ($(MTK_PLATFORM),MT6755)

# normal playback kernel buffer size
LOCAL_CFLAGS += -DKERNEL_BUFFER_SIZE_DL1=48*1024

# normal capture kernel buffer size
LOCAL_CFLAGS += -DKERNEL_BUFFER_SIZE_UL1_NORMAL=48*1024

# Drop UL glitch data
LOCAL_CFLAGS += -DDROP_MS_AFTER_BESRECORD_PROCESS=80

ifeq ($(strip $(MTK_HIGH_RESOLUTION_AUDIO_SUPPORT)),yes)
    LOCAL_CFLAGS += -DMTK_HD_AUDIO_ARCHITECTURE
endif

ifeq ($(AUDIO_POLICY_TEST),true)
  ENABLE_AUDIO_DUMP := true
endif

ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
  LOCAL_CFLAGS += -DDEBUG_AUDIO_PCM
  LOCAL_CFLAGS += -DAUDIO_HAL_PROFILE_ENTRY_FUNCTION
endif

ifeq ($(MTK_DIGITAL_MIC_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_DIGITAL_MIC_SUPPORT
endif

ifeq ($(strip $(MTK_AUDENH_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_AUDENH_SUPPORT
endif

ifeq ($(strip $(MTK_BESLOUDNESS_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_BESLOUDNESS_SUPPORT
endif

ifeq ($(strip $(MTK_BESSURROUND_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_BESSURROUND_SUPPORT
endif

ifeq ($(strip $(MTK_HDMI_MULTI_CHANNEL_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_HDMI_MULTI_CHANNEL_SUPPORT
endif

ifeq ($(strip $(MTK_AUDIO_MIC_INVERSE)),yes)
  LOCAL_CFLAGS += -DMTK_AUDIO_MIC_INVERSE
endif

ifeq ($(strip $(MTK_2IN1_SPK_SUPPORT)),yes)
  LOCAL_CFLAGS += -DUSING_2IN1_SPEAKER
endif

ifeq ($(strip $(MTK_USE_ANDROID_MM_DEFAULT_CODE)),yes)
  LOCAL_CFLAGS += -DANDROID_DEFAULT_CODE
endif

ifeq ($(strip $(DMNR_TUNNING_AT_MODEMSIDE)),yes)
  LOCAL_CFLAGS += -DDMNR_TUNNING_AT_MODEMSIDE
endif

ifeq ($(MTK_INCALL_HANDSFREE_DMNR),yes)
  LOCAL_CFLAGS += -DMTK_INCALL_HANDSFREE_DMNR
endif

ifeq ($(MTK_VOW_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_VOW_SUPPORT
endif

ifeq ($(MTK_MAGICONFERENCE_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_MAGICONFERENCE_SUPPORT
endif

ifeq ($(MTK_HAC_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_HAC_SUPPORT
endif

ifeq ($(MTK_VIBSPK_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_VIBSPK_SUPPORT
endif

ifeq ($(MTK_HIFIAUDIO_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_HIFIAUDIO_SUPPORT
endif

#ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
#  LOCAL_CFLAGS += -DCONFIG_MT_ENG_BUILD
#endif

LOCAL_CFLAGS += -DUPLINK_LOW_LATENCY
LOCAL_CFLAGS += -DDOWNLINK_LOW_LATENCY


ifeq ($(MTK_BSP_PACKAGE),yes)
 LOCAL_CFLAGS += -DMTK_BSP_PACKAGE
endif

LOCAL_CFLAGS += -DSW_BTCVSD_ENABLE

LOCAL_CFLAGS += -DMTK_SUPPORT_AUDIO_DEVICE_API3

LOCAL_CFLAGS += -DMTK_SUPPORT_BTCVSD_ALSA

# Playback must be 24bit when using sram
LOCAL_CFLAGS += -DPLAYBACK_USE_24BITS_ONLY

# Audio Lock 2.0
LOCAL_CFLAGS += -DMTK_AUDIO_LOCK_ENABLE_TRACE
ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
#    LOCAL_CFLAGS += -DMTK_AUDIO_LOCK_ENABLE_LOG
endif

LOCAL_C_INCLUDES:= \
    hardware/libhardware_legacy/include \
    hardware/libhardware/include \
    $(MTK_PATH_SOURCE)/hardware/ccci/include \
    external/tinyxml \
    $(call include-path-for, audio-utils) \
    $(call include-path-for, audio-effects) \
    $(call include-path-for, alsa-utils) \
    $(MTK_PATH_SOURCE)/hardware/audio/$(shell echo $(MTK_PLATFORM) | tr A-Z a-z)/include \
    $(MTK_PATH_SOURCE)/hardware/audio/common/utility \
    $(MTK_PATH_SOURCE)/hardware/audio/common/V3/include \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
    $(MTK_PATH_SOURCE)/external/AudioComponentEngine \
    $(MTK_PATH_SOURCE)/external/audiocustparam \
    $(MTK_PATH_SOURCE)/external/AudioSpeechEnhancement/V3/inc \
    $(MTK_PATH_SOURCE)/hardware/power/config/common/intf_types \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_utils/AudioToolkit \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/audiopolicy_parameters \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/customization \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/custom_volume \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/gain_table \
    $(MTK_PATH_CUSTOM)/hal/audioflinger/audio  \
    external/tinyalsa/include  \
    external/tinycompress/include

# MTK Audio Tuning Tool Version
ifneq ($(MTK_AUDIO_TUNING_TOOL_VERSION),)
  ifneq ($(strip $(MTK_AUDIO_TUNING_TOOL_VERSION)),V1)
    MTK_AUDIO_TUNING_TOOL_V2_PHASE:=$(shell echo $(MTK_AUDIO_TUNING_TOOL_VERSION) | sed 's/V2.//g')
    LOCAL_CFLAGS += -DMTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
    LOCAL_CFLAGS += -DMTK_AUDIO_TUNING_TOOL_V2_PHASE=$(MTK_AUDIO_TUNING_TOOL_V2_PHASE)
    LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/AudioParamParser
    LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/AudioParamParser/include
    LOCAL_C_INCLUDES += external/libxml2/include
    LOCAL_C_INCLUDES += external/icu/icu4c/source/common

    LOCAL_SRC_FILES+= \
        $(LOCAL_COMMON_PATH)/V3/speech_driver/SpeechParamParser.cpp \
        $(LOCAL_COMMON_PATH)/speech_driver/SpeechConfig.cpp

    ifneq ($(MTK_AUDIO_TUNING_TOOL_V2_PHASE),1)
      LOCAL_CFLAGS += -DMTK_AUDIO_GAIN_TABLE
      LOCAL_CFLAGS += -DMTK_NEW_VOL_CONTROL

      LOCAL_SRC_FILES += $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAGainController.cpp
      LOCAL_SRC_FILES += $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioGainTableParamParser.cpp

    endif
  endif
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

# Speaker Customization
ifeq ($(strip $(MTK_AUDIO_SPEAKER_PATH)),int_spk_amp)

else ifeq ($(strip $(MTK_AUDIO_SPEAKER_PATH)),int_lo_buf)
  LOCAL_CFLAGS += -DUSING_EXTAMP_LO
else ifeq ($(strip $(MTK_AUDIO_SPEAKER_PATH)),int_hp_buf)
  LOCAL_CFLAGS += -DUSING_EXTAMP_HP
else ifeq ($(strip $(MTK_AUDIO_SPEAKER_PATH)),2_in_1_spk)

else ifeq ($(strip $(MTK_AUDIO_SPEAKER_PATH)),3_in_1_spk)

else ifeq ($(findstring smartpa_dynamic_detect, $(MTK_AUDIO_SPEAKER_PATH)),smartpa_dynamic_detect)
  LOCAL_CFLAGS += -DSMARTPA_DYNAMIC_DETECT
else
  ifeq ($(findstring smartpa, $(MTK_AUDIO_SPEAKER_PATH)), smartpa)
    LOCAL_CFLAGS += -DEXT_SPK_SUPPORT	# for old volume control

    ifeq ($(findstring maxim, $(MTK_AUDIO_SPEAKER_PATH)), maxim)
      LOCAL_CFLAGS += -DMTK_MAXIM_SPEAKER_SUPPORT

      LOCAL_SRC_FILES += $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerSpeakerProtection.cpp \
        $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPcmDataCaptureIn.cpp
    endif
  endif
endif

# Smart Pa
LOCAL_SRC_FILES += $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioSmartPaController.cpp
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/AudioParamParser
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/AudioParamParser/include
LOCAL_C_INCLUDES += external/libxml2/include
LOCAL_C_INCLUDES += external/icu/icu4c/source/common


# POWR HAL Control for low latency & low power
LOCAL_CFLAGS += -DMTK_POWERHAL_AUDIO_LATENCY
# LOCAL_CFLAGS += -DMTK_POWERHAL_AUDIO_POWER
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


LOCAL_SRC_FILES+= \
    $(LOCAL_COMMON_PATH)/utility/audio_lock.c \
    $(LOCAL_COMMON_PATH)/utility/audio_time.c \
    $(LOCAL_COMMON_PATH)/utility/audio_ringbuf.c \
    $(LOCAL_COMMON_PATH)/aud_drv/audio_hw_hal.cpp \
    $(LOCAL_COMMON_PATH)/aud_drv/AudioMTKFilter.cpp \
    $(LOCAL_COMMON_PATH)/aud_drv/AudioMTKHeadsetMessager.cpp \
    $(LOCAL_COMMON_PATH)/aud_drv/AudioUtility.cpp \
    $(LOCAL_COMMON_PATH)/aud_drv/AudioFtmBase.cpp \
    $(LOCAL_COMMON_PATH)/aud_drv/WCNChipController.cpp \
    $(LOCAL_COMMON_PATH)/speech_driver/SpeechDriverFactory.cpp \
    $(LOCAL_COMMON_PATH)/speech_driver/SpeechDriverDummy.cpp \
    $(LOCAL_COMMON_PATH)/speech_driver/SpeechEnhancementController.cpp \
    $(LOCAL_COMMON_PATH)/speech_driver/SpeechPcmMixerBase.cpp \
    $(LOCAL_COMMON_PATH)/speech_driver/SpeechPcmMixerBGSPlayer.cpp \
    $(LOCAL_COMMON_PATH)/speech_driver/SpeechPcm2way.cpp \
    $(LOCAL_COMMON_PATH)/speech_driver/SpeechUtility.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAFMController.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioSpeechEnhanceInfo.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioSpeechEnhLayer.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioPreProcess.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSADriverUtility.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSASampleRateController.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAHardware.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSADataProcessor.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerBase.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerMixer.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerNormal.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerFast.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerVoice.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerFMTransmitter.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerBTSCO.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerBTCVSD.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerI2SHDMI.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureHandlerBase.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureHandlerNormal.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureHandlerSyncIO.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureHandlerVoice.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureHandlerFMRadio.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureHandlerANC.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureHandlerBT.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureHandlerAEC.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataClient.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataClientSyncIO.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderBase.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderNormal.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderVoice.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderVoiceUL.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderVoiceDL.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderVoiceMix.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderFMRadio.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderANC.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderBTSCO.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderBTCVSD.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderEchoRef.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderEchoRefBTCVSD.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderEchoRefBTSCO.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderEchoRefExt.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACodecDeviceBase.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACodecDeviceOutReceiverPMIC.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACodecDeviceOutEarphonePMIC.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACodecDeviceOutSpeakerPMIC.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACodecDeviceOutSpeakerEarphonePMIC.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACodecDeviceOutExtSpeakerAmp.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSADeviceConfigManager.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACodecDeviceOutReceiverSpeakerSwitch.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAParamTuner.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/LoopbackManager.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSALoopbackController.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSADeviceParser.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioBTCVSDControl.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioVolumeFactory.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/SpeechDataProcessingHandler.cpp \
    $(LOCAL_COMMON_PATH)/V3/speech_driver/SpeechDriverLAD.cpp \
    $(LOCAL_COMMON_PATH)/V3/speech_driver/SpeechMessengerECCCI.cpp \
    $(LOCAL_COMMON_PATH)/V3/speech_driver/SpeechVMRecorder.cpp \
    $(LOCAL_COMMON_PATH)/V3/speech_driver/AudioALSASpeechLoopbackController.cpp \
    $(LOCAL_COMMON_PATH)/V3/speech_driver/AudioALSASpeechStreamController.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAHardwareResourceManager.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAStreamIn.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAStreamManager.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAStreamOut.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioMixerOut.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureHandlerVOW.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSACaptureDataProviderVOW.cpp \
    $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAVoiceWakeUpController.cpp \
    aud_drv/AudioFtm.cpp \
    $(LOCAL_COMMON_PATH)/speech_driver/AudioALSASpeechPhoneCallController.cpp \
    $(LOCAL_COMMON_PATH)/aud_drv/AudioCustParamClient.cpp \
    $(LOCAL_COMMON_PATH)/aud_drv/AudioEventThreadManager.cpp

ifeq ($(MTK_ECCCI_C2K),yes)
  LOCAL_CFLAGS += -DMTK_ECCCI_C2K
endif

ifeq ($(ENABLE_AUDIO_DUMP),true)
  LOCAL_SRC_FILES += AudioDumpInterface.cpp
  LOCAL_CFLAGS += -DENABLE_AUDIO_DUMP
endif

#ifeq ($(MTK_VIBSPK_SUPPORT),yes)
  LOCAL_SRC_FILES += $(LOCAL_COMMON_PATH)/aud_drv/AudioVIBSPKControl.cpp
#endif

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
    $(LOCAL_COMMON_PATH)/speech_driver/SpeechDataEncrypter.cpp \
    $(LOCAL_COMMON_PATH)/aud_drv/AudioCustEncryptClient.cpp
    LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/AudioCustEncrypt
endif

# FOR BRINGUP
#  LOCAL_CFLAGS += -DFORCE_ROUTING_RECEIVER
#  LOCAL_CFLAGS += -DCCCI_FORCE_USE
#  LOCAL_CFLAGS += -DSPEECH_PMIC_RESET
#  LOCAL_CFLAGS += -DSPH_SKIP_A2M_BUFF_MSG
# FOR BRINGUP END

LOCAL_CFLAGS += -DMTK_SPH_MAGICLARITY_SHAPEFIR_SUPPORT
# LOCAL_CFLAGS += -DMTK_AUDIO_SPH_NETWORK_PARAM

ifeq ($(findstring MTK_AOSP_ENHANCEMENT,  $(MTK_GLOBAL_CFLAGS)),)
    LOCAL_CFLAGS += -DMTK_BASIC_PACKAGE
else
# for OFFLOAD start
    LOCAL_SRC_FILES+= \
        $(LOCAL_COMMON_PATH)/V3/aud_drv/AudioALSAPlaybackHandlerOffload.cpp
# Offload End
endif

ifeq ($(strip $(MTK_ENABLE_MD1)),yes)
  LOCAL_CFLAGS += -D__MTK_ENABLE_MD1__
endif

LOCAL_SHARED_LIBRARIES += \
    libc \
    libcutils \
    liblog \
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
    libmedia_helper

LOCAL_HEADER_LIBRARIES += libaudioclient_headers libaudio_system_headers libmedia_headers

# CallStack
LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/libudf/libladder
LOCAL_SHARED_LIBRARIES += libladder

RAT_CONFIG = $(strip $(MTK_PROTOCOL1_RAT_CONFIG))
ifneq (, $(RAT_CONFIG))
  ifneq (,$(findstring C,$(RAT_CONFIG)))
    LOCAL_CFLAGS += -DAUDIO_C2K_SUPPORT
    LOCAL_SRC_FILES+= $(LOCAL_COMMON_PATH)/V3/speech_driver/SpeechMessengerEVDO.cpp

  endif
endif

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
    LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/aee/binary/inc
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

ifeq ($(TELEPHONY_DFOSET),yes)
    LOCAL_SHARED_LIBRARIES += libdfo
endif

ifeq ($(MTK_WB_SPEECH_SUPPORT),yes)
  LOCAL_CFLAGS += -DMTK_WB_SPEECH_SUPPORT
endif

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

ifeq ($(HAVE_MATV_FEATURE),yes)
    LOCAL_CFLAGS += -DMATV_AUDIO_SUPPORT
endif

ifeq ($(MTK_BT_SUPPORT),yes)
  ifeq ($(MTK_BT_PROFILE_A2DP),yes)
  LOCAL_CFLAGS += -DWITH_A2DP
  endif
else
  ifeq ($(strip $(BOARD_HAVE_BLUETOOTH)),yes)
    LOCAL_CFLAGS += -DWITH_A2DP
  endif
endif

# SRS Processing
ifeq ($(strip $(HAVE_SRSAUDIOEFFECT_FEATURE)),yes)
LOCAL_CFLAGS += -DHAVE_SRSAUDIOEFFECT
endif
# SRS Processing

# Audio HD Record
ifeq ($(MTK_AUDIO_HD_REC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_AUDIO_HD_REC_SUPPORT
endif
# Audio HD Record

# MTK VoIP
ifeq ($(MTK_VOIP_ENHANCEMENT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_VOIP_ENHANCEMENT_SUPPORT
endif
# MTK VoIP

# DMNR 3.0
ifeq ($(strip $(MTK_HANDSFREE_DMNR_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_HANDSFREE_DMNR_SUPPORT
endif
# DMNR 3.0

# Native Audio Preprocess
ifeq ($(strip $(NATIVE_AUDIO_PREPROCESS_ENABLE)),yes)
    LOCAL_CFLAGS += -DNATIVE_AUDIO_PREPROCESS_ENABLE
endif
# Native Audio Preprocess

#Temp tag for FM support WIFI-Display output
LOCAL_CFLAGS += -DMTK_FM_SUPPORT_WFD_OUTPUT

#Record must be 24bit when using sram
LOCAL_CFLAGS += -DRECORD_INPUT_24BITS

ifeq ($(HAVE_MATV_FEATURE),yes)
    ifeq ($(MTK_MATV_ANALOG_SUPPORT),yes)
        LOCAL_CFLAGS += -DMATV_AUDIO_LINEIN_PATH
    endif
endif

# detect pulse
ifneq ($(filter $(TARGET_BUILD_VARIANT),eng userdebug),)
    LOCAL_CFLAGS += -DMTK_LATENCY_DETECT_PULSE
    LOCAL_SHARED_LIBRARIES += libmtkaudio_utils_vendor
    LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/audio_utils/common_utils/AudioDetectPulse/
endif

LOCAL_ARM_MODE := arm
LOCAL_MODULE := audio.primary.$(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both
include $(MTK_SHARED_LIBRARY)

endif
endif
