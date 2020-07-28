#ifndef __FPS_COUNTER_WRAPPER_H__
#define __FPS_COUNTER_WRAPPER_H__

#include <utils/String8.h>
#include <utils/Timers.h>
#include <ui_ext/FpsCounter.h>

namespace android {

extern "C"
{
    FpsCounter* createFpsCounter();
    void destroyFpsCounter(FpsCounter* hnd);
    bool updateFpsCounter(FpsCounter* hnd);
    void dumpFpsCounter(FpsCounter* hnd, String8* result, const char* prefix);
}

};

#endif // __FPS_COUNTER_WRAPPER_H__

