#ifndef HWC_PROXY_H_
#define HWC_PROXY_H_

#include <semaphore.h>
#include <stdlib.h>

#include <unistd.h>

#include <cutils/log.h>

#include <ui/GraphicBuffer.h>
#include <gui/IProducerListener.h>
#include <gui/IGraphicBufferProducer.h>
#include <gui/IGraphicBufferConsumer.h>
#include <gui/BufferQueue.h>
#include <gui/ConsumerBase.h>
#include <gui/BufferItem.h>

#include <hwc_tcon.h>

#include "utils/tools.h"
#include "queue.h"
#include "post_processing.h"

using namespace android;

typedef uint32_t SECHAND;

#include "overlay.h"
#include "sync.h"

/**
 * This file includs three major classes for ePaper feature. Read below meesage to understand the
 * flow of ePaper.
 *
 * ePaper can not receive the general graphic buffer, like RGBA or YUV data. It reads vendor's
 * command data. Then our system has to transfer graphic to command. Therefore, we need a HW Tcon
 * or SW Tcon to do this job. This part is an extension module to use vendor's SW Tcon library.
 *
 * The flow of SW Tcon as below.
 * 1. HWC notify it that the resolution of ePaper, then it will return the size of required buffer
 * 2. HWC provide SW Tcon with all pointer of buffer, then it will initiate buffer with constant
 *    pattern
 * 3. SW Tcon receives a new graphic buffer from HWC
 * 4. HWC try to get frame buffer from SW Tcon until there is no frame
 * 5. go to step 3, then loop again
 *
 * When SW Tcon receive a graphic buffer, it may produce several frames. Each frames are special
 * data of complete graphic buffer. You can think that the color of previous frame is lighter than
 * next frames and so on. We push these frames to ePaper, so it can show the graphic gradually.
 */

class BufferTrack
{
public:
    enum TrackInfo
    {
        NONE = 0x00,
        LAST_FRAME = 0x01,
    };

    BufferTrack(int num);
    ~BufferTrack();

    void setTrack(sp<GraphicBuffer> gb, unsigned int sequence, unsigned int subSequence, unsigned int flag);

    bool getTrack(sp<GraphicBuffer> gb, unsigned int* sequence, unsigned int* subSequence, unsigned int* flag);

    void dumpSrc(void* ptr, int size, int w, int h, unsigned int sequence);

    void dumpDst(sp<GraphicBuffer> gb, int w, int h, unsigned int sequence, unsigned int subSequence);

    void dump(void* ptr, int size, const char* filename);

    bool isEmpty(void);

private:
    struct TrackSet
    {
    public:
        uint64_t id;

        // sequence is id of source buffer
        unsigned int sequence;

        // subSequence is id of frame
        unsigned int subSequence;

        // flag used to record the TrackInfo only
        unsigned int flag;

        bool queued;
    };

    mutable Mutex m_mutex;

    int m_num;

    int m_used;

    struct TrackSet* m_set;

    bool m_dump;
};

struct SourceBufferInfo
{
public:
    int id;
    DisplayBufferQueue::DisplayBuffer buf;
    int fence;
    unsigned int syncMaker;
};

class ProxyInterface : public virtual RefBase
{
public:
    virtual ~ProxyInterface() {};

    virtual void requestBufSize(uint32_t src_width, uint32_t src_height, nsecs_t src_rate,
                                uint32_t* dst_width, uint32_t* dst_height) = 0;

    virtual void init(struct ProxyBuffer* buf, uint32_t num) = 0;

    virtual void release(struct ProxyBuffer* buf, uint32_t num) = 0;

    virtual unsigned char setSourceBuffer(int32_t mode, void *ptr, size_t size) = 0;

    virtual unsigned char getCookedBuffer(void *ptr, size_t size) = 0;

    virtual void reset(void) = 0;
};

class ProxyConsumer : public ConsumerBase, public Thread
{
public:
    class ConsumerCallBack : public virtual RefBase
    {
    public:
        virtual void onUpdate(sp<GraphicBuffer>& buf, int* releaseFd) = 0;
    };

    ProxyConsumer(int dpy, const uint32_t w, const uint32_t h, const uint32_t fmt,
                  const sp<IGraphicBufferConsumer>& consumer, sp<ConsumerCallBack> ccb);
    virtual ~ProxyConsumer();

private:
    virtual void onFrameAvailable(const BufferItem& item);

    status_t nextBuffer(sp<GraphicBuffer>& outBuffer, sp<Fence>& outFence);

    virtual void onFirstRef();

    virtual status_t readyToRun();

    virtual bool threadLoop();

    void wakeup();

    int mCurrentBufferSlot;

    sp<GraphicBuffer> mCurrentBuffer;

    Condition mCondition;

    // mEvent used to know that producer has a new frame
    sem_t mEvent;

    // mCCB used to notify EpaperPostProcessingEngine that there is a new frame which can be showed
    sp<ConsumerCallBack> mCCB;

    // mDpy indicates display id
    int mDpy;

    // mReleaseFence used to stored the release fence
    sp<Fence> mReleaseFence;
};

class ProxyProducer : public BnProducerListener, public Thread
{
friend class ProxyPost;
public:
    class ProducerCallBack : public virtual RefBase
    {
    public:
        virtual void onFrameComplete() = 0;
    };

    ProxyProducer(int dpy, const int num, const uint32_t w, const uint32_t h, const uint32_t fmt,
                  sp<OverlayEngine> ovl, BufferTrack* track,
                  const sp<IGraphicBufferProducer>& producer, sp<ProducerCallBack> pcb);
    virtual ~ProxyProducer();

    int dequeueBuffer(sp<GraphicBuffer>& gb, int *fenceFd);

    int queueBuffer(sp<GraphicBuffer>& gb, int fenceFd);

    int cancelBuffer(sp<GraphicBuffer>& gb, int fenceFd);

    // setInterface() used to set interface of tcon library
    void setInterface(sp<ProxyInterface> interface);

    // addJob() is called by EpaperPostProcessingEngine when front end has a new source buffer
    void addJob(int id);

private:
    void freeAllBuffers();
    int getSlotLocked(sp<GraphicBuffer>& gb);

    struct BufferSlot {
        sp<GraphicBuffer> buffer;
    };

    virtual void onBufferReleased();

    virtual void onFirstRef();

    virtual status_t readyToRun();

    virtual bool threadLoop();

    void wakeup();

    sp<IGraphicBufferProducer> mProducer;

    BufferSlot *mSlots;

    int mNumberSlot;

    uint32_t mFmt;

    uint32_t mUsage;

    uint32_t mWidth;

    uint32_t mHeight;

    mutable Mutex mMutex;

    // mEvent used to know that consumer has release a buffer
    sem_t mEvent;

    // mJob used to know that fornt end provide us with a new source buffer
    sem_t mJob;

    // m_interface used to communicate with tcon library
    sp<ProxyInterface> mInterface;

    // mSize is the buffer size
    uint32_t mSize;

    // mOvl used to trigger display driver
    sp<OverlayEngine> mOvl;

    // mCurrentBuffer used to stored the information of source buffer
    struct SourceBufferInfo* mCurrentBuffer;

    // mId indicates the layer id
    int mId;

    // mDpy indicates display id
    int mDpy;

    // mSequence used to mark a sequence for source buffer
    unsigned int mSequence;

    // mTrack used to stored buffer info to control power state and debug
    BufferTrack* mTrack;

    // mPcb used to notify EpaperPostProcessingEngine that the previous frame is done, then it can
    // request a new buffer from front end
    sp<ProducerCallBack> mPcb;
};

class EpaperPostProcessingEngine : public PostProcessingEngine,
                                   public ProxyConsumer::ConsumerCallBack,
                                   public ProxyProducer::ProducerCallBack
{
public:
    enum PowerState
    {
        POWER_ON,
        POWER_OFF,
    };
    EpaperPostProcessingEngine(int dpy, sp<OverlayEngine> ovl);
    virtual ~EpaperPostProcessingEngine();

    // onBufferQueued() is called when someone queue buffer to DBQ
    void onBufferQueued();

    // onVSync() is called by event thread
    void onVSync(void);

    // onUpdate() is called by ProxyConsumer
    void onUpdate(sp<GraphicBuffer>& buf, int* releaseFd);

    // registerVSyncListener() used to register callback function of other class
    void registerVSyncListener(const sp<VSyncListener>& listener);

    // onFrameComplete() is called by ProxyProducer, then notify fornt end that frame is done
    void onFrameComplete();

private:
    // prepareBuffer() used to allocate new buffer before use them
    void prepareBuffer();

    // m_producer used to procude processed data from tcon
    sp<ProxyProducer> m_producer;

    // m_consumer used to consume processed data
    sp<ProxyConsumer> m_consumer;

    // m_sync used to wait fence
    sp<SyncFence> m_sync;

    // m_interface used to communicate with tcon library
    sp<ProxyInterface> m_interface;

    // m_num stored the total buffer of buffer queue
    int m_num;

    // m_buf stored buffer information for tcon
    struct ProxyBuffer* m_buf;

    // m_out_width is the buffer width of tcon
    unsigned int m_out_width;

    // m_out_height is the buffer height of tcon
    unsigned int m_out_height;

    // m_track stored the information of processed buffer
    BufferTrack m_track;

    mutable Mutex m_mutex;

    // m_power stored the last power state
    PowerState m_power;

    // m_condition used to wait for vsync
    Condition m_condition;

    // m_seq gathers the buffer sequence
    int m_seq;

    // m_wait_vsync used to check thread whether it need to wait vsync
    bool m_wait_vsync;
};
#endif
