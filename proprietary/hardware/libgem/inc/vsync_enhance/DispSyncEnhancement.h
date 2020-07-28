#ifndef __ANDROID_SURFACE_FLINGER_DISP_SYNC_ENHANCEMENT_H__
#define __ANDROID_SURFACE_FLINGER_DISP_SYNC_ENHANCEMENT_H__

#include "vsync_enhance/DispSyncEnhancementApi.h"

#ifdef MTK_DYNAMIC_FPS_FW_SUPPORT
#include "dfps/FpsVsyncApi.h"
#endif

#include <utils/Mutex.h>

namespace android {

class GpuHidlCallbackBridge;

class IGpuHidlCallback {
public:
    virtual void onVSyncOffsetChange(int32_t offset) = 0;
    virtual ~IGpuHidlCallback() = default;
};

class DispSyncEnhancement : public DispSyncEnhancementApi, public IGpuHidlCallback {
public:
    DispSyncEnhancement();

    ~DispSyncEnhancement();

    // get some function pointer from DispSync
    void registerFunction(struct DispSyncEnhancementFunctionList* list);

    // used to change the VSync mode and fps
    status_t setVSyncMode(int32_t mode, int32_t fps, nsecs_t *period, nsecs_t *phase,
                          nsecs_t *referenceTime);

    // used to add present fence for calibration
    bool addPresentFence(bool *res);

    // used to add the sample of hw vsync
    bool addResyncSample(bool *res, nsecs_t timestamp, nsecs_t *period, nsecs_t *phase,
                         nsecs_t *referenceTime);

    // used to add event liestener
    bool addEventListener(status_t* res, Mutex *mutex, const char* name, nsecs_t phase,
                          DispSync::Callback* callback);

    // used to remove event listener
    virtual bool removeEventListener(status_t* res, Mutex* mutex,
                                     DispSync::Callback* callback);

    // notify caller that we do not want to clear parameter of DispSync
    bool obeyResync();

    // get the vsync offset of app
    nsecs_t getAppPhase() const;

    // get the vsync offset of sf
    nsecs_t getSfPhase() const;

    // dump the information of enhancement
    void dump(std::string& result);

    // get some function pointer from SurfaceFlinger
    void registerSfCallback(struct SurfaceFlingerCallbackList* list);

    // GPU HIDL callback
    void onVSyncOffsetChange(int32_t offset);

private:
    // used to compute the timestamp of previous vsync
    nsecs_t computePrevVsync(nsecs_t period, nsecs_t phase, nsecs_t referenceTime);

    // calculate the parameter of HW vsync mode
    void calculateHwModel(nsecs_t timestamp, int32_t fps, nsecs_t *period, nsecs_t *phase,
                          nsecs_t *referenceTime);

    // used to store the vsync offset of app or sf
    void storeVsyncSourcePhase(const char* name, nsecs_t phase);

    // calculate the parameter of DispSync
    void calculateModel(int32_t mode, int32_t fps, nsecs_t *period, nsecs_t *phase,
                        nsecs_t *referenceTime);

    // mVSyncMode and mFps are accessed by multi-thread, so they are protected by mutex.
    // add these functions to read it.
    int32_t getVSyncMode();
    int32_t getFps();

    Mutex mLock;
    int32_t mVSyncMode;
    int32_t mFps;

    nsecs_t mAppPhase;
    nsecs_t mSfPhase;

    // DispSyncThread function
    SetVSyncMode mSetVSyncMode;
    HasAnyEventListeners mHasAnyEventListeners;
    AddResyncSample mAddResyncSample;
    AddEventListener mAddEventListener;
    RemoveEventListener mRemoveEventListener;

    // DispSync function
    EnableHardwareVsync mEnableHardwareVsync;
    OnSwVsyncChange mOnSwVsyncChange;

    // SurfaceFlinger function
    OnVSyncOffsetChange mOnVSyncOffsetChange;

    GpuHidlCallbackBridge* mBridge;

#ifdef MTK_DYNAMIC_FPS_FW_SUPPORT
    void* mFpsPolicyHandle;
    sp<FpsVsyncApi> mFpsVsync;
#endif
};

}

#endif
