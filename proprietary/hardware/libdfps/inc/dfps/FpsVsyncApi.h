#ifndef __FPS_VSYNC_API_H__
#define __FPS_VSYNC_API_H__

#include <functional>

#include <utils/String8.h>
#include <utils/RefBase.h>

namespace android {

typedef std::function<void(int32_t, int32_t)> SwVsyncChangeCallback;

class FpsVsyncApi : public RefBase {
public:
    FpsVsyncApi() {}
    virtual ~FpsVsyncApi() {}

    virtual void registerSwVsyncChangeCallback(SwVsyncChangeCallback /*callback*/) {}
    virtual void callSwVsyncChange(int32_t /*mode*/, int32_t /*fps*/) {}
    virtual void enableTracker(int32_t /*enable*/) {}
    virtual void dumpInfo(String8& /*result*/) {}
};

extern "C"
{
    FpsVsyncApi* createFpsVsyncApi();
}

};

#endif
