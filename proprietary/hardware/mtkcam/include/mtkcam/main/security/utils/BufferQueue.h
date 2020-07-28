#ifndef _IRIS_QUEUE_H_
#define _IRIS_QUEUE_H_

#ifndef USING_MTK_ION
#error "Need MTK ION to support secure memory allocation"
#else
#include <ion/ion.h>
#endif

// TODO: replace with iris error code
#include <utils/Errors.h>

#include <vector>

#include <mutex>
#include <condition_variable>

typedef uint32_t SECHAND;
using namespace android;

namespace NSCam {
namespace security {

// ---------------------------------------------------------------------------

// IrisBufferQueue manages a pool of Iris buffer slots.
class IrisBufferQueue
{
public:
    enum { NUM_BUFFER_SLOTS = 6 };
    enum { INVALID_BUFFER_SLOT = -1 };
    enum { NO_BUFFER_AVAILABLE = -1 };

    // NOTE:
    // Please note once cache mechanism is enabled,
    // buffer producer and/or buffer consumer need(s) to
    // do cache synchronization or invalidataion explicitly to
    // avoid cache memory inconsistency.
    enum class CacheType : int8_t { NON_CACHEABLE, CACHEABLE };
    enum class SyncType : int8_t { FLUSH, INVALIDATE };

    struct ConsumerListener
    {
        virtual ~ConsumerListener() = default;

        // onBufferQueued() is called when producer queues new frame
        virtual void onBufferQueued(const void* opaqueMessage) = 0;
    };

    struct ProducerListener
    {
        virtual ~ProducerListener() = default;

        // onBufferReleased() is called when consumer releases used frame
        virtual void onBufferReleased() = 0;
    };

    struct BufferParam
    {
        unsigned int width;
        unsigned int height;
        unsigned int pitch;
        unsigned int format;
        size_t       size;
        CacheType cache_mechanism;
        bool dequeue_block;

        BufferParam(unsigned int _width = 0, unsigned int _height = 0,
            unsigned int _pitch = 0, unsigned int _format = 0,
            size_t _size = 0,
            CacheType _cache_mechanism = CacheType::NON_CACHEABLE,
            bool _dequeue_block = true)
            : width(_width), height(_height), pitch(_pitch),
              format(_format), size(_size), cache_mechanism(_cache_mechanism),
              dequeue_block(_dequeue_block)
        { }
    };

    // IrisBuffer is the buffer used by the producer/consumer sides
    // (i.e., PRODUCER <-> IrisBufferQueue <-> CONSUMER)
    struct IrisBuffer
    {
        IrisBuffer()
            : handle(0)
            , ion_fd(-1)
            , sec_handle(0)
            , va(0)
            , data_size(0)
            , data_pitch(0)
            , data_format(0)
            , frame_num(0)
            , secure(false)
            , sequence(0)
            , acquire_fence(-1)
            , release_fence(-1)
            , index(INVALID_BUFFER_SLOT)
        { }

        // handle is the output buffer handle
        ion_user_handle_t handle;

        // ion_fd is for normal buffer
        int ion_fd;

        // sec_handle is for secure buffer
        SECHAND sec_handle;

        // va of buffer handle
        uintptr_t va;

        // data_size is memory allocated size
        size_t data_size;

        // data_pitch is valid data stride
        unsigned int data_pitch;

        // data_format is data format
        unsigned int data_format;

        // frame_num is the number of the queued frame for this buffer.
        uint64_t frame_num;

        // secure means if this buffer is secure
        bool secure;

        // sequence is used as a sequence number for profiling latency purpose
        unsigned int sequence;

        // acquire_fence is used for consumer to wait producer
        int acquire_fence;

        // release_fence is used for producer to wait consumer
        int release_fence;

        // index is the slot index of this buffer
        int index;
    };

    IrisBufferQueue(int maxBuffers = NUM_BUFFER_SLOTS);
    ~IrisBufferQueue();

    // cacheSync() do cache sync on the given buffer
    status_t cacheSync(const struct IrisBuffer& buffer,
            const SyncType type = SyncType::FLUSH);

    ////////////////////////////////////////////////////////////////////////
    // PRODUCER INTERFACE

    // setBufferParam() updates parameter
    status_t setBufferParam(const BufferParam&& param);

    // dequeueBuffer() gets the next buffer slot index for the client to use,
    // this one can get secure buffer.
    status_t dequeueBuffer(IrisBuffer* buffer, bool async, bool is_secure = false);

    // queueBuffer() returns a filled buffer to the DisplayBufferQueue.
    status_t queueBuffer(
            IrisBuffer* buffer, const void* opaqueMessage = nullptr);

    // cancelBuffer() lets producer to give up a dequeued buffer
    status_t cancelBuffer(int index, bool isDropped = false);

    // setSynchronousMode() set dequeueBuffer as sync or async
    status_t setSynchronousMode(bool enabled);

    // setProducerListener() sets a Producer listener to the DisplayBufferQueue.
    void setProducerListener(const std::shared_ptr<ProducerListener> & listener);

    ////////////////////////////////////////////////////////////////////////
    // CONSUMER INTERFACE

    // acquireBuffer() attempts to acquire the next pending buffer by consumer
    status_t acquireBuffer(IrisBuffer* buffer, bool async = false);

    // releaseBuffer() releases a buffer slot from the consumer back
    status_t releaseBuffer(int index, int fence = -1);

    // setConsumerListener() sets a consumer listener to the DisplayBufferQueue.
    void setConsumerListener(const std::shared_ptr<ConsumerListener> & listener);

private:
    // allocate() is used to allocate buffer
    status_t reallocateLocked(int idx, bool is_secure);

    // drainQueueLocked() drains the buffer queue when change to asynchronous mode
    status_t drainQueueLocked(std::unique_lock<std::mutex>& lock);

    // BufferSlot is a buffer slot that contains DisplayBuffer information
    // and holds a buffer state for buffer management
    struct BufferSlot
    {
        BufferSlot()
            : state(BufferSlot::FREE)
            , handle(0)
            , ion_fd(-1)
            , sec_handle(0)
            , va(0)
            , data_size(0)
            , data_pitch(0)
            , data_format(0)
            , frame_num(0)
            , secure(false)
            , sequence(0)
            , acquire_fence(-1)
            , release_fence(-1)
        { }

        enum BufferState {
            FREE = 0,
            DEQUEUED = 1,
            QUEUED = 2,
            ACQUIRED = 3
        };

        // state is the current state of this buffer slot.
        BufferState state;

        // handle is the output buffer handle
        ion_user_handle_t handle;

        // ion_fd is for normal buffer
        int ion_fd;

        // sec_handle is for secure buffer
        SECHAND sec_handle;

        // va of buffer handle
        uintptr_t va;

        // data_size is data size
        size_t data_size;

        // data_pitch is valid data stride
        unsigned int data_pitch;

        // data_format is data format
        unsigned int data_format;

        // frame_num is the number of the queued frame for this buffer.
        uint64_t frame_num;

        // secure means if this buffer is secure
        bool secure;

        // sequence is used as a sequence number for profiling latency purpose
        unsigned int sequence;

        // acquire_fence is a fence descriptor
        // used to signal buffer is filled by producer
        int acquire_fence;

        // release_fence is a fence descriptor
        // used to signal buffer is used by consumer
        int release_fence;

    };

    BufferSlot m_slots[NUM_BUFFER_SLOTS];

    // m_buffer_param is used to store buffer information
    BufferParam m_buffer_param;

    // m_buffer_ount is for the real buffer number in queue
    int m_buffer_count;

    // m_is_synchronous points whether we're in synchronous mode or not
    bool m_is_synchronous;

    // m_dequeue_block is used to determine whether dequeueBuffer()
    // would be blocked when no free buffer to use
    bool m_dequeue_block;

    // m_dequeue_condition condition used for dequeueBuffer in synchronous mode
    std::condition_variable m_dequeue_condition;

    // m_queue is a FIFO of queued buffers used in synchronous mode
    typedef std::vector<int> Fifo;
    Fifo m_queue;

    // m_mutex is the mutex used to prevent concurrent access to the member
    // variables of BufferQueue objects. It must be locked whenever the
    // member variables are accessed.
    mutable std::mutex m_mutex;

    // m_frame_counter is the free running counter, incremented for every buffer
    // queued with the surface Texture.
    uint64_t m_frame_counter;

    // m_last_acquire_idx remembers index of last acquire buffer
    // and it is for dump purpose
    int m_last_acquire_idx;

    // m_page_size
    size_t m_page_size;

    std::shared_ptr<ProducerListener> m_producer_listener;
    std::shared_ptr<ConsumerListener> m_consumer_listener;

    // TODO: abstract ION operation
    std::pair<int, std::mutex> mIonDevFd;
};

} // namespace security
} // namespace NSCam

#endif // _IRIS_QUEUE_H_
