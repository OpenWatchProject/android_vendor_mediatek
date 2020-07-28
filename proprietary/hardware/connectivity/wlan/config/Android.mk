# --------------------------------------------------------------------
# configuration files for AOSP wpa_supplicant_8
# --------------------------------------------------------------------
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := wpa_supplicant.conf
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/wifi
LOCAL_SRC_FILES := mtk-wpa_supplicant.conf
include $(BUILD_PREBUILT)

#################Add overlay file################
include $(CLEAR_VARS)
LOCAL_MODULE := wpa_supplicant_overlay.conf
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/wifi
LOCAL_SRC_FILES := mtk-wpa_supplicant-overlay.conf
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := p2p_supplicant_overlay.conf
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR)/etc/wifi
LOCAL_SRC_FILES := mtk-p2p_wpa_supplicant-overlay.conf
include $(BUILD_PREBUILT)

