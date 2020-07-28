# Copyright (c) 2015-2016 MICROTRUST Incorporated
# All rights reserved
#
# This file and software is confidential and proprietary to MICROTRUST Inc.
# Unauthorized copying of this file and software is strictly prohibited.
# You MUST NOT disclose this file and software unless you get a license
# agreement from MICROTRUST Incorporated.

ifeq ($(MICROTRUST_TEE_SUPPORT),yes)

LOCAL_PATH    := $(call my-dir)

#-----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_CFLAGS := -DANDROID_CHANGES -Wall -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

PLATFORM_VERSION_MAJOR := $(word 1,$(subst .,$(space),$(PLATFORM_VERSION)))
LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR)

LOCAL_MODULE := gatekeeper
LOCAL_CFLAGS += -I../../android_backend/$(ANDROID_VERSION)/include
LOCAL_CFLAGS += -I../../imsg_log/include
LOCAL_CFLAGS += -I../../../optee_client/public
LOCAL_CFLAGS += -DMICROTRUST_TEE_DEBUG_BUILD
LOCAL_LDFLAGS += -L ../../imsg_log/libs/$(TARGET_ARCH_ABI) -limsg_log
LOCAL_LDFLAGS += -L../../../optee_client/libs/$(TARGET_ARCH_ABI) -lTEECommon
LOCAL_LDFLAGS += -llog -Wl,--allow-shlib-undefined

LOCAL_SRC_FILES +=\
    ut_gatekeeper.cpp

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/include \
    system/core/include \
    hardware/libhardware/include \

include $(BUILD_SHARED_LIBRARY)

#-----------------------------------------------------
include $(LOCAL_PATH)/tests/Android_NDK.mk
#-----------------------------------------------------

endif
