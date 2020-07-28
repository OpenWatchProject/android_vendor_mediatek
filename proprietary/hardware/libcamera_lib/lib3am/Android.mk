#binary precheck in makefile
ifneq ($(MTK_EMULATOR_SUPPORT),yes)

LOCAL_PATH := $(call my-dir)

ifeq ($(wildcard $(MTK_PATH_SOURCE)/hardware/libcamera),)

ifneq ($(filter $(TARGET_BUILD_VARIANT),eng userdebug),)
lib3a_src_path := $(MTK_PLATFORM_DIR)/prebuilt_eng
else
lib3a_src_path := $(MTK_PLATFORM_DIR)/prebuilt_user
endif

include $(CLEAR_VARS)
LOCAL_MODULE := lib3am
LOCAL_SRC_FILES_64 := $(lib3a_src_path)/arm64/lib3am.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_SHARED_LIBRARIES := libcutils liblog lib3a.log
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MULTILIB := 64
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := lib3am
LOCAL_SRC_FILES_32 := $(lib3a_src_path)/arm/lib3am.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_SHARED_LIBRARIES := libcutils liblog lib3a.log
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)
endif
endif
