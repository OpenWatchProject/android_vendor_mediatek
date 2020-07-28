LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/libvcodec_v2))
include $(CLEAR_VARS)
LOCAL_MODULE := vpud
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils libvpud_vcodec libvcodec_utility liblog libion
LOCAL_INIT_RC := vpud.rc
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/vpud
include $(BUILD_PREBUILT)
endif
