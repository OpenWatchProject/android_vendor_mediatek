LOCAL_PATH := $(call my-dir)

#====================VoiceCmdRecognition======================
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    VoiceCmdRecognition.cpp

LOCAL_C_INCLUDES := \
    $(TOP)/frameworks/av/include \
    $(TOP)/frameworks/native/include \
    $(TOP)/frameworks/av/include/media \
    $(TOP)/vendor/mediatek/proprietary/external \
    $(TOP)/vendor/mediatek/proprietary/external/voiceunlock2/include \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_utils/AudioToolkit \
    $(MTK_PATH_SOURCE)/hardware/audio/common/include

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libstagefright \
    libstagefright_foundation \
    libmedia \
    libstdc++ \
    libgui \
    libdl \
    libsqlite \
    libaudiotoolkit \
    libaudioclient

ifeq ($(strip $(MSSI_MTK_VOW_2E2K_SUPPORT)),yes)
    LOCAL_SHARED_LIBRARIES += libvow_training_alexa
    LOCAL_SHARED_LIBRARIES += libvowp2training_alexa
    LOCAL_SHARED_LIBRARIES += libvowp23_sid_training_alexa
    LOCAL_SHARED_LIBRARIES += libvowp25training_alexa
else
    LOCAL_SHARED_LIBRARIES += libvow_training
    LOCAL_SHARED_LIBRARIES += libvowp2training
    LOCAL_SHARED_LIBRARIES += libvowp23_sid_training
    LOCAL_SHARED_LIBRARIES += libvowp25training
endif

ifeq ($(strip $(MTK_DUAL_MIC_SUPPORT)),yes)
  LOCAL_CFLAGS += -DMTK_DUAL_MIC_SUPPORT
endif
# For AudioParamParser
LOCAL_C_INCLUDES += \
    external/libxml2/include \
    external/icu/libandroidicu/include \
    $(MTK_PATH_SOURCE)/external/AudioParamParser/ \
    $(MTK_PATH_SOURCE)/external/AudioParamParser/include
LOCAL_CFLAGS += -DSYS_IMPL

LOCAL_STATIC_LIBRARIES += libaudiostream

#LOCAL_STATIC_LIBRARIES += libdrvb
#LOCAL_SHARED_LIBRARIES += libmtk_drvb

LOCAL_LDLIBS += -llog
LOCAL_MULTILIB := both

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE := libvoicerecognition
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)
