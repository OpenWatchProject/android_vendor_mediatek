LOCAL_PATH:= $(call my-dir)

libpcap_cflags := \
  -Wno-unused-parameter \
  -D_U_="__attribute__((unused))" \

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
  pcap-linux.c pcap.c gencode.c optimize.c nametoaddr.c etherent.c \
  savefile.c sf-pcap.c sf-pcap-ng.c pcap-common.c fad-helpers.c \
  bpf/net/bpf_filter.c bpf_image.c bpf_dump.c \
  version.c

LOCAL_CFLAGS += $(libpcap_cflags)
LOCAL_CFLAGS += -Wno-sign-compare
LOCAL_CFLAGS += -DHAVE_CONFIG_H
LOCAL_CFLAGS += -include strings.h # For ffs(3).

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/include

LOCAL_MODULE:= libmtkpcap

LOCAL_PROPRIETARY_MODULE :=true
LOCAL_MODULE_OWNER := mtk

LOCAL_SHARED_LIBRARIES := libcutils \
                          liblog

include $(MTK_STATIC_LIBRARY)
