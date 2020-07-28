#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
###################################################################################
##### This Makefile include Google pure SUPPLICANT and MTK Turnkey SUPPLICANT #####
###################################################################################
LOCAL_PATH := $(call my-dir)

##### For Google SUPPLICANT #####
ifeq ($(strip $(MTK_BASIC_PACKAGE)),yes)

    WPA_SUPPL_DIR = external/wpa_supplicant_8
    WPA_SRC_FILE :=

ifneq ($(BOARD_WPA_SUPPLICANT_DRIVER),)
    CONFIG_DRIVER_$(BOARD_WPA_SUPPLICANT_DRIVER) := y
endif
ifneq ($(BOARD_HOSTAPD_DRIVER),)
    CONFIG_DRIVER_$(BOARD_HOSTAPD_DRIVER) := y
endif

include $(WPA_SUPPL_DIR)/wpa_supplicant/android.config

WPA_SUPPL_DIR_INCLUDE = $(WPA_SUPPL_DIR)/src \
	$(WPA_SUPPL_DIR)/src/common \
	$(WPA_SUPPL_DIR)/src/drivers \
	$(WPA_SUPPL_DIR)/src/l2_packet \
	$(WPA_SUPPL_DIR)/src/utils \
	$(WPA_SUPPL_DIR)/src/wps \
	$(WPA_SUPPL_DIR)/wpa_supplicant

ifdef CONFIG_DRIVER_NL80211
WPA_SUPPL_DIR_INCLUDE += external/libnl/include
WPA_SRC_FILE += mediatek_driver_cmd_nl80211.c
endif

ifdef CONFIG_DRIVER_WEXT
#error doesn't support CONFIG_DRIVER_WEXT
endif

# To force sizeof(enum) = 4
ifeq ($(TARGET_ARCH),arm)
L_CFLAGS += -mabi=aapcs-linux
endif

ifdef CONFIG_ANDROID_LOG
L_CFLAGS += -DCONFIG_ANDROID_LOG
endif

########################
include $(CLEAR_VARS)
LOCAL_SANITIZE := cfi
LOCAL_MODULE := lib_driver_cmd_mt66xx
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := libc libcutils
LOCAL_CFLAGS := $(L_CFLAGS)
LOCAL_SRC_FILES := $(WPA_SRC_FILE)
LOCAL_C_INCLUDES := $(WPA_SUPPL_DIR_INCLUDE)
include $(MTK_STATIC_LIBRARY)
########################
endif

##### For MTK SUPPLICANT #####
ifneq ($(strip $(MTK_BASIC_PACKAGE)),yes)

    WPA_SUPPL_DIR = external/wpa_supplicant_8
    WPA_SRC_FILE :=

ifneq ($(BOARD_WPA_SUPPLICANT_DRIVER),)
    CONFIG_DRIVER_$(BOARD_WPA_SUPPLICANT_DRIVER) := y
endif
ifneq ($(BOARD_HOSTAPD_DRIVER),)
    CONFIG_DRIVER_$(BOARD_HOSTAPD_DRIVER) := y
endif

include $(WPA_SUPPL_DIR)/hostapd/android.config
include $(WPA_SUPPL_DIR)/wpa_supplicant/android.config

ifeq ($(MTK_WAPI_SUPPORT), yes)
L_CFLAGS += -DCONFIG_WAPI_SUPPORT
LOCAL_SHARED_LIBRARIES += NO_WAPI_LICENSE
endif

#Default on for bsp projects
L_CFLAGS += -DCONFIG_MTK_P2P_SIGMA

ifdef CONFIG_MTK_POOR_LINK_DETECT
L_CFLAGS += -DCONFIG_MTK_POOR_LINK_DETECT
endif

ifdef CONFIG_CTRL_IFACE_MTK_HIDL
L_CFLAGS += -DCONFIG_CTRL_IFACE_MTK_HIDL
endif

WPA_SUPPL_DIR_INCLUDE = $(WPA_SUPPL_DIR)/src \
	$(WPA_SUPPL_DIR)/src/common \
	$(WPA_SUPPL_DIR)/src/drivers \
	$(WPA_SUPPL_DIR)/src/l2_packet \
	$(WPA_SUPPL_DIR)/src/utils \
	$(WPA_SUPPL_DIR)/src/wps \
	$(WPA_SUPPL_DIR)/wpa_supplicant

ifdef CONFIG_DRIVER_NL80211
WPA_SUPPL_DIR_INCLUDE += external/libnl/include
WPA_SRC_FILE += mediatek_driver_cmd_nl80211.c
endif

ifeq ($(MTK_WAPI_SUPPORT), yes)
WPA_SRC_FILE += wapi.c
endif

ifdef CONFIG_DRIVER_WEXT
#error doesn't support CONFIG_DRIVER_WEXT
endif

# To force sizeof(enum) = 4
ifeq ($(TARGET_ARCH),arm)
L_CFLAGS += -mabi=aapcs-linux
endif

ifdef CONFIG_ANDROID_LOG
L_CFLAGS += -DCONFIG_ANDROID_LOG
endif

########################

include $(CLEAR_VARS)
LOCAL_SANITIZE := cfi
LOCAL_MODULE := lib_driver_cmd_mt66xx
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_SHARED_LIBRARIES := libc libcutils
ifeq ($(CONFIG_CTRL_IFACE_MTK_HIDL), y)
LOCAL_SHARED_LIBRARIES += vendor.mediatek.hardware.wifi.supplicant@2.0
endif # CONFIG_CTRL_IFACE_MTK_HIDL
LOCAL_CFLAGS := $(L_CFLAGS)
LOCAL_SRC_FILES := $(WPA_SRC_FILE)
LOCAL_C_INCLUDES := $(WPA_SUPPL_DIR_INCLUDE)
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
include $(MTK_STATIC_LIBRARY)

########################

endif
