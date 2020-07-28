#include "mtk_queue.h"
#include "utils.h"

GPUAUXBufferQueue::GPUAUXBufferQueue(uint32_t numBuffers)
    : m_numBuffers(numBuffers),
      m_abortQueue(false),
      m_bufferTail(0)
{
    m_bufferList = (android::sp<GPUAUXBuffer> *) calloc(numBuffers, sizeof(m_bufferList[0]));
    m_queueList = (android::sp<GPUAUXBuffer> *) calloc(numBuffers, sizeof(m_queueList[0]));

    if(!m_bufferList || !m_queueList) {
        MTK_LOGE("m_bufferList or m_queueList alloc fail! \n");
    }

    m_queueSlotID.clear();
    for (uint32_t i = 0; i < m_numBuffers; ++i) {
        m_queueSlotID.add(-1);
    }

    m_bufferAvail = ((1 << m_numBuffers) - 1);
    m_queueAvail  = 0;
    m_queueFlag   = 0;

    m_queueHead = 0;
    m_queueTail = 0;
}

GPUAUXBufferQueue::~GPUAUXBufferQueue()
{
    m_abortQueue = true;
    m_producer.signal();
    m_consumer.signal();

    m_queueLock.lock();

    for (uint32_t i = 0; i < m_numBuffers; ++i) {
        m_bufferList[i] = NULL;
        m_queueList[i] = NULL;
    }

    free(m_bufferList);
    free(m_queueList);

    m_queueLock.unlock();
}

int32_t GPUAUXBufferQueue::dequeueBuffer(int32_t                 width,
                                         int32_t                 height,
                                         int32_t                 format,
                                         int32_t                 *pSlotID,
                                         android_native_buffer_t **ppBuffer,
                                         int32_t                 *pFenceFD,
                                         bool                    waitBuf)
{
    uint32_t index = -1;

    //MTK_LOGD("%s: Dequeue buffer begin\n", __func__);

    if ((NULL == pSlotID) ||
        (NULL == ppBuffer) ||
        (NULL == pFenceFD)) {
        MTK_LOGE("Invalid parameter for dequeue\n");
        return -1;
    }

    m_queueLock.lock();

    while (0 == m_bufferAvail) {
        if (true == m_abortQueue) {
            m_queueLock.unlock();

            *pSlotID  = -1;
            *ppBuffer = NULL;
            *pFenceFD = -1;

            MTK_LOGD("Abort dequeue by user\n");

            // Aborted by user
            return 0;
        } else if (true == waitBuf) {
            //MTK_LOGE("Wait for release buffer 0x%08x\n", m_bufferAvail);
            m_producer.wait(m_queueLock);
        } else {
            m_queueLock.unlock();

            *pSlotID  = -1;
            *ppBuffer = NULL;
            *pFenceFD = -1;

            MTK_LOGE("Dequeue buffer failed\n");

            // Dequeue failed
            return -1;
        }
    }

    for (uint32_t i = 0; i < m_numBuffers; i++) {
        index = (i + m_bufferTail) % m_numBuffers;

        if (0 != (m_bufferAvail & (1 << index))) {
            if (NULL != m_bufferList[index].get()) {
                if ((m_bufferList[index]->getWidth()  != width) ||
                    (m_bufferList[index]->getHeight() != height) ||
                    (m_bufferList[index]->getFormat() != format)) {
                    m_bufferList[index] = NULL;
                }
            }

            if (NULL == m_bufferList[index].get()) {
                //MTK_LOGD("Create buffer width %d, height %d, format %d", width, height, format);

                m_bufferList[index] = new GPUAUXBuffer(width, height, format);
            }

            *pSlotID  = index;
            *ppBuffer = m_bufferList[index].get();
            *pFenceFD = -1;

            //MTK_LOGD("Dequeue with slotID %d, pBuffer %p, fencdFD %d", *pSlotID, *ppBuffer, *pFenceFD);

            // Clear the availability
            m_bufferAvail &= ~(1 << index);

            m_bufferTail = (index + 1) % m_numBuffers;

            break;
        }
    }

    if (index >= m_numBuffers) {
        m_queueLock.unlock();

        MTK_LOGE("Invalid buffer slot index");
        return -1;
    }

    m_queueLock.unlock();

    //MTK_LOGD("Dequeue buffer end");

    return 0;
}


int32_t GPUAUXBufferQueue::queueBuffer(uint32_t slotID)
{
    uint32_t index;

    //MTK_LOGD("%s: Queue buffer %d begin\n", slotID, __func__);

    m_queueLock.lock();

    index = 0;
    do {
        if (m_queueTail >= m_numBuffers) {
            m_queueTail = 0;
        }

        if (0 == (m_queueFlag & (1 << m_queueTail))) {
            //MTK_LOGD("%s: Queue buffer %d from source %d\n", __func__, m_queueTail, slotID);

            m_queueList[m_queueTail]   = m_bufferList[slotID];
            m_queueSlotID.replaceAt(slotID, m_queueTail);

            // Set the availability
            m_queueFlag  |= (1 << m_queueTail);
            m_queueAvail |= (1 << m_queueTail);
            break;
        }

        m_queueTail++;

        index++;
    } while (index < m_numBuffers);

    m_queueLock.unlock();

    // Signal the consumer
    m_consumer.signal();

    //MTK_LOGD("%s: Queue buffer end\n", __func__);

    return 0;
}


int32_t GPUAUXBufferQueue::acquireBuffer(int32_t                 *pSlotID,
                                         android_native_buffer_t **ppBuffer,
                                         int32_t                 *pFenceFD,
                                         bool                    waitBuf)
{
    uint32_t index;

    //MTK_LOGD("%s: Acquire buffer begin\n", __func__);

    if ((NULL == pSlotID) ||
        (NULL == ppBuffer) ||
        (NULL == pFenceFD)) {
        MTK_LOGE("%s: Invalid parameter for acquire\n", __func__);
        return -1;
    }

    m_queueLock.lock();

    while (0 == m_queueAvail) {
        if (true == m_abortQueue) {
            m_queueLock.unlock();

            *pSlotID  = -1;
            *ppBuffer = NULL;
            *pFenceFD = -1;

            //MTK_LOGD("%s: Abort acquire by user\n", __func__);

            // Aborted by user
            return 0;
        } else if (true == waitBuf) {
            //MTK_LOGE("%s: Wait for queue buffer 0x%08x\n", __func__, m_queueAvail);
            m_consumer.wait(m_queueLock);
        } else {
            m_queueLock.unlock();

            *pSlotID  = -1;
            *ppBuffer = NULL;
            *pFenceFD = -1;

            //MTK_LOGE("%s: Acquire buffer failed\n", __func__);

            // Acquire failed
            return -1;
        }
    }

    index = 0;
    do {
        if (m_queueHead >= m_numBuffers) {
            m_queueHead = 0;
        }

        if (0 != (m_queueAvail & (1 << m_queueHead))) {
            *pSlotID  = m_queueSlotID[m_queueHead];
            *ppBuffer = static_cast<android_native_buffer_t *>(const_cast<GPUAUXBuffer *>(m_queueList[m_queueHead].get()));
            *pFenceFD = -1;

            //MTK_LOGD("%s: Acquire with slotID %d, pBuffer %p, fencdFD %d", __func__, *pSlotID, *ppBuffer, *pFenceFD);

            // Clear the availability
            m_queueAvail &= ~(1 << m_queueHead);
            break;
        }

        m_queueHead++;

        index++;
    } while (index < m_numBuffers);

    m_queueLock.unlock();

    //MTK_LOGD("%s: Acquire buffer end\n", __func__);

    return 0;
}


int32_t GPUAUXBufferQueue::releaseBuffer(uint32_t slotID)
{
    uint32_t index;

    //MTK_LOGD("%s: Release buffer %d begin\n", __func__, slotID);

    m_queueLock.lock();

    for (index = 0; index < m_numBuffers; index++) {
        if (m_queueSlotID[index] == slotID) {
            m_queueList[index]   = NULL;
            m_queueSlotID.replaceAt(-1, index);

            // Set queue slot to available
            m_queueFlag  &= ~(1 << index);
            m_queueAvail &= ~(1 << index);

            // Set the availability
            m_bufferAvail |= (1 << slotID);
            break;
        }
    }

    if (index >= m_numBuffers) {
        m_queueLock.unlock();

        MTK_LOGE("%s: Invalid buffer slot index\n", __func__);
        return -1;
    }

    m_queueLock.unlock();

    // Signal the producer
    m_producer.signal();

    //MTK_LOGD("%s: Release buffer end\n", __func__);

    return 0;
}


void GPUAUXBufferQueue::setConsumerName(const char *name)
{
    strncpy(m_name, name, sizeof(m_name) - 1);
    m_name[sizeof(m_name) - 1] = '\0';
}

