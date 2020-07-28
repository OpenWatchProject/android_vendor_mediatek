#ifndef VENDOR_MEDIATEK_HARDWARE_DFPS_V1_0_FPS_POLICY_SERVICE_BASE_H
#define VENDOR_MEDIATEK_HARDWARE_DFPS_V1_0_FPS_POLICY_SERVICE_BASE_H

#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicyService.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicy.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsInfo.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsVsync.h>

namespace vendor {
namespace mediatek {
namespace hardware{
namespace dfps {
namespace V1_0 {
namespace implementation {

using android::sp;

using android::hardware::hidl_string;

class FpsPolicyServiceBase {
public:
    virtual ~FpsPolicyServiceBase() {};

    // FpsPolicyClient interface
    virtual Error regPolicy(const sp<IFpsPolicy>& client, hidl_string name, FpsPolicyInfo info, int32_t *receiveFd) = 0;
    virtual Error unregPolicy(uint64_t sequence) = 0;
    virtual Error setFps(FpsPolicyInfo info) = 0;
    virtual Error getPanelInfo(PanelInfo *info) = 0;
    virtual Error getFpsRange(FpsRange *range) = 0;

    // FpsInfoClient interface
    virtual Error regInfo(const sp<IFpsInfo>& client, uint64_t id) = 0;
    virtual Error unregInfo(uint64_t id) = 0;
    virtual Error setSingleLayer(int32_t single) = 0;
    virtual Error setNumberDisplay(int32_t num) = 0;
    virtual Error setHwcInfo(HwcInfo info) = 0;
    virtual Error setInputWindows(hidl_string name, SimpleInputWindowInfo info) = 0;
    virtual Error setForegroundInfo(int32_t pid, hidl_string packageName) = 0;
    virtual Error setWindowFlag(int32_t flag, int32_t mask) = 0;
    virtual Error forbidAdjustingVsync(int32_t forbid) = 0;

    // FpsVsync interface
    virtual void regVsyncClient(const sp<IFpsVsync>& vsync) = 0;
    virtual void removeVsyncClient() = 0;
    virtual void enableCallback(bool enable) = 0;
    virtual Error enableTracker(int32_t enable) = 0;
    virtual void dumpInfo(uint32_t* len, char* string) = 0;
};

}; // namespace implementation
}; // namespace V1_0
}; // namespace dfps
}; // namespace hardware
}; // namespace mediatek
}; // namespace vendor

#endif
