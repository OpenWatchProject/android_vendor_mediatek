/*
 * hidl interface for wpa_supplicant daemon
 * Copyright (struct wpa_supplicant* wpa_s, c) 2004-2016, Jouni Malinen
 * <j@w1.fi>
 * Copyright (struct wpa_supplicant* wpa_s, c) 2004-2016, Roshan Pius
 * <rpius@google.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef MTK_WPA_SUPPLICANT_HIDL_IFACE_CONFIG_UTILS_H
#define MTK_WPA_SUPPLICANT_HIDL_IFACE_CONFIG_UTILS_H

#include <android-base/macros.h>

extern "C" {
#include "utils/common.h"
#include "utils/includes.h"
#include "wpa_supplicant_i.h"
#include "config.h"
}

/**
 * Utility functions to set various config parameters of an iface via HIDL
 * methods.
 */
namespace vendor {
namespace mediatek {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V2_1 {
namespace implementation {
namespace iface_config_utils {

SupplicantStatus setWapiCertList(
    struct wpa_supplicant* wpa_s, const std::string& list);
}  // namespace iface_config_utils
}  // namespace implementation
}  // namespace V2_1
}  // namespace wifi
}  // namespace supplicant
}  // namespace hardware
}  // namespace mediatek
}  //namespace vendor
#endif  // MTK_WPA_SUPPLICANT_HIDL_IFACE_CONFIG_UTILS_H
