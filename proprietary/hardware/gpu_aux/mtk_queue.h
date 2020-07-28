#ifndef __MTK_QUEUE_H__
#define __MTK_QUEUE_H__

#include "mtk_gralloc.h"

#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <utils/Condition.h>
#include <aux/aux_gralloc.h>

template <typename NATIVE_TYPE, typename TYPE, typename REF>
class MTKANativeObjectBase : public NATIVE_TYPE, public REF
{
public:
    // Disambiguate between the incStrong in REF and NATIVE_TYPE
    void incStrong(const void *id) const
    {
        REF::incStrong(id);
    }
    void decStrong(const void *id) const
    {
        REF::decStrong(id);
    }

protected:
    typedef MTKANativeObjectBase<NATIVE_TYPE, TYPE, REF> BASE;
    MTKANativeObjectBase() : NATIVE_TYPE(), REF()
    {
        NATIVE_TYPE::common.incRef = incRef;
        NATIVE_TYPE::common.decRef = decRef;
    }

    ~MTKANativeObjectBase()
    {
    }

    static inline TYPE *getSelf(NATIVE_TYPE *self)
    {
        return static_cast<TYPE *>(self);
    }
    static inline TYPE const *getSelf(NATIVE_TYPE const *self)
    {
        return static_cast<TYPE const *>(self);
    }
    static inline TYPE *getSelf(android_native_base_t *base)
    {
        return getSelf(reinterpret_cast<NATIVE_TYPE *>(base));
    }
    static inline TYPE const *getSelf(android_native_base_t const *base)
    {
        return getSelf(reinterpret_cast<NATIVE_TYPE const *>(base));
    }
    static void incRef(android_native_base_t *base)
    {
        MTKANativeObjectBase *self = getSelf(base);
        self->incStrong(self);
    }
    static void decRef(android_native_base_t *base)
    {
        MTKANativeObjectBase *self = getSelf(base);
        self->decStrong(self);
    }
};

class GPUAUXBuffer: public MTKANativeObjectBase< ANativeWindowBuffer, GPUAUXBuffer, android::RefBase >
{
private:
    enum { ownHandle, ownNone };
    int mOwner;
    AHardwareBuffer *mHWBuffer;
    android::sp<android_native_buffer_t> mWrappedBuffer;

public:
    GPUAUXBuffer(uint32_t width,
                 uint32_t height,
                 uint32_t format) : BASE(), mOwner(ownHandle), mWrappedBuffer(NULL), mHWBuffer(NULL)
    {
        android_native_buffer_t *pBuffer;
        pBuffer = static_cast<android_native_buffer_t *>(const_cast<GPUAUXBuffer *>(this));

#if GPUD_GRALLOC_VERSION_MAJOR >= 2
        AuxGralloc::getInstance().allocateBuffer(width, height, format, &pBuffer, &mHWBuffer);
#else
        uint64_t usage =  MTKGralloc::getInstance()->getUsage();
        MTKGralloc::getInstance()->allocBuffer(width, height, format, usage, &pBuffer);
#endif
    }

    GPUAUXBuffer(android_native_buffer_t *anb) : BASE(), mOwner(ownNone), mWrappedBuffer(anb)
    {
        width  = anb->width;
        height = anb->height;
        stride = anb->stride;
        format = anb->format;
        usage  = anb->usage;
        handle = anb->handle;
    }

    ~GPUAUXBuffer()
    {
        if (mOwner == ownHandle) {
#if GPUD_GRALLOC_VERSION_MAJOR >= 2
            AuxGralloc::getInstance().freeBuffer(static_cast<android_native_buffer_t *>(
                                                 const_cast<GPUAUXBuffer *>(this)), mHWBuffer);
#else
            MTKGralloc::getInstance()->freeBuffer(static_cast<android_native_buffer_t *>(
                                                  const_cast<GPUAUXBuffer *>(this)));
#endif
        }
        mWrappedBuffer = NULL;
    }

    int32_t getWidth() const
    {
        return width;
    }

    int32_t getHeight() const
    {
        return height;
    }

    int32_t getStride() const
    {
        return stride;
    }

    int32_t getFormat() const
    {
        return format;
    }

    int32_t getUsage() const
    {
        return usage;
    }

    ANativeWindowBuffer *getNativeBuffer() const
    {
        return static_cast<ANativeWindowBuffer *>(const_cast<GPUAUXBuffer *>(this));
    }
};

class GPUAUXBufferQueue : public android::RefBase
{
public:
    GPUAUXBufferQueue(uint32_t numBuffers);

    ~GPUAUXBufferQueue();

    int32_t dequeueBuffer(int32_t                 width,
                          int32_t                 height,
                          int32_t                 format,
                          int32_t                 *pSlotID,
                          android_native_buffer_t **ppBuffer,
                          int32_t                 *pFenceFD,
                          bool                    waitBuf = true);

    int32_t queueBuffer(uint32_t slotID);

    int32_t acquireBuffer(int32_t                 *pSlotID,
                          android_native_buffer_t **ppBuffer,
                          int32_t                 *pFenceFD,
                          bool                    waitBuf = true);

    int32_t releaseBuffer(uint32_t slotID);

    void setConsumerName(const char *name);

private:
    uint32_t  m_numBuffers;

    android::Mutex  m_queueLock;
    android::Condition   m_producer;
    android::Condition   m_consumer;

    bool      m_abortQueue;

    android::sp<GPUAUXBuffer> *m_bufferList;
    uint32_t  m_bufferAvail;
    uint32_t  m_bufferTail;

    android::sp<GPUAUXBuffer> *m_queueList;
    android::Vector<uint32_t> m_queueSlotID;
    uint32_t  m_queueAvail;
    uint32_t  m_queueFlag;
    uint32_t  m_queueHead;
    uint32_t  m_queueTail;

    char m_name[256] = {0};
};

#endif  // __MTK_QUEUE_H__
