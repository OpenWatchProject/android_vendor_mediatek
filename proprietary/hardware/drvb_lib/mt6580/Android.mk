LOCAL_PATH := $(call my-dir)
ifndef DRVB_VERSION
DRVB_VERSION := release
endif

$(info DRVB_VERSION=$(DRVB_VERSION))

ifeq ($(DRVB_VERSION), release)

ifeq ($(strip $(BUILD_WITH_NDK)),true)

#Use NDK Build
#Installed to /vendor
include $(CLEAR_VARS)
LOCAL_MODULE := libmtk_drvb
LOCAL_SRC_FILES := arm/libmtk_drvb.so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_SHARED_LIBRARIES := libcutils libutils
LOCAL_MULTILIB := both
include $(PREBUILT_SHARED_LIBRARY)

else

#Installed to /vendor
include $(CLEAR_VARS)
LOCAL_MODULE := libmtk_drvb
LOCAL_SRC_FILES_32 := arm/libmtk_drvb.so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_SHARED_LIBRARIES := libcutils libutils
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)

endif #ifeq ($(strip $(BUILD_WITH_NDK)),true)

endif #ifeq ($(DRVB_VERSION), release)
