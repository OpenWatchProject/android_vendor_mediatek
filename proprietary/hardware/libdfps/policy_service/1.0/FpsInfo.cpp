#include <inttypes.h>

#include <cutils/atomic.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include "dfps/FpsInfo.h"
#include "dfps/utils.h"

#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS 1
#endif

#define UNUSED(param) do { (void)(param); } while(0)

namespace android {

using hardware::Return;
using hardware::hidl_string;
using vendor::mediatek::hardware::dfps::V1_0::Error;
typedef android::hidl::base::V1_0::IBase HInterface;

class FpsInfoServiceDeathRecipient : public ::android::hardware::hidl_death_recipient {
public:
    FpsInfoServiceDeathRecipient(wp<FpsInfo> info) {
        extendObjectLifetime(OBJECT_LIFETIME_WEAK);
        mInfo = info;
    }

    ~FpsInfoServiceDeathRecipient() {
    }

    void serviceDied(uint64_t, const wp<HInterface>&) {
        ALOGE("FpsPolicyService is died!!");
        sp<FpsInfo> info = mInfo.promote();
        if (info != NULL) {
            info->updateState(FpsInfo::STATE_SERVER_DIED);
        }
    }
private:
    wp<FpsInfo> mInfo;
};

inline uint64_t getUniqueInfoId()
{
    static volatile int32_t nextId = 0;
    uint64_t id = static_cast<uint64_t>(getpid()) << 32;
    id |= static_cast<uint32_t>(android_atomic_inc(&nextId));
    return id;
}

FpsInfo::FpsInfo()
    : mState(NO_ERROR)
    , mId(0)
    , mInit(false)
    , mSingleLayer(0)
    , mNumDisplay(0)
    , mForceWindowPid(0)
    , mForbidVSync(false)
    , mStop(true)
    , mService(NULL)
    , mClient(NULL)
    , mInfo(NULL)
    , mStopOperation(false)
{
    memset(&mHwcInfo, 0, sizeof(mHwcInfo));
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    mId = getUniqueInfoId();
    mInfo = new IFpsInfo();
    connectServiceLocked(true);
#else
    UNUSED(mState);
    UNUSED(mId);
    UNUSED(mInit);
    UNUSED(mSingleLayer);
    UNUSED(mNumDisplay);
    UNUSED(mForceWindowPid);
    UNUSED(mForbidVSync);
    UNUSED(mHwcInfo);
    UNUSED(mStopOperation);
#endif
}

FpsInfo::~FpsInfo()
{
}

status_t FpsInfo::connectServiceLocked(bool init)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    if (mStopOperation) {
        return NO_INIT;
    }

    if (mState != NO_ERROR && mState != DEAD_OBJECT) {
        ALOGE("%" PRIx64 " can not connect with wrong state: %d", mId, mState);
        return mState;
    }

    if (mService == NULL) {
         mService = IFpsPolicyService::tryGetService();
         if (mService == NULL) {
             if (init) {
                 ALOGE("FpsInfo: %" PRIx64 " can not find FpsPolicyService", mId);
             } else if (mState != DEAD_OBJECT) {
                 mStopOperation = true;
                 return NO_INIT;
             }
             return NAME_NOT_FOUND;
         }

         sp<FpsInfoServiceDeathRecipient> sdr = new FpsInfoServiceDeathRecipient(this);
         if (sdr != NULL) {
             mService->linkToDeath(sdr, 0);
         }
    }

    if (mService != NULL && mClient == NULL) {
         mService->createInfoClient([&](const auto& tmpError, const auto& tmpClient)
             {
                 res = static_cast<status_t>(tmpError);
                 if (tmpError == Error::NONE) {
                     mClient = tmpClient;
                 }
             });
         if (mClient == NULL) {
             ALOGE("FpsInfo: %" PRIx64 " failed to get FpsInfoClient", mId);
             return res;
         }
    }

    if (mClient != NULL && mInit == false) {
        auto ret = mClient->regInfo(mInfo, getUniqueInfoId());
        res = static_cast<status_t>(unwrapRet(ret));
        if (res == NO_ERROR) {
            mInit = true;
            if (mState == DEAD_OBJECT) {
                recoverStateLocked();
                mState = NO_ERROR;
            }
        } else if (res == ALREADY_EXISTS) {
            ALOGW("FpsInfo: try to register an existed info");
        } else {
            ALOGE("FpsInfo: %" PRIx64 " failed to register info: %d", mId, res);
        }
    }
#else
    UNUSED(init);
#endif

    return res;
}

void FpsInfo::updateState(int32_t flag)
{
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    Mutex::Autolock l(mLock);
    if (flag & STATE_SERVER_DIED) {
        mState = DEAD_OBJECT;
        mService = NULL;
        mClient = NULL;
        mInit = false;
    }
#else
    UNUSED(flag);
#endif
}

status_t FpsInfo::setSingleLayer(const int32_t single)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to set single layer %d: wrong state %d", single, res);
        return res;
    }

    if (mSingleLayer != single) {
        auto ret = mClient->setSingleLayer(single);
        res = static_cast<status_t>(unwrapRet(ret));
        if (res == NO_ERROR) {
            mSingleLayer = single;
        } else {
            ALOGW("Failed to set single layer to %d: %d", single, res);
        }
    }
#else
    UNUSED(single);
#endif

    return res;
}

status_t FpsInfo::setNumberDisplay(const int32_t number)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to set number of display %d: wrong state %d", number, res);
        return res;
    }

    if (mNumDisplay != number) {
        auto ret = mClient->setNumberDisplay(number);
        res = static_cast<status_t>(unwrapRet(ret));
        if (res == NO_ERROR) {
            mNumDisplay = number;
        } else {
            ALOGW("Failed to set number of display to %d: %d", number, res);
        }
    }
#else
    UNUSED(number);
#endif

    return res;
}

status_t FpsInfo::setHwcInfo(const HwcInfo& info)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to set HwcInfo[s:%d|d:%d]: wrong state %d",
                info.singleLayer, info.numDisplay, res);
        return res;
    }

    if (info.singleLayer != mHwcInfo.singleLayer || info.numDisplay != mHwcInfo.numDisplay) {
        auto ret = mClient->setHwcInfo(info);
        res = static_cast<status_t>(unwrapRet(ret));
        if (res == NO_ERROR) {
            mHwcInfo = info;
        } else {
            ALOGW("Failed to set HwcInfo[s:%d|d:%d]: %d",
                    info.singleLayer, info.numDisplay, res);
        }
    }
#else
    UNUSED(info);
#endif

    return res;
}

status_t FpsInfo::setInputWindows(const String8& name, const SimpleInputWindowInfo& info)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to set InputWindowInfo[%s]: wrong state %d",
                name.string(), res);
        return res;
    }

    if (mForceWindowPid != info.ownerPid) {
        hidl_string hidlName = name.string();
        auto ret = mClient->setInputWindows(hidlName, info);
        res = static_cast<status_t>(unwrapRet(ret));
        if (res == NO_ERROR) {
            mForceWindowPid = info.ownerPid;
            mForceWindowName = name;
        } else {
            ALOGW("Failed to set InputWindowInfo[%s]: %d",
                    name.string(), res);
        }
    }
#else
    UNUSED(name);
    UNUSED(info);
#endif

    return res;
}

status_t FpsInfo::regFpsInfoListener(sp<FpsInfoListener> &/*cb*/)
{
    status_t res = NO_ERROR;

    return res;
}

status_t FpsInfo::cbGetFps(int32_t /*fps*/)
{
    return NO_ERROR;
}

status_t FpsInfo::setForegroundInfo(int32_t pid, String8& packageName)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to set fg info [%d|%s]: wrong state %d", pid, packageName.string(), res);
        return res;
    }

    hidl_string hidlName = packageName.string();
    auto ret = mClient->setForegroundInfo(pid, hidlName);
    res = static_cast<status_t>(unwrapRet(ret));
    if (res != NO_ERROR) {
        ALOGW("Failed to set fg info [%d|%s]: %d", pid, packageName.string(), res);
    }
#else
    UNUSED(pid);
    UNUSED(packageName);
#endif

    return res;
}

status_t FpsInfo::setWindowFlag(const int32_t flag, const int32_t mask)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to set window flag [%08x|%08x]: wrong state %d", flag, mask, res);
        return res;
    }

    auto ret = mClient->setWindowFlag(flag, mask);
    res = static_cast<status_t>(unwrapRet(ret));
    if (res != NO_ERROR) {
        ALOGW("Failed to set window flag [%08x|%08x]: %d", flag, mask, res);
    }
#else
    UNUSED(flag);
    UNUSED(mask);
#endif

    return res;
}

status_t FpsInfo::forbidAdjustingVsync(bool forbid)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to forbid adjusting vsync [%d]: wrong state %d", forbid, res);
        return res;
    }

    if (forbid != mForbidVSync) {
        auto ret = mClient->forbidAdjustingVsync(forbid);
        res = static_cast<status_t>(unwrapRet(ret));
        if (res != NO_ERROR) {
            ALOGW("Failed to forbid vsync [%d]: %d", forbid, res);
        }
        mForbidVSync = forbid;
    }
#else
    UNUSED(forbid);
#endif

    return res;
}

bool FpsInfo::threadLoop()
{
    do {
        {
            Mutex::Autolock l(mLock);
            if (mStop) {
                return false;
            }
        }
        int32_t ret = mLooper->pollOnce(-1);
        switch (ret) {
            case Looper::POLL_WAKE:
            case Looper::POLL_CALLBACK:
                continue;
            case Looper::POLL_ERROR:
                ALOGE("Looper::POLL_ERROR");
                continue;
            case Looper::POLL_TIMEOUT:
                // timeout (should not happen)
                continue;
            default:
                // should not happen
                ALOGE("Looper::pollOnce() returned unknown status %d", ret);
                continue;
        }
    } while (true);
    return true;
}

void FpsInfo::onFirstRef()
{
}

void FpsInfo::recoverStateLocked()
{
#ifdef MTK_DYNAMIC_FPS_FRAMEWORK_SUPPORT
    status_t res = NO_ERROR;
    auto ret = mClient->setSingleLayer(mSingleLayer);
    res |= static_cast<status_t>(unwrapRet(ret));

    ret = mClient->setNumberDisplay(mNumDisplay);
    res |= static_cast<status_t>(unwrapRet(ret));

    ret = mClient->forbidAdjustingVsync(mForbidVSync);
    res |= static_cast<status_t>(unwrapRet(ret));

    ret = mClient->setHwcInfo(mHwcInfo);
    res |= static_cast<status_t>(unwrapRet(ret));

    SimpleInputWindowInfo info;
    info.ownerPid = mForceWindowPid;
    hidl_string hidlName = mForceWindowName.string();
    ret = mClient->setInputWindows(hidlName, info);
    res = static_cast<status_t>(unwrapRet(ret));
    if (res != NO_ERROR) {
        ALOGW("Failed to recover setting of FpsInfo");
    }
#endif
}

};
