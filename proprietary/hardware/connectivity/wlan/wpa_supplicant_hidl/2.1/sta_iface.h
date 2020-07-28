/*
 * hidl interface for wpa_supplicant daemon
 * Copyright (c) 2004-2016, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2004-2016, Roshan Pius <rpius@google.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef MTK_WPA_SUPPLICANT_HIDL_STA_IFACE_H
#define MTK_WPA_SUPPLICANT_HIDL_STA_IFACE_H

#include <array>
#include <vector>

#include <android-base/macros.h>

#include <vendor/mediatek/hardware/wifi/supplicant/2.1/ISupplicantStaIface.h>
#include <vendor/mediatek/hardware/wifi/supplicant/2.1/ISupplicantStaIfaceCallback.h>
#include <vendor/mediatek/hardware/wifi/supplicant/2.0/ISupplicantStaNetwork.h>

extern "C" {
#include "utils/common.h"
#include "utils/includes.h"
#include "wpa_supplicant_i.h"
#include "config.h"
}

namespace vendor {
namespace mediatek {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V2_1 {
namespace implementation {
using android::sp;
using android::hardware::Return;
using android::hardware::hidl_string;
using android::hardware::wifi::supplicant::V1_0::SupplicantNetworkId;
using android::hardware::wifi::supplicant::V1_0::SupplicantStatus;
using android::hardware::wifi::supplicant::V1_0::IfaceType;
using namespace vendor::mediatek::hardware::wifi::supplicant::V2_0;

/**
 * Implementation of StaIface hidl object. Each unique hidl
 * object is used for control operations on a specific interface
 * controlled by wpa_supplicant.
 */
class StaIface : public ISupplicantStaIface
{
public:
    StaIface(struct wpa_global* wpa_global, const char ifname[]);
    ~StaIface() override = default;
    // HIDL does not provide a built-in mechanism to let the server
    // invalidate a HIDL interface object after creation. If any client
    // process holds onto a reference to the object in their context,
    // any method calls on that reference will continue to be directed to
    // the server.
    // However Supplicant HAL needs to control the lifetime of these
    // objects. So, add a public |invalidate| method to all |Iface| and
    // |Network| objects.
    // This will be used to mark an object invalid when the corresponding
    // iface or network is removed.
    // All HIDL method implementations should check if the object is still
    // marked valid before processing them.
    void invalidate();
    bool isValid();

    // Hidl methods exposed.
    Return<void> getName(getName_cb _hidl_cb) override;
    Return<void> getType(getType_cb _hidl_cb) override;
    Return<void> getNetwork(
        SupplicantNetworkId id, getNetwork_cb _hidl_cb) override;
    Return<void> registerCallback(
        const sp<vendor::mediatek::hardware::wifi::supplicant::V2_0::ISupplicantStaIfaceCallback>& callback,
        registerCallback_cb _hidl_cb) override;
    Return<void> registerCallback_2_1(
        const sp<V2_1::ISupplicantStaIfaceCallback>& callback,
        registerCallback_cb _hidl_cb) override;
    Return<void> setWapiCertAliasList(
        const hidl_string& list, setWapiCertAliasList_cb _hidl_cb) override;
    Return<void> getFeatureMask(getFeatureMask_cb _hidl_cb) override;

private:
    // Corresponding worker functions for the HIDL methods.
    std::pair<SupplicantStatus, std::string> getNameInternal();
    std::pair<SupplicantStatus, IfaceType> getTypeInternal();

    std::pair<SupplicantStatus, sp<ISupplicantNetwork>> getNetworkInternal(
        SupplicantNetworkId id);
    SupplicantStatus registerCallbackInternal(
        const sp<vendor::mediatek::hardware::wifi::supplicant::V2_0::ISupplicantStaIfaceCallback>& callback);
    SupplicantStatus registerCallbackInternal_2_1(
        const sp<ISupplicantStaIfaceCallback>& callback);
    SupplicantStatus setWapiCertAliasListInternal(const std::string &list);
    std::pair<SupplicantStatus, uint32_t> getFeatureMaskInternal();

    struct wpa_supplicant* retrieveIfacePtr();

    // Reference to the global wpa_struct. This is assumed to be valid for
    // the lifetime of the process.
    struct wpa_global* wpa_global_;
    // Name of the iface this hidl object controls
    const std::string ifname_;
    bool is_valid_;

    DISALLOW_COPY_AND_ASSIGN(StaIface);
};

}  // namespace implementation
}  // namespace V2_1
}  // namespace wifi
}  // namespace supplicant
}  // namespace hardware
}  // namespace mediatek
}  //namespace vendor
#endif  // MTK_WPA_SUPPLICANT_HIDL_STA_IFACE_H
