#include <ui_ext/FpsCounterWrapper.h>
#include <log/log.h>

namespace android {
    FpsCounter* createFpsCounter()
    {
        return new FpsCounter;
    }

    void destroyFpsCounter(FpsCounter* hnd)
    {
        delete hnd;
    }

    bool updateFpsCounter(FpsCounter* hnd)
    {
        return hnd->update();
    }

    void dumpFpsCounter(FpsCounter* hnd, String8* result, const char* prefix)
    {
        return hnd->dump(result, prefix);
    }
}; // namespace android
