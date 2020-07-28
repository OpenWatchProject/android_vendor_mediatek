#define DEBUG_LOG_TAG "PROXY"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "epaper_post_processing.h"

#include <sys/stat.h>
//#include <selinux/android.h>

#include <linux/disp_session.h>

#include <gui/BufferItem.h>
#include <ui/GraphicBufferMapper.h>
#include <cutils/properties.h>

#include "hardware_legacy/power.h"

#include "utils/debug.h"
#include "utils/tools.h"

#include "overlay.h"
#include "display.h"
#include "tcon.h"

#define NUM_PROXY_BUFFER 8
#define DUMP_PATH "/data/proxy_dump/"
#define DUMP_PROPERTY "vendor.debug.hwc.proxy.dump"

BufferTrack::BufferTrack(int num)
    : m_num(num)
    , m_used(0)
    , m_dump(false)
{
    m_set = new struct TrackSet[num];
}

BufferTrack::~BufferTrack()
{
    delete[] m_set;
}

void BufferTrack::setTrack(sp<GraphicBuffer> gb, unsigned int sequence, unsigned int subSequence, unsigned int flag)
{
    Mutex::Autolock lock(m_mutex);
    for (int i = 0; i < m_used; i++)
    {
        if (m_set[i].id == gb->getId())
        {
            m_set[i].sequence = sequence;
            m_set[i].subSequence = subSequence;
            m_set[i].flag = flag;
            m_set[i].queued = true;
            return;
        }
    }

    if (m_used < m_num)
    {
        m_set[m_used].id = gb->getId();
        m_set[m_used].sequence = sequence;
        m_set[m_used].subSequence = subSequence;
        m_set[m_used].flag = flag;
        m_set[m_used].queued = true;
        m_used++;
    }
    else
    {
        HWC_LOGE("buffer is full, but hwc still set track");
    }
}

bool BufferTrack::getTrack(sp<GraphicBuffer> gb, unsigned int* sequence, unsigned int* subSequence, unsigned int* flag)
{
    Mutex::Autolock lock(m_mutex);
    for (int i = 0; i < m_used; i++)
    {
        if (m_set[i].id == gb->getId())
        {
            *sequence = m_set[i].sequence;
            *subSequence = m_set[i].subSequence;
            *flag = m_set[i].flag;
            m_set[i].queued = false;
            return true;
        }
    }

    return false;
}

void BufferTrack::dump(void* ptr, int size, const char* filename)
{
    struct stat sb;
    if (stat(DUMP_PATH, &sb) != 0)
    {
        mode_t mode = umask(0);
        if (mkdir(DUMP_PATH, 0777) != 0)
        {
            HWC_LOGE("failed to create dir: %s", DUMP_PATH);
        }

        umask(mode);
        //if (selinux_android_restorecon(DUMP_PATH, 0) == -1)
        //{
        //    HWC_LOGE("failed to restorecon: %s", DUMP_PATH);
        //}
    }

    if (access(DUMP_PATH, R_OK | W_OK | X_OK) != 0)
    {
        HWC_LOGE("failed to access %s", DUMP_PATH);
    }

    char buf[128] = {0};
    sprintf(buf, "%s%s",DUMP_PATH, filename);

    FILE* f = NULL;
    HWC_LOGW("open %s", buf);
    f = fopen(buf, "wb");
    if (f == NULL) {
        return;
    }

    fwrite(ptr, size, 1, f);

    fclose(f);
}

void BufferTrack::dumpSrc(void* ptr, int size, int w, int h, unsigned int sequence)
{
    Mutex::Autolock lock(m_mutex);
    char buf[PROPERTY_VALUE_MAX];
    if (property_get(DUMP_PROPERTY, buf, NULL) > 0) {
        int val = atoi(buf);
        if (val) {
            char name[64] = {0};
            sprintf(name, "SRC_%08d_%03dx%d", sequence, w, h);
            dump(ptr, size, name);

            if (m_dump)
            {
                val--;
                char tmp[8];
                sprintf(tmp, "%d", val);
                property_set(DUMP_PROPERTY, tmp);
            }
        }

        if (m_dump != val)
        {
            m_dump = val;
        }
    }
}

void BufferTrack::dumpDst(sp<GraphicBuffer> gb, int w, int h, unsigned int sequence, unsigned int subSequence)
{
    Mutex::Autolock lock(m_mutex);
    if (m_dump) {
        void* ptr = NULL;
        char name[64] = {0};
        int size = 0;
        unsigned int format;
        gralloc_extra_query(gb->handle, GRALLOC_EXTRA_GET_FORMAT, &format);
        size = getBitsPerPixel(format) / 8;

        gb->lock(GRALLOC_USAGE_SW_READ_OFTEN, (void**)&ptr);
        sprintf(name, "DST_%08d-%03d_%dx%d", sequence, subSequence, w, h);
        dump(ptr, w * h * size, name);
        gb->unlock();
    }
}

bool BufferTrack::isEmpty(void)
{
    for (int i = 0; i < m_used; i++)
    {
        if (m_set[i].queued)
        {
            return false;
        }
    }
    return true;
}

ProxyConsumer::ProxyConsumer(int dpy, const uint32_t w, const uint32_t h, const uint32_t fmt,
                             const sp<IGraphicBufferConsumer>& consumer, sp<ConsumerCallBack> ccb)
    : ConsumerBase(consumer, false)
    , mCurrentBufferSlot(-1)
    , mCurrentBuffer(0)
    , mCCB(ccb)
    , mDpy(dpy)
    , mReleaseFence(NULL)
{
    mName = "proxy_consumer";
    mConsumer->setConsumerName(mName);
    mConsumer->setConsumerUsageBits(GRALLOC_USAGE_SW_READ_OFTEN);
    mConsumer->setDefaultBufferFormat(fmt);
    mConsumer->setDefaultBufferSize(w, h);

    sem_init(&mEvent, 0, 0);
}

ProxyConsumer::~ProxyConsumer()
{
}

status_t ProxyConsumer::nextBuffer(sp<GraphicBuffer>& outBuffer, sp<Fence>& outFence)
{
    Mutex::Autolock lock(mMutex);

    BufferItem item;
    status_t err = acquireBufferLocked(&item, 0);
    if (err == BufferQueue::NO_BUFFER_AVAILABLE)
    {
        outBuffer = mCurrentBuffer;
        return err;
    }
    else if (err != NO_ERROR)
    {
        HWC_LOGE("(%d) failed to acquir buffer: %s", mDpy, strerror(-err));
        return err;
    }

    if (mCurrentBufferSlot != BufferQueue::INVALID_BUFFER_SLOT && item.mSlot != mCurrentBufferSlot)
    {
        err = addReleaseFenceLocked(mCurrentBufferSlot, mCurrentBuffer, mReleaseFence);
        err = releaseBufferLocked(mCurrentBufferSlot, mCurrentBuffer, EGL_NO_DISPLAY, EGL_NO_SYNC_KHR);
        if (err < NO_ERROR)
        {
            HWC_LOGE("(%d) failed to release buffer: %s", mDpy, strerror(-err));
            return err;
        }
    }

    mCurrentBufferSlot = item.mSlot;
    mCurrentBuffer = mSlots[mCurrentBufferSlot].mGraphicBuffer;
    outFence = item.mFence;
    outBuffer = mCurrentBuffer;

    return NO_ERROR;
}

void ProxyConsumer::onFrameAvailable(const BufferItem& /*item*/)
{
    wakeup();
}

void ProxyConsumer::onFirstRef()
{
    run("ProxyConsumer");
}

status_t ProxyConsumer::readyToRun()
{
    return NO_ERROR;
}

void ProxyConsumer::wakeup()
{
    sem_post(&mEvent);
}

bool ProxyConsumer::threadLoop()
{
    sem_wait(&mEvent);

    sp<GraphicBuffer> buf;
    sp<Fence> acquireFence;
    status_t err = nextBuffer(buf, acquireFence);
    if (err != NO_ERROR)
    {
        HWC_LOGW("(%d) failed to get next buffer: %s", mDpy, strerror(-err));
        return true;
    }

    acquireFence->waitForever(DEBUG_LOG_TAG);
    //TODO: when NO_FENCE is not NULL, replace it by NO_FENCE
    acquireFence = new Fence();

    int releaseFence;
    mCCB->onUpdate(buf, &releaseFence);
    mReleaseFence = new Fence(releaseFence);

    return true;
}

//----------

ProxyProducer::ProxyProducer(int dpy, const int num, const uint32_t w, const uint32_t h,
                             const uint32_t fmt, sp<OverlayEngine> ovl, BufferTrack* track,
                             const sp<IGraphicBufferProducer>& producer, sp<ProducerCallBack> pcb)
    : mNumberSlot(num)
    , mFmt(fmt)
    // sw tcon will read and write the buffer queue.
    // if we do not set these flag, it will meet some performance issue
    , mUsage(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN)
    , mWidth(w)
    , mHeight(h)
    , mOvl(ovl)
    , mCurrentBuffer(NULL)
    , mId(-1)
    , mDpy(dpy)
    , mSequence(0)
    , mTrack(track)
    , mPcb(pcb)
{
    mProducer = producer;
    mSlots = new BufferSlot[mNumberSlot];
    memset(mSlots, 0, sizeof(BufferSlot) * mNumberSlot);

    mProducer->setMaxDequeuedBufferCount(mNumberSlot);
    IGraphicBufferProducer::QueueBufferOutput output;
    mProducer->connect(this, NATIVE_WINDOW_API_CPU, false, &output);

    mSize = w * h * getBitsPerPixel(fmt) / 8;

    sem_init(&mJob, 0, 0);
    sem_init(&mEvent, 0, 0);
}

ProxyProducer::~ProxyProducer()
{
    delete mSlots;
    if (mCurrentBuffer != NULL) {
        delete mCurrentBuffer;
    }
}

void ProxyProducer::freeAllBuffers()
{
    for (int i = 0; i < mNumberSlot; i++) {
        mSlots[i].buffer = 0;
    }
}

int ProxyProducer::getSlotLocked(sp<GraphicBuffer>& gb)
{
    for (int i = 0; i < mNumberSlot; i++) {
        if (mSlots[i].buffer != NULL && mSlots[i].buffer->handle == gb->handle) {
            return i;
        }
    }
    HWC_LOGE("(%d) getSlotFromBufferLocked: unknown buffer: %p", mDpy, gb->handle);
    return BAD_VALUE;
}

int ProxyProducer::cancelBuffer(sp<GraphicBuffer>& gb, int fenceFd)
{
    Mutex::Autolock lock(mMutex);
    int i = getSlotLocked(gb);
    if (i < 0) {
        return i;
    }

    //TODO: when NO_FENCE is not NULL, replace it by NO_FENCE
    sp<Fence> fence(fenceFd >= 0 ? new Fence(fenceFd) : new Fence());
    mProducer->cancelBuffer(i, fence);
    return OK;
}

int ProxyProducer::dequeueBuffer(sp<GraphicBuffer>& gb, int *fenceFd)
{
    int buf = -1;
    sp<Fence> fence;

    status_t err = mProducer->dequeueBuffer(&buf, &fence, 0, 0, mFmt, mUsage, nullptr);
    if (err < 0) {
        HWC_LOGE("(%d) failed to dequeueBuffer: %d", mDpy, err);
        return err;
    }

    Mutex::Autolock lock(mMutex);
    sp<GraphicBuffer>& gbuf(mSlots[buf].buffer);

    //ALOGE_IF(fence == NULL, "testProducer::dequeueBuffer: received null Fence! buf=%d", buf);
    if (err & IGraphicBufferProducer::RELEASE_ALL_BUFFERS) {
        freeAllBuffers();
    }

    if ((err & IGraphicBufferProducer::BUFFER_NEEDS_REALLOCATION) || gbuf == 0) {
        err = mProducer->requestBuffer(buf, &gbuf);
        if (err != NO_ERROR) {
            HWC_LOGE("(%d) failed to requestBuffer failed: %d", mDpy, err);
            mProducer->cancelBuffer(buf, fence);
            return err;
        }
    }

    if (fence != NULL && fence->isValid()) {
        *fenceFd = fence->dup();
        if (*fenceFd == -1) {
            HWC_LOGE("(%d) dequeueBuffer: error duping fence: %d", mDpy, errno);
        }
    } else {
        *fenceFd = -1;
    }

    gb = gbuf;

    return OK;
}

int ProxyProducer::queueBuffer(sp<GraphicBuffer>& gb, int fenceFd)
{
    Mutex::Autolock lock(mMutex);

    int i = getSlotLocked(gb);
    if (i < 0) {
        return i;
    }

    Rect crop(0, 0, mWidth, mHeight);
    //TODO: when NO_FENCE is not NULL, replace it by NO_FENCE
    sp<Fence> fence(fenceFd >= 0 ? new Fence(fenceFd) : new Fence());
    IGraphicBufferProducer::QueueBufferOutput output;
    IGraphicBufferProducer::QueueBufferInput input(0, false, HAL_DATASPACE_UNKNOWN, crop, NATIVE_WINDOW_SCALING_MODE_FREEZE, 0, fence, 0);
    status_t err = mProducer->queueBuffer(i, input, &output);
    if (err != OK) {
        HWC_LOGE("(%d) failed to queue buffer", mDpy);
    }

    return err;
}

void ProxyProducer::onBufferReleased()
{
    wakeup();
}

void ProxyProducer::onFirstRef()
{
    run("ProxyProducer");
}

status_t ProxyProducer::readyToRun()
{
    return NO_ERROR;
}

void ProxyProducer::wakeup()
{
    sem_post(&mEvent);
}

void ProxyProducer::addJob(int id)
{
    HWC_LOGD("(%d) addJob_%d", mDpy, id);
    if (mId != id) {
        mId = id;
    }
    sem_post(&mJob);
}

bool ProxyProducer::threadLoop()
{
    sem_wait(&mJob);

    DisplayBufferQueue::DisplayBuffer newBuffer;
    mOvl->acquireBuffer(&newBuffer, mId);

    if (mCurrentBuffer == NULL)
    {
        mCurrentBuffer = new struct SourceBufferInfo;
        mCurrentBuffer->id = mId;
    }
    else
    {
        if (newBuffer.out_handle != NULL && newBuffer.out_handle != mCurrentBuffer->buf.out_handle)
        {
            // we get a new source buffer, so unlock the older and release it
            GraphicBufferMapper::getInstance().unlock(mCurrentBuffer->buf.out_handle);
            mOvl->releaseBuffer(&mCurrentBuffer->buf, mCurrentBuffer->id/*, mCurrentBuffer->fence*/);
        }
        else
        {
            return true;
        }
    }

    mCurrentBuffer->buf = newBuffer;
    mCurrentBuffer->fence = -1;
    void *ptr = NULL;
    int size = 0;
    gralloc_extra_query(mCurrentBuffer->buf.out_handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &size);
    DisplayData* disp_data = &DisplayManager::getInstance().m_data[mDpy];
    Rect src_crop(disp_data->width, disp_data->height);
    // lock the new source buffer for sw tcon
    GraphicBufferMapper::getInstance().lock(mCurrentBuffer->buf.out_handle, GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN, src_crop, &ptr);

    // set the new source buffer to sw tcon, then it will pre-process it in here
    mInterface->setSourceBuffer(0, ptr, size);
    mTrack->dumpSrc(ptr, size, disp_data->width, disp_data->height, mSequence);

    bool hasFrame = true;
    unsigned int subSequence = 0;
    // when sw tcon received a source buffer, it can produce several frame
    // therefore we need to call getCookedBuffer() until it returns false
    do {
        sp<GraphicBuffer> gb;
        int release_fence = -1;
        status_t err;

        // try to get a new buffer to fill new frame data
        while (true) {
            err = dequeueBuffer(gb, &release_fence);
            if (err == OK) {
                break;
            }
            sem_wait(&mEvent);
        }

        unsigned char *ptr = NULL;
        gb->lockAsync(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&ptr, release_fence);

        // require sw tcon fill a new frame in this buffer
        hasFrame = mInterface->getCookedBuffer(ptr, mSize);
        HWC_LOGD("(%d) producre image[%d-%d]", mDpy, mSequence, subSequence);
        int acquire_fence = -1;
        gb->unlockAsync(&acquire_fence);

        // record this frame information in track
        mTrack->setTrack(gb, mSequence, subSequence, hasFrame? BufferTrack::NONE: BufferTrack::LAST_FRAME);
        err = queueBuffer(gb , acquire_fence);
        if (err != NO_ERROR) {
            HWC_LOGW("(%d) can not push buffer to comsumer", mDpy);
        }
        subSequence++;
    } while(hasFrame);

    mSequence++;
    // notify front end that we can start processing new source buffer
    mPcb->onFrameComplete();

    return true;
}

void ProxyProducer::setInterface(sp<ProxyInterface> interface)
{
    mInterface = interface;
}

//----------

EpaperPostProcessingEngine::EpaperPostProcessingEngine(int dpy, sp<OverlayEngine> ovl)
    : PostProcessingEngine(dpy, ovl)
    , m_producer(NULL)
    , m_consumer(NULL)
    , m_sync(new SyncFence(dpy))
    , m_num(NUM_PROXY_BUFFER)
    , m_buf(new struct ProxyBuffer[NUM_PROXY_BUFFER])
    , m_out_width(0)
    , m_out_height(0)
    , m_track(NUM_PROXY_BUFFER)
    , m_power(POWER_OFF)
    , m_seq(0)
    , m_wait_vsync(false)
{
}

EpaperPostProcessingEngine::~EpaperPostProcessingEngine()
{
    m_interface->release(m_buf, m_num);
    m_producer = NULL;
    m_consumer = NULL;
    delete[] m_buf;
}

void EpaperPostProcessingEngine::onBufferQueued()
{
    // initiate member variable in here, because display data is not ready when create this class
    if (m_producer == NULL || m_consumer == NULL)
    {
        sp<IGraphicBufferProducer> producer;
        sp<IGraphicBufferConsumer> consumer;
        BufferQueue::createBufferQueue(&producer, &consumer);

        DisplayData* disp_data = &DisplayManager::getInstance().m_data[m_dpy];

        m_interface = new class TconInterface();
        m_interface->requestBufSize(disp_data->width, disp_data->height, 1000000000 / disp_data->refresh, &m_out_width, &m_out_height);

        m_consumer = new class ProxyConsumer(m_dpy, m_out_width, m_out_height, HAL_PIXEL_FORMAT_BGRA_8888, consumer, this);
        m_producer = new class ProxyProducer(m_dpy, m_num, m_out_width, m_out_height, HAL_PIXEL_FORMAT_BGRA_8888, m_overlay, &m_track, producer, this);

        m_producer->setInterface(m_interface);
        prepareBuffer();
    }

    m_producer->addJob(m_id);
}

void EpaperPostProcessingEngine::onVSync(void)
{
    Mutex::Autolock lock(m_mutex);
    m_wait_vsync = false;
    m_condition.signal();
}

void EpaperPostProcessingEngine::registerVSyncListener(const sp<VSyncListener>& listener)
{
    Mutex::Autolock lock(m_mutex);
    m_vsync_callbacks.add(listener);
}

void EpaperPostProcessingEngine::onFrameComplete()
{
    Mutex::Autolock lock(m_mutex);
    const size_t count = m_vsync_callbacks.size();
    for (size_t i = 0; i < count; i++)
    {
        const sp<VSyncListener>& callback(m_vsync_callbacks[i]);
        callback->onVSync();
    }
}

void EpaperPostProcessingEngine::onUpdate(sp<GraphicBuffer>& buf, int* releaseFd)
{
    EXTD_TRIGGER_MODE triggerMode = TRIGGER_NORMAL;
    unsigned int sequence, subSequence, flag;

    // get this frame info from track
    m_track.getTrack(buf, &sequence, &subSequence, &flag);
    HWC_LOGD("(%d): update image[%d-%d]  flag: %d", m_dpy, sequence, subSequence, flag);
    m_track.dumpDst(buf, m_out_width, m_out_height, sequence, subSequence);

    // if it is a last frame for this souce buffer and front end has no buffer,
    // notify display driver that it has to close DPI after this frame is complete
    if (flag & BufferTrack::LAST_FRAME && m_track.isEmpty())
    {
        m_power = POWER_OFF;
        triggerMode = TRIGGER_SUSPEND;
        //DisplayManager::getInstance().requestVSync(m_dpy, false);
        HWC_LOGD("(%d) detect last frame. turn DPI off", m_dpy);
    }
    else if (m_power == POWER_OFF)
    {
        m_power = POWER_ON;
        triggerMode = TRIGGER_RESUME;
        //DisplayManager::getInstance().requestVSync(m_dpy, true);
        HWC_LOGD("(%d) detect first frame. turn DPI on", m_dpy);
    }
    // DPI need to transfer this frame, so kick wakelock timer to avoid suspending device
    DisplayManager::getInstance().setWakelockTimerState(DisplayManager::WAKELOCK_TIMER_PLAYOFF);

    m_overlay->updateInput(m_id, buf, releaseFd, m_seq);
    m_seq++;

    m_overlay->waitUntilAvailable();

    int channelNum = m_overlay->getAvailableInputNum();
    for (int i = 0; i < channelNum; i++)
    {
        if (i != m_id)
        {
            m_overlay->disableInput(i);
        }
    }

    m_overlay->setInputs(8);
    m_overlay->disableOutput();

    {
        Mutex::Autolock lock(m_mutex);
        if (triggerMode != TRIGGER_RESUME && m_wait_vsync)
        {
            if (m_condition.waitRelative(m_mutex, ms2ns(40)) == TIMED_OUT)
            {
                HWC_LOGW("(%d) timed out waiting for VSync", m_dpy);
            }
        }
        m_wait_vsync = true;
    }

    m_overlay->trigger(false, 1, DISP_NO_PRESENT_FENCE, -1, triggerMode);
}

void EpaperPostProcessingEngine::prepareBuffer()
{
    // dequeue the maximum number of buffer queue and lock them
    for (int i = 0; i < m_num; i++)
    {
        int release_fence = -1;
        int err = m_producer->dequeueBuffer(m_buf[i].gb, &release_fence);
        if (err != OK)
        {
            HWC_LOGE("failed to dequeue buffer for initial");
        }
        gralloc_extra_query(m_buf[i].gb->handle, GRALLOC_EXTRA_GET_ALLOC_SIZE, &m_buf[i].size);
        m_buf[i].gb->lockAsync(GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&m_buf[i].ptr, release_fence);
    }

    // sw tcon will write some constant pattern in these buffers
    m_interface->init(m_buf, m_num);

    // cancel all buffer and unlock them
    for (int i = 0; i < m_num; i++)
    {
        int acquire_fence = -1;
        m_buf[i].gb->unlockAsync(&acquire_fence);
        m_producer->cancelBuffer(m_buf[i].gb, acquire_fence);
    }
}
