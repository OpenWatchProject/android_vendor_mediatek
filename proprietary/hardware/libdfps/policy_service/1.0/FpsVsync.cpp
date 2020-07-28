#include <cutils/atomic.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include "dfps/FpsVsync.h"
#include "dfps/utils.h"
#include "dfps/dynamicfps_defs.h"

#define UNUSED(param) do { (void)(param); } while(0)

extern "C" {
    static void sw_vsync_change_hook(dfps_callback_data_t callbackData, int32_t mode, int32_t fps) {
        auto device = static_cast<android::FpsVsync*>(callbackData);
        device->callSwVsyncChange(mode, fps);
    }
}

namespace android {

using hardware::Return;
using hardware::Void;
using vendor::mediatek::hardware::dfps::V1_0::Error;
typedef android::hidl::base::V1_0::IBase HInterface;

class FpsVsyncServiceDeathRecipient : public ::android::hardware::hidl_death_recipient {
public:
    FpsVsyncServiceDeathRecipient(wp<FpsVsync> vsync) {
        extendObjectLifetime(OBJECT_LIFETIME_WEAK);
        mVsync = vsync;
    }

    ~FpsVsyncServiceDeathRecipient() {
    }

    void serviceDied(uint64_t, const wp<HInterface>&) {
        ALOGE("FpsPolicyService is died!!");
        sp<FpsVsync> vsync = mVsync.promote();
        if (vsync != NULL) {
            vsync->updateState(FpsVsync::STATE_SERVER_DIED);
        }
    }
private:
    wp<FpsVsync> mVsync;
};

FpsVsync::FpsVsync()
    : mInit(false)
    , mState(NO_ERROR)
    , mEnableTracker(false)
    , mStopOperation(false)
{
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    mState = assertStateLocked();
    connectServiceLocked(true);
#else
    UNUSED(mStopOperation);
#endif
}

FpsVsync::~FpsVsync()
{
}

status_t FpsVsync::assertStateLocked()
{
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    mVsync = new IFpsVsync();
#endif
    return NO_ERROR;
}

namespace {
class VsyncCallback : public IVsyncCallback {
public:
    VsyncCallback(FpsVsync* vsync) : mVsync(vsync) {}

    Return<void> onSwVsyncChange(int32_t mode, int32_t fps) override
    {
        sw_vsync_change_hook(mVsync, mode, fps);
        return Void();
    }

private:
    FpsVsync* mVsync;
};
}

void FpsVsync::updateState(int32_t flag)
{
    Mutex::Autolock l(mLock);
    if (flag & STATE_SERVER_DIED) {
        mState = DEAD_OBJECT;
        mService = NULL;
        mClient = NULL;
        mInit = false;
    }
}

status_t FpsVsync::connectServiceLocked(bool init)
{
    status_t res = NO_ERROR;

#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    if (mStopOperation) {
        return NO_INIT;
    }

    if (mState != NO_ERROR && mState != DEAD_OBJECT) {
        ALOGE("FpsVsync can not connect with wrong state: %d", mState);
        return mState;
    }

    if (mService == NULL) {
        mService = IFpsPolicyService::tryGetService();
        if (mService == NULL) {
            if (init) {
                ALOGI("FpsVsync can not find FpsPolicyService");
            } else if (mState != DEAD_OBJECT) {
                mStopOperation = true;
                return NO_INIT;
            }
            return NAME_NOT_FOUND;
        }

        sp<FpsVsyncServiceDeathRecipient> sdr = new FpsVsyncServiceDeathRecipient(this);
        if (sdr != NULL) {
            mService->linkToDeath(sdr, 0);
        }
        ALOGE("FpsVsync link done");
    }

    if (mService != NULL && mClient == NULL) {
        mService->createVsyncClient([&](const auto& tmpError, const auto& tmpClient)
            {
                res = static_cast<status_t>(tmpError);
                if (tmpError == Error::NONE) {
                    mClient = tmpClient;
                }
            });
        if (mClient == NULL) {
            ALOGE("failed to get FpsVsyncClient");
            return res;
        }
    }

    if (mClient != NULL && mInit == false) {
        sp<VsyncCallback> callback = new VsyncCallback(this);
        mClient->regVsync(mVsync, callback);
        mInit = true;
        if (mState == DEAD_OBJECT) {
            recoverStateLocked();
            mState = NO_ERROR;
        }
    }
#else
    UNUSED(init);
#endif

    return res;
}

void FpsVsync::registerSwVsyncChangeCallback(SwVsyncChangeCallback callback)
{
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    mSwVsyncChange = callback;
#else
    UNUSED(callback);
#endif
}

void FpsVsync::callSwVsyncChange(int32_t mode, int32_t fps)
{
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    if (mSwVsyncChange) {
        mSwVsyncChange(mode, fps);
    }
#else
    UNUSED(mode);
    UNUSED(fps);
#endif
}

void FpsVsync::enableTracker(int32_t enable)
{
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    status_t res = NO_ERROR;
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to enable tracker [%d]: wrong state %d", enable, res);
        return;
    }

    if (enable != mEnableTracker) {
        auto ret = mClient->enableTracker(enable);
        res = static_cast<status_t>(unwrapRet(ret));
        if (res != NO_ERROR) {
            ALOGW("Failed to enable tracker [%d]: %d", enable, res);
        }
        mEnableTracker = enable;
    }
#else
    UNUSED(enable);
#endif
}

void FpsVsync::dumpInfo(String8& result)
{
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    status_t res = NO_ERROR;
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to dump info: wrong state %d", res);
        return;
    }

    mClient->dumpInfo([&](const auto& tmpInfo) {
        result.append(tmpInfo.c_str());
    });
#else
    UNUSED(result);
#endif
}

void FpsVsync::recoverStateLocked()
{
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    status_t res = NO_ERROR;
    auto ret = mClient->enableTracker(mEnableTracker);
    res = static_cast<status_t>(unwrapRet(ret));
    if (res != NO_ERROR) {
        ALOGW("Failed to recover setting of FpsVsync");
    }
#endif
}

FpsVsyncApi* createFpsVsyncApi() {
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    return new FpsVsync();
#else
    return NULL;
#endif
}

};
