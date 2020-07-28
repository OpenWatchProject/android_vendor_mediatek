# Copyright 2006 The Android Open Source Project

ifeq ($(MTK_RIL_MODE), c6m_1rild)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_VENDOR_MODULE := true

LOCAL_SRC_FILES:= \
    ril.cpp \
    ril_event.cpp\
    RilSapSocket.cpp \
    ril_service.cpp \
    sap_service.cpp \
    RilOpProxy.cpp \
    radioConfig_service.cpp

LOCAL_SHARED_LIBRARIES := \
    libmtkrillog \
    libmtkcutils \
    libmtkhardware_legacy \
    libmtkrilutils \
    libmtkproperty \
    libmtkutils \
    libmtkconfigutils \

include $(LOCAL_PATH)/hidl.mk

LOCAL_STATIC_LIBRARIES := \
    libprotobuf-c-nano-enable_malloc \

#LOCAL_CFLAGS := -DANDROID_MULTI_SIM -DDSDA_RILD1
LOCAL_CFLAGS += -Wno-unused-parameter

#use android binder, HIDL
LOCAL_CFLAGS += -DMTK_USE_HIDL
LOCAL_SHARED_LIBRARIES += libbinder libhidlbase libhidltransport

#used to check if support telephonyware
LOCAL_CFLAGS += -DMTK_TELEPHONYWARE_SUPPORT

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

ifeq ($(HAVE_AEE_FEATURE),yes)
      LOCAL_SHARED_LIBRARIES += libaedv
      LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

ifeq ($(MTK_TC1_FEATURE),yes)
      LOCAL_CFLAGS += -DMTK_TC1_FEATURE
endif
LOCAL_CFLAGS += -DMTK_MUX_CHANNEL_64
LOCAL_CFLAGS += -DMTK_IMS_CHANNEL_SUPPORT

LOCAL_C_INCLUDES += external/nanopb-c \
    $(MTK_ROOT)/external/aee/binary/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/log \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/property \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/utils \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/mtkrilutils \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/config \

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/../../include

LOCAL_MODULE:= librilfusion
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_CLANG := true
LOCAL_SANITIZE := integer

LOCAL_MULTILIB := first

include $(MTK_SHARED_LIBRARY)


endif
