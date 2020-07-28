LOCAL_PATH := $(my-dir)

############################################################################################################### System
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
MtkAudioComponent.cpp \
MtkAudioBitConverter.cpp \
MtkAudioLoud.cpp

LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
    $(MTK_PATH_SOURCE)/external/limiter \
    $(MTK_PATH_SOURCE)/external/shifter \
    $(MTK_PATH_SOURCE)/external/bessound_HD \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_utils/AudioToolkit \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc

LOCAL_SHARED_LIBRARIES := \
    libaudiocompensationfilter \
    libcutils \
    liblog \
    libutils \
    libbessound_hd_mtk \
    libmtklimiter \
    libmtkshifter \
    libaudiotoolkit

ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
  LOCAL_CFLAGS += -DCONFIG_MT_ENG_BUILD
endif

LOCAL_CFLAGS += -DSYS_IMPL

LOCAL_MODULE := libaudiocomponentengine

LOCAL_MODULE_TAGS := optional
#ifeq ($(MTK_AUDIO_A64_SUPPORT),yes)
LOCAL_MULTILIB := both
#else
#LOCAL_MULTILIB := 32
#endif
include $(MTK_SHARED_LIBRARY)

############################################################################################################### Vendor
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
MtkAudioComponent.cpp \
MtkAudioBitConverter.cpp \
MtkAudioSrc.cpp \
MtkAudioDcRemoval.cpp \
MtkAudioLoud.cpp

LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/external/nvram/libnvram \
    $(MTK_PATH_SOURCE)/external/AudioCompensationFilter \
    $(MTK_PATH_SOURCE)/external/blisrc/blisrc32 \
    $(MTK_PATH_SOURCE)/external/limiter \
    $(MTK_PATH_SOURCE)/external/shifter \
    $(MTK_PATH_SOURCE)/external/audiodcremoveflt \
    $(MTK_PATH_SOURCE)/external/bessound_HD \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc


LOCAL_SHARED_LIBRARIES := \
    libaudiocompensationfilter_vendor \
    libcutils \
    liblog \
    libutils \
    libblisrc32_vendor \
    libbessound_hd_mtk_vendor \
    libmtklimiter_vendor \
    libaudiodcrflt_vendor \
    libmtkshifter_vendor

ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
  LOCAL_CFLAGS += -DCONFIG_MT_ENG_BUILD
endif
LOCAL_CFLAGS += -DENABLE_DC_REMOVE
LOCAL_CFLAGS += -DENABLE_SRC

LOCAL_MODULE := libaudiocomponentengine_vendor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional
#ifeq ($(MTK_AUDIO_A64_SUPPORT),yes)
LOCAL_MULTILIB := both
#else
#LOCAL_MULTILIB := 32
#endif
include $(MTK_SHARED_LIBRARY)


############################## libaudiocomponentenginec used by vendor only
include $(CLEAR_VARS)

LOCAL_C_INCLUDES:=  $(LOCAL_PATH)

LOCAL_SRC_FILES := \
    MtkAudioSrcInC.c \
    MtkAudioBitConverterc.c

LOCAL_MODULE := libaudiocomponentenginec
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_C_INCLUDES := \
    $(MTK_PATH_SOURCE)/external/blisrc/blisrc32 \
    $(MTK_PATH_SOURCE)/external/limiter \
    $(MTK_PATH_SOURCE)/external/shifter \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libblisrc32_vendor \
    libmtklimiter_vendor \
    libmtkshifter_vendor

LOCAL_LDLIBS := -llog
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MULTILIB := both
include $(MTK_SHARED_LIBRARY)
