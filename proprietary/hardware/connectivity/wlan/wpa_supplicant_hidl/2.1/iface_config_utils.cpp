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

#include "hidl_manager.h"
#include "iface_config_utils.h"

namespace {
/* If other feature needs this function, please remove this compile option control
** We add it because only WAPI feature calls this function, and WAPI feature can be
** disabled, then build error occurs
*/
#ifdef CONFIG_WAPI_SUPPORT
void processConfigUpdate(struct wpa_supplicant* wpa_s, uint32_t changed_param)
{
    wpa_s->conf->changed_parameters |= changed_param;
    wpa_supplicant_update_config(wpa_s);
}

// Free any existing pointer stored in |dst| and store the provided string value
// there.
int freeAndSetStringConfigParam(
    struct wpa_supplicant* wpa_s, const std::string& value, uint32_t max_size,
    uint32_t changed_param, char** dst)
{
    if (value.size() > max_size) {
        return -1;
    }
    WPA_ASSERT(dst);
    os_free(static_cast<void*>(*dst));
    *dst = os_strdup(value.c_str());
    processConfigUpdate(wpa_s, changed_param);
    return 0;
}
#endif
}  // namespace

namespace vendor {
namespace mediatek {
namespace hardware {
namespace wifi {
namespace supplicant {
namespace V2_1 {
namespace implementation {
namespace iface_config_utils {
using android::hardware::wifi::supplicant::V1_0::SupplicantStatus;
using android::hardware::wifi::supplicant::V1_0::SupplicantStatusCode;

/* WAPI start */
SupplicantStatus setWapiCertList(
    struct wpa_supplicant* wpa_s, const std::string& list)
{
    WPA_ASSERT(wpa_s);
#ifdef CONFIG_WAPI_SUPPORT
    if (freeAndSetStringConfigParam(
        wpa_s, list, 1024,
        CFG_CHANGED_WAPI_CERT_ALIAS_LIST, &wpa_s->conf->wapi_cert_alias_list)) {
        return {SupplicantStatusCode::FAILURE_ARGS_INVALID, ""};
    }
    return {SupplicantStatusCode::SUCCESS, ""};
#else
    return {SupplicantStatusCode::FAILURE_UNKNOWN, ""};
#endif
}
/* WAPI end */
}  // namespace iface_config_utils
}  // namespace implementation
}  // namespace V2_1
}  // namespace wifi
}  // namespace supplicant
}  // namespace hardware
}  // namespace mediatek
}  //namespace vendor