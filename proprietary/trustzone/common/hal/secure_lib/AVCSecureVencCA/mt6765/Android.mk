LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/trustzone/common/hal/secure))
include $(CLEAR_VARS)
LOCAL_MODULE := AVCSecureVencCA
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES_64 := libTEECommon liblog libion libion_mtk libutils
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64/AVCSecureVencCA.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/trustzone/common/hal/secure))
include $(CLEAR_VARS)
LOCAL_MODULE := AVCSecureVencCA
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libTEECommon liblog libion libion_mtk libutils
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/AVCSecureVencCA.so
include $(BUILD_PREBUILT)
endif
