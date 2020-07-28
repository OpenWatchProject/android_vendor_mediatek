#define LOG_TAG "DfpsPassthrough"

#include "FpsInfoClient.h"

#include <log/log.h>

namespace vendor {
namespace mediatek {
namespace hardware{
namespace dfps {
namespace V1_0 {
namespace implementation {

FpsInfoClient::FpsInfoClient(FpsPolicyServiceBase& hal)
    : mHal(hal)
{
}

FpsInfoClient::~FpsInfoClient()
{
}

Return<Error> FpsInfoClient::regInfo(const sp<IFpsInfo>& info, uint64_t id)
{
    Error err = mHal.regInfo(info, id);

    return err;
}

Return<Error> FpsInfoClient::unregInfo(uint64_t id)
{
    Error err = mHal.unregInfo(id);

    return err;
}

Return<Error> FpsInfoClient::setSingleLayer(int32_t single)
{
    Error err = mHal.setSingleLayer(single);

    return err;
}

Return<Error> FpsInfoClient::setNumberDisplay(int32_t num)
{
    Error err = mHal.setNumberDisplay(num);

    return err;
}

Return<Error> FpsInfoClient::setHwcInfo(const HwcInfo& info)
{
    Error err = mHal.setHwcInfo(info);;

    return err;
}

Return<Error> FpsInfoClient::setInputWindows(const hidl_string& name, const SimpleInputWindowInfo& info)
{
    Error err = mHal.setInputWindows(name, info);

    return err;
}

Return<Error> FpsInfoClient::setForegroundInfo(int32_t pid, const hidl_string& packageName)
{
    Error err = mHal.setForegroundInfo(pid, packageName);

    return err;
}

Return<Error> FpsInfoClient::setWindowFlag(int32_t flag, int32_t mask)
{
    Error err = mHal.setWindowFlag(flag, mask);

    return err;
}

Return<Error> FpsInfoClient::forbidAdjustingVsync(int32_t forbid)
{
    Error err = mHal.forbidAdjustingVsync(forbid);

    return err;
}

}; // namespace implementation
}; // namespace V1_0
}; // namespace dfps
}; // namespace hardware
}; // namespace mediatek
}; // namespace vendor
