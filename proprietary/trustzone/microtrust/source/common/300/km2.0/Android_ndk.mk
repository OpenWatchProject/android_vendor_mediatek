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

# Add new source files here
LOCAL_SRC_FILES +=\
    ut_km_api.c \
    ut_km_ioctl.c \
    ut_keymaster.cpp

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/inc \
    external/openssl/include \
    system/core/include \

LOCAL_CFLAGS := -DANDROID_CHANGES -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

#Pass android version to KM TA For NDK compile
ifeq ($(ANDROID_VERSION), M)
LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=6
endif
ifeq ($(ANDROID_VERSION), N)
LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=7
endif
ifeq ($(ANDROID_VERSION), O)
LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=8
endif

ifeq ($(ANDROID_VERSION), P)
LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=9
endif

ifeq ($(ANDROID_VERSION), R)
LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=10
endif

# For NDK build
ifneq ($(ANDROID_VENDOR_PATH),)
LOCAL_MODULE := keymaster
LOCAL_CFLAGS += -I../../../android_backend/$(ANDROID_VERSION)/include
LOCAL_CFLAGS += -I../../../imsg_log/include
LOCAL_CFLAGS += -DMICROTRUST_TEE_DEBUG_BUILD
LOCAL_LDFLAGS := -L../../../imsg_log/libs/$(TARGET_ARCH_ABI) -limsg_log
LOCAL_LDFLAGS += -L../../../android_backend/$(ANDROID_VERSION)/prebuilt/$(TARGET_ARCH_ABI) -lcutils
LOCAL_LDFLAGS += -llog -Wl,--allow-shlib-undefined
else
# For Android build
LOCAL_SHARED_LIBRARIES := libimsg_log libcutils
PLATFORM_VERSION_MAJOR := $(word 1,$(subst .,$(space),$(PLATFORM_VERSION)))
LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR)
LOCAL_MODULE := keystore.$(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
endif

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

# Add new source files here
LOCAL_SRC_FILES +=\
    ut_km_ioctl.c \
	ut_km_tac.c \
    ut_kmsetkey.cpp

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/inc \
    external/openssl/include \
    system/core/include \

LOCAL_CFLAGS := -DANDROID_CHANGES -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw


# For NDK build
ifneq ($(ANDROID_VENDOR_PATH),)
LOCAL_MODULE := kmsetkey
LOCAL_CFLAGS += -I../../../android_backend/$(ANDROID_VERSION)/include
LOCAL_CFLAGS += -I../../../imsg_log/include
LOCAL_CFLAGS += -I../../../../optee_client/public
LOCAL_CFLAGS += -DMICROTRUST_TEE_DEBUG_BUILD
LOCAL_LDFLAGS := -L../../../imsg_log/libs/$(TARGET_ARCH_ABI) -limsg_log
LOCAL_LDFLAGS += -L../../../../optee_client/libs/$(TARGET_ARCH_ABI) -lTEECommon
LOCAL_LDFLAGS += -llog -Wl,--allow-shlib-undefined
else
# For Android build
LOCAL_SHARED_LIBRARIES := libimsg_log libTEECommon
LOCAL_MODULE := kmsetkey.$(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
endif

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
# For NDK build nothing
ifeq ($(ANDROID_VENDOR_PATH),)

# Add new source files here
LOCAL_SRC_FILES +=\
    ut_km_ioctl.c \
	ut_km_tac.c \
    ut_kmsetkey.cpp

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/inc \
    external/openssl/include \
    system/core/include \

LOCAL_CFLAGS := -DANDROID_CHANGES -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SHARED_LIBRARIES := libimsg_log libTEECommon
LOCAL_MODULE := kmsetkey.default
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_SHARED_LIBRARY)

endif

# For wechat function
include $(CLEAR_VARS)

# Add new source files here
LOCAL_SRC_FILES +=\
    ut_km_ioctl.c \
    ut_kmwechat.cpp \
	ut_km_wechat_tac.c

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/inc \
    external/openssl/include \
    system/core/include \

LOCAL_CFLAGS := -DANDROID_CHANGES -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw


# For NDK build
ifneq ($(ANDROID_VENDOR_PATH),)
LOCAL_MODULE := kmwechat
LOCAL_CFLAGS += -I../../../android_backend/$(ANDROID_VERSION)/include
LOCAL_CFLAGS += -I../../../imsg_log/include
LOCAL_CFLAGS += -I../../../../optee_client/public
LOCAL_CFLAGS += -DMICROTRUST_TEE_DEBUG_BUILD
LOCAL_LDFLAGS := -L../../../imsg_log/libs/$(TARGET_ARCH_ABI) -limsg_log
LOCAL_LDFLAGS += -L../../../../optee_client/libs/$(TARGET_ARCH_ABI) -lTEECommon
LOCAL_LDFLAGS += -L../../../android_backend/$(ANDROID_VERSION)/prebuilt/$(TARGET_ARCH_ABI) -lcrypto
LOCAL_LDFLAGS += -llog -Wl,--allow-shlib-undefined
else
# For Android build
LOCAL_SHARED_LIBRARIES := libimsg_log libTEECommon libcrypto
LOCAL_MODULE := kmwechat.$(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
endif

include $(BUILD_SHARED_LIBRARY)
#-----------------------------------------------------

endif
