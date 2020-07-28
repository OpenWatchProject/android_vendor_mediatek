#include <cutils/log.h>
#include "sf_debug/SFDebugAPI.h"
#include <ui_ext/GraphicBufferUtil.h>

namespace android {

void dumpBufferWrap(const buffer_handle_t& handle, const char* prefix){
    getGraphicBufferUtil().dump(handle, prefix, "/data/SF_dump");
}

};
