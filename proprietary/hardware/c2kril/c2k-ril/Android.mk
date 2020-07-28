# Copyright 2006 The Android Open Source Project

# XXX using libutils for simulator build only...

ifneq ($(MTK_RIL_MODE), c6m_1rild)

LOCAL_PATH:= $(call my-dir)

#GIT_COMMIT := $(shell cd $(LOCAL_PATH); git rev-parse --verify --short HEAD 2>/dev/null)
#SVN_COMMIT := $(shell svn info 2>/dev/null | grep '^Last Changed Re' | awk '{print $$NF}')
#
#ifneq ($(strip $(GIT_COMMIT)),)
#        REPO_VERSION := \"git_$(GIT_COMMIT)\"
#else
#        REPO_VERSION := \"svn_$(SVN_COMMIT)\"
#endif
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ril_callbacks.c \
    atchannel.c \
    misc.c \
    at_tok.c \
    csim_fcp_parser.c \
    ril_sim.c \
    ril_phb.c \
    ril_nw.c \
    ril_cc.c \
    icc.c \
    ril_utk.c \
    ril_oem.c \
    atchannel_config.c \
    sms.c \
    ril_sms.c \
    c2k_ril_data.c \
    ril_radio.c \
    ril_wp.c

LOCAL_CFLAGS += -DC2K_RIL

#LOCAL_SRC_FILES += via-audio-ril.c

LOCAL_SHARED_LIBRARIES := \
    libcutils libutils libc2kril libnetutils librilutils libratconfig liblog \
    libmtkrilutils libifcutils_mtk

LC_MTK_PLATFORM := $(shell echo $(MTK_PLATFORM) | tr A-Z a-z )

ifneq (,$(filter $(strip $(LC_MTK_PLATFORM)), mt6735 mt6753 mt6737t))
    LOCAL_SHARED_LIBRARIES += libc2kutils
    LOCAL_CFLAGS += -DC2K_RIL_LEGACY
endif

LOCAL_STATIC_LIBRARIES := \
    libprotobuf-c-nano-enable_malloc libqemu_pipe libbase

# for asprinf
LOCAL_CFLAGS += -D_GNU_SOURCE -D_POSIX_SOURCE
LOCAL_C_INCLUDES := $(KERNEL_HEADERS)
LOCAL_C_INCLUDES += external/alsa-lib/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include/c2k
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ril/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ccci/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/ims/include/mal_header/include
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/system/netdagent/include
LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/mtkrilutils
LOCAL_C_INCLUDES += system/core/include

ifeq ($(HAVE_AEE_FEATURE),yes)
    LOCAL_SHARED_LIBRARIES += libaedv
    LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
    LOCAL_C_INCLUDES += $(MTK_ROOT)/external/aee/binary/inc
endif

LOCAL_C_INCLUDES += external/nanopb-c

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif

ifeq ($(HAS_VIA_ADDED_RIL_REQUEST), true)
LOCAL_CFLAGS += -DVIA_ADDED_RIL_REQUEST
endif

ifeq ($(MTK_ECCCI_C2K),yes)
LOCAL_CFLAGS += -DMTK_ECCCI_C2K
endif

ifeq ($(TARGET_DEVICE),apollo)
#  LOCAL_CFLAGS += -DVIATELECOM_APOLLO
endif
ifeq ($(TARGET_DEVICE),marco)
#  LOCAL_CFLAGS += -DVIATELECOM_MARCO
endif

LOCAL_MULTILIB := first
ifeq (foo,foo)
  #build shared library
  LOCAL_SHARED_LIBRARIES += \
      libcutils libutils

#  LOCAL_SHARED_LIBRARIES += libasound
#  LOCAL_LDLIBS += -lpthread
  LOCAL_CFLAGS += -DRIL_SHLIB

#ALOG_VERSION := 4.1.1 4.2.1 4.2.2
#ifeq ($(findstring $(PLATFORM_VERSION),$(ALOG_VERSION)), $(PLATFORM_VERSION))
#LOCAL_CFLAGS += -DANDROID_JB
#endif
#ifeq ($(PLATFORM_VERSION), 4.2.2)
LOCAL_CFLAGS += -DMT6589
LOCAL_CFLAGS += -DUSE_STATUSD
#endif
  LOCAL_PRELINK_MODULE := false
  LOCAL_MODULE:= libviatelecom-withuim-ril
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
  include $(MTK_SHARED_LIBRARY)
else
  #build executable
  LOCAL_SHARED_LIBRARIES += \
      libc2kril
  LOCAL_MODULE:= libviatelecom-withuim-ril
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
  LOCAL_PRELINK_MODULE := false
  include $(MTK_EXECUTABLE)
endif

endif
