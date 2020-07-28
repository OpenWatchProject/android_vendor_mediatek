LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libcamera_core))
include $(CLEAR_VARS)
LOCAL_MODULE := liblog3a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .a
LOCAL_PROPRIETARY_MODULE := true
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64/liblog3a.a
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libcamera_core))
include $(CLEAR_VARS)
LOCAL_MODULE := liblog3a
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .a
LOCAL_PROPRIETARY_MODULE := true
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/liblog3a.a
include $(BUILD_PREBUILT)
endif
