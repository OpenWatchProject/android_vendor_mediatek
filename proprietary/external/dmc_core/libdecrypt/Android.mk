LOCAL_PATH:= $(call my-dir)

tcpdump_src_files := print-esp.c \
                     ascii_strcasecmp.c \
                     strtoaddr.c

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(tcpdump_src_files)
LOCAL_CFLAGS += -DHAVE_CONFIG_H
LOCAL_CFLAGS += -D_U_="__attribute__((unused))"
LOCAL_SHARED_LIBRARIES += libssl libcrypto liblog libsysutils libcutils

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_PROPRIETARY_MODULE :=true
LOCAL_MODULE_OWNER := mtk

LOCAL_MODULE := libdecrypt
include $(MTK_SHARED_LIBRARY)
