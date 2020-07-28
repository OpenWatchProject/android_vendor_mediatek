LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  DrmCtaPlugIn_test.cpp \

LOCAL_C_INCLUDES := \
  bionic \
  external/gtest/include \
  frameworks/av/include \
  frameworks/native/include \
  $(MTK_PATH_SOURCE)/frameworks/av/libdrm/common/mtkutil/include \
  $(MTK_PATH_SOURCE)/frameworks/av/libdrm/common/mtkutil/cta \
  $(MTK_PATH_SOURCE)/frameworks/av/libdrm/cta/drmctaplugin/include \
  frameworks/av/drm/libdrmframework/include \
  frameworks/av/drm/libdrmframework/plugins/common/include \
  frameworks/av/drm/libdrmframework/plugins/common/util/include \

LOCAL_STATIC_LIBRARIES := \
  libgtest \
  libdrmframeworkcommon

LOCAL_SHARED_LIBRARIES := \
  libcrypto \
  libcutils \
  libdl \
  liblog \
  libstagefright_foundation \
  libutils \
  libssl \
  libdrmmtkutil

# Needed to use gMock 1.7.0 on Android
LOCAL_CFLAGS += \
  -DGTEST_HAS_TR1_TUPLE \
  -DGTEST_USE_OWN_TR1_TUPLE \



LOCAL_MODULE := libdrmctaplugin_test
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE_TAGS := tests

include $(MTK_EXECUTABLE)
