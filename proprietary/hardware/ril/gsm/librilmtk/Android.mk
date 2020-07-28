#
# Copyright (C) 2014 MediaTek Inc.
#
# Modification based on code covered by the below mentioned copyright
# and/or permission notice(s).
#

# Copyright 2014 The Android Open Source Project

ifneq ($(MTK_RIL_MODE), c6m_1rild)

ifneq ($(GOOGLE_RELEASE_RIL), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ril.cpp \
    ril_event.cpp \
    librilmtk-prop.cpp \
    RilSocket.cpp \
    RilSapSocket.cpp \

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libutils \
    libbinder \
    libcutils \
    libhardware_legacy \
    librilutils \
    libmtkrilutils

LOCAL_STATIC_LIBRARIES := \
    libprotobuf-c-nano-enable_malloc \

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../include

# The static library being included above is a prebuilt and was built before the
# build system was fixed for b/16853291, which causes PIC issues.
LOCAL_LDFLAGS := -Wl,-Bsymbolic

LOCAL_CFLAGS := -DMTK_RIL

LOCAL_CFLAGS += -Wno-unused-parameter

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

ifeq ($(MTK_MUX_CHANNEL), 64)
    LOCAL_CFLAGS += -DMTK_MUX_CHANNEL_64
endif

ifeq ($(MTK_EAP_SIM_AKA),yes)
    LOCAL_CFLAGS += -DMTK_EAP_SIM_AKA
endif

ifneq ($(strip $(TARGET_BUILD_VARIANT)), eng)
    LOCAL_CFLAGS += -DFATAL_ERROR_HANDLE
endif

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

LOCAL_C_INCLUDES += external/nanopb-c \
                    $(MTK_ROOT)/external/aee/binary/inc \
                    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/mtkrilutils \
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ims/include/mal_header/include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/../include

LOCAL_MODULE:= librilmtk
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_CLANG := true
LOCAL_SANITIZE := integer

include $(MTK_SHARED_LIBRARY)

# =========================================
ifneq ($(ANDROID_BIONIC_TRANSITION),)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ril.cpp \
    ril_ims.cpp

LOCAL_STATIC_LIBRARIES := \
    libutils_static \
    libcutils \
    librilutils_static \
    libprotobuf-c-nano-enable_malloc

LOCAL_CFLAGS :=

LOCAL_MODULE:= libril_static
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

include $(MTK_STATIC_LIBRARY)
endif # ANDROID_BIONIC_TRANSITION
endif # ($(GOOGLE_RELEASE_RIL),yes)

endif
