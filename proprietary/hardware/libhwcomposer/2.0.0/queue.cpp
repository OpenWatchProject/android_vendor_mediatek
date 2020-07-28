#define DEBUG_LOG_TAG "DBQ"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include <cutils/properties.h>

#include <hardware/gralloc.h>

#include "utils/debug.h"
#include "utils/tools.h"
#include "overlay.h"
#include "display.h"
#include "queue.h"
#include "sync.h"
#include "platform.h"
#include "hwc2.h"
#include "grallocdev.h"

// Macros for including the BufferQueue name in log messages
#define QLOGV(x, ...) HWC_LOGV("(%s:%p) " x, m_client_name.string(), this, ##__VA_ARGS__)
#define QLOGD(x, ...) HWC_LOGD("(%s:%p) " x, m_client_name.string(), this, ##__VA_ARGS__)
#define QLOGI(x, ...) HWC_LOGI("(%s:%p) " x, m_client_name.string(), this, ##__VA_ARGS__)
#define QLOGW(x, ...) HWC_LOGW("(%s:%p) " x, m_client_name.string(), this, ##__VA_ARGS__)
#define QLOGE(x, ...) HWC_LOGE("(%s:%p) " x, m_client_name.string(), this, ##__VA_ARGS__)

//#define QUEUE_DEBUG
#ifdef QUEUE_DEBUG
#define DBG_LOGD(x, ...) HWC_LOGD("(%s:%p) " x, m_client_name.string(), this, ##__VA_ARGS__)
#define DBG_LOGW(x, ...) HWC_LOGW("(%s:%p) " x, m_client_name.string(), this, ##__VA_ARGS__)
#else
#define DBG_LOGD(x, ...)
#define DBG_LOGW(x, ...)
#endif

#define HWC_ATRACE_BUFFER_INDEX(string, index)                                \
    if (ATRACE_ENABLED()) {                                                   \
        char ___traceBuf[1024];                                               \
        snprintf(___traceBuf, 1024, "%s: %d", (string), (index));             \
        android::ScopedTrace ___bufTracer(ATRACE_TAG, ___traceBuf);           \
    }

// ---------------------------------------------------------------------------

DisplayBufferQueue::DisplayBufferQueue(int type, int slot_num)
    : m_queue_type(type)
    , m_is_synchronous(true)
    , m_frame_counter(0)
    , m_last_acquire_idx(INVALID_BUFFER_SLOT)
    , m_listener(NULL)
    , m_buffer_count(slot_num)
{

    if (m_queue_type <= QUEUE_TYPE_NONE || m_queue_type > QUEUE_TYPE_OVL)
    {
        QLOGE("Initialzie with invalied Type (%d)", m_queue_type);
        m_queue_type = QUEUE_TYPE_NONE;

        m_client_name = String8::format("noinit");
    }
    else
    {
#ifdef MTK_USER_BUILD
        m_client_name = String8::format("%s", (m_queue_type == QUEUE_TYPE_BLT) ?
                                                "q2" : "q1");
#else
        m_client_name = String8::format("%s", (m_queue_type == QUEUE_TYPE_BLT) ?
                                                "blt" : "ovl");
#endif
        QLOGI("Buffer queue is created with size(%d)", m_buffer_count);
    }
}

DisplayBufferQueue::~DisplayBufferQueue()
{
    QLOGI("Buffer queue is destroyed");

    for (int i = 0; i < m_buffer_count; i++)
    {
        BufferSlot* slot = &m_slots[i];

        if (NULL == slot->out_handle) continue;

        if (slot->release_fence != -1) ::protectedClose(slot->release_fence);

        if (slot->out_handle != nullptr)
        {
            QLOGI("Free Slot(%d), handle=%p, %u -> 0",
                i, slot->out_handle, slot->data_size);

            GrallocDevice::getInstance().free(slot->out_handle);
        }

        slot->out_handle = NULL;
        slot->data_size = 0;
    }

    m_listener = NULL;
}

status_t DisplayBufferQueue::setBufferParam(BufferParam& param)
{
    AutoMutex l(m_mutex);

    m_buffer_param = param;

    return NO_ERROR;
}

status_t DisplayBufferQueue::updateBufferSize(unsigned int width, unsigned int height)
{
    AutoMutex l(m_mutex);

    if (m_buffer_param.width != width ||
        m_buffer_param.height != height)
    {
        m_buffer_param.width = width;
        m_buffer_param.height = height;
        m_buffer_param.size = (width * height * getBitsPerPixel(m_buffer_param.format) / 8);
    }

    return NO_ERROR;
}

status_t DisplayBufferQueue::reallocate(int idx)
{
    BufferSlot* slot = &m_slots[idx];

    slot->data_pitch = m_buffer_param.pitch;
    slot->data_format = m_buffer_param.format;

    if (slot->data_size == m_buffer_param.size)
        return NO_ERROR;

    QLOGI("Reallocate Slot(%d), pool(%d -> %d) size(%d -> %d)",
          idx, slot->pool_id, m_buffer_param.pool_id,
          slot->data_size, m_buffer_param.size);

    // allocate new buffer
    {
        HWC_ATRACE_CALL();

        // release old buffer
        if (slot->out_handle)
        {
            QLOGD("Free Old Slot(%d), handle=%p", idx, slot->out_handle);

            GrallocDevice::getInstance().free(slot->out_handle);
            slot->out_handle = NULL;
        }

        {
            GrallocDevice::AllocParam param;
            param.width  = m_buffer_param.width;
            param.height = m_buffer_param.height;
            param.format = m_buffer_param.format;
            param.usage  = GRALLOC_USAGE_HW_COMPOSER;
            // this buffer will be accessed by SW, so add SW flag
            if (m_buffer_param.sw_usage)
            {
                param.usage |= GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN;
            }

            if (NO_ERROR != GrallocDevice::getInstance().alloc(param))
            {
                QLOGE("Failed to allocate memory size(%d)", m_buffer_param.size);
                slot->out_handle = NULL;
                slot->data_size = 0;
                return -EINVAL;
            }

            slot->out_handle = param.handle;
            slot->pool_id = m_buffer_param.pool_id;

            QLOGD("Alloc Slot(%d), handle=%p", idx, slot->out_handle);
        }

        slot->data_size = m_buffer_param.size;
        slot->protect = m_buffer_param.protect;

        slot->out_ion_fd = -1;
        slot->out_sec_handle = 0;

        PrivateHandle priv_handle;
        status_t err = getIonFd(slot->out_handle, &priv_handle);
        if (NO_ERROR != err)
        {
            GrallocDevice::getInstance().free(slot->out_handle);
            slot->out_handle = NULL;
            slot->data_size = 0;
            return -EINVAL;
        }

        slot->out_ion_fd = priv_handle.ion_fd;
    }

    return NO_ERROR;
}

status_t DisplayBufferQueue::drainQueueLocked()
{
    while (m_is_synchronous && !m_queue.isEmpty())
    {
        m_dequeue_condition.wait(m_mutex);
    }

    return NO_ERROR;
}

status_t DisplayBufferQueue::dequeueBuffer(
    DisplayBuffer* buffer, bool async, bool is_secure)
{
    HWC_ATRACE_CALL();

    AutoMutex l(m_mutex);

    int found = INVALID_BUFFER_SLOT;
    bool tryAgain = true;
    while (tryAgain)
    {
        found = INVALID_BUFFER_SLOT;
        for (int i = 0; i < m_buffer_count; i++)
        {
            const int state = m_slots[i].state;
            if (state == BufferSlot::FREE)
            {
                // return the oldest of the free buffers to avoid
                // stalling the producer if possible.
                if (found < 0)
                {
                    found = i;
                }
                else if (m_slots[i].frame_num < m_slots[found].frame_num)
                {
                    found = i;
                }
            }
        }

        // if no buffer is found, wait for a buffer to be released
        tryAgain = (found == INVALID_BUFFER_SLOT);
        if (tryAgain)
        {
            if (CC_LIKELY(m_buffer_param.dequeue_block))
            {
                QLOGW("dequeueBuffer: cannot find available buffer, wait...");
                status_t res = m_dequeue_condition.waitRelative(m_mutex, ms2ns(16));
                QLOGW("dequeueBuffer: wake up to find available buffer (%s)",
                        (res == TIMED_OUT) ? "TIME OUT" : "WAKE");
            }
            else
            {
                QLOGW("dequeueBuffer: cannot find available buffer, exit...");
                return -EBUSY;
            }
        }
    }

    const int idx = found;

    HWC_ATRACE_BUFFER_INDEX("dequeue", idx);

    reallocate(idx);

    if (1 == HWCMediator::getInstance().m_features.svp)
    {
        //QLOGD(" #SVP ------------------------------------------------ handleSecureBuffer DBQ");
        handleSecureBuffer(is_secure, m_slots[idx].out_handle, &m_slots[idx].out_sec_handle, NULL);
        m_slots[idx].secure = is_secure;
    }

    // buffer is now in DEQUEUED state
    m_slots[idx].state = BufferSlot::DEQUEUED;

    buffer->out_handle           = m_slots[idx].out_handle;
    buffer->out_ion_fd           = m_slots[idx].out_ion_fd;
    buffer->out_sec_handle       = m_slots[idx].out_sec_handle;
    buffer->data_size            = m_slots[idx].data_size;
    buffer->data_pitch           = m_slots[idx].data_pitch;
    buffer->data_format          = m_slots[idx].data_format;
    buffer->data_info.src_crop.makeInvalid();
    buffer->data_info.dst_crop.makeInvalid();
    buffer->data_info.is_sharpen = false;
    buffer->timestamp            = m_slots[idx].timestamp;
    buffer->frame_num            = m_slots[idx].frame_num;
    buffer->release_fence        = m_slots[idx].release_fence;
    buffer->index                = idx;
    buffer->ext_sel_layer        = -1;
    m_slots[idx].release_fence = -1;

    DBG_LOGD("dequeueBuffer (idx=%d, fence=%d) (handle=%p, ion=%d)",
        idx, buffer->release_fence, buffer->out_handle, buffer->out_ion_fd);

    // wait release fence
    if (!async)
    {
        sp<SyncFence> fence(new SyncFence(m_buffer_param.disp_id));
        fence->wait(buffer->release_fence, 1000, DEBUG_LOG_TAG);
        buffer->release_fence = -1;
    }

    return NO_ERROR;
}

status_t DisplayBufferQueue::queueBuffer(DisplayBuffer* buffer)
{
    HWC_ATRACE_CALL();

    sp<ConsumerListener> listener;

    {
        AutoMutex l(m_mutex);

        const int idx = buffer->index;

        HWC_ATRACE_BUFFER_INDEX("queue", idx);

        if (idx < 0 || idx >= m_buffer_count)
        {
            QLOGE("queueBuffer: slot index out of range [0, %d]: %d",
                     m_buffer_count, idx);
            return -EINVAL;
        }
        else if (m_slots[idx].state != BufferSlot::DEQUEUED)
        {
            QLOGE("queueBuffer: slot %d is not owned by the client "
                     "(state=%d)", idx, m_slots[idx].state);
            return -EINVAL;
        }

        // if queue not empty, means consumer is slower than producer
        // * in sync mode, may cause lag (but size 1 should be OK for triple buffer)
        // * in async mode, frame drop
        //bool dump_fifo = false;
        bool dump_fifo = false;
        if (true == m_is_synchronous)
        {
            // fifo depth 1 is ok for multiple buffer, but 2 would cause lag
            if (1 < m_queue.size())
            {
                QLOGW("queued:%d (lag), type:%d", m_queue.size(), m_queue_type);
                dump_fifo = true;
            }
        }
        else
        {
            // frame drop is fifo is not empty
            if (0 < m_queue.size())
            {
                QLOGW("queued:%d (drop frame), type:%d", m_queue.size(), m_queue_type);
                dump_fifo = true;
            }
        }

        // dump current fifo data, and the new coming one
        if (true == dump_fifo)
        {
            // print from the oldest to the latest queued buffers
            const BufferSlot *slot = NULL;

            Fifo::const_iterator i(m_queue.begin());
            while (i != m_queue.end())
            {
                slot = &(m_slots[*i]);
                QLOGD("    [idx:%d] handle:%p", *i, slot->out_handle);
                ++i;
            }

            QLOGD("NEW [idx:%d] handle:%p", idx, m_slots[idx].out_handle);
        }

        if (m_is_synchronous)
        {
            // In synchronous mode we queue all buffers in a FIFO
            m_queue.push_back(idx);

            listener = m_listener;
        }
        else
        {
            // In asynchronous mode we only keep the most recent buffer.
            if (m_queue.empty())
            {
                m_queue.push_back(idx);
            }
            else
            {
                Fifo::iterator front(m_queue.begin());
                // buffer currently queued is freed
                m_slots[*front].state = BufferSlot::FREE;
                // and we record the new buffer index in the queued list
                *front = idx;
            }

            listener = m_listener;
        }

        m_slots[idx].src_handle           = buffer->src_handle;
        m_slots[idx].data_info.src_crop   = buffer->data_info.src_crop;
        m_slots[idx].data_info.dst_crop   = buffer->data_info.dst_crop;
        m_slots[idx].data_info.is_sharpen = buffer->data_info.is_sharpen;
        m_slots[idx].data_color_range     = buffer->data_color_range;
        m_slots[idx].dataspace            = buffer->dataspace;
        m_slots[idx].timestamp            = buffer->timestamp;
        m_slots[idx].state                = BufferSlot::QUEUED;
        m_slots[idx].frame_num            = (++m_frame_counter);
        m_slots[idx].alpha_enable         = buffer->alpha_enable;
        m_slots[idx].alpha                = buffer->alpha;
        m_slots[idx].blending             = buffer->blending;
        m_slots[idx].sequence             = buffer->sequence;
        m_slots[idx].acquire_fence        = buffer->acquire_fence;
        m_slots[idx].is_s3d_slot          = buffer->is_s3d_buffer;
        m_slots[idx].s3d_slot_type        = buffer->s3d_buffer_type;
        m_slots[idx].ext_sel_layer        = buffer->ext_sel_layer;
        DBG_LOGD("(%d) queueBuffer (idx=%d, fence=%d)",
            m_buffer_param.disp_id, idx, m_slots[idx].acquire_fence);

        m_dequeue_condition.broadcast();

        if (DisplayManager::m_profile_level & PROFILE_BLT)
        {
            HWC_ATRACE_INT(m_client_name.string(), m_queue.size());
        }
    }

    if (listener != NULL) listener->onBufferQueued();

    return NO_ERROR;
}

status_t DisplayBufferQueue::cancelBuffer(int index)
{
    HWC_ATRACE_CALL();
    HWC_ATRACE_BUFFER_INDEX("cancel", index);

    AutoMutex l(m_mutex);

    if (index == INVALID_BUFFER_SLOT) return -EINVAL;

    if (index < 0 || index >= m_buffer_count)
    {
        QLOGE("cancelBuffer: slot index out of range [0, %d]: %d",
                m_buffer_count, index);
        return -EINVAL;
    }
    else if (m_slots[index].state != BufferSlot::DEQUEUED)
    {
        QLOGE("cancelBuffer: slot %d is not owned by the client (state=%d)",
                index, m_slots[index].state);
        return -EINVAL;
    }

    QLOGD("cancelBuffer (%d)", index);

    m_slots[index].state = BufferSlot::FREE;
    m_slots[index].frame_num = 0;
    m_slots[index].acquire_fence = -1;
    m_slots[index].release_fence = -1;

    m_dequeue_condition.broadcast();
    return NO_ERROR;
}

status_t DisplayBufferQueue::setSynchronousMode(bool enabled)
{
    AutoMutex l(m_mutex);

    if (m_is_synchronous != enabled)
    {
        // drain the queue when changing to asynchronous mode
        if (!enabled) drainQueueLocked();

        m_is_synchronous = enabled;
        m_dequeue_condition.broadcast();
    }

    return NO_ERROR;
}

void DisplayBufferQueue::dumpLocked(int /*idx*/)
{
}

void DisplayBufferQueue::dump(QUEUE_DUMP_CONDITION /*cond*/)
{
}

status_t DisplayBufferQueue::acquireBuffer(
    DisplayBuffer* buffer, bool async)
{
    HWC_ATRACE_CALL();

    AutoMutex l(m_mutex);

    // check if queue is empty
    // In asynchronous mode the list is guaranteed to be one buffer deep.
    // In synchronous mode we use the oldest buffer.
    if (!m_queue.empty())
    {
        Fifo::iterator front(m_queue.begin());
        int idx = *front;

        HWC_ATRACE_BUFFER_INDEX("acquire", idx);

        // buffer is now in ACQUIRED state
        m_slots[idx].state = BufferSlot::ACQUIRED;

        buffer->out_handle           = m_slots[idx].out_handle;
        buffer->out_ion_fd           = m_slots[idx].out_ion_fd;
        buffer->out_sec_handle       = m_slots[idx].out_sec_handle;
        buffer->data_size            = m_slots[idx].data_size;
        buffer->data_pitch           = m_slots[idx].data_pitch;
        buffer->data_format          = m_slots[idx].data_format;
        buffer->data_color_range     = m_slots[idx].data_color_range;
        buffer->dataspace            = m_slots[idx].dataspace;
        buffer->data_info.src_crop   = m_slots[idx].data_info.src_crop;
        buffer->data_info.dst_crop   = m_slots[idx].data_info.dst_crop;
        buffer->data_info.is_sharpen = m_slots[idx].data_info.is_sharpen;
        buffer->timestamp            = m_slots[idx].timestamp;
        buffer->frame_num            = m_slots[idx].frame_num;
        buffer->protect              = m_slots[idx].protect;
        buffer->secure               = m_slots[idx].secure;
        buffer->alpha_enable         = m_slots[idx].alpha_enable;
        buffer->alpha                = m_slots[idx].alpha;
        buffer->blending             = m_slots[idx].blending;
        buffer->sequence             = m_slots[idx].sequence;
        buffer->acquire_fence        = m_slots[idx].acquire_fence;
        buffer->index                = idx;
        buffer->is_s3d_buffer        = m_slots[idx].is_s3d_slot;
        buffer->s3d_buffer_type      = m_slots[idx].s3d_slot_type;
        buffer->ext_sel_layer        = m_slots[idx].ext_sel_layer;
        m_slots[idx].acquire_fence = -1;

        DBG_LOGD("acquireBuffer (idx=%d, fence=%d)",
            idx, buffer->acquire_fence);

        // remember last acquire buffer's index
        m_last_acquire_idx = idx;

        m_queue.erase(front);
        m_dequeue_condition.broadcast();

        if (DisplayManager::m_profile_level & PROFILE_TRIG)
        {
            HWC_ATRACE_INT(m_client_name.string(), m_queue.size());
        }
    }
    else
    {
        DBG_LOGW("acquireBuffer fail");

        buffer->index = INVALID_BUFFER_SLOT;
        return NO_BUFFER_AVAILABLE;
    }

    // wait acquire fence
    if (!async)
    {
        sp<SyncFence> fence(new SyncFence(m_buffer_param.disp_id));
        fence->wait(buffer->acquire_fence, 1000, DEBUG_LOG_TAG);
        buffer->acquire_fence = -1;
    }

    return NO_ERROR;
}

status_t DisplayBufferQueue::releaseBuffer(int index, int fence)
{
    HWC_ATRACE_CALL();
    HWC_ATRACE_BUFFER_INDEX("release", index);

    AutoMutex l(m_mutex);

    if (index == INVALID_BUFFER_SLOT) return -EINVAL;

    if (index < 0 || index >= m_buffer_count)
    {
        QLOGE("releaseBuffer: slot index out of range [0, %d]: %d, fence:%d",
                m_buffer_count, index, fence);
        return -EINVAL;
    }

    if (m_slots[index].state != BufferSlot::ACQUIRED)
    {
        QLOGE("attempted to release buffer(%d) fence:%d with state(%d)",
            index, fence, m_slots[index].state);
        return -EINVAL;
    }

    m_slots[index].state = BufferSlot::FREE;
    if (m_slots[index].release_fence != -1)
    {
        QLOGW("release fence existed! buffer(%d) with state(%d) fence:%d",
            index, m_slots[index].state, fence);
        ::protectedClose(m_slots[index].release_fence);
    }
    m_slots[index].release_fence = fence;

    DBG_LOGD("releaseBuffer (idx=%d, fence=%d)", index, fence);

    m_dequeue_condition.broadcast();
    return NO_ERROR;
}

void DisplayBufferQueue::setConsumerListener(
    const sp<ConsumerListener>& listener)
{
    QLOGI("setConsumerListener");
    AutoMutex l(m_mutex);
    m_listener = listener;
}
