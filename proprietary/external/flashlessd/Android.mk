# Copyright 2012 Viatelcom

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MULTILIB := 32
LOCAL_MODULE:=flashlessd
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_TAGS:= optional

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include/c2k
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/nvram/libnvram
LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/external/nvram/libfile_op

LOCAL_SRC_FILES:= \
	flashlessd.c \
	c2k_support.c

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libc2kutils \
	libnvram \
	libfile_op \
	liblog \
	libutils

LOCAL_LDLIBS := -lc

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif

LOCAL_CFLAGS += -DMD3_SUPPORT_3g

#ifeq ($(CDMA_CHIP_VER),82C_PHONE)
LOCAL_CFLAGS += -DCBP_PHY_SDIO
LOCAL_CFLAGS += -DUSE_STATUSD
#endif

include $(MTK_EXECUTABLE)
