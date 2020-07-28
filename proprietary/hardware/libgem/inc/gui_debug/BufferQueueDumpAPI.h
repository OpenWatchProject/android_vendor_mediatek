#ifndef ANDROID_GUI_BUFFERQUEUEDUMPAPI_H
#define ANDROID_GUI_BUFFERQUEUEDUMPAPI_H

#include <ui/Fence.h>
#include <ui/GraphicBuffer.h>

#include <utils/String8.h>

namespace android {
// ----------------------------------------------------------------------------

// class for BufferQueue backup and dump utils impl
class BufferQueueDumpAPI {

public:
    BufferQueueDumpAPI() {}

    // name for this dump
    virtual void setName(const String8& /*name*/) {}

    // trigger the dump process
    virtual void dump(String8& /*result*/, const char* /*prefix*/) {}

    // related functions into original BufferQueue APIs
    virtual void onAcquireBuffer(const int& /*slot*/,
                         const sp<GraphicBuffer>& /*buffer*/,
                         const sp<Fence>& /*fence*/,
                         const int64_t& /*timestamp*/,
                         const uint32_t& /*transform*/,
                         const uint64_t ) {}
    virtual void onReleaseBuffer(const int& /*slot*/) {}
    virtual void onFreeBuffer(const int& /*slot*/) {}

    virtual void onConsumerDisconnect() {}

    virtual status_t drawDebugLineToGraphicBuffer(
        const sp<GraphicBuffer>& /*gb*/, uint32_t /*cnt*/, uint8_t /*val*/) { return false; }

    virtual void onSetIonInfo(const sp<GraphicBuffer>& /*gb*/, const int /*p_pid*/,
                              const int& /*id*/, const int& /*length*/, const String8 /*cname*/) {}
    virtual bool onCheckFps(String8* /*result*/){ return false;}
    virtual ~BufferQueueDumpAPI() {}
};

extern "C"
{
    BufferQueueDumpAPI *createInstance();
}
// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_GUI_BUFFERQUEUEDUMP_H
