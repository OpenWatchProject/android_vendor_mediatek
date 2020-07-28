#ifndef __FPSPOLICY_H__
#define __FPSPOLICY_H__

#include <vector>

#include <utils/String8.h>
#include <utils/KeyedVector.h>
#include <utils/Singleton.h>
#include <utils/Looper.h>

#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicyService.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicyClient.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicy.h>

namespace android {

using vendor::mediatek::hardware::dfps::V1_0::IFpsPolicyService;
using vendor::mediatek::hardware::dfps::V1_0::IFpsPolicyClient;
using vendor::mediatek::hardware::dfps::V1_0::IFpsPolicy;
using vendor::mediatek::hardware::dfps::V1_0::FpsPolicyInfo;
using vendor::mediatek::hardware::dfps::V1_0::FpsRange;
using vendor::mediatek::hardware::dfps::V1_0::PanelInfo;
using vendor::mediatek::hardware::dfps::V1_0::FpsRequest;

class FpsPolicy : public Thread {
public:
    class FpsRequestListener : public virtual RefBase{
    public:
        virtual ~FpsRequestListener() {};
        virtual void onRequestChanged(const FpsRequest& request) = 0;
    };

    // FpsPolicy is used to adjust system fps. Each api has different priority,
    // so module shall use corrent api, or else it may cause that system will
    // choose wrong fps setting.
    FpsPolicy(int32_t api, String8& name);
    virtual ~FpsPolicy();

    // setFps() is used to set expected fps, but this api does not guarantee
    // that the system will change refresh rate to expected fps.
    status_t setFps(int32_t fps, int32_t mode = MODE_DEFAULT, int32_t targetPid = 0,
                    uint64_t glContextId = 0);

    // setVideoMode() is used to change the display to video mode or command mode.
    status_t setVideoMode(bool enable);

    // setFlag() is used to control some feature.
    status_t setFlag(uint32_t flag, uint32_t mask);

    // getPanelInfoNumber() is used to get the number of FpsRange
    status_t getPanelInfoNumber(int32_t *num);

    // getFpsRange() is used to get the fps range of specific index
    status_t getFpsRange(int32_t index, FpsRange *range);

    // updateState() is used to update the service state. Binder exception will
    // update this info, so does not call it directly.
    void updateState(int32_t flag);

    status_t registerFpsRequestListener(const sp<FpsRequestListener>& cb);

    void cancelFpsRequestListener();

    // this definition need to be the same with rrc_drv.h
    enum {
        API_UNKNOWN = -1,
        API_GIFT = 0,
        API_VIDEO,
        API_RRC_INPUT,
        API_RRC_VIDEO,
        API_THERMAL,
        API_LOADING,
        API_WHITELIST,
        API_MAXIMUM,
    };

    enum {
        MODE_DEFAULT = 0,
        MODE_FRR,
        MODE_ARR,
        MODE_INTERNAL_SW,
        MODE_MAXIMUM,
    };

    enum {
        STATE_SERVER_DIED = 0x01,
    };

    enum {
        FLAG_NONE = 0x00,
        FLAG_USE_VIDEO_MODE = 0x01,
    };

private:
    status_t assertStateLocked(int api, String8& name);
    status_t connectServiceLocked(bool init = false);
    int32_t isValidFpsLocked(int32_t fps, int32_t mode);
    status_t getPanelInfoNumberLocked(int32_t *num);
    static int cb_fpsRequestReceiver(int fd, int events, void* data);
    int fpsRequestReceive(int fd, int events);
    void recoverStateLocked();

    virtual bool threadLoop();
    virtual void onFirstRef();

    bool mInit;
    String8 mName;
    status_t mState;
    int32_t mReceivedFd;
    FpsPolicyInfo mInfo;

    PanelInfo mPanelInfo;
    std::vector<FpsRange> mRange;

    sp<IFpsPolicyService> mService;
    sp<IFpsPolicyClient> mClient;
    sp<IFpsPolicy> mPolicy;

    mutable Mutex mLock;
    sp<Looper> mLooper;
    FpsRequest mRequest;
    sp<FpsRequestListener> mCb;
    bool mStop;
    bool mStopOperation;
};

};

#endif
