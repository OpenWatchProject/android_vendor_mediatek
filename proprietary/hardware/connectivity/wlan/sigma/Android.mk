LOCAL_PATH:= $(call my-dir)

SIGMA_BIN_DIR = /data/bin/
EAP_CERT_DIR = /data/misc/wifi
WPA_SUPPL_DIR = external/wpa_supplicant_8
SIGMA_OUT_DIR = $(PRODUCT_OUT)/testcases/sigma
HS20_DIR = /data/misc/wifi/hs20

# wfa options, do not modify
CFLAGS := -g -O2 -D_REENTRANT -DWFA_WMM_PS_EXT -DWFA_WMM_AC -DWFA_VOICE_EXT -DWFA_STA_TB -Wall -I../inc

# socket path for wpa_ctrl
CFLAGS += -DCONFIG_CTRL_IFACE_CLIENT_DIR=\"/data/vendor/wifi/wpa/sockets\"
CFLAGS += -DMTK_SYSTEM_VER=\"$(strip $(MTK_INTERNAL_BUILD_VERNO))\"

# path to sigma toolkits
#CFLAGS += -DBIN_PATH=\"$(SIGMA_BIN_DIR)\"
CFLAGS += -DIWPRIV=\"$(SIGMA_BIN_DIR)iwpriv\"
CFLAGS += -DDHCPCD=\"$(SIGMA_BIN_DIR)dhcpcd\"
CFLAGS += -DSETIPCONFIG=\"$(SIGMA_BIN_DIR)mtk_setipconfig.sh\"
CFLAGS += -DGETIPCONFIG=\"$(SIGMA_BIN_DIR)mtk_getipconfig.sh\"
CFLAGS += -DEAP_CERT_PATH=\"$(EAP_CERT_DIR)\"
CFLAGS += -DGETPID=\"$(SIGMA_BIN_DIR)getpid.sh\"
CFLAGS += -DGETPSTATS=\"$(SIGMA_BIN_DIR)getpstats.sh\"
CFLAGS += -DSTOPPING=\"$(SIGMA_BIN_DIR)stoping.sh\"
CFLAGS += -DUPDATEPID=\"$(SIGMA_BIN_DIR)updatepid.sh\"
CFLAGS += -DWFAPING=\"$(SIGMA_BIN_DIR)wfaping.sh\"
CFLAGS += -DWFAPING6=\"$(SIGMA_BIN_DIR)wfaping6.sh\"
CFLAGS += -DHS20_PATH=\"$(HS20_DIR)\"

CFLAGS += -DMODE_WMM_PS=\"WMMPS\"
CFLAGS += -DMODE_WMM_AC=\"WMMAC\"
CFLAGS += -DMODE_VOE=\"VoE\"
CFLAGS += -DMODE_P2P=\"P2P\"
CFLAGS += -DMODE_AP=\"AP\"
CFLAGS += -DMODE_TDLS=\"TDLS\"
CFLAGS += -DMODE_TG_N=\"TGn\"
CFLAGS += -DMODE_TG_AC=\"TGac\"
CFLAGS += -DMODE_WPA3=\"WPA3\"

CFLAGS += -DCONFIG_MTK_COMMON
CFLAGS += -DCONFIG_MTK_AP
CFLAGS += -DCONFIG_MTK_P2P
CFLAGS += -DCONFIG_MTK_WMM_VOE
CFLAGS += -DCONFIG_MTK_TDLS
CFLAGS += -DCONFIG_MTK_WPA3
CFLAGS += -DCONFIG_MTK_WMM_PS
CFLAGS += -DCONFIG_MTK_PPR2

########################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    lib/wfa_sock.c \
    lib/wfa_tlv.c \
    lib/wfa_cmdtbl.c \
    lib/wfa_tg.c \
    lib/wfa_miscs.c \
    lib/wfa_thr.c \
    lib/wfa_wmmps.c \

# mtk proprierary configure and setup
LOCAL_SRC_FILES += \
    mediatek/mtk_cs.c \

# files copied from wpa_supplicant
LOCAL_SRC_FILES += \
    mediatek/wpa/wpa_helpers.c \
    mediatek/wpa/wpa_ctrl.c \
    mediatek/wpa/os_unix.c \

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libnetutils

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \
    $(WPA_SUPPL_DIR)/src/utils \
    $(WPA_SUPPL_DIR)/src/common \
    system/core/libnetutils/include

LOCAL_MODULE := libwfadut_static
LOCAL_CFLAGS := $(CFLAGS) -DANDROID_LOG_NAME=\"wfa_dut\" -DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX -DANDROID
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := tests
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
include $(BUILD_STATIC_LIBRARY)

########################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    dut/wfa_dut.c \
    dut/wfa_dut_init.c \

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libnetutils

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \

LOCAL_STATIC_LIBRARIES += libwfadut_static
LOCAL_CFLAGS := $(CFLAGS)
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := tests
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(SIGMA_OUT_DIR)/scripts
LOCAL_MODULE:= wfa_dut

include $(BUILD_EXECUTABLE)

########################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    lib/wfa_sock.c \
    lib/wfa_tlv.c \
    lib/wfa_ca_resp.c \
    lib/wfa_cmdproc.c \
    lib/wfa_miscs.c \
    lib/wfa_typestr.c \
    ca/wfa_ca.c \

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/inc \

LOCAL_CFLAGS := $(CFLAGS) -DANDROID_LOG_NAME=\"wfa_ca\"
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := tests
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(SIGMA_OUT_DIR)/scripts
LOCAL_MODULE:= wfa_ca

include $(BUILD_EXECUTABLE)

########################

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    console_src/wfa_con.c \
    console_src/wfa_sndrcv.c \
    console_src/wfa_util.c

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    libc \
    liblog

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/inc

LOCAL_CFLAGS := $(CFLAGS) -DANDROID_LOG_NAME=\"wfa_con\"
LOCAL_STATIC_LIBRARIES += libwfadut_static
LOCAL_CFLAGS := $(CFLAGS)
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := tests
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_PATH := $(SIGMA_OUT_DIR)/scripts
LOCAL_MODULE:= wfa_con

include $(BUILD_EXECUTABLE)