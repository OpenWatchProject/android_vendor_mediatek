LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/frameworks/opt/agps))
include $(CLEAR_VARS)
LOCAL_MODULE := mtk_agpsd
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libcutils libdl liblog libutils libandroid_net
LOCAL_INIT_RC := mtk_agpsd_p.rc
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/mtk_agpsd
include $(BUILD_PREBUILT)
endif
