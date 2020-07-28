LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libcamera_drv))
include $(CLEAR_VARS)
LOCAL_MODULE := libcam.halsensor.hwintegration
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES_64 := liblog
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include/libhw_integration/mt6768
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64/libcam.halsensor.hwintegration.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libcamera_drv))
include $(CLEAR_VARS)
LOCAL_MODULE := libcam.halsensor.hwintegration
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include/libhw_integration/mt6768
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/libcam.halsensor.hwintegration.so
include $(BUILD_PREBUILT)
endif
