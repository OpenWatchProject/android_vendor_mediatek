# Copyright (c) 2015-2016 MICROTRUST Incorporated
# All rights reserved
#
# This file and software is confidential and proprietary to MICROTRUST Inc.
# Unauthorized copying of this file and software is strictly prohibited.
# You MUST NOT disclose this file and software unless you get a license
# agreement from MICROTRUST Incorporated.

ifeq ($(MICROTRUST_TEE_SUPPORT),yes)

LOCAL_PATH := $(call my-dir)

#-----------------------------------------------------
include $(CLEAR_VARS)

FILE_BEGIN := ueventd.
FILE_END0 := .rc
FILE_END1 := .emmc.rc
FILE_END2 := .ufs.rc
FILE_END3 := .nand.rc

MTK_PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )
ANDROID_SOURCE_PATH := device/mediatek/$(MTK_PLATFORM)/

FILE_NAME0 := $(ANDROID_SOURCE_PATH)$(FILE_BEGIN)$(MTK_PLATFORM)$(FILE_END0)
FILE_NAME1 := $(ANDROID_SOURCE_PATH)$(FILE_BEGIN)$(MTK_PLATFORM)$(FILE_END1)
FILE_NAME2 := $(ANDROID_SOURCE_PATH)$(FILE_BEGIN)$(MTK_PLATFORM)$(FILE_END2)
FILE_NAME3 := $(ANDROID_SOURCE_PATH)$(FILE_BEGIN)$(MTK_PLATFORM)$(FILE_END3)
ifeq ($(shell test -f $(FILE_NAME0) && echo yes), yes)
PROINFO_TMP := $(shell awk '/proinfo/ {print $$1}' $(FILE_NAME0))
PROINFO_PATH := "\"$(PROINFO_TMP)\""
else ifeq ($(shell test -f $(FILE_NAME1) && echo yes), yes)
PROINFO_TMP := $(shell awk '/proinfo/ {print $$1}' $(FILE_NAME1))
PROINFO_PATH := "\"$(PROINFO_TMP)\""
else ifeq ($(shell test -f $(FILE_NAME2) && echo yes), yes)
PROINFO_TMP := $(shell awk '/proinfo/ {print $$1}' $(FILE_NAME2))
PROINFO_PATH := "\"$(PROINFO_TMP)\""
else ifeq ($(shell test -f $(FILE_NAME3) && echo yes), yes)
PROINFO_TMP := $(shell awk '/proinfo/ {print $$1}' $(FILE_NAME3))
PROINFO_PATH := "\"$(PROINFO_TMP)\""
else
$(error "[[[***get partition info failed, please handle it***]]]")
endif

$(info $(PROINFO_TMP))
$(info $(PROINFO_PATH))
LOCAL_MODULE := teei_daemon
LOCAL_SRC_FILES := main.c \

PLATFORM_VERSION_MAJOR := $(word 1,$(subst .,$(space),$(PLATFORM_VERSION)))
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_CFLAGS := -Werror -DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR)
LOCAL_LDFLAGS := -llog
LOCAL_CFLAGS += -DPRO_NODE=$(PROINFO_PATH)

ifeq ($(strip $(MTK_SEC_VIDEO_PATH_SUPPORT)), yes)
LOCAL_CFLAGS += -DMTK_DRM_SUPPORT
endif
ifeq ($(strip $(MTK_CAM_SECURITY_SUPPORT)), yes)
LOCAL_CFLAGS += -DMTK_CAM_SUPPORT
endif

LOCAL_INIT_RC := microtrust.rc
# Support multiple FP TA
#LOCAL_CFLAGS += -DISEE_MULTIPLE_FP_TA

LOCAL_C_INCLUDES +=\
    system/core/include \

# For Android build
LOCAL_SHARED_LIBRARIES := libimsg_log libcutils
# LOCAL_CFLAGS += -DMICROTRUST_TEE_DEBUG_BUILD
LOCAL_CFLAGS += -DSYSTEM_THH_PATH='"/vendor/thh/"'

include $(BUILD_EXECUTABLE)
#-----------------------------------------------------

endif
