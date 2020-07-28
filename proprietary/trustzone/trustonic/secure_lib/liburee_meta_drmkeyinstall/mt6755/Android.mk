LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/trustzone/trustonic/secure))
include $(CLEAR_VARS)
LOCAL_MODULE := liburee_meta_drmkeyinstall
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES_64 := libcutils libnetutils libc liblog libMcClient libsec_mem libion libion_mtk
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64/liburee_meta_drmkeyinstall.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/trustzone/trustonic/secure))
include $(CLEAR_VARS)
LOCAL_MODULE := liburee_meta_drmkeyinstall
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SHARED_LIBRARIES := libcutils libnetutils libc liblog libMcClient libsec_mem libion libion_mtk
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/liburee_meta_drmkeyinstall.so
include $(BUILD_PREBUILT)
endif
