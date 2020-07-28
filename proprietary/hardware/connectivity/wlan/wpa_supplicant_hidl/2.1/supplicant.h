/*
 * hidl interface for wpa_supplicant daemon
 * Copyright (c) 2004-2016, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2004-2016, Roshan Pius <rpius@google.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef MTK_WPA_SUPPLICANT_HIDL_SUPPLICANT_H
#define MTK_WPA_SUPPLICANT_HIDL_SUPPLICANT_H

#include <android-base/macros.h>

#include <android/hardware/wifi/supplicant/1.0/types.h>
#include <vendor/mediatek/hardware/wifi/supplicant/2.0/ISupplicant.h>
#include <vendor/mediatek/hardware/wifi/supplicant/2.0/ISupplicantCallback.h>
#include <vendor/mediatek/hardware/wifi/supplicant/2.0/ISupplicantIface.h>

extern "C" {
#include "utils/common.h"
#include "utils/includes.h"
#include "utils/wpa_debug.h"
#include "wpa_supplicant_i.h"
}

using android::hardware::wifi::supplicant::V1_0::SupplicantStatus;
using android::hardware::wifi::supplicant::V1_0::SupplicantStatusCode;

namespace vendor {
namespace mediatek {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V2_1 {
namespace implementation {
using android::sp;
using android::hardware::Return;
using android::hardware::wifi::supplicant::V1_0::SupplicantStatus;
using namespace vendor::mediatek::hardware::wifi::supplicant::V2_0;

/**
 * Implementation of the supplicant hidl object. This hidl
 * object is used core for global control operations on
 * wpa_supplicant.
 */
class Supplicant : public ISupplicant
{
public:
    Supplicant(struct wpa_global* global);
    ~Supplicant() override = default;
    bool isValid();

    // Hidl methods exposed.
    Return<void> getInterface(
        const android::hardware::wifi::supplicant::V1_0::ISupplicant::IfaceInfo& iface_info,
        getInterface_cb _hidl_cb) override;
    Return<void> registerCallback(
        const sp<ISupplicantCallback>& callback,
        registerCallback_cb _hidl_cb) override;

private:
    // Corresponding worker functions for the HIDL methods.
    std::pair<SupplicantStatus, sp<ISupplicantIface>> getInterfaceInternal(
        const android::hardware::wifi::supplicant::V1_0::ISupplicant::IfaceInfo& iface_info);
    SupplicantStatus registerCallbackInternal(
        const sp<ISupplicantCallback>& callback);

    // Raw pointer to the global structure maintained by the core.
    struct wpa_global* wpa_global_;

    DISALLOW_COPY_AND_ASSIGN(Supplicant);
};

}  // namespace implementation
}  // namespace V2_1
}  // namespace wifi
}  // namespace supplicant
}  // namespace hardware
}  // namespace mediatek
}  //namespace vendor
#endif  // MTK_WPA_SUPPLICANT_HIDL_SUPPLICANT_H
