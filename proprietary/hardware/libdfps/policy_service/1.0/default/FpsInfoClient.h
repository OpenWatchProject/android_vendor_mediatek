#ifndef VENDOR_MEDIATEK_HARDWARE_DFPS_V1_0_FPS_INFO_CLIENT_H
#define VENDOR_MEDIATEK_HARDWARE_DFPS_V1_0_FPS_INFO_CLIENT_H

#include "FpsPolicyServiceBase.h"

#include <vendor/mediatek/hardware/dfps/1.0/IFpsInfoClient.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsInfo.h>

namespace vendor {
namespace mediatek {
namespace hardware{
namespace dfps {
namespace V1_0 {
namespace implementation {

using android::hardware::Return;
using android::hardware::hidl_string;
using ::android::sp;

class FpsInfoClient : public IFpsInfoClient {
public:
    FpsInfoClient(FpsPolicyServiceBase& hal);
    virtual ~FpsInfoClient();

    // IFpsInfoClient interface
    Return<Error> regInfo(const sp<IFpsInfo>& info, uint64_t id) override;
    Return<Error> unregInfo(uint64_t id) override;
    Return<Error> setSingleLayer(int32_t single) override;
    Return<Error> setNumberDisplay(int32_t num) override;
    Return<Error> setHwcInfo(const HwcInfo& info) override;
    Return<Error> setInputWindows(const hidl_string& name, const SimpleInputWindowInfo& info) override;
    Return<Error> setForegroundInfo(int32_t pid, const hidl_string& packageName) override;
    Return<Error> setWindowFlag(int32_t flag, int32_t mask) override;
    Return<Error> forbidAdjustingVsync(int32_t forbid) override;

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
