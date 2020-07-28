/*
 * hidl interface for wpa_supplicant daemon
 * Copyright (c) 2004-2016, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2004-2016, Roshan Pius <rpius@google.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef MTK_WPA_SUPPLICANT_HIDL_HIDL_MANAGER_H
#define MTK_WPA_SUPPLICANT_HIDL_HIDL_MANAGER_H

#include <map>
#include <string>

#include <vendor/mediatek/hardware/wifi/supplicant/2.0/ISupplicantCallback.h>
#include <vendor/mediatek/hardware/wifi/supplicant/2.1/ISupplicantStaIfaceCallback.h>
#include <vendor/mediatek/hardware/wifi/supplicant/2.0/ISupplicantStaNetworkCallback.h>

#include "supplicant.h"
#include "sta_iface.h"
#include "sta_network.h"

extern "C" {
#include "utils/common.h"
#include "utils/includes.h"
}

namespace vendor {
namespace mediatek {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V2_1 {
namespace implementation {
using namespace vendor::mediatek::hardware::wifi::supplicant::V2_0;
using namespace vendor::mediatek::hardware::wifi::supplicant::V2_1;
using V2_0::ISupplicantStaIfaceCallback;

/**
 * HidlManager is responsible for managing the lifetime of all
 * hidl objects created by wpa_supplicant. This is a singleton
 * class which is created by the supplicant core and can be used
 * to get references to the hidl objects.
 */
class HidlManager
{
public:
    static HidlManager *getInstance();
    static void destroyInstance();

    // Methods called from wpa_supplicant core.
    int registerHidlService(struct wpa_global *global);
    int registerInterface(struct wpa_supplicant *wpa_s);
    int unregisterInterface(struct wpa_supplicant *wpa_s);
    int registerNetwork(
        struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid);
    int unregisterNetwork(
        struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid);
    int getStaIfaceHidlObjectByIfname(
        const std::string &ifname,
        android::sp<ISupplicantStaIface> *iface_object);
    int getStaNetworkHidlObjectByIfnameAndNetworkId(
        const std::string &ifname, int network_id,
        android::sp<ISupplicantStaNetwork> *network_object);
    int addSupplicantCallbackHidlObject(
        const android::sp<ISupplicantCallback> &callback);
    int addStaIfaceCallbackHidlObject(
        const std::string &ifname,
        const android::sp<ISupplicantStaIfaceCallback> &callback);
#if CONFIG_P2P_HIDL
    int getP2pIfaceHidlObjectByIfname(
        const std::string &ifname,
        android::sp<ISupplicantP2pIface> *iface_object);
    int getP2pNetworkHidlObjectByIfnameAndNetworkId(
        const std::string &ifname, int network_id,
        android::sp<ISupplicantP2pNetwork> *network_object);
    int addP2pIfaceCallbackHidlObject(
        const std::string &ifname,
        const android::sp<ISupplicantP2pIfaceCallback> &callback);
    int addP2pNetworkCallbackHidlObject(
        const std::string &ifname, int network_id,
        const android::sp<ISupplicantP2pNetworkCallback> &callback);
#endif
    int addStaNetworkCallbackHidlObject(
        const std::string &ifname, int network_id,
        const android::sp<ISupplicantStaNetworkCallback> &callback);
    void notifyWNMRxEssDisassocImminentNotice(
        struct wpa_supplicant *wpa_s, u32 pmf_enabled,
        u32 reauth_delay, const char *url);
    void notifyDataStallReason(
        struct wpa_supplicant *wpa_s, uint32_t reason);
    void notifyAssocFreqChanged(struct wpa_supplicant *wpa_s);

private:
    HidlManager() = default;
    ~HidlManager() = default;
    HidlManager(const HidlManager &) = default;
    HidlManager &operator=(const HidlManager &) = default;

    void removeSupplicantCallbackHidlObject(
        const android::sp<ISupplicantCallback> &callback);
    void removeStaIfaceCallbackHidlObject(
        const std::string &ifname,
        const android::sp<ISupplicantStaIfaceCallback> &callback);
    void removeStaNetworkCallbackHidlObject(
        const std::string &ifname, int network_id,
        const android::sp<ISupplicantStaNetworkCallback> &callback);
    void callWithEachSupplicantCallback(
        const std::function<android::hardware::Return<void>(
        android::sp<ISupplicantCallback>)> &method);
    void callWithEachStaIfaceCallback(
        const std::string &ifname,
        const std::function<android::hardware::Return<void>(
        android::sp<ISupplicantStaIfaceCallback>)> &method);
    void callWithEachStaIfaceCallback_2_1(
        const std::string &ifname,
        const std::function<android::hardware::Return<void>(
        android::sp<V2_1::ISupplicantStaIfaceCallback>)> &method);
#if CONFIG_P2P_HIDL
    void removeP2pIfaceCallbackHidlObject(
        const std::string &ifname,
        const android::sp<ISupplicantP2pIfaceCallback> &callback);
    void removeP2pNetworkCallbackHidlObject(
        const std::string &ifname, int network_id,
        const android::sp<ISupplicantP2pNetworkCallback> &callback);


    void callWithEachP2pIfaceCallback(
        const std::string &ifname,
        const std::function<android::hardware::Return<void>(
        android::sp<ISupplicantP2pIfaceCallback>)> &method);

    void callWithEachP2pNetworkCallback(
        const std::string &ifname, int network_id,
        const std::function<android::hardware::Return<void>(
        android::sp<ISupplicantP2pNetworkCallback>)> &method);
#endif
    void callWithEachStaNetworkCallback(
        const std::string &ifname, int network_id,
        const std::function<android::hardware::Return<void>(
        android::sp<ISupplicantStaNetworkCallback>)> &method);

    // Singleton instance of this class.
    static HidlManager *instance_;
    // The main hidl service object.
    android::sp<Supplicant> supplicant_object_;
    // Map of all the STA interface specific hidl objects controlled by
    // wpa_supplicant. This map is keyed in by the corresponding
    // |ifname|.
    std::map<const std::string, android::sp<StaIface>>
        sta_iface_object_map_;
    // Map of all the STA network specific hidl objects controlled by
    // wpa_supplicant. This map is keyed in by the corresponding
    // |ifname| & |network_id|.
    std::map<const std::string, android::sp<StaNetwork>>
        sta_network_object_map_;
    // Callback registered for the main hidl service object.
    std::vector<android::sp<ISupplicantCallback>> supplicant_callbacks_;
    // Map of all the callbacks registered for STA interface specific
    // hidl objects controlled by wpa_supplicant.  This map is keyed in by
    // the corresponding |ifname|.
    std::map<
        const std::string,
        std::vector<android::sp<ISupplicantStaIfaceCallback>>>
        sta_iface_callbacks_map_;
    // Map of all the callbacks registered for STA network specific
    // hidl objects controlled by wpa_supplicant.  This map is keyed in by
    // the corresponding |ifname| & |network_id|.
    std::map<
        const std::string,
        std::vector<android::sp<ISupplicantStaNetworkCallback>>>
        sta_network_callbacks_map_;
#if CONFIG_P2P_HIDL
    // Map of all the P2P interface specific hidl objects controlled by
    // wpa_supplicant. This map is keyed in by the corresponding
    // |ifname|.
    std::map<const std::string, android::sp<P2pIface>>
        p2p_iface_object_map_;
    // Map of all the P2P network specific hidl objects controlled by
    // wpa_supplicant. This map is keyed in by the corresponding
    // |ifname| & |network_id|.
    std::map<const std::string, android::sp<P2pNetwork>>
        p2p_network_object_map_;
    // Map of all the callbacks registered for P2P interface specific
    // hidl objects controlled by wpa_supplicant.  This map is keyed in by
    // the corresponding |ifname|.
    std::map<
        const std::string,
        std::vector<android::sp<ISupplicantP2pIfaceCallback>>>
        p2p_iface_callbacks_map_;
    // Map of all the callbacks registered for P2P network specific
    // hidl objects controlled by wpa_supplicant.  This map is keyed in by
    // the corresponding |ifname| & |network_id|.
    std::map<
        const std::string,
        std::vector<android::sp<ISupplicantP2pNetworkCallback>>>
        p2p_network_callbacks_map_;
#endif
};
}  // namespace implementation
}  // namespace V2_1
}  // namespace wifi
}  // namespace supplicant
}  // namespace hardware
}  // namespace mediatek
}  //namespace vendor
#endif  // MTK_WPA_SUPPLICANT_HIDL_HIDL_MANAGER_H
