LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(strip $(MTK_VOIP_HANDSFREE_DMNR)),yes)
LOCAL_CFLAGS += -DMTK_VOIP_HANDSFREE_DMNR
endif


LOCAL_SRC_FILES := \
    mtk_speech_enh_api.c \
    mtk_speech_enh_paramProvider.c

LOCAL_LDLIBS := -llog
LOCAL_C_INCLUDES := \
    external/libxml2/include \
    external/icu/libandroidicu/include \
    $(JNI_H_INCLUDE) \
    $(MTK_PATH_SOURCE)/external/AudioParamParser/include \
    $(MTK_PATH_SOURCE)/external/aurisys/interface \
    $(MTK_PATH_SOURCE)/external/aurisys/lib_speech_enh/libMtkSpeechEnh/include \
    $(MTK_PATH_SOURCE)/external/AudioParamParser

LOCAL_SHARED_LIBRARIES := \
    libMtkSpeechEnh \
    libcutils \
    liblog \
    libutils \
    libaudioutils

LOCAL_MODULE := lib_speech_enh
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
# LOCAL_MODULE_CLASS := SHARED_LIBRARIES
# LOCAL_SRC_FILES_arm := lib_speech_enh.so
LOCAL_MODULE_SUFFIX := .so
# LOCAL_SRC_FILES_arm64 := lib64_speech_enh.so
LOCAL_MULTILIB := both

# include $(BUILD_PREBUILT)

include $(MTK_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
