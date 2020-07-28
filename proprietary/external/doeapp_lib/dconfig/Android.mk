LOCAL_PATH := $(call my-dir)

ifeq (,$(wildcard vendor/mediatek/proprietary/external/doeapp))
include $(CLEAR_VARS)
LOCAL_MODULE := dconfig
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := mtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libtinyxml2
LOCAL_SRC_FILES := $(call get-prebuilt-src-arch,arm arm64)_$(TARGET_ARCH_VARIANT)_$(TARGET_CPU_VARIANT)/dconfig
include $(BUILD_PREBUILT)
endif

ifeq (,$(wildcard vendor/mediatek/proprietary/external/doeapp))
include $(CLEAR_VARS)
LOCAL_MODULE := dconfig
LOCAL_IS_HOST_MODULE := true
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 64
LOCAL_SRC_FILES_64 := x86_64/dconfig
include $(BUILD_PREBUILT)
endif
