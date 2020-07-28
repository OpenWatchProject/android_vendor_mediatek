#include "vsync_enhance/DispSyncEnhancement.h"

#include <dlfcn.h>
#include <inttypes.h>
#include <math.h>

#include <android-base/stringprintf.h>
#include <log/log.h>
#include <utils/String8.h>

#include <vendor/mediatek/hardware/gpu/1.0/IGraphicExt.h>

using vendor::mediatek::hardware::gpu::V1_0::IGraphicExt;
using vendor::mediatek::hardware::gpu::V1_0::IGraphicExtCallback;

using android::base::StringAppendF;

namespace android {

class GpuHidlCallbackBridge : public IGraphicExtCallback {
public:
    GpuHidlCallbackBridge(IGpuHidlCallback *callback) : mCallBack(callback) {}
    android::hardware::Return<void> sendVsyncOffsetFunc(int32_t /*protocol*/, int32_t i32Data) {
        mCallBack->onVSyncOffsetChange(i32Data);
        return hardware::Void();
    }

private:
    IGpuHidlCallback* mCallBack;
};

DispSyncEnhancement::DispSyncEnhancement()
    : mVSyncMode(VSYNC_MODE_CALIBRATED_SW_VSYNC)
    , mFps(DS_DEFAULT_FPS)
    , mAppPhase(0)
    , mSfPhase(0)
    , mSetVSyncMode(NULL)
    , mHasAnyEventListeners(NULL)
    , mAddResyncSample(NULL)
    , mAddEventListener(NULL)
    , mRemoveEventListener(NULL)
    , mOnSwVsyncChange(NULL)
    , mBridge(NULL)
#ifdef MTK_DYNAMIC_FPS_FW_SUPPORT
    , mFpsPolicyHandle(NULL)
    , mFpsVsync(NULL)
#endif
{
#ifdef MTK_DYNAMIC_FPS_FW_SUPPORT
    typedef FpsVsyncApi* (*createFpsVsyncPrototype)();
    mFpsPolicyHandle = dlopen("libfpspolicy_fw.so", RTLD_LAZY);
    if (mFpsPolicyHandle) {
        createFpsVsyncPrototype creatPtr = reinterpret_cast<createFpsVsyncPrototype>(dlsym(mFpsPolicyHandle, "createFpsVsyncApi"));
        if (creatPtr) {
            mFpsVsync = creatPtr();
        } else {
            ALOGW("Failed to get function: createFpsVsyncApi");
        }
    } else {
        ALOGW("Failed to load libfpspolicy_fw.so");
    }
#endif
}

DispSyncEnhancement::~DispSyncEnhancement() {
    if (mBridge != NULL) {
        delete mBridge;
    }
#ifdef MTK_DYNAMIC_FPS_FW_SUPPORT
    if (mFpsPolicyHandle) {
        dlclose(mFpsPolicyHandle);
    }
#endif
}

void DispSyncEnhancement::registerFunction(struct DispSyncEnhancementFunctionList* list) {
    mSetVSyncMode = list->setVSyncMode;
    mHasAnyEventListeners = list->hasAnyEventListeners;
    mAddResyncSample = list->addResyncSample;
    mAddEventListener = list->addEventListener;
    mEnableHardwareVsync = list->enableHardwareVsync;
    mRemoveEventListener = list->removeEventListener;
    mOnSwVsyncChange = list->onSwVsyncChange;
#ifdef MTK_DYNAMIC_FPS_FW_SUPPORT
    if (mFpsVsync != NULL) {
        mFpsVsync->registerSwVsyncChangeCallback(mOnSwVsyncChange);
    }
#endif
}

status_t DispSyncEnhancement::setVSyncMode(int32_t mode, int32_t fps, nsecs_t *period, nsecs_t *phase, nsecs_t *referenceTime) {
    status_t res = NO_ERROR;
    int32_t currentFps = getFps();
    int32_t currentMode = getVSyncMode();
    if (currentMode != mode || currentFps != fps) {
        if (fps <= 0) {
            fps = DS_DEFAULT_FPS;
        }
        calculateModel(mode, fps, period, phase, referenceTime);
        if (mSetVSyncMode) {
            res = mSetVSyncMode(mode, fps, *period, *phase, *referenceTime);
        }
        {
            Mutex::Autolock lock(mLock);
            mVSyncMode = mode;
            mFps = fps;
        }
    }
    return res;
}

bool DispSyncEnhancement::addPresentFence(bool* res) {
    int32_t currentMode = getVSyncMode();
    if (currentMode == VSYNC_MODE_PASS_HW_VSYNC) {
        if (mHasAnyEventListeners == NULL) {
            return false;
        }
        *res =  mHasAnyEventListeners();
        return true;
    } else if  (currentMode == VSYNC_MODE_INTERNAL_SW_VSYNC) {
        *res =  false;
        return true;
    }
    return false;
}

bool DispSyncEnhancement::addResyncSample(bool* res, nsecs_t timestamp, nsecs_t* period, nsecs_t* phase, nsecs_t* referenceTime) {
    int32_t currentFps = getFps();
    int32_t currentMode = getVSyncMode();
    if (currentMode == VSYNC_MODE_PASS_HW_VSYNC) {
        if (mAddResyncSample == NULL || mHasAnyEventListeners == NULL) {
            return false;
        }
        calculateHwModel(timestamp, currentFps, period, phase, referenceTime);
        mAddResyncSample(*period, *phase, *referenceTime);
        *res =  mHasAnyEventListeners();
        return true;
    } else if (currentMode == VSYNC_MODE_INTERNAL_SW_VSYNC) {
        *res =  false;
        return true;
    }
    return false;
}

bool DispSyncEnhancement::addEventListener(status_t* res, Mutex* mutex, const char* name, nsecs_t phase, DispSync::Callback* callback) {
    if (mHasAnyEventListeners == NULL || mAddEventListener == NULL || mEnableHardwareVsync == NULL) {
        return false;
    }

    bool firstListener = false;
    {
        Mutex::Autolock lock(mutex);
        if (!mHasAnyEventListeners()) {
            firstListener = true;
        }
        *res = mAddEventListener(name, phase, callback);
        storeVsyncSourcePhase(name, phase);
    }
    if (firstListener) {
#ifdef MTK_DYNAMIC_FPS_FW_SUPPORT
        if (mFpsVsync != NULL) {
            mFpsVsync->enableTracker(true);
        }
#endif
        if (getVSyncMode() == VSYNC_MODE_PASS_HW_VSYNC) {
            mEnableHardwareVsync();
        }
    }
    return true;
}

bool DispSyncEnhancement::removeEventListener(status_t* res, Mutex* mutex, DispSync::Callback* callback) {
    if (mRemoveEventListener == NULL || mHasAnyEventListeners == NULL) {
        return false;
    }

    {
        Mutex::Autolock lock(mutex);
        *res = mRemoveEventListener(callback);
    }
#ifdef MTK_DYNAMIC_FPS_FW_SUPPORT
    if (!mHasAnyEventListeners() && mFpsVsync != NULL) {
        mFpsVsync->enableTracker(false);
    }
#endif
    return true;
}

bool DispSyncEnhancement::obeyResync() {
    bool res = false;
    int32_t currentMode = getVSyncMode();
    if (currentMode == VSYNC_MODE_CALIBRATED_SW_VSYNC || currentMode == VSYNC_MODE_PASS_HW_VSYNC) {
        res = true;
    }
    return res;
}

void DispSyncEnhancement::dump(std::string& result) {
    int32_t currentMode = getVSyncMode();
    int32_t currentFps = getFps();
    result.append("DispSync information:\n");
    StringAppendF(&result, "  mVSyncMode: %d\n", currentMode);
    if (currentMode != VSYNC_MODE_CALIBRATED_SW_VSYNC) {
        StringAppendF(&result, "  mFps: %d\n", currentFps);
    } else {
        result.append("\n");
    }
#ifdef MTK_DYNAMIC_FPS_FW_SUPPORT
    StringAppendF(&result, "FpsVsync(%p)\n", mFpsVsync.get());
    if (mFpsVsync != NULL) {
        String8 infoString;
        mFpsVsync->dumpInfo(infoString);
        StringAppendF(&result, "%s", infoString.string());
    }
#endif
}

void DispSyncEnhancement::calculateModel(const int32_t mode, const int32_t fps, nsecs_t* period, nsecs_t* phase, nsecs_t* referenceTime) {
    if (mode == VSYNC_MODE_INTERNAL_SW_VSYNC) {
        nsecs_t newPeriod = 1000000000 / fps;
        nsecs_t prevVsync = computePrevVsync(*period, *phase, *referenceTime);
        double sampleX = cos(0);
        double sampleY = sin(0);
        double scale = 2.0 * M_PI / double(newPeriod);
        nsecs_t newPhase = nsecs_t(atan2(sampleY, sampleX) / scale);
        *period = newPeriod;
        *phase = newPhase;
        *referenceTime = prevVsync;
    }
}

void DispSyncEnhancement::calculateHwModel(nsecs_t timestamp, int32_t fps, nsecs_t* period, nsecs_t* phase, nsecs_t* referenceTime) {
    nsecs_t newPeriod = 1000000000 / fps;
    *referenceTime = timestamp;
    *period = newPeriod;
    *phase = 0;
}

void DispSyncEnhancement::storeVsyncSourcePhase(const char* name, nsecs_t phase) {
    if (!strcmp(name, "app")) {
        mAppPhase = phase;
    } else if (!strcmp(name, "sf")) {
        mSfPhase = phase;
    }
}

nsecs_t DispSyncEnhancement::getAppPhase() const {
    return mAppPhase;
}

nsecs_t DispSyncEnhancement::getSfPhase() const {
    return mSfPhase;
}

nsecs_t DispSyncEnhancement::computePrevVsync(nsecs_t period, nsecs_t phase, nsecs_t referenceTime) {
    nsecs_t now = systemTime(SYSTEM_TIME_MONOTONIC);
    return (((now - referenceTime - phase) / period)) * period + referenceTime + phase;
}

void DispSyncEnhancement::onVSyncOffsetChange(int32_t offset) {
    if (mOnVSyncOffsetChange != NULL) {
        mOnVSyncOffsetChange(offset);
    }
}

void DispSyncEnhancement::registerSfCallback(struct SurfaceFlingerCallbackList* list) {
    mOnVSyncOffsetChange = list->onVSyncOffsetChange;

    if (mBridge == NULL) {
        sp<IGraphicExt> ge = IGraphicExt::getService();
        if (ge == NULL) {
            ALOGW("failed to get GraphicExtService");
            return;
        }
        mBridge = new GpuHidlCallbackBridge(this);

        android::hardware::Return<int32_t> res(0);
        res = ge->setupSendVsyncOffsetFuncCallback(mBridge);
        if (res.isOk()) {
            if (res != 0) {
                ALOGW("failed to register GPU HIDL callback");
            }
        } else {
            ALOGW("failed to register GPU HIDL callback, hidl fail");
        }
    }
}

int32_t DispSyncEnhancement::getVSyncMode() {
    Mutex::Autolock lock(mLock);
    return mVSyncMode;
}

int32_t DispSyncEnhancement::getFps() {
    Mutex::Autolock lock(mLock);
    return mFps;
}

DispSyncEnhancementApi* createDispSyncEnhancement() {
    return new DispSyncEnhancement();
}

}
