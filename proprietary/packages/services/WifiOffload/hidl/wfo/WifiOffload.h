#ifndef VENDOR_MEDIATEK_HARDWARE_WFO_V1_0_WIFIOFFLOAD_H
#define VENDOR_MEDIATEK_HARDWARE_WFO_V1_0_WIFIOFFLOAD_H

#include <vendor/mediatek/hardware/wfo/1.0/IWifiOffload.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace mediatek {
namespace hardware {
namespace wfo {
namespace V1_0 {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::wfo::V1_0::IWifiOffload;
using ::vendor::mediatek::hardware::wfo::V1_0::IWifiOffloadCallback;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct WifiOffload : public IWifiOffload {
    // Methods from ::vendor::mediatek::hardware::wfo::V1_0::IWifiOffload follow.
    Return<void> setWifiOffloadCallback(const sp<IWifiOffloadCallback>& wifiOffloadCallback) override;
    Return<void> nativeInit() override;
    Return<void> nativeClose() override;
    Return<int32_t> nativeGetRatType(uint8_t simIdx) override;
    Return<void> nativeSetWosProfile(uint8_t simId, bool volteEnabled, bool wfcEnabled, bool vilteEnabled,
        const hidl_string& fqdn, bool wifiEnabled, uint8_t wfcMode, uint8_t dataRoaming_enabled,
        bool isAllowTurnOffIms) override;
    Return<void> nativeSetWifiStatus(bool wifiConnected, const hidl_string& ifName, const hidl_string& ipv4,
        const hidl_string& ipv6, const hidl_string& mac) override;
    Return<void> nativeSetCallState(uint8_t simIdx, int32_t callId, int32_t callType, int32_t callState) override;
    Return<void> nativeSetServiceState(uint8_t mdIdx, uint8_t simIdx, bool isDataRoaming, uint8_t ratType,
        uint8_t serviceState, const hidl_string& locatedPlmn) override;
    Return<void> nativeSetVoiceState(uint8_t mdIdx, uint8_t simIdx,
            bool isVoiceRoaming, uint8_t voiceRatType, uint8_t voiceRegState) override;
    Return<void> nativeSetSimInfo(uint8_t simId, const hidl_string& imei, const hidl_string& imsi,
        const hidl_string& mccMnc, const hidl_string& impi, int32_t simType, bool simReady,
        bool isMainSim) override;
    Return<void> nativeSetWifiQuality(uint32_t rssi, uint32_t snr) override;
    Return<void> nativeSetWfcSupported(uint8_t simId, int32_t isWfcSupported) override;
    Return<void> nativeSetRadioState(uint8_t simId, uint8_t mdIdx, uint8_t radioState) override;
    Return<void> nativeSetLocationInfo(uint8_t simId, const hidl_string& countryId) override;
    Return<int32_t> nativeConnectToMal() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
};

extern "C" IWifiOffload* HIDL_FETCH_IWifiOffload(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace wfo
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor

#endif  // VENDOR_MEDIATEK_HARDWARE_WFO_V1_0_WIFIOFFLOAD_H
