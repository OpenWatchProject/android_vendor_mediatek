# Copyright (c) 2015-2016 MICROTRUST Incorporated
# All rights reserved
#
# This file and software is confidential and proprietary to MICROTRUST Inc.
# Unauthorized copying of this file and software is strictly prohibited.
# You MUST NOT disclose this file and software unless you get a license
# agreement from MICROTRUST Incorporated.

$(info ut_gatekeeper MICROTRUST_TEE_SUPPORT=$(MICROTRUST_TEE_SUPPORT))

ifeq ($(MICROTRUST_TEE_SUPPORT), yes)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := gatekeeper.$(TARGET_BOARD_PLATFORM)
LOCAL_CFLAGS := -DANDROID_CHANGES -Wall -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

PLATFORM_VERSION_MAJOR := $(word 1,$(subst .,$(space),$(PLATFORM_VERSION)))
LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR)
ifneq ($(PLATFORM_VERSION_MAJOR), 6)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
endif

# Add new source files here
LOCAL_SRC_FILES +=\
    ut_gatekeeper.cpp \

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/include \
    system/core/include \
    hardware/libhardware/include \

# LOCAL_CFLAGS += -DGATEKEEPER_DATA_SAVE_IN_RAM
LOCAL_SHARED_LIBRARIES := libimsg_log liblog libTEECommon

include $(BUILD_SHARED_LIBRARY)

endif
