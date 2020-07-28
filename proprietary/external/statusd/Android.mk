# Copyright 2013 VIA Telecom
#
#Author(s) Xie Jin <jinx@via-telecom.com.cn>

#
ifeq ($(MTK_PLATFORM), MT6735)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= \
    src/com_intf.c

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libbinder \
	libcutils \
	libc2kutils \
	liblog

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif

LOCAL_PRELINK_MODULE := false
LOCAL_MULTILIB := 32
LOCAL_MODULE:= libipcom
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk

LOCAL_CFLAGS += -DCOM_ANDROID

LOCAL_LDLIBS += -lc
LOCAL_PRELINK_MODULE := false

include $(MTK_SHARED_LIBRARY)



include $(CLEAR_VARS)
LOCAL_MULTILIB := 32
LOCAL_MODULE:= statusd
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_INIT_RC := statusd.rc

LOCAL_SRC_FILES:=  \
	src/statusd.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../system/core/include/private
#LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/system/core/include/private

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libc2kutils \
	liblog \
	libutils

LOCAL_MODULE_TAGS := optional

## Note: Suffix will be temp if compile the module by mm in the directory
ifeq ($(strip $(REPO_VERSION)),)
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=\"temp\"
else
LOCAL_CFLAGS += -DVIA_SUFFIX_VERSION=$(REPO_VERSION)
endif

LOCAL_CFLAGS += -DSTATUS_ANDROID
LOCAL_CFLAGS += -DDEBUGGER

LOCAL_LDFLAGS += -g
#LOCAL_LDLIBS := -lpthread

include $(MTK_EXECUTABLE)
endif
