#ifndef __ANDROID_SURFACE_FLINGER_DISP_SYNC_ENHANCEMENT_API_H__
#define __ANDROID_SURFACE_FLINGER_DISP_SYNC_ENHANCEMENT_API_H__

#include <functional>

#include <Scheduler/DispSync.h>

#include "vsync_enhance/DispSyncEnhancementDef.h"

namespace android {

#define DS_DEFAULT_FPS 60

typedef std::function<status_t(int32_t, int32_t, nsecs_t, nsecs_t, nsecs_t)> SetVSyncMode;
typedef std::function<void(nsecs_t, nsecs_t, nsecs_t)> AddResyncSample;
typedef std::function<status_t(const char*, nsecs_t, DispSync::Callback*)> AddEventListener;
typedef std::function<status_t(void)> HasAnyEventListeners;
typedef std::function<void(void)> EnableHardwareVsync;
typedef std::function<status_t(DispSync::Callback* callback)> RemoveEventListener;
typedef std::function<void(int32_t, int32_t)> OnSwVsyncChange;
typedef std::function<void(nsecs_t)> OnVSyncOffsetChange;

struct DispSyncEnhancementFunctionList {
    SetVSyncMode setVSyncMode;
    AddResyncSample addResyncSample;
    AddEventListener addEventListener;
    HasAnyEventListeners hasAnyEventListeners;
    EnableHardwareVsync enableHardwareVsync;
    RemoveEventListener removeEventListener;
    OnSwVsyncChange onSwVsyncChange;
};

struct SurfaceFlingerCallbackList {
    OnVSyncOffsetChange onVSyncOffsetChange;
};

class DispSyncEnhancementApi {
public:
    DispSyncEnhancementApi() {}

    virtual ~DispSyncEnhancementApi() {}

    // get some function pointer from DispSync
    virtual void registerFunction(struct DispSyncEnhancementFunctionList* /*list*/) {}

    // used to change the VSync mode and fps
    virtual status_t setVSyncMode(int32_t /*mode*/, int32_t /*fps*/, nsecs_t* /*period*/,
                                  nsecs_t* /*phase*/, nsecs_t* /*referenceTime*/) { return NO_ERROR; }

    // used to add present fence for calibration
    virtual bool addPresentFence(bool* /*res*/) { return false; }

    // used to add the sample of hw vsync
    virtual bool addResyncSample(bool* /*res*/, nsecs_t /*timestamp*/, nsecs_t* /*period*/,
                                 nsecs_t* /*phase*/, nsecs_t* /*referenceTime*/) { return false; }

    // used to add event listener
    virtual bool addEventListener(status_t* /*res*/, Mutex* /*mutex*/, const char* /*name*/,
                                  nsecs_t /*phase*/, DispSync::Callback* /*callback*/) { return false; }

    // used to remove event listener
    virtual bool removeEventListener(status_t* /*res*/, Mutex* /*mutex*/,
                                     DispSync::Callback* /*callback*/) { return false; }

    // notify caller that we do not want to clear parameter of DispSync
    virtual bool obeyResync() { return false; }

    // get the vsync offset of app
    virtual nsecs_t getAppPhase() const { return 0;}

    // get the vsync offset of sf
    virtual nsecs_t getSfPhase() const { return 0; }

    // dump the information of enhancement
    virtual void dump(std::string& /*result*/) {}

    // get some function pointer from SurfaceFlinger
    virtual void registerSfCallback(struct SurfaceFlingerCallbackList* /*list*/);
};

extern "C"
{
    DispSyncEnhancementApi* createDispSyncEnhancement();
}

}

#endif
