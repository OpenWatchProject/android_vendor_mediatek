#ifndef VENDOR_MEDIATEK_HARDWARE_DFPS_V1_0_FPS_VSYNC_CLIENT_H
#define VENDOR_MEDIATEK_HARDWARE_DFPS_V1_0_FPS_VSYNC_CLIENT_H

#include "FpsPolicyServiceBase.h"

#include <vendor/mediatek/hardware/dfps/1.0/IFpsVsyncClient.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsVsyncClient.h>

namespace vendor {
namespace mediatek {
namespace hardware{
namespace dfps {
namespace V1_0 {
namespace implementation {

using android::hardware::Return;
using android::hardware::hidl_string;
using android::sp;

class FpsVsyncClient : public IFpsVsyncClient {
public:
    FpsVsyncClient(FpsPolicyServiceBase& hal);
    virtual ~FpsVsyncClient();

    void onSwVsyncChange(int32_t mode, int32_t fps);

    // IFpsPolicyClient interface
    Return<void> regVsync(const sp<IFpsVsync>& vsync, const sp<IVsyncCallback>& callback) override;
    Return<Error> enableTracker(int32_t enable) override;
    Return<void> dumpInfo(dumpInfo_cb hidl_cb) override;

private:
    FpsPolicyServiceBase& mHal;

    sp<IVsyncCallback> mCallback;
};

}; // namespace implementation
}; // namespace V1_0
}; // namespace dfps
}; // namespace hardware
}; // namespace mediatek
}; // namespace vendor

#endif
