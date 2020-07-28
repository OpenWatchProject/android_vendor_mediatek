#ifndef VENDOR_MEDIATEK_HARDWARE_DFPS_V1_0_FPS_POLICY_CLIENT_H
#define VENDOR_MEDIATEK_HARDWARE_DFPS_V1_0_FPS_POLICY_CLIENT_H

#include "FpsPolicyServiceBase.h"

#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicyClient.h>

namespace vendor {
namespace mediatek {
namespace hardware{
namespace dfps {
namespace V1_0 {
namespace implementation {

using android::hardware::Return;
using android::hardware::hidl_string;
using android::sp;

class FpsPolicyClient : public IFpsPolicyClient {
public:
    FpsPolicyClient(FpsPolicyServiceBase& hal);
    virtual ~FpsPolicyClient();

    // IFpsPolicyClient interface
    Return<void> regPolicy(const sp<IFpsPolicy>& policy, const hidl_string& name, const FpsPolicyInfo& info, regPolicy_cb hidl_cb) override;
    Return<Error> unregPolicy(uint64_t sequence) override;
    Return<Error> setFps(const FpsPolicyInfo& info) override;
    Return<void> getPanelInfo(getPanelInfo_cb hidl_cb) override;
    Return<void> getFpsRange(const FpsRange& range, getFpsRange_cb hidl_cb) override;

private:
    FpsPolicyServiceBase& mHal;
};

}; // namespace implementation
}; // namespace V1_0
}; // namespace dfps
}; // namespace hardware
}; // namespace mediatek
}; // namespace vendor

#endif
