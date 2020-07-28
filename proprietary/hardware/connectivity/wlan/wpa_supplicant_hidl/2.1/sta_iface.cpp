/*
 * hidl interface for wpa_supplicant daemon
 * Copyright (c) 2004-2016, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2004-2016, Roshan Pius <rpius@google.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "hidl_manager.h"
#include "hidl/1.2/hidl_return_util.h"
#include "iface_config_utils.h"
#include "sta_iface.h"

extern "C" {
#include "wps_supplicant.h"
}

namespace vendor {
namespace mediatek {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V2_1 {
namespace implementation {
using android::hardware::wifi::supplicant::V1_2::implementation::hidl_return_util::validateAndCall;
using android::hardware::wifi::supplicant::V1_0::SupplicantStatusCode;
using android::hardware::wifi::supplicant::V1_0::SupplicantStatus;
using namespace vendor::mediatek::hardware::wifi::supplicant::V2_0;
using namespace vendor::mediatek::hardware::wifi::supplicant::V2_1;
using V2_0::ISupplicantStaIfaceCallback;

StaIface::StaIface(struct wpa_global *wpa_global, const char ifname[])
    : wpa_global_(wpa_global), ifname_(ifname), is_valid_(true)
{
}

void StaIface::invalidate() { is_valid_ = false; }
bool StaIface::isValid()
{
    return (is_valid_ && (retrieveIfacePtr() != nullptr));
}

Return<void> StaIface::getName(getName_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_IFACE_INVALID,
        &StaIface::getNameInternal, _hidl_cb);
}

Return<void> StaIface::getType(getType_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_IFACE_INVALID,
        &StaIface::getTypeInternal, _hidl_cb);
}

Return<void> StaIface::getNetwork(
    SupplicantNetworkId id, getNetwork_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_IFACE_INVALID,
        &StaIface::getNetworkInternal, _hidl_cb, id);
}

Return<void> StaIface::registerCallback(
    const sp<ISupplicantStaIfaceCallback> &callback,
    registerCallback_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_IFACE_INVALID,
        &StaIface::registerCallbackInternal, _hidl_cb, callback);
}

Return<void> StaIface::registerCallback_2_1(
    const sp<V2_1::ISupplicantStaIfaceCallback> &callback,
    registerCallback_cb _hidl_cb)
{
    sp<V2_0::ISupplicantStaIfaceCallback> callback_2_0 = callback;
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_IFACE_INVALID,
        &StaIface::registerCallbackInternal, _hidl_cb, callback_2_0);
}

std::pair<SupplicantStatus, std::string> StaIface::getNameInternal()
{
    return {{SupplicantStatusCode::SUCCESS, ""}, ifname_};
}

std::pair<SupplicantStatus, IfaceType> StaIface::getTypeInternal()
{
    return {{SupplicantStatusCode::SUCCESS, ""}, IfaceType::STA};
}

std::pair<SupplicantStatus, sp<ISupplicantNetwork>>
StaIface::getNetworkInternal(SupplicantNetworkId id)
{
    android::sp<ISupplicantStaNetwork> network;
    struct wpa_supplicant *wpa_s = retrieveIfacePtr();
    struct wpa_ssid *ssid = wpa_config_get_network(wpa_s->conf, id);
    if (!ssid) {
        return {{SupplicantStatusCode::FAILURE_NETWORK_UNKNOWN, ""},
            network};
    }
    HidlManager *hidl_manager = HidlManager::getInstance();
    if (!hidl_manager ||
        hidl_manager->getStaNetworkHidlObjectByIfnameAndNetworkId(
        wpa_s->ifname, ssid->id, &network)) {
        return {{SupplicantStatusCode::FAILURE_UNKNOWN, ""}, network};
    }
    return {{SupplicantStatusCode::SUCCESS, ""}, network};
}

SupplicantStatus StaIface::registerCallbackInternal(
    const sp<ISupplicantStaIfaceCallback> &callback)
{
    HidlManager *hidl_manager = HidlManager::getInstance();
    if (!hidl_manager ||
        hidl_manager->addStaIfaceCallbackHidlObject(ifname_, callback)) {
        return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
    }
    return {SupplicantStatusCode::SUCCESS, ""};
}

/**
 * Retrieve the underlying |wpa_supplicant| struct
 * pointer for this iface.
 * If the underlying iface is removed, then all RPC method calls on this object
 * will return failure.
 */
wpa_supplicant *StaIface::retrieveIfacePtr()
{
    return wpa_supplicant_get_iface(wpa_global_, ifname_.c_str());
}

Return<void> StaIface::setWapiCertAliasList(
     const hidl_string& list, setWapiCertAliasList_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_IFACE_INVALID,
        &StaIface::setWapiCertAliasListInternal, _hidl_cb, list);
}

Return<void> StaIface::getFeatureMask(getFeatureMask_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_IFACE_INVALID,
        &StaIface::getFeatureMaskInternal, _hidl_cb);
}

SupplicantStatus StaIface::setWapiCertAliasListInternal(const std::string &list)
{
    return iface_config_utils::setWapiCertList(retrieveIfacePtr(), list);
}

std::pair<SupplicantStatus, uint32_t>
StaIface::getFeatureMaskInternal()
{
    uint32_t feature_mask = 0;
#ifdef CONFIG_WAPI_SUPPORT
    feature_mask |= ISupplicantStaIface::FeatureMask::WAPI;
#endif
    return {SupplicantStatus{SupplicantStatusCode::SUCCESS, ""}, feature_mask};
}

}  // namespace implementation
}  // namespace V2_1
}  // namespace wifi
}  // namespace supplicant
}  // namespace hardware
}  // namespace mediatek
}  //namespace vendor
