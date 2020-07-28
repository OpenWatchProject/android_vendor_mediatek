#ifndef __FPSVSYNC_H__
#define __FPSVSYNC_H__

#include <utils/String8.h>
#include <utils/KeyedVector.h>
#include <utils/Singleton.h>
#include <utils/Looper.h>
#include <utils/RefBase.h>

#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicyService.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsVsyncClient.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsVsync.h>
#include <vendor/mediatek/hardware/dfps/1.0/IVsyncCallback.h>

#include "FpsVsyncApi.h"

namespace android {

using vendor::mediatek::hardware::dfps::V1_0::IFpsPolicyService;
using vendor::mediatek::hardware::dfps::V1_0::IFpsVsyncClient;
using vendor::mediatek::hardware::dfps::V1_0::IFpsVsync;
using vendor::mediatek::hardware::dfps::V1_0::IVsyncCallback;

//typedef std::function<void(int32_t, int32_t)> SwVsyncChangeCallback;

//class FpsVsync : public RefBase {
class FpsVsync : public FpsVsyncApi {
public:
    FpsVsync();
    ~FpsVsync();

    void registerSwVsyncChangeCallback(SwVsyncChangeCallback callback);
    void callSwVsyncChange(int32_t mode, int32_t fps);
    void enableTracker(int32_t enable);
    void dumpInfo(String8& result);

    void updateState(int32_t flag);

    enum {
        STATE_SERVER_DIED = 0x01,
    };

private:
    status_t assertStateLocked();
    status_t connectServiceLocked(bool init = false);
    void registerCallback();
    void recoverStateLocked();

    mutable Mutex mLock;
    bool mInit;
    status_t mState;
    int32_t mEnableTracker;

    SwVsyncChangeCallback mSwVsyncChange;

    sp<IFpsPolicyService> mService;
    sp<IFpsVsyncClient> mClient;
    sp<IFpsVsync> mVsync;
    bool mStopOperation;
};

};

#endif
