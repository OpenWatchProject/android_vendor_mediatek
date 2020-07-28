LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    MtkAudioLoudc.c

LOCAL_MODULE := libaudioloudc

LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
    $(MTK_PATH_SOURCE)/external/bessound_HD \
    $(MTK_PATH_SOURCE)/external/aurisys/interface \
    $(MTK_PATH_SOURCE)/hardware/audio/common/utility \
    $(MTK_PATH_CUSTOM)/hal/audioflinger/audio \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/customization

LOCAL_SHARED_LIBRARIES := \
    libaudiocompensationfilterc \
    libnvram \
    libbessound_hd_mtk_vendor \
    libcutils \
    libutils \
    liblog

ifeq ($(MTK_BESLOUDNESS_SUPPORT),yes)
    ifeq ($(MTK_BESLOUDNESS_RUN_WITH_HAL),yes)
        LOCAL_CFLAGS += -DMTK_BESLOUDNESS_RUN_WITH_HAL
    endif
endif

ifneq ($(strip $(MTK_AUDIO_NUMBER_OF_SPEAKER)),)
	ifeq ($(shell test $(MTK_AUDIO_NUMBER_OF_SPEAKER) -ge 2; echo $$?),0)
		LOCAL_CFLAGS += -DMTK_ENABLE_STEREO_SPEAKER
	endif
endif
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both
include $(MTK_SHARED_LIBRARY)
