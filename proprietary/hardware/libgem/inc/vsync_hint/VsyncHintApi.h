#ifndef __MTK_SURFACE_FLINGER_VSYNC_HINT_API_H__
#define __MTK_SURFACE_FLINGER_VSYNC_HINT_API_H__

#include <utils/Timers.h>

namespace android {

// class for notify VSync to other native module
class VsyncHintApi {
public:
    VsyncHintApi() {}
    virtual ~VsyncHintApi() {}

    // notify VSync event to other module
    virtual void notifyVsync(int /*type*/, nsecs_t /*period*/) {}

    enum {
        VSYNC_TYPE_UNKNOWN = 0,
        VSYNC_TYPE_SF,
        VSYNC_TYPE_APP,
    };
};

extern "C"
{
    VsyncHintApi* createVsyncHint();
}

}

#endif
