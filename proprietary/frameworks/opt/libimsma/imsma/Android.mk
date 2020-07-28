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

MY_IMS_PATH := $(MTK_PATH_SOURCE)/frameworks/opt/libimsma

LOCAL_SRC_FILES := \
    ImsMediaReceiver.cpp  \
    ImsMediaSender.cpp \
    ImsMa.cpp \
    ImsMaDebugUtil.cpp

LOCAL_C_INCLUDES := \
    frameworks/native/include/media/hardware \
    frameworks/native/include/gui \
    frameworks/av/media/libstagefright \
    frameworks/av/media \
    $(MY_IMS_PATH)/sink \
    $(MY_IMS_PATH)/source \
    $(MY_IMS_PATH)/comutils \
    $(MY_IMS_PATH)/rtp/include \
    $(MY_IMS_PATH)/signal \
    system/netd/include \


#(warning $(LOCAL_SRC_FILES))

LOCAL_SHARED_LIBRARIES := \
    libbinder                       \
    libcutils                       \
    libgui                          \
    libmedia                        \
    libstagefright                  \
    libstagefright_foundation       \
    libui                           \
    libutils                        \
    libsink                         \
    libsource                       \
    libvcodec_cap                   \
    libimsma_rtp                    \
    libcomutils\
    liblog \
    libsignal	\
    libnetd_client


#for debug

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif
LOCAL_CFLAGS += -Werror -Wno-error=deprecated-declarations -Wall

LOCAL_MODULE := libimsma
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk

#LOCAL_MODULE_TAGS := optional

include $(MTK_SHARED_LIBRARY)

################################################################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    VTSample.cpp

LOCAL_C_INCLUDES := \
    frameworks/native/include/media/hardware \
    frameworks/native/include/gui \
    frameworks/av/media/libstagefright \
    frameworks/av/media \
    $(MY_IMS_PATH)/sink \
    $(MY_IMS_PATH)/source \
    $(MY_IMS_PATH)/comutils \
		$(MY_IMS_PATH)/rtp/include
		
LOCAL_SHARED_LIBRARIES := \
    libbinder \
    libcutils \
    libgui    \
    libutils  \
    libimsma  \
    libvcodec_cap \
    libimsma_rtp  \
    libcomutils\
    liblog\
    libmedia

LOCAL_MODULE := MATest
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk

#LOCAL_MODULE_TAGS := debug

include $(MTK_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif