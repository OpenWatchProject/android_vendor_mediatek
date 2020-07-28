#binary pre checkin makefile
ifneq ($(MTK_EMULATOR_SUPPORT),yes)

LOCAL_PATH := $(call my-dir)

ifeq ($(wildcard $(MTK_PATH_SOURCE)/hardware/libcamera),)

ifneq ($(filter $(TARGET_BUILD_VARIANT),eng userdebug),)
libn3d3a_src_path := $(MTK_PLATFORM_DIR)/prebuilt_eng
else
libn3d3a_src_path := $(MTK_PLATFORM_DIR)/prebuilt_user
endif

include $(CLEAR_VARS)
LOCAL_MODULE := libn3d3a
LOCAL_SRC_FILES_64 := $(libn3d3a_src_path)/arm64/libn3d3a.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_SHARED_LIBRARIES := libcutils liblog
LOCAL_MULTILIB := 64
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libn3d3a
LOCAL_SRC_FILES_32 := $(libn3d3a_src_path)/arm/libn3d3a.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_SHARED_LIBRARIES := libcutils liblog
LOCAL_MULTILIB := 32
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_PREBUILT)

endif
endif
