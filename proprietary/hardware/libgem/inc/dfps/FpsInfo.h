#ifndef __FPSINFO_H__
#define __FPSINFO_H__

#include <utils/String8.h>
#include <utils/KeyedVector.h>
#include <utils/Singleton.h>
#include <utils/Looper.h>

#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicyService.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsInfoClient.h>
#include <vendor/mediatek/hardware/dfps/1.0/IFpsInfo.h>

namespace android {

using vendor::mediatek::hardware::dfps::V1_0::IFpsPolicyService;
using vendor::mediatek::hardware::dfps::V1_0::IFpsInfoClient;
using vendor::mediatek::hardware::dfps::V1_0::IFpsInfo;
using vendor::mediatek::hardware::dfps::V1_0::HwcInfo;
using vendor::mediatek::hardware::dfps::V1_0::SimpleInputWindowInfo;

class FpsInfo : public Thread {
public:
    class FpsInfoListener : public virtual RefBase{
    public:
        virtual ~FpsInfoListener() {};
        virtual void onFpsChanged(const int32_t fps) = 0;
    };
    FpsInfo();
    virtual ~FpsInfo();

    // updateState() is used to update the service state. Binder exception will
    // update this info, so does not called it directly.
    void updateState(int32_t flag);

    // setSingleLayer() is used to notify fps policy service that the number of layer
    // is in the screen. It should be called by HWC.
    status_t setSingleLayer(int32_t single);

    // setNumberDisplay() is used to notify fps policy service of how many displays
    // system have. It should be called by HWC.
    status_t setNumberDisplay(int32_t number);

    // setHwcInfo() is used to notify fps policy service of the status of HWC. It
    // should be called by HWC.
    status_t setHwcInfo(const HwcInfo& info);

    // setInputWindows is used to notify the fps policy service of the focused window.
    // It should be called by WMS.
    status_t setInputWindows(const String8& name, const SimpleInputWindowInfo& info);

    // regFpsInfoListener() is used to register the callback function to fps policy
    // service. When the system fps is changed, it will call this callback function.
    // Does not make free with this api. If the module which registe callback function
    // is dead, it may caused that refresh rate of system is wrong. The worst case is
    // that system will hang.
    status_t regFpsInfoListener(sp<FpsInfoListener> &cb);

    // When fps is changed, fps policy service uses this function to call registered
    // callback function. It should be called by fps policy service.
    virtual status_t cbGetFps(int32_t fps);

    // setForegroundInfo is used to set current foreground window infomation. It
    // be called by PerfService
    status_t setForegroundInfo(int32_t pid, String8& packageName);

    // setWindowFlag is used to set current status of window(like multi-window and
    // so on...). It should be called by WMS.
    status_t setWindowFlag(const int32_t flag, const int32_t mask);

    // forbidAdjustingVSync is used to ban variation of VSync period. It should be
    // called by VR Hal
    status_t forbidAdjustingVsync(bool forbid);

    enum {
        FLAG_MULTI_WINDOW = 0x01,
    };

    enum {
        STATE_SERVER_DIED = 0x01,
    };

private:
    status_t connectServiceLocked(bool init = false);
    void recoverStateLocked();

    virtual bool threadLoop();
    virtual void onFirstRef();

    mutable Mutex mLock;
    status_t mState;
    uint64_t mId;
    bool mInit;

    int32_t mSingleLayer;
    int32_t mNumDisplay;
    HwcInfo mHwcInfo;

    int32_t mForceWindowPid;
    String8 mForceWindowName;
    bool mForbidVSync;

    bool mStop;
    sp<Looper> mLooper;

    sp<IFpsPolicyService> mService;
    sp<IFpsInfoClient> mClient;
    sp<IFpsInfo> mInfo;
    bool mStopOperation;
};

};

#endif
