/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "GnssVisibilityControl"

#include "GnssVisibilityControl.h"
#include <log/log.h>

namespace android {
namespace hardware {
namespace gnss {
namespace visibility_control {
namespace V1_0 {
namespace implementation {

sp<V1_0::IGnssVisibilityControlCallback> GnssVisibilityControl::sIGnssVisibilityControlCbIface = nullptr;

GnssVisibilityControlCallback_ext GnssVisibilityControl::sGnssVisibilityControlCbs = {
    .size = sizeof(GnssVisibilityControlCallback_ext),
    .nfw_notify_cb = nfwNotifyCb,
    .is_in_emergency_session = isInEmergencySession
};

GnssVisibilityControl::GnssVisibilityControl(
        const GnssVisibilityControlInterface* gnssVisibilityControlIface)
        : mGnssVisibilityControlInterface(gnssVisibilityControlIface){}


// Methods from ::android::hardware::gnss::visibility_control::V1_0::IGnssVisibilityControl follow.
Return<bool> GnssVisibilityControl::enableNfwLocationAccess(
        const hidl_vec<hidl_string>& proxyApps) {
    if (mGnssVisibilityControlInterface == nullptr) {
        ALOGE("%s: GnssVisibilityControl interface is unavailable", __func__);
        return false;
    }

    std::string os = "";
    bool first = true;
    for (const auto& proxyApp : proxyApps) {
        if (first) {
            first = false;
        } else {
            os += " ";
        }

        os += proxyApp;
    }
    ALOGE("%s: GnssVisibilityControl enableNfwLocationAccess: %s", __func__, os.c_str());

    return mGnssVisibilityControlInterface->vc_enable_nfw_location_access(
            os.c_str(), strlen(os.c_str()));
}

Return<bool> GnssVisibilityControl::setCallback(
        const sp<V1_0::IGnssVisibilityControlCallback>& callback) {
    ALOGE("%s: GnssVisibilityControl interface", __func__);
    if (mGnssVisibilityControlInterface == nullptr) {
        ALOGE("%s: GnssVisibilityControl interface is unavailable", __func__);
        return false;
    }

    sIGnssVisibilityControlCbIface = callback;

    return mGnssVisibilityControlInterface->vc_set_callback(&sGnssVisibilityControlCbs);
}

void GnssVisibilityControl::nfwNotifyCb(NfwNotification notification) {
    if (sIGnssVisibilityControlCbIface == nullptr) {
        ALOGE("%s: MeasurementCorrection Callback Interface configured incorrectly", __func__);
        return;
    }
    IGnssVisibilityControlCallback::NfwNotification nf;
    nf.proxyAppPackageName.setToExternal(
            notification.proxyAppPackageName, strlen(notification.proxyAppPackageName));
    nf.protocolStack = static_cast<IGnssVisibilityControlCallback::NfwProtocolStack>(
            notification.protocolStack);
    nf.otherProtocolStackName.setToExternal(
            notification.otherProtocolStackName, strlen(notification.otherProtocolStackName));
    nf.requestor = static_cast<IGnssVisibilityControlCallback::NfwRequestor>(
            notification.requestor);
    nf.requestorId.setToExternal(
            notification.requestorId, strlen(notification.requestorId));
    nf.responseType = static_cast<IGnssVisibilityControlCallback::NfwResponseType>(
            notification.responseType);
    nf.inEmergencyMode = notification.inEmergencyMode;
    nf.isCachedLocation = notification.isCachedLocation;

    auto ret = sIGnssVisibilityControlCbIface->nfwNotifyCb(nf);
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
    }
}

bool GnssVisibilityControl::isInEmergencySession() {
    if (sIGnssVisibilityControlCbIface == nullptr) {
        ALOGE("%s: MeasurementCorrection Callback Interface configured incorrectly", __func__);
        return false;
    }

    auto ret = sIGnssVisibilityControlCbIface->isInEmergencySession();
    if (!ret.isOk()) {
        ALOGE("%s: Unable to invoke callback", __func__);
        return false;
    }
    return true;
}


}  // namespace implementation
}  // namespace V1_0
}  // namespace visibility_control
}  // namespace gnss
}  // namespace hardware
}  // namespace android
