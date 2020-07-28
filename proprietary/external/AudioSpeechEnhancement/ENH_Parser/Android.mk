LOCAL_PATH := $(my-dir)

############################################################################################################### Vendor
include $(CLEAR_VARS)
LOCAL_C_INCLUDES :=  $(LOCAL_PATH)
LOCAL_C_INCLUDES += \
  external/libxml2/include \
  external/icu/libandroidicu/include \
  $(JNI_H_INCLUDE) \
  $(MTK_PATH_SOURCE)/external/AudioParamParser/include \
  $(MTK_PATH_SOURCE)/external/aurisys/interface \
  $(MTK_PATH_SOURCE)/external/AudioParamParser \
  $(TOPDIR)vendor/mediatek/proprietary/hardware/audio/common/V3/include \
  $(TOPDIR)frameworks/av/media/libmedia \
  $(TOPDIR)frameworks/av/include \
	$(TOPDIR)hardware/libhardware/include \

# incall handfree DMNR
ifeq ($(MTK_INCALL_HANDSFREE_DMNR),yes)
  LOCAL_CFLAGS += -DMTK_INCALL_HANDSFREE_DMNR
endif

LOCAL_SRC_FILES := \
  SpeechParser.cpp

LOCAL_SHARED_LIBRARIES := \
  libcutils \
  liblog \
  libutils \
  libaudioutils \
  libmedia_helper

LOCAL_HEADER_LIBRARIES += libaudioclient_headers
LOCAL_MODULE := libspeechparser_vendor
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := both
LOCAL_ARM_MODE := arm

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
include $(MTK_SHARED_LIBRARY)
