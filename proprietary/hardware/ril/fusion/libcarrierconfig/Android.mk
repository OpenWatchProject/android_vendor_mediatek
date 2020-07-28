ifeq ($(MTK_RIL_MODE), c6m_1rild)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_VENDOR_MODULE := true

LOCAL_SRC_FILES:= \
    carrierconfig.c \

LOCAL_SHARED_LIBRARIES:= \
  libmtkcutils

ifneq ($(wildcard vendor/mediatek/internal/dims_enable),)
    LOCAL_CFLAGS += -D__DIMS_INTERNAL__
endif

LOCAL_C_INCLUDES = $(LOCAL_PATH)/../../include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../mtk-ril/framework/include

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/../include

LOCAL_MODULE:= libcarrierconfig
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_CLANG := true
LOCAL_SANITIZE := integer

include $(MTK_SHARED_LIBRARY)

endif
