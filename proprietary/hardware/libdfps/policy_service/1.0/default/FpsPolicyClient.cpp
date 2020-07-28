#define LOG_TAG "DfpsPassthrough"

#include "FpsPolicyClient.h"

#include <type_traits>
#include <log/log.h>

namespace vendor {
namespace mediatek {
namespace hardware{
namespace dfps {
namespace V1_0 {
namespace implementation {

using android::hardware::Void;
using android::wp;

FpsPolicyClient::FpsPolicyClient(FpsPolicyServiceBase& hal)
    : mHal(hal)
{
}

FpsPolicyClient::~FpsPolicyClient()
{
}

Return<void> FpsPolicyClient::regPolicy(const sp<IFpsPolicy>& policy, const hidl_string& name, const FpsPolicyInfo& info, regPolicy_cb hidl_cb)
{
    int32_t receiveFd = -1;
    Error err = mHal.regPolicy(policy, name, info, &receiveFd);
    hidl_cb(err, receiveFd);

    return Void();
}

Return<Error> FpsPolicyClient::unregPolicy(uint64_t sequence)
{
    Error err = mHal.unregPolicy(sequence);

    return err;
}

Return<Error> FpsPolicyClient::setFps(const FpsPolicyInfo& info)
{
    Error err = mHal.setFps(info);

    return err;
}

Return<void> FpsPolicyClient::getPanelInfo(getPanelInfo_cb hidl_cb)
{
    PanelInfo info;
    Error err = mHal.getPanelInfo(&info);;
    hidl_cb(err, info);

    return Void();
}

Return<void> FpsPolicyClient::getFpsRange(const FpsRange& range, getFpsRange_cb hidl_cb)
{
    FpsRange rangeDst;
    rangeDst.index = range.index;
    Error err = mHal.getFpsRange(&rangeDst);
    hidl_cb(err, rangeDst);

    return Void();
}

}; // namespace implementation
}; // namespace V1_0
}; // namespace dfps
}; // namespace hardware
}; // namespace mediatek
}; // namespace vendor
