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

LOCAL_SRC_FILES:= \
        Source.cpp             \
        EncoderSource.cpp    \
        VTCameraSource.cpp

LOCAL_C_INCLUDES:= \
        $(TOP)/frameworks/native/include/media/openmax \
        $(TOP)/frameworks/native/include/media/hardware \
        $(TOP)/frameworks/av/media/libstagefright \
        $(TOP)/frameworks/av/media \
        $(TOP)/frameworks/av/include \
        $(TOP)/system/core/include/utils \
        $(MY_IMS_PATH)/comutils \
        $(MY_IMS_PATH)/signal

#(warning $(LOCAL_SRC_FILES))

LOCAL_SHARED_LIBRARIES:= \
        libbinder                       \
        libcutils                       \
        libgui                          \
        libmedia                        \
        libstagefright                  \
        libstagefright_foundation       \
        libmedia_omx\
        libui                           \
        libutils     \
        libcamera_client\
        libcomutils \
        libvcodec_cap\
        liblog \
        libsignal      \
        libhardware

        
#        libgralloc_extra_sys        
#   ///*for HUAWEI hidl*/  
#for debug

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

LOCAL_CFLAGS += -Werror -Wno-error=deprecated-declarations -Wall

#Cross crypto handeling
LOCAL_CFLAGS += -DPLATFORM_VERSION=$(PLATFORM_VERSION)
ifeq ($(PLATFORM_VERSION),R)
LOCAL_HEADER_LIBRARIES += libmediadrm_headers
endif

LOCAL_MODULE:= libsource
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS:= optional

include $(MTK_SHARED_LIBRARY)
################################################################################

include $(CLEAR_VARS)

LOCAL_C_INCLUDES:= \
    $(TOP)/frameworks/av/camera/ndk/include/camera \
    $(TOP)/frameworks/av/media/ndk/include/media \
    $(TOP)/frameworks/native/libs/nativewindow/include \
    $(TOP)/frameworks/native/include/media/hardware \
    $(MY_IMS_PATH)/comutils \
    $(MY_IMS_PATH)/source

LOCAL_SRC_FILES:= \
        main.cpp

LOCAL_SHARED_LIBRARIES:= \
        libbinder                       \
        libcutils                       \
        libcamera_client	            \
        libgui                          \
        libmedia                        \
        libstagefright                  \
        libstagefright_foundation       \
        libui                           \
        libutils                        \
        libcamera2ndk                   \
        libcamera_metadata              \
        libsource                       \
        libcomutils\
        liblog\
        libmedia

LOCAL_MODULE:= source
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk
LOCAL_32_BIT_ONLY := true

LOCAL_MODULE_TAGS:= optional

include $(MTK_EXECUTABLE)

endif

#include $(call all-makefiles-under,$(LOCAL_PATH))


