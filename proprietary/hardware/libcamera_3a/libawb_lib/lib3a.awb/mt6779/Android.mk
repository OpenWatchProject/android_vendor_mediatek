LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libcamera_3a/libawb))
include $(CLEAR_VARS)
LOCAL_MODULE := lib3a.awb
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES_64 := libc++ lib3a.log
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64/lib3a.awb.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libcamera_3a/libawb))
include $(CLEAR_VARS)
LOCAL_MODULE := lib3a.awb
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libc++ lib3a.log
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/lib3a.awb.so
include $(BUILD_PREBUILT)
endif
