LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/trustzone/common/hal/secure))
include $(CLEAR_VARS)
LOCAL_MODULE := libtlcWidevineModularDrm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES_64 := libTEECommon liblog libutils libdl libcutils libion_mtk libion
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64/libtlcWidevineModularDrm.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/trustzone/common/hal/secure))
include $(CLEAR_VARS)
LOCAL_MODULE := libtlcWidevineModularDrm
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libTEECommon liblog libutils libdl libcutils libion_mtk libion
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/libtlcWidevineModularDrm.so
include $(BUILD_PREBUILT)
endif
