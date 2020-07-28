
#ifndef MEDIA_ADAPTER_H
#define MEDIA_ADAPTER_H

#include <media/stagefright/foundation/ABase.h>
#include <media/MediaSource.h>
#include <media/stagefright/MediaBufferBase.h>
#include <media/stagefright/MetaData.h>
#include <utils/threads.h>

namespace android
{

// Convert the MediaMuxer's push model into MPEG4Writer's pull model.
// Used only by the MediaMuxer for now.
struct MediaAdapter : public MediaSource, public MediaBufferObserver {
public:
    // MetaData is used to set the format and returned at getFormat.
    MediaAdapter(const sp<MetaData> &meta);
    virtual ~MediaAdapter();
    /////////////////////////////////////////////////
    // Inherited functions from MediaSource
    /////////////////////////////////////////////////

    virtual status_t start(MetaData *params = NULL);
    virtual status_t stop();
    virtual sp<MetaData> getFormat();
    virtual status_t read(
        MediaBufferBase **buffer, const ReadOptions *options = NULL);

    /////////////////////////////////////////////////
    // Inherited functions from MediaBufferObserver
    /////////////////////////////////////////////////

    virtual void signalBufferReturned(MediaBufferBase *buffer);

    /////////////////////////////////////////////////
    // Non-inherited functions:
    /////////////////////////////////////////////////

    // pushBuffer() will wait for the read() finish, and read() will have a
    // deep copy, such that after pushBuffer return, the buffer can be re-used.
    status_t pushBuffer(MediaBuffer *buffer);

private:
    Mutex mAdapterLock;
    // Make sure the read() wait for the incoming buffer.
    Condition mBufferReadCond;
    // Make sure the pushBuffer() wait for the current buffer consumed.
    Condition mBufferReturnedCond;

    MediaBuffer *mCurrentMediaBuffer;

    bool mStarted;
    sp<MetaData> mOutputFormat;

    DISALLOW_EVIL_CONSTRUCTORS(MediaAdapter);
};

}  // namespace android

#endif  // MEDIA_ADAPTER_H
