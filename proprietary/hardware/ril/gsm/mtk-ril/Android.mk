#
# Copyright (C) 2014 MediaTek Inc.
#
# Modification based on code covered by the below mentioned copyright
# and/or permission notice(s).
#

# Copyright 2014 The Android Open Source Project

# XXX using libutils for simulator build only...
#

ifneq ($(MTK_RIL_MODE), c6m_1rild)

ifneq ($(GOOGLE_RELEASE_RIL), yes)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ril_callbacks.c \
    ril_cc.c \
    ril_ss.c \
    ril_sim.c \
    ril_phb.c \
    ril_stk.c \
    ril_nw.c \
    ril_nws.c \
    ril_data.c \
    ril_sms.c \
    ril_c2k_sms.c \
    ril_oem.c \
    ril_ims.c \
    atchannels.c \
    misc.c \
    at_tok.c  \
    usim_fcp_parser.c \
    ril_embms.c \
    ril_gsm_util.cpp \
    ril_radio.c \
    base64.c \
    ril_wp.c \
    ril_mal.c \
    ril_ims_cc.c \
    mtk_ril_dsbp.c \
    ril_simlock.c

LOCAL_SHARED_LIBRARIES := \
    libcutils libutils libnetutils libifcutils_mtk librilmtk librilutils libsysenv liblog libmtkrilutils \
    libratconfig libhardware_legacy

    # for asprinf

LOCAL_STATIC_LIBRARIES := \
    libprotobuf-c-nano-enable_malloc \

LOCAL_CFLAGS := -D_GNU_SOURCE -DMTK_RIL -D__CCMNI_SUPPORT__

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

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

ifeq ($(MTK_IPV6_SUPPORT),yes)
    LOCAL_CFLAGS += -DMTK_IPV6_SUPPORT -DINET6
endif

ifeq ($(MTK_TC1_FEATURE),yes)
    LOCAL_CFLAGS += -DMTK_TC1_FEATURE
endif

ifeq ($(strip $(MTK_EXTERNAL_SIM_SUPPORT)), yes)
    LOCAL_STATIC_LIBRARIES += mtk-ril-prop-vsim
    LOCAL_CFLAGS += -DMTK_EXTERNAL_SIM_SUPPORT
#else ifeq ($(strip $(RELEASE_BRM)), yes)
#    LOCAL_STATIC_LIBRARIES += mtk-ril-prop-vsim
endif

LOCAL_C_INCLUDES :=

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ccci/include \
    $(LOCAL_PATH)/../../include \
    $(MTK_PATH_SOURCE)/external/libsysenv \
    $(MTK_PATH_SOURCE)/hardware/ims/include/mal_header/include/ \
    $(MTK_ROOT)/external/aee/binary/inc \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/mtkrilutils \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/compiler/ \

//LOCAL_C_INCLUDES += $(TARGET_OUT_HEADER)/librilutils
LOCAL_C_INCLUDES += external/nanopb-c
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ims/include/mal_header/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/system/netdagent/include

# build shared library
LOCAL_SHARED_LIBRARIES += \
    libssl libcrypto


# The static library being included above is a prebuilt and was built before the
# build system was fixed for b/16853291, which causes PIC issues.
LOCAL_LDFLAGS := -Wl,-Bsymbolic
LOCAL_CFLAGS += -DRIL_SHLIB
LOCAL_MODULE:= mtk-ril
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(MTK_SHARED_LIBRARY)

endif #GOOGLE_RELEASE_RIL

endif
