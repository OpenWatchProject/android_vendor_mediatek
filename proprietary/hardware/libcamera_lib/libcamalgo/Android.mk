#binary precheck in makefile
ifneq ($(MTK_EMULATOR_SUPPORT),yes)

LOCAL_PATH := $(call my-dir)

ifeq ($(wildcard $(MTK_PATH_SOURCE)/hardware/libcamera),)

ifneq ($(filter $(TARGET_BUILD_VARIANT),eng userdebug),)
libcamalgo_src_path := $(MTK_PLATFORM_DIR)/prebuilt_eng
else
libcamalgo_src_path := $(MTK_PLATFORM_DIR)/prebuilt_user
endif

include $(CLEAR_VARS)
PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
ifeq ($(PLATFORM), $(filter $(PLATFORM), mt6755 mt6570 mt6572 mt6580 mt6582 mt6735 mt6752 mt8163 mt8167 mt6739 mt8173 mt6761))
LOCAL_SHARED_LIBRARIES := libcamdrv
else
LOCAL_SHARED_LIBRARIES := libcamdrv_imem
endif
ifeq ($(PLATFORM), $(filter $(PLATFORM), mt6757 mt6799))
LOCAL_SHARED_LIBRARIES += libmtkcam_tsf
endif

LOCAL_MODULE := libcamalgo
LOCAL_SRC_FILES_64 := $(libcamalgo_src_path)/arm64/libcamalgo.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_SHARED_LIBRARIES += liblog libcutils libmtk_drvb libEGL libGLESv2 libutils libbinder libgralloc_extra
LOCAL_MULTILIB := 64
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z)
ifeq ($(PLATFORM), $(filter $(PLATFORM), mt6755 mt6570 mt6572 mt6580 mt6582 mt6735 mt6752 mt8163 mt8167 mt6739 mt8173 mt6761))
LOCAL_SHARED_LIBRARIES := libcamdrv
else
LOCAL_SHARED_LIBRARIES := libcamdrv_imem
endif
ifeq ($(PLATFORM), $(filter $(PLATFORM), mt6757 mt6799))
LOCAL_SHARED_LIBRARIES += libmtkcam_tsf
endif

LOCAL_MODULE := libcamalgo
LOCAL_SRC_FILES_32 := $(libcamalgo_src_path)/arm/libcamalgo.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_SHARED_LIBRARIES += liblog libcutils libmtk_drvb libEGL libGLESv2 libutils libbinder libgralloc_extra
LOCAL_MULTILIB := 32
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_PREBUILT)
endif
endif
