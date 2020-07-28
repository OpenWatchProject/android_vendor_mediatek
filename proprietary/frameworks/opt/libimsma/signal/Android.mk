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
    ImsSignal.cpp

LOCAL_C_INCLUDES := \
    $(MY_IMS_PATH)/signal \
    $(TOP)/frameworks/av/media/libstagefright \
    $(TOP)/system/core/include/utils

#(warning $(LOCAL_SRC_FILES))

LOCAL_SHARED_LIBRARIES := \
    libcutils                       \
    libutils \
    libstagefright_foundation \
    liblog

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif
LOCAL_CFLAGS += -Werror -Wno-error=deprecated-declarations -Wall

LOCAL_MODULE := libsignal
#LOCAL_PROPRIETARY_MODULE := true
#LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := optional

include $(MTK_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))

endif

