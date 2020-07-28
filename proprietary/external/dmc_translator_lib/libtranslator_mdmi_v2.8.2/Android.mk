LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/external/dmc_translator))
include $(CLEAR_VARS)
LOCAL_MODULE := libtranslator_mdmi_v2.8.2
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES_64 := libasn1c_core libasn1c_mdmi libtranslator_utils libcutils libutils libhidlbase libhidltransport libhwbinder
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := arm64_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libtranslator_mdmi_v2.8.2.so
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/external/dmc_translator))
include $(CLEAR_VARS)
LOCAL_MODULE := libtranslator_mdmi_v2.8.2
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_SUFFIX := .so
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libasn1c_core libasn1c_mdmi libtranslator_utils libcutils libutils libhidlbase libhidltransport libhwbinder
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES_32 := arm_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/libtranslator_mdmi_v2.8.2.so
include $(BUILD_PREBUILT)
endif
