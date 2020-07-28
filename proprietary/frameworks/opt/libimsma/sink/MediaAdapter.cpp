

//#define LOG_NDEBUG 0
#define LOG_TAG "[sink][recorder]MediaAdapter"
#include <utils/Log.h>

#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaAdapter.h>
#include <media/stagefright/MediaBufferBase.h>
#include "comutils.h"
namespace android
{

MediaAdapter::MediaAdapter(const sp<MetaData> &meta)
    : mCurrentMediaBuffer(NULL),
      mStarted(false),
      mOutputFormat(meta)
{
}

MediaAdapter::~MediaAdapter()
{
    Mutex::Autolock autoLock(mAdapterLock);
    mOutputFormat.clear();
    CHECK(mCurrentMediaBuffer == NULL);
}

status_t MediaAdapter::start(MetaData * /* params */)
{
    Mutex::Autolock autoLock(mAdapterLock);

    if(!mStarted) {
        mStarted = true;
    }

    return OK;
}

status_t MediaAdapter::stop()
{
    Mutex::Autolock autoLock(mAdapterLock);

    if(mStarted) {
        mStarted = false;

        // If stop() happens immediately after a pushBuffer(), we should
        // clean up the mCurrentMediaBuffer
        if(mCurrentMediaBuffer != NULL) {
            mCurrentMediaBuffer->release();
            mCurrentMediaBuffer = NULL;
        }

        // While read() is still waiting, we should signal it to finish.
        mBufferReadCond.signal();
    }

    return OK;
}

sp<MetaData> MediaAdapter::getFormat()
{
    Mutex::Autolock autoLock(mAdapterLock);
    return mOutputFormat;
}

void MediaAdapter::signalBufferReturned(MediaBufferBase *buffer)
{
    Mutex::Autolock autoLock(mAdapterLock);
    CHECK(buffer != NULL);
    buffer->setObserver(0);
    buffer->release();
    VT_LOGV("buffer returned %p", buffer);
    mBufferReturnedCond.signal();
}

status_t MediaAdapter::read(
    MediaBufferBase **buffer, const ReadOptions * /* options */)
{
    Mutex::Autolock autoLock(mAdapterLock);

    if(!mStarted) {
        VT_LOGV("Read before even started!");
        return ERROR_END_OF_STREAM;
    }

    while(mCurrentMediaBuffer == NULL && mStarted) {
        VT_LOGV("waiting @ read()");
        mBufferReadCond.wait(mAdapterLock);
    }

    if(!mStarted) {
        VT_LOGV("read interrupted after stop");
        CHECK(mCurrentMediaBuffer == NULL);
        return ERROR_END_OF_STREAM;
    }

    CHECK(mCurrentMediaBuffer != NULL);

    *buffer = mCurrentMediaBuffer;
    mCurrentMediaBuffer = NULL;
    (*buffer)->setObserver(this);

    return OK;
}

status_t MediaAdapter::pushBuffer(MediaBuffer *buffer)
{
    if(buffer == NULL) {
        VT_LOGE("pushBuffer get an NULL buffer");
        return -EINVAL;
    }

    Mutex::Autolock autoLock(mAdapterLock);

    if(!mStarted) {
        VT_LOGE("pushBuffer called before start");
        return INVALID_OPERATION;
    }

    mCurrentMediaBuffer = buffer;
    mBufferReadCond.signal();

    VT_LOGD("wait for the buffer returned @ pushBuffer! %p", buffer);
    mBufferReturnedCond.wait(mAdapterLock);

    return OK;
}

}  // namespace android

