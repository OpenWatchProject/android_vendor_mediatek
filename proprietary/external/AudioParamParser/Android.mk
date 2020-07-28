LOCAL_PATH:= $(call my-dir)

ifeq ($(strip $(MTK_APP_FORCE_ENABLE_CUS_XML_SUPPORT)),yes)
	AUD_PARAM_PARSER_COMMON_CFLAGS += APP_FORCE_ENABLE_CUS_XML
endif

ifeq ($(strip $(TARGET_BUILD_VARIANT)),eng)
	AUD_PARAM_PARSER_COMMON_CFLAGS += -DCONFIG_MT_ENG_BUILD
endif

AUD_PARAM_PARSER_COMMON_CFLAGS += \
	-Wno-date-time \
	-Wno-unused-parameter \
	-Wno-unused-function \
	-DXML_CUS_FOLDER_ON_DEVICE=\"$(XML_CUS_FOLDER_ON_DEVICE)\"

AUD_PARAM_PARSER_COMMON_SRC_FILES:= \
	AudioCategory.c \
	AudioParam.c \
	AudioParamFieldInfo.c \
	AudioParamParser.c \
	AudioParamTreeView.c \
	AudioParamUnit.c \
	AudioType.c \
	AudioUtils.c \
	UnitTest.c \
	guicon.cpp

AUD_PARAM_PARSER_COMMON_C_INCLUDES += \
	external/libxml2/include \
	external/icu/libandroidicu/include \
	$(JNI_H_INCLUDE) \
	$(LOCAL_PATH)/include \
	$(call include-path-for, audio-utils) \

AUD_PARAM_PARSER_COMMON_SHARED_LIBRARIES += \
	libutils \
	libcutils \
	liblog \
	libbinder \
	libxml2


include $(CLEAR_VARS)

LOCAL_CFLAGS += $(AUD_PARAM_PARSER_COMMON_CFLAGS)
LOCAL_CFLAGS += -DSYS_IMPL
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(AUD_PARAM_PARSER_COMMON_SRC_FILES)
LOCAL_C_INCLUDES := $(AUD_PARAM_PARSER_COMMON_C_INCLUDES)

LOCAL_SHARED_LIBRARIES += $(AUD_PARAM_PARSER_COMMON_SHARED_LIBRARIES)
LOCAL_SHARED_LIBRARIES += \
	libaudioclient \
	libhidlbase \
	android.hardware.audio@5.0 \
	vendor.mediatek.hardware.audio@5.1

LOCAL_HEADER_LIBRARIES += libutils_headers
LOCAL_MODULE := libaudio_param_parser-sys
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both

include $(MTK_SHARED_LIBRARY)


include $(CLEAR_VARS)

LOCAL_CFLAGS += $(AUD_PARAM_PARSER_COMMON_CFLAGS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(AUD_PARAM_PARSER_COMMON_SRC_FILES)
LOCAL_C_INCLUDES := \
	$(AUD_PARAM_PARSER_COMMON_C_INCLUDES) \
	$(MTK_PATH_SOURCE)/external/audiocustparam \
	$(MTK_PATH_SOURCE)/hardware/audio/common/include \
	$(MTK_PATH_SOURCE)/external/audio_utils/common_headers/cgen/cfgfileinc
LOCAL_SHARED_LIBRARIES += \
	$(AUD_PARAM_PARSER_COMMON_SHARED_LIBRARIES) \
	libaudiocustparam_vendor \
	libaudioprimarydevicehalifclient

LOCAL_HEADER_LIBRARIES += libutils_headers
LOCAL_MODULE := libaudio_param_parser-vnd
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MULTILIB := both

include $(MTK_SHARED_LIBRARY)


#########################################################
# audio_param_test
#########################################################

AUD_PARAM_PARSER_COMMON_SRC_FILES := test/main.c

AUD_PARAM_PARSER_COMMON_C_INCLUDES := \
	external/libxml2/include \
	external/icu/libandroidicu/include \
	$(MTK_PATH_SOURCE)/external/AudioParamParser/ \
	$(MTK_PATH_SOURCE)/external/AudioParamParser/include

AUD_PARAM_PARSER_COMMON_SHARED_LIBRARIES := \
	liblog \
	libxml2

include $(CLEAR_VARS)

LOCAL_CFLAGS += -DSYS_IMPL -DXML_CUS_FOLDER_ON_DEVICE=\"$(XML_CUS_FOLDER_ON_DEVICE)\" $(AUD_PARAM_PARSER_COMMON_CFLAGS)
LOCAL_SRC_FILES := $(AUD_PARAM_PARSER_COMMON_SRC_FILES)
LOCAL_C_INCLUDES := $(AUD_PARAM_PARSER_COMMON_C_INCLUDES)
LOCAL_SHARED_LIBRARIES += \
	$(AUD_PARAM_PARSER_COMMON_SHARED_LIBRARIES) \
	libaudio_param_parser-sys \
	libandroidicu
LOCAL_HEADER_LIBRARIES += libutils_headers
LOCAL_MODULE := audio_param_test-sys
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

include $(MTK_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_CFLAGS += -DXML_CUS_FOLDER_ON_DEVICE=\"$(XML_CUS_FOLDER_ON_DEVICE)\" $(AUD_PARAM_PARSER_COMMON_CFLAGS)
LOCAL_SRC_FILES := $(AUD_PARAM_PARSER_COMMON_SRC_FILES)
LOCAL_C_INCLUDES := $(AUD_PARAM_PARSER_COMMON_C_INCLUDES)
LOCAL_SHARED_LIBRARIES += \
	$(AUD_PARAM_PARSER_COMMON_SHARED_LIBRARIES) \
	libaudio_param_parser-vnd
LOCAL_HEADER_LIBRARIES += libutils_headers
LOCAL_MODULE := audio_param_test-vnd
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(MTK_EXECUTABLE)

include $(LOCAL_PATH)/DeployAudioParam.mk

include $(LOCAL_PATH)/GenAudioParamOptionsXml.mk

