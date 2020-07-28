ifeq ($(TARGET_BUILD_PDK),)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(findstring MTK_AOSP_ENHANCEMENT,  $(MTK_GLOBAL_CFLAGS)),)
	LOCAL_CFLAGS += -DMTK_BASIC_PACKAGE
endif

LOCAL_SRC_FILES:=       \
	audiocommand.cpp

LOCAL_C_INCLUDES=       \
       $(MTK_PATH_SOURCE)/frameworks/base/include/media \
       external/tinyalsa/include

LOCAL_C_INCLUDES+=       \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_utils/AudioToolkit

LOCAL_CFLAGS += -fexceptions

LOCAL_SHARED_LIBRARIES := libcutils libutils libbinder libmedia libaudioflinger libtinyalsa libaudiotoolkit

LOCAL_SHARED_LIBRARIES += libaudioclient liblog

ifeq ($(MSSI_HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaed
    LOCAL_C_INCLUDES += \
    $(MTK_PATH_SOURCE)/external/aee/binary/inc
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif
ifeq ($(MTK_AUDIO),yes)
LOCAL_C_INCLUDES+= \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_headers \
    $(MTK_PATH_SOURCE)/external/audio_utils/common_utils/AudioToolkit
endif

LOCAL_MODULE:= audiocommand
LOCAL_PROPRIETARY_MODULE := false
LOCAL_MODULE_OWNER := mtk

LOCAL_MULTILIB := 32

include $(MTK_EXECUTABLE)
endif #ifeq ($(TARGET_BUILD_PDK),)
