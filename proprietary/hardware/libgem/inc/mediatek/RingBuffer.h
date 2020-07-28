#pragma GCC system_header

#ifndef ANDROID_GUI_RINGBUFFER_H
#define ANDROID_GUI_RINGBUFFER_H

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include <inttypes.h>
#include <utils/Vector.h>
#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
#include <utils/RefBase.h>

#include <cutils/log.h>

namespace android {

// ----------------------------------------------------------------------------
template <typename TYPE>
class RingBuffer : public LightRefBase< RingBuffer<TYPE> > {
public:

    // for custom data push
    class Pusher : public LightRefBase<Pusher> {
    protected:
        RingBuffer<TYPE>& mRingBuffer;

        // use friend privillege to access necessary data
        TYPE& editHead();

    public:
        Pusher(RingBuffer<TYPE>& rb) : mRingBuffer(rb) {}
        virtual bool push(const TYPE& in) = 0;
        virtual ~Pusher() {}
    };

    // for custom data dump
    class Dumper : public LightRefBase<Dumper> {
    protected:
        RingBuffer<TYPE>& mRingBuffer;

        // use friend privillege to access necessary data
        const TYPE& getItem(uint32_t idx);

    public:
        Dumper(RingBuffer<TYPE>& rb) : mRingBuffer(rb) {}
        virtual ~Dumper() {}
        virtual void dump(String8& result, const char* prefix) = 0;
    };

private:
    Vector<TYPE> mBuffer;

    uint32_t mHead;
    uint32_t mSize;
    uint32_t mCount;

    Mutex mLock;

    sp<Pusher> mPusher;
    sp<Dumper> mDumper;

public:
    RingBuffer()
        : mHead(0)
        , mSize(0)
        , mCount(0) {}

    ~RingBuffer() {}

    uint32_t getSize() { return mSize; }
    uint32_t getCount() { return mCount; }
    uint32_t getValidSize() { return (mSize < mCount) ? mSize : mCount; }
    Mutex getLock() { return mLock; }

    // reset buffers and counters
    void resize(uint32_t size);

    // push data to head, and return the count
    uint32_t push(const TYPE& in);

    // just print some info if no dumper assigned
    void dump(String8& result, const char* prefix);

    void setPusher(sp<Pusher>& pusher);
    void setDumper(sp<Dumper>& dumper);

    friend class Pusher;
    friend class Dumper;
};

// ----------------------------------------------------------------------------
template <typename TYPE>
TYPE& RingBuffer<TYPE>::Pusher::editHead() {
    return mRingBuffer.mBuffer.editItemAt(mRingBuffer.mHead);
}

template <typename TYPE>
const TYPE& RingBuffer<TYPE>::Dumper::getItem(uint32_t idx) {
    uint32_t oldest = (mRingBuffer.mCount > mRingBuffer.mSize)
                    ? mRingBuffer.mHead
                    : 0;
    idx = (oldest + idx) % mRingBuffer.mSize;
    return mRingBuffer.mBuffer[idx];
}

template <typename TYPE>
void RingBuffer<TYPE>::resize(uint32_t size) {
    Mutex::Autolock lock(mLock);

    if (mSize != size) {
        ALOGI("[%s](this:%p) resize(%u => %u)", __func__, this, mSize, size);

        mSize = size;
        mHead = 0;
        mCount = 0;

        // free old backup data and reset storage
        mBuffer.clear();
        mBuffer.resize(mSize);
    }
}

template <typename TYPE>
uint32_t RingBuffer<TYPE>::push(const TYPE& in) {
    Mutex::Autolock lock(mLock);

    if (mPusher != NULL) {
        if (!mPusher->push(in))
            return mCount;
    } else {
        mBuffer.replaceAt(in, mHead);
    }

    // Update pointer
    mHead++;
    if (mHead >= mSize)
        mHead = 0;

    return mCount++;
}

template <typename TYPE>
void RingBuffer<TYPE>::dump(String8& result, const char* prefix) {
    Mutex::Autolock lock(mLock);

    ALOGI("[%s] %p (mHead:%" PRIx32 ", mSize:%" PRIx32 ", mCount:%" PRIx32 ")", __func__, this, mHead, mSize, mCount);

    if (mDumper != NULL) {
        mDumper->dump(result, prefix);
    }
}

template <typename TYPE>
void RingBuffer<TYPE>::setPusher(sp<Pusher>& pusher) {
    Mutex::Autolock lock(mLock);
    mPusher = pusher;
}

template <typename TYPE>
void RingBuffer<TYPE>::setDumper(sp<Dumper>& dumper) {
    Mutex::Autolock lock(mLock);
    mDumper = dumper;
}


// ----------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_GUI_RINGBUFFER_H
