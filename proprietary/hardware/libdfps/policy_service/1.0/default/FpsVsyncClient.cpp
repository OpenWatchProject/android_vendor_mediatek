#define LOG_TAG "DfpsPassthrough"

#include "FpsVsyncClient.h"

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

FpsVsyncClient::FpsVsyncClient(FpsPolicyServiceBase& hal)
    : mHal(hal)
{
}

FpsVsyncClient::~FpsVsyncClient()
{
    mHal.removeVsyncClient();
}

void FpsVsyncClient::onSwVsyncChange(int32_t mode, int32_t fps)
{
    auto ret = mCallback->onSwVsyncChange(mode, fps);
    ALOGE_IF(!ret.isOk(), "failed to send onVsync: %s", ret.description().c_str());
}

Return<void> FpsVsyncClient::regVsync(const sp<IFpsVsync>& vsync, const sp<IVsyncCallback>& callback)
{
    // no locking as we require this function to be called only once
    mCallback = callback;
    mHal.enableCallback(callback != nullptr);
    mHal.regVsyncClient(vsync);

    return Void();
}

Return<Error> FpsVsyncClient::enableTracker(int32_t enable)
{
    Error err = mHal.enableTracker(enable);;

    return err;
}

Return<void> FpsVsyncClient::dumpInfo(dumpInfo_cb hidl_cb)
{
    uint32_t len = 0;
    mHal.dumpInfo(&len, nullptr);

    std::vector<char> buf(len + 1);
    mHal.dumpInfo(&len, buf.data());
    buf.resize(len + 1);
    buf[len] = '\0';

    hidl_string buf_reply;
    buf_reply.setToExternal(buf.data(), len);
    hidl_cb(buf_reply);

    return Void();
}

}; // namespace implementation
}; // namespace V1_0
}; // namespace dfps
}; // namespace hardware
}; // namespace mediatek
}; // namespace vendor
