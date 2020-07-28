//#define LOG_TAG "FpsPolicy"

#include <cutils/atomic.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include "dfps/FpsPolicy.h"
#include "dfps/utils.h"

#include "dfps/string_def.h"

#define UNUSED(param) do { (void)(param); } while(0)

namespace android {

using hardware::Return;
using hardware::hidl_string;
using vendor::mediatek::hardware::dfps::V1_0::Error;
typedef android::hidl::base::V1_0::IBase HInterface;

class FpsPolicyServiceDeathRecipient : public ::android::hardware::hidl_death_recipient {
public:
    FpsPolicyServiceDeathRecipient(wp<FpsPolicy> policy) {
        extendObjectLifetime(OBJECT_LIFETIME_WEAK);
        mPolicy = policy;
    }

    ~FpsPolicyServiceDeathRecipient() {
    }

    void serviceDied(uint64_t, const wp<HInterface>&) {
        ALOGE("FpsPolicyService is died !!");
        sp<FpsPolicy> policy = mPolicy.promote();
        if (policy != NULL) {
            policy->updateState(FpsPolicy::STATE_SERVER_DIED);
        }
    }
private:
    wp<FpsPolicy> mPolicy;
};

inline uint64_t getUniqueSequence()
{
    static volatile int32_t nextSequence = 0;
    uint64_t sequence = static_cast<uint64_t>(getpid()) << 32;
    sequence |= static_cast<uint32_t>(android_atomic_inc(&nextSequence));
    return sequence;
}

FpsPolicy::FpsPolicy(int api, String8& name)
    : mInit(false)
    , mName("")
    , mState(NO_ERROR)
    , mReceivedFd(-1)
    , mService(NULL)
    , mClient(NULL)
    , mPolicy(NULL)
    , mLooper(NULL)
    , mCb(NULL)
    , mStop(true)
    , mStopOperation(false)
{
    memset(&mInfo, 0, sizeof(FpsPolicyInfo));
    memset(&mPanelInfo, 0, sizeof(PanelInfo));
    memset(&mRequest, 0, sizeof(FpsRequest));
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    mState = assertStateLocked(api, name);
    connectServiceLocked(true);
#else
    UNUSED(api);
    UNUSED(name);

    UNUSED(mInit);
    UNUSED(mState);
    UNUSED(mReceivedFd);
    UNUSED(mPanelInfo);
    UNUSED(mRequest);
    UNUSED(mStopOperation);
#endif
}

FpsPolicy::~FpsPolicy()
{
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    if (mInit && mClient != NULL) {
        mClient->unregPolicy(mInfo.sequence);
    }
#endif
}

status_t FpsPolicy::assertStateLocked(int api, String8& name)
{
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    mInfo.sequence = getUniqueSequence();
    mInfo.pid = getpid();
    mInfo.api = api;
    mInfo.fps = -1;
    mInfo.mode = MODE_DEFAULT;
    mInfo.flag = 0;
    mInfo.targetPid = 0;
    mInfo.glContextId = 0;

    mName.appendFormat("%s_%d_%s", getApiString(api), mInfo.pid, name.string());
    mPolicy = new IFpsPolicy();

    if (api >= API_MAXIMUM) {
        ALOGE("%s use an invalid api: %d", mName.string(), api);
        return INVALID_OPERATION;
    }
#else
    UNUSED(api);
    UNUSED(name);
#endif
    return NO_ERROR;
}

status_t FpsPolicy::setFps(int32_t fps, int32_t mode, int32_t targetPid, uint64_t glContextId)
{
    status_t res = NO_ERROR;

#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to set FPS %d  Mode %d: connect error %d",
                fps, mode, res);
        return res;
    }

    if (!isValidFpsLocked(fps, mode)) {
        ALOGW("Failed to set FPS %d  Mode %d: does not support this value", fps, mode);
        return BAD_VALUE;
    }

    if (mInfo.fps != fps || mInfo.mode != mode) {
        FpsPolicyInfo info(mInfo);
        info.fps = fps;
        info.mode = mode;
        info.targetPid = targetPid;
        info.glContextId = glContextId;
        auto ret = mClient->setFps(info);
        res = static_cast<status_t>(unwrapRet(ret));
        if (res == NO_ERROR) {
            mInfo.fps = fps;
            mInfo.mode = mode;
            mInfo.targetPid = targetPid;
            mInfo.glContextId = glContextId;
        }
        else {
            ALOGW("Failed to set FPS to %d: %d", fps, res);
        }
    }
#else
    UNUSED(fps);
    UNUSED(mode);
    UNUSED(targetPid);
    UNUSED(glContextId);
#endif

    return res;
}

status_t FpsPolicy::setVideoMode(bool enable)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    res = setFlag(enable ? FLAG_USE_VIDEO_MODE : FLAG_NONE, FLAG_USE_VIDEO_MODE);
#else
    UNUSED(enable);
#endif
    return res;
}

status_t FpsPolicy::setFlag(uint32_t flag, uint32_t mask)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    Mutex::Autolock l(mLock);
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to set flag to (f=%x|m=%x): connect error %d", flag, mask, res);
        return res;
    }

    uint32_t newFlag = flag & mask;
    if (mInfo.flag != newFlag) {
        FpsPolicyInfo info(mInfo);
        info.flag = newFlag;
        auto ret = mClient->setFps(info);
        res = static_cast<status_t>(unwrapRet(ret));
        if (res == NO_ERROR) {
            mInfo.flag = newFlag;
        }
        else {
            ALOGW("Failed to set flag to (f=%x|m=%x): %d", flag, mask, res);
        }
    }
#else
    UNUSED(flag);
    UNUSED(mask);
#endif
    return res;
}

status_t FpsPolicy::getPanelInfoNumber(int32_t* num)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    Mutex::Autolock l(mLock);
    res = getPanelInfoNumberLocked(num);
#else
    UNUSED(num);
#endif
    return res;
}

status_t FpsPolicy::getPanelInfoNumberLocked(int32_t* num)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res != NO_ERROR) {
        ALOGW("Failed to get panel info: connect error %d", res);
        return res;
    }

    if (mPanelInfo.num == 0) {
        mClient->getPanelInfo([&](const auto& tmpError, const auto& info)
            {
                res = static_cast<status_t>(tmpError);
                if (tmpError == Error::NONE) {
                    mPanelInfo = info;
                }
            });
        if (res != NO_ERROR) {
            ALOGW("Failed to get panel fps");
            *num = 1;
        } else {
            if (mRange.size()) {
                mRange.clear();
            }
            mRange.resize(mPanelInfo.num);
            for (int i = 0; i < mPanelInfo.num; i++) {
                mRange[i].index = i;
                mClient->getFpsRange(mRange[i], [&](const auto& tmpError, const auto& range)
                    {
                        res = static_cast<status_t>(tmpError);
                        if (tmpError == Error::NONE) {
                            mRange[i] = range;
                        }
                    });
                if (res != NO_ERROR) {
                    mRange[i].min = 60;
                    mRange[i].max = 60;
                    ALOGW("Failed to get fps range[%d]: %d", i, res);
                }
            }
            *num = mPanelInfo.num;
        }
    } else {
        *num = mPanelInfo.num;
    }
#else
    UNUSED(num);
#endif

    return res;
}

status_t FpsPolicy::getFpsRange(int32_t index, FpsRange* range)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    Mutex::Autolock l(mLock);
    if (mPanelInfo.num == 0) {
        range->min = 60;
        range->max = 60;
        res = NO_INIT;
    } else if (index >= mPanelInfo.num || index < 0) {
        range->min = 60;
        range->max = 60;
        res = BAD_VALUE;
    } else {
        range->min = mRange[index].min;
        range->max = mRange[index].max;
    }
#else
    UNUSED(index);
    UNUSED(range);
#endif

    return res;
}

void FpsPolicy::updateState(int32_t flag)
{
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
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

status_t FpsPolicy::connectServiceLocked(bool init)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    if (mStopOperation) {
        return NO_INIT;
    }

    if (mState != NO_ERROR && mState != DEAD_OBJECT) {
        ALOGE("%s can not connect with wrong state: %d", mName.string(), mState);
        return mState;
    }

    if (mService == NULL) {
        mService = IFpsPolicyService::tryGetService();
        if (mService == NULL) {
            if (init) {
                ALOGI("%s can not find FpsPolicyService", mName.string());
            } else if (mState != DEAD_OBJECT) {
                mStopOperation = true;
                return NO_INIT;
            }
            return NAME_NOT_FOUND;
        }

        sp<FpsPolicyServiceDeathRecipient> sdr = new FpsPolicyServiceDeathRecipient(this);
        if (sdr != NULL) {
            mService->linkToDeath(sdr, 0);
        }
    }

    if (mService != NULL && mClient == NULL) {
        mService->createPolicyClient([&](const auto& tmpError, const auto& tmpClient)
            {
                res = static_cast<status_t>(tmpError);
                if (tmpError == Error::NONE) {
                    mClient = tmpClient;
                }
            });
        if (mClient == NULL) {
            ALOGE("%s failed to get FpsPolicyClient", mName.string());
            return res;
        }
    }

    if (mClient != NULL && mInit == false) {
        hidl_string hidlName = mName.string();
        mClient->regPolicy(mPolicy, hidlName, mInfo, [&](const auto& tmpError, const auto& receiveFd)
            {
                res = static_cast<status_t>(tmpError);
                if (tmpError == Error::NONE) {
                    mReceivedFd = receiveFd;
                }
            });
        if (res == NO_ERROR) {
            mInit = true;
            if (mState == DEAD_OBJECT) {
                recoverStateLocked();
                mState = NO_ERROR;
            }
        } else if (res == ALREADY_EXISTS) {
            ALOGW("try to register an existed policy");
        } else {
            ALOGE("%s failed to register policy: %d", mName.string(), res);
        }
    }
#else
    UNUSED(init);
#endif

    return res;
}

int32_t FpsPolicy::isValidFpsLocked(int32_t fps, int32_t mode)
{
    int res = 0;
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    if (fps == -1) {
        return 1;
    }

    if (mode != MODE_ARR) {
        return 1;
    }

    if (mPanelInfo.num == 0) {
        int num;
        getPanelInfoNumberLocked(&num);
    }

    if (mPanelInfo.num == 0) {
        if (fps == 60) {
            res = 1;
        }
    } else {
        for (int i = 0; i < mPanelInfo.num; i++) {
            if (mRange[i].min <= fps && fps <= mRange[i].max) {
                res = 1;
                break;
            }
        }
    }
#else
    UNUSED(fps);
    UNUSED(mode);
#endif

    return res;
}

int FpsPolicy::cb_fpsRequestReceiver(int fd, int events, void* data)
{
    int res = 0;
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    sp<FpsPolicy> sfp = reinterpret_cast< FpsPolicy* >(data);
    if (sfp != NULL) {
        res = sfp->fpsRequestReceive(fd, events);
    }
#else
    UNUSED(fd);
    UNUSED(events);
    UNUSED(data);
#endif
    return res;
}

int FpsPolicy::fpsRequestReceive(int /*fd*/, int /*events*/)
{
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    //TODO: need to use other way to get info
    //DupBitTube::recvObjects(mChannel, &mRequest, 1);
    if (mCb != NULL) {
        mCb->onRequestChanged(mRequest);
    }
#endif
    return 1;
}

bool FpsPolicy::threadLoop()
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

void FpsPolicy::onFirstRef()
{
}

status_t FpsPolicy::registerFpsRequestListener(const sp<FpsRequestListener>& cb)
{
    status_t res = NO_ERROR;
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    Mutex::Autolock l(mLock);
    if (mCb != NULL) {
        ALOGW("Failed to register request listener: cb is not null");
        return INVALID_OPERATION;
    }

    res = connectServiceLocked();
    if (res == NO_INIT) {
        return res;
    } else if (res == NO_ERROR) {
        if (mLooper == NULL) {
            mCb = cb;
            mLooper = new Looper(true);
            //TODO: need to use other way to get info
            //mLooper->addFd(mChannel->getFd(), 0, Looper::EVENT_INPUT,
            //               FpsPolicy::cb_fpsRequestReceiver, this);
            mStop = false;
            run("receiveFpsRequest");
        }
    } else {
        ALOGW("Failed to register request listener: %d", res);
    }
#else
    UNUSED(cb);
#endif
    return res;
}

void FpsPolicy::cancelFpsRequestListener()
{
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    {
        Mutex::Autolock l(mLock);
        if (mStop == false) {
            mStop = true;
            requestExit();
            if (mLooper != NULL) {
                mLooper->wake();
            }
        }
    }
    join();
    {
        Mutex::Autolock l(mLock);
        if (mLooper != NULL) {
            mLooper = NULL;
        }
        mCb = NULL;
    }
#endif
}

void FpsPolicy::recoverStateLocked()
{
#ifdef MTK_DYNAMIC_FPS_POLICY_SUPPORT
    status_t res = NO_ERROR;
    auto ret = mClient->setFps(mInfo);
    res = static_cast<status_t>(unwrapRet(ret));
    if (res != NO_ERROR) {
        ALOGW("Failed to recover setting of FpsPolicy %s",  mName.string());
    }
#endif
}

};
