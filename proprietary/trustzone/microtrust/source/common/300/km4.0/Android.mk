# Copyright (c) 2018-2019 MICROTRUST Incorporated
# All rights reserved
#
# This file and software is confidential and proprietary to MICROTRUST Inc.
# Unauthorized copying of this file and software is strictly prohibited.
# You MUST NOT disclose this file and software unless you get a license
# agreement from MICROTRUST Incorporated.


ifeq ($(MICROTRUST_TEE_SUPPORT),yes)

ifeq ($(strip $(KEYMASTER_VERSION)), 4.0)

$(info "MICROTRUST_TEE_KM_VERSION is 4.0")
#keymaseter service
#-----------------------------------------------------------------------------
LOCAL_PATH    := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := android.hardware.keymaster@4.0-service.beanpod

LOCAL_INIT_RC := android.hardware.keymaster@4.0-service.beanpod.rc

LOCAL_CFLAGS := -DANDROID_CHANGES
LOCAL_MODULE_TAGS := optional

#make bin to vendor/bin
LOCAL_PROPRIETARY_MODULE := true

#make bin to vendor/bin/hw
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_OWNER := mtk

LOCAL_SRC_FILES +=\
    beanpod_keymaster_ipc.cpp \
    BeanpodKeymaster4Device.cpp \
    BeanpodKeymaster.cpp \
    service.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := liblog libcutils libdl libbase libutils \
						libhardware libhidlbase libhidltransport \
						libkeymaster_messages libkeymaster4 \
						android.hardware.keymaster@4.0 libTEECommon
#-----------------------------------------------------------------------------
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

# Add new source files here
LOCAL_SRC_FILES +=\
    ut_km_ioctl.cpp \
    ut_km_tac.cpp \
    ut_kmsetkey.cpp

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/include \
    external/openssl/include \
	hardware/libhardware/include \
    system/core/include

LOCAL_CFLAGS := -DANDROID_CHANGES -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

PLATFORM_VERSION_MAJOR := $(word 1,$(subst .,$(space),$(PLATFORM_VERSION)))
LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR)

LOCAL_SHARED_LIBRARIES := libimsg_log libTEECommon liblog
LOCAL_MODULE := kmsetkey.$(TARGET_BOARD_PLATFORM)
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

# Add new source files here
LOCAL_SRC_FILES +=\
    ut_km_ioctl.cpp \
    ut_km_tac.cpp \
    ut_kmsetkey.cpp

LOCAL_C_INCLUDES +=\
    $(LOCAL_PATH)/include \
    external/openssl/include \
	hardware/libhardware/include \
    system/core/include

LOCAL_CFLAGS := -DANDROID_CHANGES -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := hw

PLATFORM_VERSION_MAJOR := $(word 1,$(subst .,$(space),$(PLATFORM_VERSION)))
LOCAL_CFLAGS+=-DPLATFORM_VERSION_MAJOR=$(PLATFORM_VERSION_MAJOR)
LOCAL_SHARED_LIBRARIES := libimsg_log libTEECommon liblog
LOCAL_MODULE := kmsetkey.default
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := bp_kmsetkey_ca
LOCAL_INIT_RC := microtrust.bp_kmsetkey_ca.rc

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	    kmsetkey_ca.cpp

LOCAL_C_INCLUDES:= \
	    $(LOCAL_PATH)/include

LOCAL_SHARED_LIBRARIES := \
	    libdl \
		libhardware \
		libcutils \
		liblog \
		libutils

LOCAL_CFLAGS := -Wall -Werror -g
LOCAL_LDFLAGS := -Wl,--unresolved-symbols=ignore-all

include $(BUILD_EXECUTABLE)

endif
endif
