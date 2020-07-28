LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libvcodec))
include $(CLEAR_VARS)
LOCAL_MODULE := libvcodecdrv
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils libdl liblog libvcodec_utility libvcodec_oal libmp4enc_sa.ca7 libvp8dec_sa.ca7 libvc1dec_sa.ca7 libvp8enc_sa.ca7 libmtk_drvb libion
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/libvcodecdrv.so
include $(BUILD_PREBUILT)
endif
