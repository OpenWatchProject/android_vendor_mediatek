#ifndef VENDOR_MEDIATEK_HARDWARE_DFPS_V1_0_DFPS_HAL_H
#define VENDOR_MEDIATEK_HARDWARE_DFPS_V1_0_DFPS_HAL_H

#include <mutex>
#include <vector>

#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicyService.h>
#include <dfps/dynamicfps_defs.h>

#include "FpsPolicyServiceBase.h"

namespace vendor {
namespace mediatek {
namespace hardware{
namespace dfps {
namespace V1_0 {
namespace implementation {

using android::hardware::Return;
using android::hardware::hidl_string;
using android::sp;
using android::wp;

class FpsVsyncClient;
class FpsPolicyDeathRecipient;
class FpsInfoDeathRecipient;

struct PolicyPackage {
    uint64_t mSequence;
    sp<IFpsPolicy> mPolicy;
    sp<FpsPolicyDeathRecipient> mRecipient;
};

struct InfoPackage {
    uint64_t mId;
    sp<IFpsInfo> mInfo;
    sp<FpsInfoDeathRecipient> mRecipient;
};

class DfpsHal : public IFpsPolicyService, public FpsPolicyServiceBase {
public:
    DfpsHal(const hw_module_t* module);
    virtual ~DfpsHal();

    // IFpsPolicyService interface
    Return<void> createPolicyClient(createPolicyClient_cb hidl_cb) override;
    Return<void> createInfoClient(createInfoClient_cb hidl_cb) override;
    Return<void> createVsyncClient(createVsyncClient_cb hidl_cb) override;
    Return<void> dumpDebugInfo(dumpDebugInfo_cb hidl_cb) override;

    // FpsPolicyCleint of FpsPolicyServiceBase interface
    Error regPolicy(const sp<IFpsPolicy>& client, hidl_string name, FpsPolicyInfo info, int32_t *receiveFd) override;
    Error unregPolicy(uint64_t sequence) override;
    Error setFps(FpsPolicyInfo info) override;
    Error getPanelInfo(PanelInfo *info) override;
    Error getFpsRange(FpsRange *range) override;

    // FpsInfoClient of FpsPolicyServiceBase interface
    Error regInfo(const sp<IFpsInfo>& client, uint64_t id) override;
    Error unregInfo(uint64_t id) override;
    Error setSingleLayer(int32_t single) override;
    Error setNumberDisplay(int32_t num) override;
    Error setHwcInfo(HwcInfo info) override;
    Error setInputWindows(hidl_string name, SimpleInputWindowInfo info) override;
    Error setForegroundInfo(int32_t pid, hidl_string packageName) override;
    Error setWindowFlag(int32_t flag, int32_t mask) override;
    Error enableTracker(int32_t enable) override;
    Error forbidAdjustingVsync(int32_t forbid) override;

    // FpsVsyncCleint of FpsPolicyServiceBase interface
    void regVsyncClient(const sp<IFpsVsync>& vsync) override;
    void removeVsyncClient() override;
    void enableCallback(bool enable) override;
    void dumpInfo(uint32_t* len, char* string) override;

    // for binder
    void unregPolicy(uint64_t sequence, int32_t flag);
    void unregInfo(uint64_t id, int32_t flag);

    void removeInfoPackage(uint64_t id);
    void removePolicyPackage(uint64_t sequence);
private:
    template<typename T>
    void initDispatch(dfps_function_descriptor_t desc, T* outPfn);
    void initDispatch();
    sp<FpsVsyncClient> getVsyncClient();

    static void swVsyncChangeHook(dfps_callback_data_t callbackData, int32_t mode, int32_t fps);

    void transferFpsPolicyInfo(dfps_fps_policy_info_t *dst, FpsPolicyInfo& src);
    void transferPanelInfo(PanelInfo *dst, dfps_panel_info_t& src);
    void transferFpsRange(FpsRange *dst, dfps_fps_range_t &src);
    void transferSimpleInputWindowInfo(dfps_simple_input_window_info_t *dst, SimpleInputWindowInfo& src);
    void transferHwcInfo(dfps_hwc_info_t *dst, HwcInfo& src);

    struct {
        DFPS_PFN_REG_POLICY regPolicy;
        DFPS_PFN_UNREG_POLICY unregPolicy;
        DFPS_PFN_SET_FPS setFps;
        DFPS_PFN_SET_SINGLE_LAYER setSingleLayer;
        DFPS_PFN_SET_NUMBER_DISPLAY setNumberDisplay;
        DFPS_PFN_SET_HWC_INFO setHwcInfo;
        DFPS_PFN_SET_INPUT_WINDOWS setInputWindows;
        DFPS_PFN_REG_INFO regInfo;
        DFPS_PFN_UNREG_INFO unregInfo;
        DFPS_PFN_GET_PANEL_INFO getPanelInfo;
        DFPS_PFN_SET_FOREGROUND_INFO setForegroundInfo;
        DFPS_PFN_GET_FPS_RANGE getFpsRange;
        DFPS_PFN_SET_WINDOW_FLAG setWindowFlag;
        DFPS_PFN_ENABLE_TRACKER enableTracker;
        DFPS_PFN_FORBID_ADJUST_VSYNC forbidAdjustVsync;
        DFPS_PFN_DUMP dump;
        DFPS_PFN_REGISTER_CALLBACK registerCallback;
    } mDispatch;

    std::mutex mClientMutex;
    wp<FpsVsyncClient> mClient;
    dfps_device_t* mDevice;

    std::vector<PolicyPackage> mPolicyPackageList;
    std::vector<InfoPackage> mInfoPackageList;
};

extern "C" IFpsPolicyService* HIDL_FETCH_IFpsPolicyService(const char* name);

}; // namespace implementation
}; // namespace V1_0
}; // namespace dfps
}; // namespace hardware
}; // namespace mediatek
}; // namespace vendor

#endif
