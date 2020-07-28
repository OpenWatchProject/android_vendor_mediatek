/*
 * hidl interface for wpa_supplicant daemon
 * Copyright (c) 2004-2016, Jouni Malinen <j@w1.fi>
 * Copyright (c) 2004-2016, Roshan Pius <rpius@google.com>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include <algorithm>
#include <regex>

#include "hidl_manager.h"

namespace {
#if CONFIG_P2P_HIDL
/**
 * Check if the provided |wpa_supplicant| structure represents a P2P iface or
 * not.
 */
constexpr bool isP2pIface(const struct wpa_supplicant *wpa_s)
{
    return (wpa_s->global->p2p_init_wpa_s == wpa_s);
}
#endif
/**
 * Creates a unique key for the network using the provided |ifname| and
 * |network_id| to be used in the internal map of |ISupplicantNetwork| objects.
 * This is of the form |ifname|_|network_id|. For ex: "wlan0_1".
 *
 * @param ifname Name of the corresponding interface.
 * @param network_id ID of the corresponding network.
 */
const std::string getNetworkObjectMapKey(
    const std::string &ifname, int network_id)
{
    return ifname + "_" + std::to_string(network_id);
}

/**
 * Add callback to the corresponding list after linking to death on the
 * corresponding hidl object reference.
 */
template <class CallbackType>
int registerForDeathAndAddCallbackHidlObjectToList(
    const android::sp<CallbackType> &callback,
    const std::function<void(const android::sp<CallbackType> &)>
    &on_hidl_died_fctor,
    std::vector<android::sp<CallbackType>> &callback_list)
{
#if 0   // TODO(b/31632518): HIDL object death notifications.
    auto death_notifier = new CallbackObjectDeathNotifier<CallbackType>(
        callback, on_hidl_died_fctor);
    // Use the |callback.get()| as cookie so that we don't need to
    // store a reference to this |CallbackObjectDeathNotifier| instance
    // to use in |unlinkToDeath| later.
    // NOTE: This may cause an immediate callback if the object is already
    // dead, so add it to the list before we register for callback!
    if (android::hardware::IInterface::asBinder(callback)->linkToDeath(
        death_notifier, callback.get()) != android::OK) {
        wpa_printf(
            MSG_ERROR,
            "Error registering for death notification for "
            "supplicant callback object");
        callback_list.erase(
            std::remove(
            callback_list.begin(), callback_list.end(), callback),
            callback_list.end());
        return 1;
    }
#endif  // TODO(b/31632518): HIDL object death notifications.
    callback_list.push_back(callback);
    return 0;
}

template <class ObjectType>
int addHidlObjectToMap(
    const std::string &key, const android::sp<ObjectType> object,
    std::map<const std::string, android::sp<ObjectType>> &object_map)
{
    // Return failure if we already have an object for that |key|.
    if (object_map.find(key) != object_map.end())
        return 1;
    object_map[key] = object;
    if (!object_map[key].get())
        return 1;
    return 0;
}

template <class ObjectType>
int removeHidlObjectFromMap(
    const std::string &key,
    std::map<const std::string, android::sp<ObjectType>> &object_map)
{
    // Return failure if we dont have an object for that |key|.
    const auto &object_iter = object_map.find(key);
    if (object_iter == object_map.end())
        return 1;
    object_iter->second->invalidate();
    object_map.erase(object_iter);
    return 0;
}

template <class CallbackType>
int addIfaceCallbackHidlObjectToMap(
    const std::string &ifname, const android::sp<CallbackType> &callback,
    const std::function<void(const android::sp<CallbackType> &)>
    &on_hidl_died_fctor,
    std::map<const std::string, std::vector<android::sp<CallbackType>>>
    &callbacks_map)
{
    if (ifname.empty())
        return 1;

    auto iface_callback_map_iter = callbacks_map.find(ifname);
    if (iface_callback_map_iter == callbacks_map.end())
        return 1;
    auto &iface_callback_list = iface_callback_map_iter->second;

    // Register for death notification before we add it to our list.
    return registerForDeathAndAddCallbackHidlObjectToList<CallbackType>(
        callback, on_hidl_died_fctor, iface_callback_list);
}

template <class CallbackType>
int addNetworkCallbackHidlObjectToMap(
    const std::string &ifname, int network_id,
    const android::sp<CallbackType> &callback,
    const std::function<void(const android::sp<CallbackType> &)>
    &on_hidl_died_fctor,
    std::map<const std::string, std::vector<android::sp<CallbackType>>>
    &callbacks_map)
{
    if (ifname.empty() || network_id < 0)
        return 1;

    // Generate the key to be used to lookup the network.
    const std::string network_key =
        getNetworkObjectMapKey(ifname, network_id);
    auto network_callback_map_iter = callbacks_map.find(network_key);
    if (network_callback_map_iter == callbacks_map.end())
        return 1;
    auto &network_callback_list = network_callback_map_iter->second;

    // Register for death notification before we add it to our list.
    return registerForDeathAndAddCallbackHidlObjectToList<CallbackType>(
        callback, on_hidl_died_fctor, network_callback_list);
}

template <class CallbackType>
int removeAllIfaceCallbackHidlObjectsFromMap(
    const std::string &ifname,
    std::map<const std::string, std::vector<android::sp<CallbackType>>>
    &callbacks_map)
{
    auto iface_callback_map_iter = callbacks_map.find(ifname);
    if (iface_callback_map_iter == callbacks_map.end())
        return 1;
#if 0   // TODO(b/31632518): HIDL object death notifications.
    const auto &iface_callback_list = iface_callback_map_iter->second;
    for (const auto &callback : iface_callback_list) {
        if (android::hardware::IInterface::asBinder(callback)
            ->unlinkToDeath(nullptr, callback.get()) !=
            android::OK) {
            wpa_printf(
                MSG_ERROR,
                "Error deregistering for death notification for "
                "iface callback object");
        }
    }
#endif  // TODO(b/31632518): HIDL object death notifications.
    callbacks_map.erase(iface_callback_map_iter);
    return 0;
}

template <class CallbackType>
int removeAllNetworkCallbackHidlObjectsFromMap(
    const std::string &network_key,
    std::map<const std::string, std::vector<android::sp<CallbackType>>>
    &callbacks_map)
{
    auto network_callback_map_iter = callbacks_map.find(network_key);
    if (network_callback_map_iter == callbacks_map.end())
        return 1;
#if 0   // TODO(b/31632518): HIDL object death notifications.
    const auto &network_callback_list = network_callback_map_iter->second;
    for (const auto &callback : network_callback_list) {
        if (android::hardware::IInterface::asBinder(callback)
            ->unlinkToDeath(nullptr, callback.get()) !=
            android::OK) {
            wpa_printf(
                MSG_ERROR,
                "Error deregistering for death "
                "notification for "
                "network callback object");
        }
    }
#endif  // TODO(b/31632518): HIDL object death notifications.
    callbacks_map.erase(network_callback_map_iter);
    return 0;
}

template <class CallbackType>
void removeIfaceCallbackHidlObjectFromMap(
    const std::string &ifname, const android::sp<CallbackType> &callback,
    std::map<const std::string, std::vector<android::sp<CallbackType>>>
    &callbacks_map)
{
    if (ifname.empty())
        return;

    auto iface_callback_map_iter = callbacks_map.find(ifname);
    if (iface_callback_map_iter == callbacks_map.end())
        return;

    auto &iface_callback_list = iface_callback_map_iter->second;
    iface_callback_list.erase(
        std::remove(
        iface_callback_list.begin(), iface_callback_list.end(),
        callback),
        iface_callback_list.end());
}

template <class CallbackType>
void removeNetworkCallbackHidlObjectFromMap(
    const std::string &ifname, int network_id,
    const android::sp<CallbackType> &callback,
    std::map<const std::string, std::vector<android::sp<CallbackType>>>
    &callbacks_map)
{
    if (ifname.empty() || network_id < 0)
        return;

    // Generate the key to be used to lookup the network.
    const std::string network_key =
        getNetworkObjectMapKey(ifname, network_id);

    auto network_callback_map_iter = callbacks_map.find(network_key);
    if (network_callback_map_iter == callbacks_map.end())
        return;

    auto &network_callback_list = network_callback_map_iter->second;
    network_callback_list.erase(
        std::remove(
        network_callback_list.begin(), network_callback_list.end(),
        callback),
        network_callback_list.end());
}

template <class CallbackType>
void callWithEachIfaceCallback(
    const std::string &ifname,
    const std::function<
    android::hardware::Return<void>(android::sp<CallbackType>)> &method,
    const std::map<const std::string, std::vector<android::sp<CallbackType>>>
    &callbacks_map)
{
    if (ifname.empty())
        return;

    auto iface_callback_map_iter = callbacks_map.find(ifname);
    if (iface_callback_map_iter == callbacks_map.end())
        return;
    const auto &iface_callback_list = iface_callback_map_iter->second;
    for (const auto &callback : iface_callback_list) {
        if (!method(callback).isOk()) {
            wpa_printf(
                MSG_ERROR, "Failed to invoke HIDL iface callback");
        }
    }
}

template <class CallbackTypeV2_0, class CallbackTypeV2_1>
void callWithEachIfaceCallback_2_1(
    const std::string &ifname,
    const std::function<
        android::hardware::Return<void>(android::sp<CallbackTypeV2_1>)> &method,
    const std::map<const std::string, std::vector<android::sp<CallbackTypeV2_0>>>
        &callbacks_map)
{
    if (ifname.empty())
        return;

    auto iface_callback_map_iter = callbacks_map.find(ifname);
    if (iface_callback_map_iter == callbacks_map.end())
        return;
    const auto &iface_callback_list = iface_callback_map_iter->second;
    for (const auto &callback : iface_callback_list) {
        android::sp<CallbackTypeV2_1> callback_2_1 =
            CallbackTypeV2_1::castFrom(callback);
        if (callback_2_1 == nullptr)
            continue;

        if (!method(callback_2_1).isOk()) {
            wpa_printf(
                MSG_ERROR, "Failed to invoke HIDL iface callback");
        }
    }
}

template <class CallbackType>
void callWithEachNetworkCallback(
    const std::string &ifname, int network_id,
    const std::function<
    android::hardware::Return<void>(android::sp<CallbackType>)> &method,
    const std::map<const std::string, std::vector<android::sp<CallbackType>>>
    &callbacks_map)
{
    if (ifname.empty() || network_id < 0)
        return;

    // Generate the key to be used to lookup the network.
    const std::string network_key =
        getNetworkObjectMapKey(ifname, network_id);
    auto network_callback_map_iter = callbacks_map.find(network_key);
    if (network_callback_map_iter == callbacks_map.end())
        return;
    const auto &network_callback_list = network_callback_map_iter->second;
    for (const auto &callback : network_callback_list) {
        if (!method(callback).isOk()) {
            wpa_printf(
                MSG_ERROR,
                "Failed to invoke HIDL network callback");
        }
    }
}
}  // namespace

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

HidlManager *HidlManager::instance_ = NULL;

HidlManager *HidlManager::getInstance()
{
    if (!instance_)
        instance_ = new HidlManager();
    return instance_;
}

void HidlManager::destroyInstance()
{
    if (instance_)
        delete instance_;
    instance_ = NULL;
}

int HidlManager::registerHidlService(struct wpa_global *global)
{
    // Create the main hidl service object and register it.
    supplicant_object_ = new Supplicant(global);
    if (supplicant_object_->registerAsService() != android::NO_ERROR) {
        return 1;
    }
    return 0;
}

/**
 * Register an interface to hidl manager.
 *
 * @param wpa_s |wpa_supplicant| struct corresponding to the interface.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::registerInterface(struct wpa_supplicant *wpa_s)
{
    if (!wpa_s)
        return 1;
#if CONFIG_P2P_HIDL
    if (isP2pIface(wpa_s)) {
        if (addHidlObjectToMap<P2pIface>(
            wpa_s->ifname,
            new P2pIface(wpa_s->global, wpa_s->ifname),
            p2p_iface_object_map_)) {
            wpa_printf(
                MSG_ERROR,
                "Failed to register P2P interface with HIDL "
                "control: %s",
                wpa_s->ifname);
            return 1;
        }
        p2p_iface_callbacks_map_[wpa_s->ifname] =
            std::vector<android::sp<ISupplicantP2pIfaceCallback>>();
    } else {
#endif
        if (addHidlObjectToMap<StaIface>(
            wpa_s->ifname,
            new StaIface(wpa_s->global, wpa_s->ifname),
            sta_iface_object_map_)) {
            wpa_printf(
                MSG_ERROR,
                "Failed to register STA interface with HIDL "
                "control: %s",
                wpa_s->ifname);
            return 1;
        }
        sta_iface_callbacks_map_[wpa_s->ifname] =
            std::vector<android::sp<ISupplicantStaIfaceCallback>>();
#if CONFIG_P2P_HIDL
    }
#endif
    return 0;
}

/**
 * Unregister an interface from hidl manager.
 *
 * @param wpa_s |wpa_supplicant| struct corresponding to the interface.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::unregisterInterface(struct wpa_supplicant *wpa_s)
{
    if (!wpa_s)
        return 1;
#if CONFIG_P2P_HIDL
    if (isP2pIface(wpa_s)) {
        if (removeHidlObjectFromMap(
            wpa_s->ifname, p2p_iface_object_map_)) {
            wpa_printf(
                MSG_ERROR,
                "Failed to unregister P2P interface with HIDL "
                "control: %s",
                wpa_s->ifname);
            return 1;
        }
        if (removeAllIfaceCallbackHidlObjectsFromMap(
            wpa_s->ifname, p2p_iface_callbacks_map_)) {
            return 1;
        }
    } else {
#endif
        if (removeHidlObjectFromMap(
            wpa_s->ifname, sta_iface_object_map_)) {
            wpa_printf(
                MSG_ERROR,
                "Failed to unregister STA interface with HIDL "
                "control: %s",
                wpa_s->ifname);
            return 1;
        }
        if (removeAllIfaceCallbackHidlObjectsFromMap(
            wpa_s->ifname, sta_iface_callbacks_map_)) {
            return 1;
        }
#if CONFIG_P2P_HIDL
    }
#endif
    return 0;
}

/**
 * Register a network to hidl manager.
 *
 * @param wpa_s |wpa_supplicant| struct corresponding to the interface on which
 * the network is added.
 * @param ssid |wpa_ssid| struct corresponding to the network being added.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::registerNetwork(
    struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid)
{
    if (!wpa_s || !ssid)
        return 1;

    // Generate the key to be used to lookup the network.
    const std::string network_key =
        getNetworkObjectMapKey(wpa_s->ifname, ssid->id);
#if CONFIG_P2P_HIDL
    if (isP2pIface(wpa_s)) {
        if (addHidlObjectToMap<P2pNetwork>(
            network_key,
            new P2pNetwork(wpa_s->global, wpa_s->ifname, ssid->id),
            p2p_network_object_map_)) {
            wpa_printf(
                MSG_ERROR,
                "Failed to register P2P network with HIDL "
                "control: %d",
                ssid->id);
            return 1;
        }
        p2p_network_callbacks_map_[network_key] =
            std::vector<android::sp<ISupplicantP2pNetworkCallback>>();
        // Invoke the |onNetworkAdded| method on all registered
        // callbacks.
        callWithEachP2pIfaceCallback(
            wpa_s->ifname,
            std::bind(
            &ISupplicantP2pIfaceCallback::onNetworkAdded,
            std::placeholders::_1, ssid->id));
    } else {
#endif
        if (addHidlObjectToMap<StaNetwork>(
            network_key,
            new StaNetwork(wpa_s->global, wpa_s->ifname, ssid->id),
            sta_network_object_map_)) {
            wpa_printf(
                MSG_ERROR,
                "Failed to register STA network with HIDL "
                "control: %d",
                ssid->id);
            return 1;
        }
        sta_network_callbacks_map_[network_key] =
            std::vector<android::sp<ISupplicantStaNetworkCallback>>();
#if CONFIG_P2P_HIDL
    }
#endif
    return 0;
}

/**
 * Unregister a network from hidl manager.
 *
 * @param wpa_s |wpa_supplicant| struct corresponding to the interface on which
 * the network is added.
 * @param ssid |wpa_ssid| struct corresponding to the network being added.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::unregisterNetwork(
    struct wpa_supplicant *wpa_s, struct wpa_ssid *ssid)
{
    if (!wpa_s || !ssid)
        return 1;

    // Generate the key to be used to lookup the network.
    const std::string network_key =
        getNetworkObjectMapKey(wpa_s->ifname, ssid->id);

#if CONFIG_P2P_HIDL
    if (isP2pIface(wpa_s)) {
        if (removeHidlObjectFromMap(
            network_key, p2p_network_object_map_)) {
            wpa_printf(
                MSG_ERROR,
                "Failed to unregister P2P network with HIDL "
                "control: %d",
                ssid->id);
            return 1;
        }
        if (removeAllNetworkCallbackHidlObjectsFromMap(
            network_key, p2p_network_callbacks_map_))
            return 1;

        // Invoke the |onNetworkRemoved| method on all registered
        // callbacks.
        callWithEachP2pIfaceCallback(
            wpa_s->ifname,
            std::bind(
            &ISupplicantP2pIfaceCallback::onNetworkRemoved,
            std::placeholders::_1, ssid->id));
    } else {
#endif
        if (removeHidlObjectFromMap(
            network_key, sta_network_object_map_)) {
            wpa_printf(
                MSG_ERROR,
                "Failed to unregister STA network with HIDL "
                "control: %d",
                ssid->id);
            return 1;
        }
        if (removeAllNetworkCallbackHidlObjectsFromMap(
            network_key, sta_network_callbacks_map_))
        return 1;
#if CONFIG_P2P_HIDL
    }
#endif
    return 0;
}

/**
 * Retrieve the |ISupplicantStaIface| hidl object reference using the provided
 * ifname.
 *
 * @param ifname Name of the corresponding interface.
 * @param iface_object Hidl reference corresponding to the iface.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::getStaIfaceHidlObjectByIfname(
    const std::string &ifname, android::sp<ISupplicantStaIface> *iface_object)
{
    if (ifname.empty() || !iface_object)
        return 1;

    auto iface_object_iter = sta_iface_object_map_.find(ifname);
    if (iface_object_iter == sta_iface_object_map_.end())
        return 1;

    *iface_object = iface_object_iter->second;
    return 0;
}

/**
 * Retrieve the |ISupplicantStaNetwork| hidl object reference using the provided
 * ifname and network_id.
 *
 * @param ifname Name of the corresponding interface.
 * @param network_id ID of the corresponding network.
 * @param network_object Hidl reference corresponding to the network.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::getStaNetworkHidlObjectByIfnameAndNetworkId(
    const std::string &ifname, int network_id,
    android::sp<ISupplicantStaNetwork> *network_object)
{
    if (ifname.empty() || network_id < 0 || !network_object)
        return 1;

    // Generate the key to be used to lookup the network.
    const std::string network_key =
        getNetworkObjectMapKey(ifname, network_id);

    auto network_object_iter = sta_network_object_map_.find(network_key);
    if (network_object_iter == sta_network_object_map_.end())
        return 1;

    *network_object = network_object_iter->second;
    return 0;
}

/**
 * Add a new |ISupplicantCallback| hidl object reference to our
 * global callback list.
 *
 * @param callback Hidl reference of the |ISupplicantCallback| object.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::addSupplicantCallbackHidlObject(
    const android::sp<ISupplicantCallback> &callback)
{
    // Register for death notification before we add it to our list.
    auto on_hidl_died_fctor = std::bind(
        &HidlManager::removeSupplicantCallbackHidlObject, this,
        std::placeholders::_1);
    return registerForDeathAndAddCallbackHidlObjectToList<
        ISupplicantCallback>(
        callback, on_hidl_died_fctor, supplicant_callbacks_);
}

/**
 * Add a new iface callback hidl object reference to our
 * interface callback list.
 *
 * @param ifname Name of the corresponding interface.
 * @param callback Hidl reference of the callback object.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::addStaIfaceCallbackHidlObject(
    const std::string &ifname,
    const android::sp<ISupplicantStaIfaceCallback> &callback)
{
    const std::function<void(
        const android::sp<ISupplicantStaIfaceCallback> &)>
        on_hidl_died_fctor = std::bind(
        &HidlManager::removeStaIfaceCallbackHidlObject, this, ifname,
        std::placeholders::_1);
    return addIfaceCallbackHidlObjectToMap(
        ifname, callback, on_hidl_died_fctor, sta_iface_callbacks_map_);
}

/**
 * Add a new network callback hidl object reference to our network callback
 * list.
 *
 * @param ifname Name of the corresponding interface.
 * @param network_id ID of the corresponding network.
 * @param callback Hidl reference of the callback object.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::addStaNetworkCallbackHidlObject(
    const std::string &ifname, int network_id,
    const android::sp<ISupplicantStaNetworkCallback> &callback)
{
    const std::function<void(
        const android::sp<ISupplicantStaNetworkCallback> &)>
        on_hidl_died_fctor = std::bind(
        &HidlManager::removeStaNetworkCallbackHidlObject, this, ifname,
        network_id, std::placeholders::_1);
    return addNetworkCallbackHidlObjectToMap(
        ifname, network_id, callback, on_hidl_died_fctor,
        sta_network_callbacks_map_);
}

/**
 * Removes the provided |ISupplicantCallback| hidl object reference
 * from our global callback list.
 *
 * @param callback Hidl reference of the |ISupplicantCallback| object.
 */
void HidlManager::removeSupplicantCallbackHidlObject(
    const android::sp<ISupplicantCallback> &callback)
{
    supplicant_callbacks_.erase(
        std::remove(
        supplicant_callbacks_.begin(), supplicant_callbacks_.end(),
        callback),
        supplicant_callbacks_.end());
}

/**
 * Removes the provided iface callback hidl object reference from
 * our interface callback list.
 *
 * @param ifname Name of the corresponding interface.
 * @param callback Hidl reference of the callback object.
 */
void HidlManager::removeStaIfaceCallbackHidlObject(
    const std::string &ifname,
    const android::sp<ISupplicantStaIfaceCallback> &callback)
{
    return removeIfaceCallbackHidlObjectFromMap(
        ifname, callback, sta_iface_callbacks_map_);
}

/**
 * Removes the provided network callback hidl object reference from
 * our network callback list.
 *
 * @param ifname Name of the corresponding interface.
 * @param network_id ID of the corresponding network.
 * @param callback Hidl reference of the callback object.
 */
void HidlManager::removeStaNetworkCallbackHidlObject(
    const std::string &ifname, int network_id,
    const android::sp<ISupplicantStaNetworkCallback> &callback)
{
    return removeNetworkCallbackHidlObjectFromMap(
        ifname, network_id, callback, sta_network_callbacks_map_);
}

/**
 * Helper function to invoke the provided callback method on all the
 * registered |ISupplicantCallback| callback hidl objects.
 *
 * @param method Pointer to the required hidl method from
 * |ISupplicantCallback|.
 */
void HidlManager::callWithEachSupplicantCallback(
    const std::function<Return<void>(android::sp<ISupplicantCallback>)> &method)
{
    for (const auto &callback : supplicant_callbacks_) {
        if (!method(callback).isOk()) {
            wpa_printf(MSG_ERROR, "Failed to invoke HIDL callback");
        }
    }
}

/**
 * Helper fucntion to invoke the provided callback method on all the
 * registered iface callback hidl objects for the specified
 * |ifname|.
 *
 * @param ifname Name of the corresponding interface.
 * @param method Pointer to the required hidl method from
 * |ISupplicantIfaceCallback|.
 */
void HidlManager::callWithEachStaIfaceCallback(
    const std::string &ifname,
    const std::function<Return<void>(android::sp<ISupplicantStaIfaceCallback>)>
    &method)
{
    callWithEachIfaceCallback(ifname, method, sta_iface_callbacks_map_);
}

/**
 * Helper fucntion to invoke the provided callback method on all the
 * registered V2.1 iface callback hidl objects for the specified
 * |ifname|.
 *
 * @param ifname Name of the corresponding interface.
 * @param method Pointer to the required hidl method from
 * |V2_1::ISupplicantIfaceCallback|.
 */
void HidlManager::callWithEachStaIfaceCallback_2_1(
    const std::string &ifname,
    const std::function<Return<void>
        (android::sp<V2_1::ISupplicantStaIfaceCallback>)> &method)
{
    callWithEachIfaceCallback_2_1(ifname, method, sta_iface_callbacks_map_);
}

/**
 * Helper function to invoke the provided callback method on all the
 * registered network callback hidl objects for the specified
 * |ifname| & |network_id|.
 *
 * @param ifname Name of the corresponding interface.
 * @param network_id ID of the corresponding network.
 * @param method Pointer to the required hidl method from
 * |ISupplicantP2pNetworkCallback| or |ISupplicantStaNetworkCallback| .
 */
void HidlManager::callWithEachStaNetworkCallback(
    const std::string &ifname, int network_id,
    const std::function<
    Return<void>(android::sp<ISupplicantStaNetworkCallback>)> &method)
{
    callWithEachNetworkCallback(
        ifname, network_id, method, sta_network_callbacks_map_);
}

/**
 * Notify all listeners about the reception of WNM ESS immient disassoc
 * notification from the server.
 *
 * @param wpa_s |wpa_supplicant| struct corresponding to the interface.
 * @param code Deauth reason code sent from server.
 * @param reauth_delay Reauthentication delay in seconds sent from server.
 * @param url URL of the server.
 */
void HidlManager::notifyWNMRxEssDisassocImminentNotice(
    struct wpa_supplicant *wpa_s, u32 pmf_enabled, u32 reauth_delay, const char *url)
{
    if (!wpa_s || !url)
        return;

    if (sta_iface_object_map_.find(wpa_s->ifname) ==
        sta_iface_object_map_.end())
        return;

    callWithEachStaIfaceCallback(
        wpa_s->ifname,
        std::bind(
        &ISupplicantStaIfaceCallback::onEssDisassocImminentNotice,
        std::placeholders::_1, wpa_s->bssid, pmf_enabled, reauth_delay, url));
}

/**
 * Notify all listeners about the reception of data stall reason notification
 * from the server.
 *
 * @param wpa_s |wpa_supplicant| struct corresponding to the interface.
 * @param reason data stall reason code
 */
void HidlManager::notifyDataStallReason(
    struct wpa_supplicant *wpa_s, uint32_t reason)
{
    if (!wpa_s)
        return;

    if (sta_iface_object_map_.find(wpa_s->ifname) == sta_iface_object_map_.end())
        return;

    callWithEachStaIfaceCallback_2_1(
        wpa_s->ifname, std::bind(
        &V2_1::ISupplicantStaIfaceCallback::onDataStallNotice,
            std::placeholders::_1, reason));
}

/**
 * Notify all listeners about the reception of associated frequency has changed
 * notification from the server.
 *
 * @param wpa_s |wpa_supplicant| struct corresponding to the interface.
 * @param reason data stall reason code
 */
void HidlManager::notifyAssocFreqChanged(struct wpa_supplicant *wpa_s)
{
    if (!wpa_s)
        return;

    if (sta_iface_object_map_.find(wpa_s->ifname) == sta_iface_object_map_.end())
        return;

    callWithEachStaIfaceCallback_2_1(
        wpa_s->ifname, std::bind(
                   &V2_1::ISupplicantStaIfaceCallback::onAssocFreqChanged,
                   std::placeholders::_1, wpa_s->assoc_freq));
}
#if CONFIG_P2P_HIDL
/**
 * Retrieve the |ISupplicantP2pIface| hidl object reference using the provided
 * ifname.
 *
 * @param ifname Name of the corresponding interface.
 * @param iface_object Hidl reference corresponding to the iface.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::getP2pIfaceHidlObjectByIfname(
    const std::string &ifname, android::sp<ISupplicantP2pIface> *iface_object)
{
    if (ifname.empty() || !iface_object)
        return 1;

    auto iface_object_iter = p2p_iface_object_map_.find(ifname);
    if (iface_object_iter == p2p_iface_object_map_.end())
        return 1;

    *iface_object = iface_object_iter->second;
    return 0;
}

/**
 * Retrieve the |ISupplicantP2pNetwork| hidl object reference using the provided
 * ifname and network_id.
 *
 * @param ifname Name of the corresponding interface.
 * @param network_id ID of the corresponding network.
 * @param network_object Hidl reference corresponding to the network.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::getP2pNetworkHidlObjectByIfnameAndNetworkId(
    const std::string &ifname, int network_id,
    android::sp<ISupplicantP2pNetwork> *network_object)
{
    if (ifname.empty() || network_id < 0 || !network_object)
        return 1;

    // Generate the key to be used to lookup the network.
    const std::string network_key =
        getNetworkObjectMapKey(ifname, network_id);

    auto network_object_iter = p2p_network_object_map_.find(network_key);
    if (network_object_iter == p2p_network_object_map_.end())
        return 1;

    *network_object = network_object_iter->second;
    return 0;
}

/**
 * Add a new iface callback hidl object reference to our
 * interface callback list.
 *
 * @param ifname Name of the corresponding interface.
 * @param callback Hidl reference of the callback object.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::addP2pIfaceCallbackHidlObject(
    const std::string &ifname,
    const android::sp<ISupplicantP2pIfaceCallback> &callback)
{
    const std::function<void(
        const android::sp<ISupplicantP2pIfaceCallback> &)>
        on_hidl_died_fctor = std::bind(
        &HidlManager::removeP2pIfaceCallbackHidlObject, this, ifname,
        std::placeholders::_1);
    return addIfaceCallbackHidlObjectToMap(
        ifname, callback, on_hidl_died_fctor, p2p_iface_callbacks_map_);
}

/**
 * Add a new network callback hidl object reference to our network callback
 * list.
 *
 * @param ifname Name of the corresponding interface.
 * @param network_id ID of the corresponding network.
 * @param callback Hidl reference of the callback object.
 *
 * @return 0 on success, 1 on failure.
 */
int HidlManager::addP2pNetworkCallbackHidlObject(
    const std::string &ifname, int network_id,
    const android::sp<ISupplicantP2pNetworkCallback> &callback)
{
    const std::function<void(
        const android::sp<ISupplicantP2pNetworkCallback> &)>
        on_hidl_died_fctor = std::bind(
        &HidlManager::removeP2pNetworkCallbackHidlObject, this, ifname,
        network_id, std::placeholders::_1);
    return addNetworkCallbackHidlObjectToMap(
        ifname, network_id, callback, on_hidl_died_fctor,
        p2p_network_callbacks_map_);
}

/**
 * Removes the provided iface callback hidl object reference from
 * our interface callback list.
 *
 * @param ifname Name of the corresponding interface.
 * @param callback Hidl reference of the callback object.
 */
void HidlManager::removeP2pIfaceCallbackHidlObject(
    const std::string &ifname,
    const android::sp<ISupplicantP2pIfaceCallback> &callback)
{
    return removeIfaceCallbackHidlObjectFromMap(
        ifname, callback, p2p_iface_callbacks_map_);
}

/**
 * Removes the provided network callback hidl object reference from
 * our network callback list.
 *
 * @param ifname Name of the corresponding interface.
 * @param network_id ID of the corresponding network.
 * @param callback Hidl reference of the callback object.
 */
void HidlManager::removeP2pNetworkCallbackHidlObject(
    const std::string &ifname, int network_id,
    const android::sp<ISupplicantP2pNetworkCallback> &callback)
{
    return removeNetworkCallbackHidlObjectFromMap(
        ifname, network_id, callback, p2p_network_callbacks_map_);
}

/**
 * Helper fucntion to invoke the provided callback method on all the
 * registered iface callback hidl objects for the specified
 * |ifname|.
 *
 * @param ifname Name of the corresponding interface.
 * @param method Pointer to the required hidl method from
 * |ISupplicantIfaceCallback|.
 */
void HidlManager::callWithEachP2pIfaceCallback(
    const std::string &ifname,
    const std::function<Return<void>(android::sp<ISupplicantP2pIfaceCallback>)>
    &method)
{
    callWithEachIfaceCallback(ifname, method, p2p_iface_callbacks_map_);
}

/**
 * Helper function to invoke the provided callback method on all the
 * registered network callback hidl objects for the specified
 * |ifname| & |network_id|.
 *
 * @param ifname Name of the corresponding interface.
 * @param network_id ID of the corresponding network.
 * @param method Pointer to the required hidl method from
 * |ISupplicantP2pNetworkCallback| or |ISupplicantStaNetworkCallback| .
 */
void HidlManager::callWithEachP2pNetworkCallback(
    const std::string &ifname, int network_id,
    const std::function<
    Return<void>(android::sp<ISupplicantP2pNetworkCallback>)> &method)
{
    callWithEachNetworkCallback(
        ifname, network_id, method, p2p_network_callbacks_map_);
}
#endif

}  // namespace implementation
}  // namespace V2_1
}  // namespace wifi
}  // namespace supplicant
}  // namespace hardware
}  // namespace mediatek
}  //namespace vendor
