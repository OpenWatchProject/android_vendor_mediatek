#
# Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
# All Rights Reserved.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libgf_ca
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_CFLAGS += -D_POSIX_THREADS
LOCAL_CFLAGS += -DTBASE_API_LEVEL=5
LOCAL_CFLAGS += -DLOG_ANDROID -UNDEBUG

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../public \
    $(PRODUCT_OUT)/trustzone/trustlet/gf_ta/Public \
    $(TEE_TRUSTLET_OUTPUT_PATH)/gf_ta/Public \
    $(LOCAL_PATH)/../../../../../../bsp/common/400b/t-sdk/TlcSdk/Out/Public/GP \
    $(LOCAL_PATH)/../../../../../../bsp/common/400b/t-sdk/TlcSdk/Out/Public \
 #   $(LOCAL_PATH)/../../../trustzone/t-sdk/TlcSdk/Out/Public/GP \
 #   $(LOCAL_PATH)/../../../trustzone/t-sdk/TlcSdk/Out/Public \
 #   $(LOCAL_PATH)/../../../trustzone/t-sdk/TlSdk/Out/Public/MobiCore/inc

ifeq ($(PRODUCT_OUT),)
    HAVE_UUID_FILE := $(shell test ! -f $(TEE_TRUSTLET_OUTPUT_PATH)/gf_ta/Public/gf_ta_uuid.h && echo no)
    ifeq ($(HAVE_UUID_FILE), no)
        $(shell mkdir -p $(TEE_TRUSTLET_OUTPUT_PATH)/gf_ta/Public)
        $(shell cp -f $(LOCAL_PATH)/../gf_ta_uuid.h $(TEE_TRUSTLET_OUTPUT_PATH)/gf_ta/Public/gf_ta_uuid.h)
    endif
else
    HAVE_UUID_FILE := $(shell test ! -f $(PRODUCT_OUT)/trustzone/trustlet/gf_ta/Public/gf_ta_uuid.h && echo no)
    ifeq ($(HAVE_UUID_FILE), no)
        $(shell mkdir -p $(PRODUCT_OUT)/trustzone/trustlet/gf_ta/Public)
        $(shell cp -f $(LOCAL_PATH)/../gf_ta_uuid.h $(PRODUCT_OUT)/trustzone/trustlet/gf_ta/Public/gf_ta_uuid.h)
    endif
endif

LOCAL_SRC_FILES := trustonic/gf_ca_entry.c

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
    libMcClient \
    liblog

LOCAL_MULTILIB := both
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
include $(MTK_SHARED_LIBRARY)

$(call import-add-path,/)
$(call import-module,$(COMP_PATH_MobiCoreDriverLib))
