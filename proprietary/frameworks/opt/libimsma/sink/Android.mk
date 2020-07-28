############################################################

build_vilte =
ifeq ($(strip $(MSSI_MTK_VILTE_SUPPORT)),yes)
    build_vilte = yes
endif

ifeq ($(strip $(MSSI_MTK_VIWIFI_SUPPORT)),yes)
    build_vilte = yes
endif

ifdef build_vilte

############################################################
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
MY_IMS_PATH :=  $(TOP)/$(MTK_ROOT)/frameworks/opt/libimsma
MY_VTS_PATH :=  $(TOP)/$(MTK_ROOT)/frameworks/opt/vtservice
MY_OMX_PATH :=  $(TOP)/$(MTK_ROOT)/frameworks/native/include/media/openmax

LOCAL_SRC_FILES:= \
        Sink.cpp             \
        Recorder.cpp               \
        Renderer.cpp                 \
        EncoderContext.cpp         \
        MediaAdapter.cpp      \
        MediaMuxer.cpp

LOCAL_C_INCLUDES:= \
        $(TOP)/frameworks/av/media/libstagefright \
        $(TOP)/frameworks/av/media \
        $(TOP)/frameworks/av/include \
        $(MY_OMX_PATH) \
        $(TOP)/frameworks/native/include/media/openmax \
        $(MY_IMS_PATH)/comutils \



#(warning $(LOCAL_SRC_FILES))

LOCAL_SHARED_LIBRARIES:= \
        libbinder                       \
        libcutils                       \
        libgui                          \
        libmedia                        \
        libstagefright                  \
        libstagefright_foundation       \
        libui                           \
        libutils \
        libcomutils  \
        libvcodec_cap \
        libvt_avsync\
        liblog\
        libmedia\
        libmedia_omx\
        libaudioclient

#for new android version check
ifeq ($(PLATFORM_VERSION),R)
LOCAL_HEADER_LIBRARIES += libmediadrm_headers
endif
LOCAL_CFLAGS += -DPLATFORM_VERSION=$(PLATFORM_VERSION)

#for debug
ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

LOCAL_CFLAGS += -Werror -Wno-error=deprecated-declarations -Wall

LOCAL_MODULE:= libsink
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk

#LOCAL_MODULE_TAGS:= optional

include $(MTK_SHARED_LIBRARY)

################################################################################

 include $(CLEAR_VARS)


LOCAL_SRC_FILES:= \
        main.cpp

LOCAL_SHARED_LIBRARIES:= \
        libbinder                       \
        libcutils                       \
        libgui                          \
        libmedia                        \
        libstagefright                  \
        libstagefright_foundation       \
        libui                           \
        libutils\
        libsink\
        liblog \
        libmedia\
        libaudioclient

LOCAL_MODULE:= sink
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk

#LOCAL_MODULE_TAGS := debug

include $(MTK_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif
