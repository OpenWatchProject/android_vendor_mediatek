LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/hardware/dpframework_prot))
include $(CLEAR_VARS)
LOCAL_MODULE := libdpframework_prot
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_UNINSTALLABLE_MODULE := true
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include/common $(LOCAL_PATH)/include/mt6735/D2
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm/libdpframework_prot.a
include $(BUILD_PREBUILT)
endif
