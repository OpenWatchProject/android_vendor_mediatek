# Copyright 2010 VIA Telecom
#
#Author(s) Xie Jin <jinx@via-telecom.com.cn>

#
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= stools
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_SRC_FILES:=  \
	stools.c

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/include/c2k

LOCAL_SHARED_LIBRARIES := \
		libcutils \
		libipcom \
		liblog

LOCAL_MODULE_TAGS := optional

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif
LOCAL_CFLAGS += -DSTATUS_ANDROID
LOCAL_CFLAGS += -DDEBUGGER

#LOCAL_LDLIBS := -lpthread
#
#CAL_LDLIBS += -lpthread

include $(MTK_EXECUTABLE)
