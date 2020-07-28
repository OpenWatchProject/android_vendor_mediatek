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
#include "sta_network.h"

extern "C" {
#include "wps_supplicant.h"
#include "eapol_supp/eapol_supp_sm.h"
#include "rsn_supp/wpa.h"
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

StaNetwork::StaNetwork(
    struct wpa_global *wpa_global, const char ifname[], int network_id)
    : wpa_global_(wpa_global),
      ifname_(ifname),
      network_id_(network_id),
      is_valid_(true)
{
}

void StaNetwork::invalidate() { is_valid_ = false; }
bool StaNetwork::isValid()
{
    return (is_valid_ && (retrieveNetworkPtr() != nullptr));
}

Return<void> StaNetwork::getId(getId_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
        &StaNetwork::getIdInternal, _hidl_cb);
}

Return<void> StaNetwork::getInterfaceName(getInterfaceName_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
        &StaNetwork::getInterfaceNameInternal, _hidl_cb);
}

Return<void> StaNetwork::getType(getType_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
        &StaNetwork::getTypeInternal, _hidl_cb);
}

Return<void> StaNetwork::registerCallback(
    const sp<ISupplicantStaNetworkCallback> &callback,
    registerCallback_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
        &StaNetwork::registerCallbackInternal, _hidl_cb, callback);
}

Return<void> StaNetwork::setEapOcsp(Ocsp ocsp, setEapOcsp_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
        &StaNetwork::setEapOcspInternal, _hidl_cb, ocsp);
}

std::pair<SupplicantStatus, uint32_t> StaNetwork::getIdInternal()
{
    return {{SupplicantStatusCode::SUCCESS, ""}, network_id_};
}

std::pair<SupplicantStatus, std::string> StaNetwork::getInterfaceNameInternal()
{
    return {{SupplicantStatusCode::SUCCESS, ""}, ifname_};
}

std::pair<SupplicantStatus, IfaceType> StaNetwork::getTypeInternal()
{
    return {{SupplicantStatusCode::SUCCESS, ""}, IfaceType::STA};
}

SupplicantStatus StaNetwork::registerCallbackInternal(
    const sp<ISupplicantStaNetworkCallback> &callback)
{
    HidlManager *hidl_manager = HidlManager::getInstance();
    if (!hidl_manager ||
        hidl_manager->addStaNetworkCallbackHidlObject(
        ifname_, network_id_, callback)) {
        return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
    }
    return {SupplicantStatusCode::SUCCESS, ""};
}

/**
 * Retrieve the underlying |wpa_ssid| struct pointer for
 * this network.
 * If the underlying network is removed or the interface
 * this network belong to
 * is removed, all RPC method calls on this object will
 * return failure.
 */
struct wpa_ssid *StaNetwork::retrieveNetworkPtr()
{
    wpa_supplicant *wpa_s = retrieveIfacePtr();
    if (!wpa_s)
        return nullptr;
    return wpa_config_get_network(wpa_s->conf, network_id_);
}

/**
 * Retrieve the underlying |wpa_supplicant| struct
 * pointer for
 * this network.
 */
struct wpa_supplicant *StaNetwork::retrieveIfacePtr()
{
    return wpa_supplicant_get_iface(wpa_global_, ifname_.c_str());
}

Return<void> StaNetwork::setWapiCertAlias(
    const hidl_string &alias, setWapiCertAlias_cb _hidl_cb)
{
    return validateAndCall(
        this, SupplicantStatusCode::FAILURE_NETWORK_INVALID,
        &StaNetwork::setWapiCertAliasInternal, _hidl_cb, alias);
}

SupplicantStatus StaNetwork::setWapiCertAliasInternal(const std::string &alias)
{
#ifdef CONFIG_WAPI_SUPPORT
    struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
    if (setStringFieldAndResetState(
        alias.c_str(), &(wpa_ssid->wapi_cert_alias), "wapi cert alias")) {
        return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
    }
    return {SupplicantStatusCode::SUCCESS, ""};
#else
    return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
#endif
}

SupplicantStatus StaNetwork::setEapOcspInternal(Ocsp ocsp)
{
    struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();
    wpa_ssid->eap.ocsp = static_cast<int>(ocsp);
    wpa_printf(MSG_MSGDUMP, "ocsp: %d", wpa_ssid->eap.ocsp);
    resetInternalStateAfterParamsUpdate();
    return {SupplicantStatusCode::SUCCESS, ""};
}

/**
 * Reset internal wpa_supplicant state machine state
 * after params update (except
 * bssid).
 */
void StaNetwork::resetInternalStateAfterParamsUpdate()
{
    struct wpa_supplicant *wpa_s = retrieveIfacePtr();
    struct wpa_ssid *wpa_ssid = retrieveNetworkPtr();

    wpa_sm_pmksa_cache_flush(wpa_s->wpa, wpa_ssid);

    if (wpa_s->current_ssid == wpa_ssid || wpa_s->current_ssid == NULL) {
		/*
		 * Invalidate the EAP session cache if
		 * anything in the
		 * current or previously used
		 * configuration changes.
		 */
        eapol_sm_invalidate_cached_session(wpa_s->eapol);
    }
}

/**
 * Helper function to set value in a string field in |wpa_ssid| structue
 * instance for this network.
 * This function frees any existing data in these fields.
 */
int StaNetwork::setStringFieldAndResetState(
    const char *value, uint8_t **to_update_field, const char *hexdump_prefix)
{
    return setStringFieldAndResetState(
        value, (char **)to_update_field, hexdump_prefix);
}

/**
 * Helper function to set value in a string field in |wpa_ssid| structue
 * instance for this network.
 * This function frees any existing data in these fields.
 */
int StaNetwork::setStringFieldAndResetState(
    const char *value, char **to_update_field, const char *hexdump_prefix)
{
    int value_len = strlen(value);
    if (*to_update_field) {
        os_free(*to_update_field);
    }
    *to_update_field = dup_binstr(value, value_len);
    if (!(*to_update_field)) {
        return 1;
    }
    wpa_hexdump_ascii(
        MSG_MSGDUMP, hexdump_prefix, *to_update_field, value_len);
    resetInternalStateAfterParamsUpdate();
    return 0;
}

/**
 * Helper function to set value in a string key field in |wpa_ssid| structue
 * instance for this network.
 * This function frees any existing data in these fields.
 */
int StaNetwork::setStringKeyFieldAndResetState(
    const char *value, char **to_update_field, const char *hexdump_prefix)
{
    int value_len = strlen(value);
    if (*to_update_field) {
        str_clear_free(*to_update_field);
    }
    *to_update_field = dup_binstr(value, value_len);
    if (!(*to_update_field)) {
        return 1;
    }
    wpa_hexdump_ascii_key(
        MSG_MSGDUMP, hexdump_prefix, *to_update_field, value_len);
    resetInternalStateAfterParamsUpdate();
    return 0;
}

/**
 * Helper function to set value in a string field with a corresponding length
 * field in |wpa_ssid| structue instance for this network.
 * This function frees any existing data in these fields.
 */
int StaNetwork::setByteArrayFieldAndResetState(
    const uint8_t *value, const size_t value_len, uint8_t **to_update_field,
    size_t *to_update_field_len, const char *hexdump_prefix)
{
    if (*to_update_field) {
        os_free(*to_update_field);
    }
    *to_update_field = (uint8_t *)os_malloc(value_len);
    if (!(*to_update_field)) {
        return 1;
    }
    os_memcpy(*to_update_field, value, value_len);
    *to_update_field_len = value_len;

    wpa_hexdump_ascii(
        MSG_MSGDUMP, hexdump_prefix, *to_update_field,
        *to_update_field_len);
    resetInternalStateAfterParamsUpdate();
    return 0;
}

/**
 * Helper function to set value in a string key field with a corresponding
 * length field in |wpa_ssid| structue instance for this network.
 * This function frees any existing data in these fields.
 */
int StaNetwork::setByteArrayKeyFieldAndResetState(
    const uint8_t *value, const size_t value_len, uint8_t **to_update_field,
    size_t *to_update_field_len, const char *hexdump_prefix)
{
    if (*to_update_field) {
        bin_clear_free(*to_update_field, *to_update_field_len);
    }
    *to_update_field = (uint8_t *)os_malloc(value_len);
    if (!(*to_update_field)) {
        return 1;
    }
    os_memcpy(*to_update_field, value, value_len);
    *to_update_field_len = value_len;

    wpa_hexdump_ascii_key(
        MSG_MSGDUMP, hexdump_prefix, *to_update_field,
        *to_update_field_len);
    resetInternalStateAfterParamsUpdate();
    return 0;
}

}  // namespace implementation
}  // namespace V2_1
}  // namespace wifi
}  // namespace supplicant
}  // namespace hardware
}  // namespace mediatek
}  //namespace vendor