#ifndef __SF_DEBUG_API_H__
#define __SF_DEBUG_API_H__

#include <utils/String8.h>
#include <system/window.h>

namespace android {

extern "C"
{
    void dumpBufferWrap(const buffer_handle_t& handle, const char* prefix);
}

};

#endif
