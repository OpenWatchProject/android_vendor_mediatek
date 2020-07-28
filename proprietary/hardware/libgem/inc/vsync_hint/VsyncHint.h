#ifndef __MTK_SURFACE_FLINGER_VSYNC_HINT_H__
#define __MTK_SURFACE_FLINGER_VSYNC_HINT_H__

#include "vsync_hint/VsyncHintApi.h"

namespace android {

// class for notify VSync to other native module
class VsyncHint : public VsyncHintApi {
public:
    VsyncHint();
    ~VsyncHint();

    // notify VSync event to other module
    void notifyVsync(int type, nsecs_t period);

};

}

#endif
