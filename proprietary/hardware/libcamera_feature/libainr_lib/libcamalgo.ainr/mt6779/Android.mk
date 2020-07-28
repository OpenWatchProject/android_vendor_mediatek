LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libcamera_feature/libainr))
include $(CLEAR_VARS)
LOCAL_MODULE := libcamalgo.ainr
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES_64 := libmtk_drvb liblog libcutils libutils libcamalgo.utility libvpu libion libion_mtk
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64/libcamalgo.ainr.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libcamera_feature/libainr))
include $(CLEAR_VARS)
LOCAL_MODULE := libcamalgo.ainr
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libmtk_drvb liblog libcutils libutils libcamalgo.utility libvpu libion libion_mtk
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/libcamalgo.ainr.so
include $(BUILD_PREBUILT)
endif
