# Copyright 2006 The Android Open Source Project

ifneq ($(MTK_RIL_MODE), c6m_1rild)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    rild.c


LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libdl \
    liblog \
    librilproxy \
    libsysenv

LOCAL_HEADER_LIBRARIES := libcutils_headers

# Temporary hack for broken vendor RILs.
LOCAL_WHOLE_STATIC_LIBRARIES := \
    librilproxyutils_static

LOCAL_CFLAGS := -DRIL_SHLIB

ifneq ($(MTK_NUM_MODEM_PROTOCOL),1)
    LOCAL_CFLAGS += -DANDROID_MULTI_SIM
endif

ifeq ($(MTK_NUM_MODEM_PROTOCOL), 2)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

ifeq ($(MTK_NUM_MODEM_PROTOCOL), 3)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_3
endif

ifeq ($(MTK_NUM_MODEM_PROTOCOL), 4)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_4
endif

LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE:= rilproxy
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := rilproxy.rc

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/librilproxy \
    $(MTK_PATH_SOURCE)/external/libsysenv

include $(MTK_EXECUTABLE)

# For radiooptions binary
# =======================
include $(CLEAR_VARS)

#LOCAL_SRC_FILES:= \
#    radiooptions.c

#LOCAL_SHARED_LIBRARIES := \
#    liblog \
#    libcutils \

#LOCAL_CFLAGS := \

#LOCAL_MODULE:= radiooptions
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#LOCAL_MODULE_TAGS := debug

#include $(MTK_EXECUTABLE)

endif
