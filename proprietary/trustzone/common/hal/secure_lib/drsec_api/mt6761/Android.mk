LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/trustzone/common/hal/secure))
include $(CLEAR_VARS)
LOCAL_MODULE := drsec_api
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .lib
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/public $(LOCAL_PATH)/trustlets/sec/common/plat/mt6761/public
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/drsec_api.lib
include $(BUILD_PREBUILT)
endif
