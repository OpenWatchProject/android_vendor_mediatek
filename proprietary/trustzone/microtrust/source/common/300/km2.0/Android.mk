# Copyright (c) 2015-2016 MICROTRUST Incorporated
# All rights reserved
#
# This file and software is confidential and proprietary to MICROTRUST Inc.
# Unauthorized copying of this file and software is strictly prohibited.
# You MUST NOT disclose this file and software unless you get a license
# agreement from MICROTRUST Incorporated.

ifeq ($(MICROTRUST_TEE_SUPPORT),yes)

ifneq ($(strip $(KEYMASTER_VERSION)), 4.0)

$(info "MICROTRUST_TEE_KM_VERSION is 3.0")

LOCAL_PATH    := $(call my-dir)

#-----------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := keystore.$(TARGET_BOARD_PLATFORM)
# Add new source files here
LOCAL_SRC_FILES +=\
    ut_km_api.c \
    ut_km_ioctl.c \
    ut_keymaster.cpp \

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/inc \
    external/openssl/include \
    system/core/include \
    hardware/libhardware/include \

LOCAL_CFLAGS := -DANDROID_CHANGES -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

#Pass android version to KM TA
PLATFORM_VERSION_MAJOR := $(word 1,$(subst .,$(space),$(PLATFORM_VERSION)))
#PLATFORM_VERSION_MAJOR := 10 ### wait for build/core/version_defaults.mk
$(info Microtrust @ Android version: $(PLATFORM_VERSION_MAJOR))

LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# For Android build
LOCAL_SHARED_LIBRARIES := liblog libimsg_log libcutils


include $(BUILD_SHARED_LIBRARY)

#-----------------------------------------------------
include $(CLEAR_VARS)

LOCAL_MODULE := kmsetkey.$(TARGET_BOARD_PLATFORM)
# Add new source files here
LOCAL_SRC_FILES +=\
    ut_km_ioctl.c \
	ut_km_tac.c \
    ut_kmsetkey.cpp \

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/inc \
    external/openssl/include \
    system/core/include \
    hardware/libhardware/include \

LOCAL_CFLAGS := -DANDROID_CHANGES -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# For Android build
LOCAL_SHARED_LIBRARIES := liblog libimsg_log libTEECommon


include $(BUILD_SHARED_LIBRARY)

#-----------------------------------------------------
# For wechat function
include $(CLEAR_VARS)

LOCAL_MODULE := kmwechat.$(TARGET_BOARD_PLATFORM)
# Add new source files here
LOCAL_SRC_FILES +=\
    ut_km_ioctl.c \
    ut_kmwechat.cpp \
    ut_km_wechat_tac.c \

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/inc \
    external/openssl/include \
    system/core/include \
    hardware/libhardware/include \

LOCAL_CFLAGS := -DANDROID_CHANGES -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# For Android build
LOCAL_SHARED_LIBRARIES := liblog libimsg_log libTEECommon


include $(BUILD_SHARED_LIBRARY)

#-----------------------------------------------------
##### for kmsetkey (default) ###########

include $(CLEAR_VARS)

LOCAL_MODULE := kmsetkey.default
# Add new source files here
LOCAL_SRC_FILES +=\
    ut_km_ioctl.c \
	ut_km_tac.c \
    ut_kmsetkey.cpp \

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/inc \
    external/openssl/include \
    system/core/include \
    hardware/libhardware/include \

LOCAL_CFLAGS := -DANDROID_CHANGES -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

# For Android build
LOCAL_SHARED_LIBRARIES := liblog libimsg_log libTEECommon


include $(BUILD_SHARED_LIBRARY)

#-----------------------------------------------------
endif

endif
