LOCAL_PATH := $(my-dir)

############################################################################################################### Vendor
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

ifeq ($(findstring MTK_AOSP_ENHANCEMENT,  $(MTK_GLOBAL_CFLAGS)),)
    LOCAL_CFLAGS += -DMTK_BASIC_PACKAGE
endif

LOCAL_C_INCLUDES:= \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_CUSTOM)/hal/audioflinger/audio \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/customization

LOCAL_SRC_FILES := \
    AudioCustParam.cpp

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libutils \
    libnvram \
    libcustom_nvram

# Speaker Monitor
ifeq ($(MTK_SPEAKER_MONITOR_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_SPEAKER_MONITOR_SUPPORT
endif
# Speaker Monitor

# Audio HD Record
ifeq ($(MTK_AUDIO_HD_REC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_AUDIO_HD_REC_SUPPORT
endif
# Audio HD Record

# Wearable support
ifeq ($(strip $(MTK_WEARABLE_PLATFORM )),yes)
    LOCAL_CFLAGS += -DMTK_WEARABLE_PLATFORM
endif

# Dual Mic Support
ifeq ($(MTK_DUAL_MIC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_DUAL_MIC_SUPPORT
endif
# Dual Mic Support

# DMNR3.0 Support
ifeq ($(MTK_HANDSFREE_DMNR_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_HANDSFREE_DMNR_SUPPORT
endif
# DMNR3.0 Support

# VOIP enhance Support
ifeq ($(MTK_VOIP_ENHANCEMENT_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_VOIP_ENHANCEMENT_SUPPORT
endif
# VOIP enhance Support

# Automatic Speech Recognition Support
ifeq ($(MTK_ASR_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_ASR_SUPPORT
endif
# Automatic Speech Recognition Support

# VoIP normal mode DMNR Support
ifeq ($(MTK_VOIP_NORMAL_DMNR),yes)
    LOCAL_CFLAGS += -DMTK_VOIP_NORMAL_DMNR
endif
# VoIP normal mode DMNR Support

# VoIP handsfree mode DMNR Support
ifeq ($(MTK_VOIP_HANDSFREE_DMNR),yes)
    LOCAL_CFLAGS += -DMTK_VOIP_HANDSFREE_DMNR
endif
# VoIP handsfree mode DMNR Support

# Incall handsfree mode DMNR Support
ifeq ($(MTK_INCALL_HANDSFREE_DMNR),yes)
    LOCAL_CFLAGS += -DMTK_INCALL_HANDSFREE_DMNR
endif
# Incall handsfree mode DMNR Support

# Incall normal mode DMNR Support
ifneq ($(MTK_INCALL_NORMAL_DMNR),no)
    LOCAL_CFLAGS += -DMTK_INCALL_NORMAL_DMNR
endif
# Incall normal mode DMNR Support

# wifi only
ifeq ($(MTK_TB_WIFI_3G_MODE),WIFI_ONLY)
    LOCAL_CFLAGS += -DMTK_WIFI_ONLY_SUPPORT
endif
# wifi only

# 3g data
ifeq ($(MTK_TB_WIFI_3G_MODE),3GDATA_SMS)
    LOCAL_CFLAGS += -DMTK_3G_DATA_ONLY_SUPPORT
endif

ifeq ($(MTK_TB_WIFI_3G_MODE),3GDATA_ONLY)
    LOCAL_CFLAGS += -DMTK_3G_DATA_ONLY_SUPPORT
endif
# 3g data

# check if there is receiver
ifeq ($(DISABLE_EARPIECE),yes)
    LOCAL_CFLAGS += -DMTK_DISABLE_EARPIECE
endif
# check if there is receiver

# WB Speech Support
ifeq ($(MTK_WB_SPEECH_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_WB_SPEECH_SUPPORT
endif
#WB Speech Support

# DMNR tuning at modem side
ifeq ($(DMNR_TUNNING_AT_MODEMSIDE),yes)
    LOCAL_CFLAGS += -DDMNR_TUNNING_AT_MODEMSIDE
endif
# DMNR tuning at modem side

# Voice Unlock
ifeq ($(MTK_VOICE_UNLOCK_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_VOICE_UNLOCK_SUPPORT
endif
# Voice Unlock

# Voice UI
ifeq ($(MTK_VOICE_UI_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_VOICE_UI_SUPPORT
endif
# Voice UI

# DMNR 2.5 complex arch
#ifeq ($(DMNR_COMPLEX_ARCH_SUPPORT),yes)
    LOCAL_CFLAGS += -DDMNR_COMPLEX_ARCH_SUPPORT
#endif
# DMNR 2.5 complex arch

# Magic Conference Support
ifeq ($(MTK_MAGICONFERENCE_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_MAGICONFERENCE_SUPPORT
endif
# Magic Conference Support

# HAC Support
ifeq ($(MTK_HAC_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_HAC_SUPPORT
endif
# HAC Support

# TC1 Gain Table Support
ifeq ($(MTK_SUPPORT_TC1_TUNNING),yes)
    LOCAL_CFLAGS += -DMTK_AUDIO_GAIN_TABLE
endif
# TC1 Gain Table Support

# MTK Audio Tuning Tool Version
ifneq ($(MTK_AUDIO_TUNING_TOOL_VERSION),)
    ifneq ($(strip $(MTK_AUDIO_TUNING_TOOL_VERSION)),V1)
        MTK_AUDIO_TUNING_TOOL_V2_PHASE:=$(shell echo $(MTK_AUDIO_TUNING_TOOL_VERSION) | sed 's/V2.//g')
        LOCAL_CFLAGS += -DMTK_AUDIO_HIERARCHICAL_PARAM_SUPPORT
        LOCAL_CFLAGS += -DMTK_AUDIO_TUNING_TOOL_V2_PHASE=$(MTK_AUDIO_TUNING_TOOL_V2_PHASE)
        LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/AudioParamParser
        LOCAL_C_INCLUDES += $(TOPDIR)vendor/mediatek/proprietary/external/AudioParamParser/include
        LOCAL_C_INCLUDES += $(TOPDIR)external/libxml2/include
        LOCAL_C_INCLUDES += $(TOPDIR)external/icu/libandroidicu/include
    endif
endif

LOCAL_MODULE := libaudiocustparam_vendor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

#ifeq ($(MTK_AUDIO_A64_SUPPORT),yes)
    #LOCAL_MULTILIB := both
#else
    #LOCAL_MULTILIB := 32
#endif

LOCAL_ARM_MODE := arm

include $(MTK_SHARED_LIBRARY)
