
# we provide the libraries with debug symbols. The symbols are stripped
# by the build system for the release version. Using generic build, as
# lib APIs are not platform dependent


LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_BUILD_VARIANT),eng)
  LOCAL_INSTALL_MODE ?= Debug
else
  LOCAL_INSTALL_MODE ?= Release
endif

#-------------------------------------------------------------------------------
# Module: "TeeClient" java library (TeeClient.jar)
#-------------------------------------------------------------------------------
include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := TeeClient:Bin/java/$(LOCAL_INSTALL_MODE)/TeeClient.jar
include $(BUILD_MULTI_PREBUILT)
