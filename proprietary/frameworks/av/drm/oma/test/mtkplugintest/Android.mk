
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

base := $(TOP)/frameworks/av

LOCAL_SRC_FILES:= \
    mtkplugintest.cpp

LOCAL_MODULE := mtkplugintest
LOCAL_MODULE_OWNER := mtk

LOCAL_C_INCLUDES += \
    $(base)/drm/libdrmframework/include \
    $(base)/drm/libdrmframework/plugins/common/include \
    $(base)/drm/libdrmframework/plugins/common/util/include \
    $(base)/include \
    $(TOP)/frameworks/native/include \
    $(LOCAL_PATH)/include \
    external/libxml2/include \
    $(MTK_PATH_SOURCE)/frameworks/av/drm/include \
    $(MTK_PATH_SOURCE)/frameworks/av/libdrm/common/mtkutil/include \
    $(MTK_PATH_SOURCE)/frameworks/av/libdrm/oma/mtkplugin/include

LOCAL_SHARED_LIBRARIES := \
    libandroidicu \
    libutils \
    libcutils \
    libdl \
    libcrypto \
    libssl \
    libdrmframework \
    libdrmmtkwhitelist \
    libdrmmtkutil

LOCAL_STATIC_LIBRARIES := \
    libdrmframeworkcommon \
    libxml2

LOCAL_PRELINK_MODULE := false

LOCAL_MODULE_TAGS := optional

include $(MTK_EXECUTABLE)
