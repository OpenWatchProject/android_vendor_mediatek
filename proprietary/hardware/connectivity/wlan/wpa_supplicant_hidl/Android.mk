ifeq ($(CONFIG_CTRL_IFACE_MTK_HIDL), y)
### Hidl service library ###
########################
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libwpa_mtk_hidl
LOCAL_VENDOR_MODULE := true
# Apply wpa_supplicant's C & CPP Flags
LOCAL_CPPFLAGS := $(MTK_HIDL_CPPFLAGS)
LOCAL_CFLAGS := $(MTK_HIDL_CFLAGS)

#if need to add p2p related hidl interface, please enable below line
#LOCAL_CFLAGS += -DCONFIG_P2P_HIDL
SUPP_PATH := external/wpa_supplicant_8
LOCAL_C_INCLUDES := $(SUPP_PATH)/src $(SUPP_PATH)/wpa_supplicant $(SUPP_PATH)/src/common $(SUPP_PATH)/src/utils
MTK_HIDL_INTERFACE_VERSION := 2.1
LOCAL_SRC_FILES := \
    $(MTK_HIDL_INTERFACE_VERSION)/hidl.cpp \
    $(MTK_HIDL_INTERFACE_VERSION)/hidl_manager.cpp \
    $(MTK_HIDL_INTERFACE_VERSION)/iface_config_utils.cpp \
    $(MTK_HIDL_INTERFACE_VERSION)/sta_iface.cpp \
    $(MTK_HIDL_INTERFACE_VERSION)/sta_network.cpp \
    $(MTK_HIDL_INTERFACE_VERSION)/supplicant.cpp
LOCAL_SHARED_LIBRARIES := \
    vendor.mediatek.hardware.wifi.supplicant@2.0 \
    vendor.mediatek.hardware.wifi.supplicant@2.1 \
    libbase \
    libhidlbase \
    libhidltransport \
    libutils \
    liblog
LOCAL_EXPORT_C_INCLUDE_DIRS := \
    $(LOCAL_PATH)/$(MTK_HIDL_INTERFACE_VERSION)
include $(BUILD_STATIC_LIBRARY)
endif # CONFIG_CTRL_IFACE_MTK_HIDL == y