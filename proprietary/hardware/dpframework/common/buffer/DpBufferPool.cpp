#include "DpBufferPool.h"

#include <sync.h>
#include <errno.h>

#include "DpMemory.h"
#include "DpPlatform.h"
#include <sys/stat.h>
#include "DpWriteBMP.h"
#include "DpWriteBin.h"
#include "tile_mdp_reg.h"
#include "tile_driver.h"

#if CONFIG_FOR_OS_WINDOWS
    #define DUMP_ROOT   ".\\out\\"
#else
    #define DUMP_ROOT   (DpDriver::getInstance()->getdumpBufferFolder())
#endif  // CONFIG_FOR_OS_WINDOWS

DpBufferPoolBase::DpBufferPoolBase(BUFFER_TYPE_ENUM type)
    : m_bufferType(type),
      m_bufferCount(0),
      m_bufferID(0),
      m_availCount(0),
      m_queueCount(0),
      m_userToken(0),
      m_abortPool(false)
{
}


DpBufferPoolBase::~DpBufferPoolBase()
{
    BufferList::iterator  iterator;

    DPLOGI("DpBufferPoolBase: delete memory buffer pool\n");

    AutoMutex lock(m_poolMutex);

    for(iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        if (true == iterator->decorate)
        {
            switch(iterator->plane)
            {
                case 3:
                    delete iterator->pProxy[2];
                    iterator->pProxy[2] = NULL;
                case 2:
                    delete iterator->pProxy[1];
                    iterator->pProxy[1] = NULL;
                case 1:
                    delete iterator->pProxy[0];
                    iterator->pProxy[0] = NULL;
                    break;
                default:
                    DPLOGE("DpBufferPoolBase: invalid plane count: %d", iterator->plane);
                    assert(0);
                    break;
            }
            iterator->decorate = false;
        }

        delete &(*iterator);
    }

    m_bufferCount = 0;
}


DP_STATUS_ENUM DpBufferPoolBase::setSecureMode(DpSecure)
{
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::pollBufferStatus(CLIENT_TYPE_ENUM client,
                                                  bool             waitBuf)
{
    AutoMutex lock(m_poolMutex);

    DPLOGI("DpBufferPoolBase: poll buffer status begin\n");
    DPLOGI("DpBufferPoolBase: available count(%d), queue count(%d)\n", m_availCount, m_queueCount);

    assert(m_availCount >= 0);
    assert(m_queueCount >= 0);

    if (CLIENT_PRODUCER == client)
    {
        while (0 == m_availCount)
        {
            if (true == waitBuf)
            {
                m_producer.wait(m_poolMutex);
            }
            else
            {
                DPLOGE("DpBufferPoolBase: no output buffer available\n");
                return DP_STATUS_BUFFER_FULL;
            }
        }
    }
    else
    {
        while (0 == m_queueCount)
        {
            if (true == waitBuf)
            {
                m_consumer.wait(m_poolMutex);
            }
            else
            {
                DPLOGE("DpBufferPoolBase: no input buffer available\n");
                return DP_STATUS_BUFFER_EMPTY;
            }
        }
    }

    DPLOGI("DpBufferPoolBase: poll buffer status end\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::pollBufferStatus(CLIENT_TYPE_ENUM,
                                                  int32_t,
                                                  int32_t,
                                                  int32_t,
                                                  int32_t,
                                                  int32_t,
                                                  bool)
{
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::queueBuffer(int32_t bufferID,
                                             bool    exclusive)
{
    DP_TRACE_CALL();
    BufferList::iterator iterator;
    BufferNode           *pCurNode;

    if (bufferID < 0)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    pCurNode = findBufferPool(bufferID);
    if (0 == pCurNode)
    {
        return DP_STATUS_INVALID_PARAX;
    }
    /*
    if (STATE_DEQUEUED != pCurNode->state)
    {
        return DP_STATUS_INVALID_STATE;
    }
    */

    if (true == exclusive)
    {
        // Remove the old nodes in the queue
        for (iterator = m_bufferQueue.begin(); iterator != m_bufferQueue.end(); iterator++)
        {
            iterator->state = STATE_FREE;
            // Increase free buffer count
            m_availCount++;
        }

        m_bufferQueue.clear();
    }

    m_bufferQueue.push_back(pCurNode);
    pCurNode->state = STATE_QUEUED;
    // Increase queued buffer count
    m_queueCount++;
    // Wakeup consumer
    m_consumer.signal();

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::cancelBuffer(int32_t bufferID)
{
    DP_TRACE_CALL();
    BufferList::iterator iterator;

    if (bufferID < 0)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    for (iterator = m_bufferQueue.begin(); iterator != m_bufferQueue.end(); iterator++)
    {
        if ((bufferID     == iterator->bufID) &&
            (STATE_QUEUED == iterator->state))
        {
            iterator->state = STATE_FREE;
            m_bufferQueue.remove(iterator);
            // Increase free buffer count
            m_availCount++;

            m_producer.signal();

            return DP_STATUS_RETURN_SUCCESS;
        }
    }

    return DP_STATUS_INVALID_BUFFER;
}


DP_STATUS_ENUM DpBufferPoolBase::releaseBuffer(int32_t bufferID)
{
    DP_TRACE_CALL();
    BufferList::iterator iterator;

    if (bufferID < 0)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    for (iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        if (bufferID == iterator->bufID)
        {
            if (STATE_ACQUIRED == iterator->state)
            {
                iterator->state = STATE_FREE;
                // Increase free buffer count
                m_availCount++;
                // Wakeup producer
                m_producer.signal();

                return DP_STATUS_RETURN_SUCCESS;
            }
            else
            {
                return DP_STATUS_INVALID_STATE;
            }
        }
    }

    return DP_STATUS_INVALID_BUFFER;
}

DP_STATUS_ENUM DpBufferPoolBase::waitBufferFence(int32_t bufferID)
{
    DP_TRACE_CALL();
    BufferList::iterator iterator;
    int32_t fenceFd = -1;
    bool found = false;

    if (bufferID < 0)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    m_poolMutex.lock();

    for (iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        if (bufferID == iterator->bufID)
        {
            fenceFd = iterator->fenceFd;
            found = true;
            break;
        }
    }

    m_poolMutex.unlock();

    if (!found)
    {
        return DP_STATUS_INVALID_BUFFER;
    }

    if (-1 != fenceFd)
    {
        DPLOGI("DpBufferPoolBase: Wait fenceFd(%d)\n", fenceFd);

        int32_t err;

        err = sync_wait(fenceFd, 1000);

        if (err < 0)
        {
            DPLOGE("DpBufferPoolBase: fence %d sync_wait failed: %s\n", fenceFd, strerror(errno));
            dumpFenceInfo(fenceFd);
            close(fenceFd);

            return DP_STATUS_OPERATION_FAILED;
        }

        close(fenceFd);
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::getHWBaseWithID(int32_t      bufferID,
                                                 DpEngineType type,
                                                 uint32_t     base[3],
                                                 uint32_t     size[3])
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM       status;
    BufferList::iterator iterator;

    if (bufferID < 0)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    for (iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        if (bufferID == iterator->bufID)
        {
            status = decorateProxy(&(*iterator));
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }

            if ((DP_MEMORY_ION == iterator->memType) && // ION buffers by single FD
                (iterator->addr[0] == iterator->addr[1]) && (iterator->plane != 1))
            {
                base[0] = iterator->pProxy[0]->mapHWAddress(type, 0);
                if (0 == base[0])
                {
                    return DP_STATUS_INVALID_BUFFER;
                }

                if (0 == (iterator->pProxy[0]->mapHWAddress(type, 1)))
                {
                    return DP_STATUS_INVALID_BUFFER;
                }

                if (3 == iterator->plane)
                {
                    if (0 == (iterator->pProxy[0]->mapHWAddress(type, 2)))
                    {
                        return DP_STATUS_INVALID_BUFFER;
                    }
                }

                size[0] = iterator->size[0];

                base[1] = base[0] + size[0];
                size[1] = iterator->size[1];

                if (3 == iterator->plane)
                {
                    base[2] = base[1] + size[1];
                    size[2] = iterator->size[2];
                }
            }
            else
            {
                switch (iterator->plane)
                {
                    case 3:
                        base[2] = iterator->pProxy[2]->mapHWAddress(type, 2);
                        size[2] = iterator->size[2];

                        if (0 == base[2])
                        {
                            return DP_STATUS_INVALID_BUFFER;
                        }
                    case 2:
                        base[1] = iterator->pProxy[1]->mapHWAddress(type, 1);
                        size[1] = iterator->size[1];

                        if (0 == base[1])
                        {
                            return DP_STATUS_INVALID_BUFFER;
                        }
                    case 1:
                        base[0] = iterator->pProxy[0]->mapHWAddress(type, 0);
                        size[0] = iterator->size[0];

                        if (0 == base[0])
                        {
                            return DP_STATUS_INVALID_BUFFER;
                        }
                        break;
                    default:
                        DPLOGE("DpBufferPoolBase: invalid plane count: %d\n", iterator->plane);
                        assert(0);
                        return DP_STATUS_INVALID_STATE;
                }
            }
            break;
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::getSWBaseWithID(int32_t  bufferID,
                                                 void     *pBase[3],
                                                 uint32_t size[3])
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM       status;
    BufferList::iterator iterator;

    if (bufferID < 0)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    for (iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        if (bufferID == iterator->bufID)
        {
            status = decorateProxy(&(*iterator));
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }

            if ((DP_MEMORY_ION == iterator->memType) && // ION buffers by single FD
                (iterator->addr[0] == iterator->addr[1]) && (iterator->plane != 1))
            {
                pBase[0] = iterator->pProxy[0]->mapSWAddress();
                size[0]  = iterator->size[0];

                pBase[1] = (uint8_t*)pBase[0] + size[0];
                size[1]  = iterator->size[1];

                if (3 == iterator->plane)
                {
                    pBase[2] = (uint8_t*)pBase[1] + size[1];
                    size[2]  = iterator->size[2];
                }
            }
            else
            {
                switch (iterator->plane)
                {
                    case 3:
                        pBase[2] = iterator->pProxy[2]->mapSWAddress();
                        size[2]  = iterator->size[2];
                    case 2:
                        pBase[1] = iterator->pProxy[1]->mapSWAddress();
                        size[1]  = iterator->size[1];
                    case 1:
                        pBase[0] = iterator->pProxy[0]->mapSWAddress();
                        size[0]  = iterator->size[0];
                        break;
                    default:
                        DPLOGE("DpBufferPoolBase: invalid plane count: %d\n", iterator->plane);
                        assert(0);
                        return DP_STATUS_INVALID_STATE;
                }
            }
            break;
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::flushReadBuffer(int32_t bufferID)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    BufferNode     *pCurNode;

    if (bufferID < 0)
    {
        DPLOGE("DpBufferPoolBase: invalid buffer ID\n");
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    pCurNode = findBufferPool(bufferID);
    if (0 == pCurNode)
    {
        DPLOGE("DpBufferPoolBase: can't find node\n");
        return DP_STATUS_INVALID_PARAX;
    }

    status = decorateProxy(pCurNode);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpBufferPoolBase: decorate proxy failed(%d)\n", status);
        return status;
    }

#if (CONFIG_FOR_OS_WINDOWS || CONFIG_FOR_FLUSH_RANGE)
    switch(pCurNode->plane)
    {
        case 3:
            if (0 != pCurNode->pProxy[2])
            {
                pCurNode->pProxy[2]->flushMemory(DpMemory::FLUSH_BEFORE_HW_READ_BUFFER_RANGE);
            }
        case 2:
            if (0 != pCurNode->pProxy[1])
            {
                pCurNode->pProxy[1]->flushMemory(DpMemory::FLUSH_BEFORE_HW_READ_BUFFER_RANGE);
            }
        case 1:
            if (0 != pCurNode->pProxy[0])
            {
                pCurNode->pProxy[0]->flushMemory(DpMemory::FLUSH_BEFORE_HW_READ_BUFFER_RANGE);
            }
            break;
        default:
            DPLOGE("DpBufferPoolBase: invalid plane count: %d\n", pCurNode->plane);
            assert(0);
            return DP_STATUS_INVALID_STATE;
    }
#else
    // Only need to flush one plane
    pCurNode->pProxy[0]->flushMemory(DpMemory::FLUSH_BEFORE_HW_READ_WHOLE_CACHE);
#endif  // CONFIG_FOR_FLUSH_RANGE

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::flushWriteBuffer(int32_t bufferID)
{
    DP_TRACE_CALL();
    BufferNode     *pCurNode;
    DP_STATUS_ENUM status;

    if (bufferID < 0)
    {
        DPLOGE("DpBufferPoolBase: invalid buffer ID\n");
        return DP_STATUS_INVALID_PARAX;
    }

#if (CONFIG_FOR_OS_WINDOWS || CONFIG_FOR_FLUSH_RANGE)
    AutoMutex lock(m_poolMutex);

    pCurNode = findBufferPool(bufferID);
    if (0 == pCurNode)
    {
        DPLOGE("DpBufferPoolBase: can't find node\n");
        return DP_STATUS_INVALID_PARAX;
    }

    status = decorateProxy(pCurNode);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpBufferPoolBase: decorate proxy failed(%d)\n", status);
        return status;
    }

    switch(pCurNode->plane)
    {
        case 3:
            if (0 != pCurNode->pProxy[2])
            {
                pCurNode->pProxy[2]->flushMemory(DpMemory::FLUSH_AFTER_HW_WRITE_BUFFER_RANGE);
            }
        case 2:
            if (0 != pCurNode->pProxy[1])
            {
                pCurNode->pProxy[1]->flushMemory(DpMemory::FLUSH_AFTER_HW_WRITE_BUFFER_RANGE);
            }
        case 1:
            if (0 != pCurNode->pProxy[0])
            {
                pCurNode->pProxy[0]->flushMemory(DpMemory::FLUSH_AFTER_HW_WRITE_BUFFER_RANGE);
            }
            break;
        default:
            DPLOGE("DpBufferPoolBase: invalid plane count: %d\n", pCurNode->plane);
            assert(0);
            return DP_STATUS_INVALID_STATE;
    }
#else
    DP_UNUSED(pCurNode);
    DP_UNUSED(status);
#endif  // (CONFIG_FOR_OS_WINDOWS || CONFIG_FOR_FLUSH_RANGE)
    DPLOGI("DpBufferPoolBase: flush write buffer end\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::abortPoll(CLIENT_TYPE_ENUM client)
{
    AutoMutex lock(m_poolMutex);

    m_abortPool = true;

    if (CLIENT_PRODUCER == client)
    {
        m_producer.signal();
    }
    else
    {
        m_consumer.signal();
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::activateBuffer()
{
    AutoMutex lock(m_poolMutex);

    m_abortPool = false;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::dumpDebugInfo(CLIENT_TYPE_ENUM client)
{
    char bufferInfoStr[256] = "";

    DP_UNUSED(client);
    {
        AutoMutex lock(m_poolMutex);

        DPLOGI("DpBufferPoolBase: dump %s debug info begin\n", CLIENT_PRODUCER == client ? "producer" : "consumer");
        DPLOGD("DpBufferPoolBase: total buffer count %d\n", m_bufferCount);
        DPLOGD("DpBufferPoolBase: free buffer count %d\n", m_availCount);
        DPLOGD("DpBufferPoolBase: queue buffer count %d\n", m_queueCount);
    }
    dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
    DPLOGD("DpBufferPoolBase: %s\n", bufferInfoStr);
    DPLOGI("DpBufferPoolBase: dump %s debug info end\n", CLIENT_PRODUCER == client ? "producer" : "consumer");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::dumpBufferInfo(char* bufferInfoStr, uint32_t strLen)
{
    BufferList::iterator iterator;
    void     *pVABase[3] = {0};
    uint32_t mvaBase[3] = {0};
    uint32_t bufSize[3] = {0};

    AutoMutex lock(m_poolMutex);

    for (iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        switch(iterator->plane)
        {
            case 3:
                pVABase[2]  = (void*)(iterator->addr[2]);
                mvaBase[2]  = iterator->addrMVA[2];
                bufSize[2] = iterator->size[2];
            case 2:
                pVABase[1]  = (void*)(iterator->addr[1]);
                mvaBase[1]  = iterator->addrMVA[1];
                bufSize[1] = iterator->size[1];
            case 1:
                pVABase[0]  = (void*)(iterator->addr[0]);
                mvaBase[0]  = iterator->addrMVA[0];
                bufSize[0] = iterator->size[0];
                break;
            default:
                DPLOGE("DpBasicBufferPool: invalid plane count: %d\n", iterator->plane);
                assert(0);
        }

        // Only one buffer in list, merge buffer info with in/output information
        if(m_bufferList.size() == 1)
        {
            snprintf(bufferInfoStr, strLen,
                    "Buffer %d: plane=%d bufVA={%p,%p,%p} bufMVA={0x%x,0x%x,0x%x} bufSize={%d,%d,%d} fence=%d",
                    iterator->bufID, iterator->plane,
                    pVABase[0], pVABase[1], pVABase[2],
                    mvaBase[0], mvaBase[1], mvaBase[2],
                    bufSize[0], bufSize[1], bufSize[2],
                    iterator->fenceFd);
            break;
        }

        DPLOGD("Buffer %d: plane=%d bufVA={%p,%p,%p} bufMVA={0x%x,0x%x,0x%x} bufSize={%d,%d,%d} fence=%d\n",
                iterator->bufID, iterator->plane,
                pVABase[0], pVABase[1], pVABase[2],
                mvaBase[0], mvaBase[1], mvaBase[2],
                bufSize[0], bufSize[1], bufSize[2],
                iterator->fenceFd);
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::dumpBuffer(int32_t       bufferID,
                                            DpColorFormat format,
                                            int32_t       width,
                                            int32_t       height,
                                            int32_t       YPitch,
                                            int32_t       UVPitch,
                                            const char    *keyword)
{
    char     name[256] = "";
    void     *pBase[3] = {0};
    uint32_t size[3]   = {0};
    DP_STATUS_ENUM status;
    struct stat st;

    memset(&st, 0, sizeof(st));

    status = getSWBaseWithID(bufferID, pBase, size);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpBufferPoolBase: dump buffer %d fail: cannot get buffer VA\n", bufferID);
        return status;
    }

    AutoMutex lock(m_poolMutex);

    if (0 == DP_COLOR_GET_BLOCK_MODE(format))
    {
        width = (YPitch << 3) / DP_COLOR_BITS_PER_PIXEL(format);

        if (size[0] != uint32_t(height * YPitch))
        {
            height = size[0] / YPitch;
        }
    }

#if CONFIG_FOR_OS_ANDROID
    // create dir for data dump
    if (stat(DUMP_ROOT, &st) == -1)
    {
        if (mkdir(DUMP_ROOT, 0770) == -1 && errno != EEXIST)
        {
            DPLOGE("mkdir %s error\n", DUMP_ROOT);
        }
    }

    int current_tid = gettid();
#else
    int current_tid = 0;
#endif  // CONFIG_FOR_OS_ANDROID

    DPLOGD("DpBufferPoolBase: dump buffer format %#010x, width %d, height %d, Y pitch %d, UV pitch %d\n", format, width, height, YPitch, UVPitch);

    sprintf(name, "%s%05d_%s.bmp", DUMP_ROOT, current_tid, keyword);
    DPLOGD("DpBufferPoolBase: dump file %s\n", name);

    if (DP_STATUS_RETURN_SUCCESS != utilWriteBMP(name,
                                                 pBase[0],
                                                 pBase[1],
                                                 pBase[2],
                                                 format,
                                                 width,
                                                 height,
                                                 YPitch,
                                                 UVPitch))
    {
        DPLOGE("dump buffer %s error\n", name);
    }

    sprintf(name, "%s%05d_%s.bin", DUMP_ROOT, current_tid, keyword);
    DPLOGD("DpBufferPoolBase: dump file %s\n", name);

    if (DP_STATUS_RETURN_SUCCESS != utilWriteBin(name,
                                                 pBase[0],
                                                 pBase[1],
                                                 pBase[2],
                                                 format,
                                                 width,
                                                 height,
                                                 YPitch,
                                                 UVPitch))
    {
        DPLOGE("dump buffer %s error\n", name);
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBufferPoolBase::decorateProxy(BufferNode *pCurNode)
{
    DP_TRACE_CALL();
    uint32_t size;

    if (true == pCurNode->decorate)
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    if (DP_MEMORY_ION == pCurNode->memType)
    {
        // ION buffers by single FD
        if ((pCurNode->addr[0] == pCurNode->addr[1]) && (pCurNode->plane != 1))
        {
            size = pCurNode->size[0] + pCurNode->size[1] + pCurNode->size[2];

            DPLOGI("New ion proxy - FD:%d, size :%d", pCurNode->addr[0], size);

            pCurNode->pProxy[0] = DpMemory::Factory(pCurNode->memType, (unsigned long)pCurNode->addr[0], size);
            assert(0 != pCurNode->pProxy[0]);
        }
        else
        {
            switch (pCurNode->plane)
            {
                case 3:
                    pCurNode->pProxy[2] = DpMemory::Factory(pCurNode->memType, (unsigned long)pCurNode->addr[2], pCurNode->size[2]);
                    assert(0 != pCurNode->pProxy[2]);
                case 2:
                    pCurNode->pProxy[1] = DpMemory::Factory(pCurNode->memType, (unsigned long)pCurNode->addr[1], pCurNode->size[1]);
                    assert(0 != pCurNode->pProxy[1]);
                case 1:
                    pCurNode->pProxy[0] = DpMemory::Factory(pCurNode->memType, (unsigned long)pCurNode->addr[0], pCurNode->size[0]);
                    assert(0 != pCurNode->pProxy[0]);
                    break;
                default:
                    DPLOGE("DpBufferPoolBase: invalid plane count: %d\n", pCurNode->plane);
                    assert(0);
                    return DP_STATUS_INVALID_STATE;
            }
        }
    }
    else if (DP_MEMORY_MVA == pCurNode->memType)
    {
        switch (pCurNode->plane)
        {
            case 3:
                pCurNode->pProxy[2] = DpMemory::Factory(pCurNode->memType, (void*)pCurNode->addr[2], pCurNode->addrMVA[2], pCurNode->size[2]);
                assert(0 != pCurNode->pProxy[2]);
            case 2:
                pCurNode->pProxy[1] = DpMemory::Factory(pCurNode->memType, (void*)pCurNode->addr[1], pCurNode->addrMVA[1], pCurNode->size[1]);
                assert(0 != pCurNode->pProxy[1]);
            case 1:
                pCurNode->pProxy[0] = DpMemory::Factory(pCurNode->memType, (void*)pCurNode->addr[0], pCurNode->addrMVA[0], pCurNode->size[0]);
                assert(0 != pCurNode->pProxy[0]);
                break;
            default:
                DPLOGE("DpBufferPoolBase: invalid plane count: %d\n", pCurNode->plane);
                assert(0);
                return DP_STATUS_INVALID_STATE;
        }
    }
    else
    {
        switch (pCurNode->plane)
        {
            case 3:
                pCurNode->pProxy[2] = DpMemory::Factory(pCurNode->memType, (void*)pCurNode->addr[2], pCurNode->size[2]);
                assert(0 != pCurNode->pProxy[2]);
            case 2:
                pCurNode->pProxy[1] = DpMemory::Factory(pCurNode->memType, (void*)pCurNode->addr[1], pCurNode->size[1]);
                assert(0 != pCurNode->pProxy[1]);
            case 1:
                pCurNode->pProxy[0] = DpMemory::Factory(pCurNode->memType, (void*)pCurNode->addr[0], pCurNode->size[0]);
                assert(0 != pCurNode->pProxy[0]);
                break;
            default:
                DPLOGE("DpBufferPoolBase: invalid plane count: %d\n", pCurNode->plane);
                assert(0);
                return DP_STATUS_INVALID_STATE;
        }
    }

    pCurNode->decorate = true;

    return DP_STATUS_RETURN_SUCCESS;
}


int32_t DpBufferPoolBase::insertBufferNode(DP_MEMORY_ENUM memType,
                                           int32_t        plane,
                                           void           **pAddr,
                                           uint32_t       *pMVAddr,
                                           uint32_t       *pSize,
                                           DpMemory       *pProxy[3],
                                           int32_t        fenceFd)
{
    int32_t    nodeID;
    BufferNode *pNode;
    DP_STATUS_ENUM status;

    nodeID = nextBufferID();

    pNode = new BufferNode();
    if (NULL == pNode)
    {
        return -1;
    }

    memset(pNode, 0x0, sizeof(BufferNode));

    pNode->bufID   = nodeID;
    pNode->memType = memType;
    pNode->plane   = plane;

    switch (plane)
    {
        case 3:
            if (NULL != pMVAddr)
            {
                pNode->addrMVA[2] = pMVAddr[2];
            }
            if (NULL != pAddr)
            {
                pNode->addr[2]    = pAddr[2];
            }
            pNode->size[2] = pSize[2];
            if (NULL != pProxy)
            {
                pNode->pProxy[2]  = pProxy[2];
            }
        case 2:
            if (NULL != pMVAddr)
            {
                pNode->addrMVA[1] = pMVAddr[1];
            }
            if (NULL != pAddr)
            {
                pNode->addr[1]    = pAddr[1];
            }
            pNode->size[1] = pSize[1];
            if (NULL != pProxy)
            {
                pNode->pProxy[1]  = pProxy[1];
            }
        case 1:
            if (NULL != pMVAddr)
            {
                pNode->addrMVA[0] = pMVAddr[0];
            }
            if (NULL != pAddr)
            {
                pNode->addr[0]    = pAddr[0];
            }
            pNode->size[0] = pSize[0];
            if (NULL != pProxy)
            {
                pNode->pProxy[0]  = pProxy[0];
            }
            break;
        default:
            delete pNode;
            return -1;
    }

    pNode->state    = STATE_FREE;
    pNode->decorate = false;

    if (1 == plane)
    {
        if (NULL != pNode->pProxy[0])
        {
            pNode->decorate = true;
        }
    }
    else if (2 == plane)
    {
        if ((NULL != pNode->pProxy[0]) &&
            (NULL != pNode->pProxy[1]))
        {
            pNode->decorate = true;
        }
    }
    else if (3 == plane)
    {
        if ((NULL != pNode->pProxy[0]) &&
            (NULL != pNode->pProxy[1]) &&
            (NULL != pNode->pProxy[2]))
        {
            pNode->decorate = true;
        }
    }

    if (memType == DP_MEMORY_ION)
    {
        // ION buffers by single FD
        if ((1 != plane) && (NULL != pAddr) && (pAddr[0] == pAddr[1]))
        {
            if (NULL != pNode->pProxy[0])
            {
                pNode->decorate = true;
            }
        }

        // hold ION buffer on insertion
        if (false == pNode->decorate)
        {
            status = decorateProxy(pNode);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                delete pNode;
                return -1;
            }
        }
    }

    pNode->fenceFd = fenceFd;

    {
        AutoMutex lock(m_poolMutex);

        // Got a free buffer
        m_bufferCount++;
        m_availCount++;

        m_bufferList.push_back(pNode);
        m_producer.signal();
    }

    return nodeID;
}

DP_STATUS_ENUM DpBufferPoolBase::dumpFenceInfo(int fd)
{
    if (-1 == fd)
    {
        return DP_STATUS_INVALID_FILE;
    }
#if 0   // TODO: implement mdp_sync_fence_info (libsync removed it since android q)
    // sync point info
    struct sync_fence_info_data *info = sync_fence_info(fd);
    if (info)
    {
        struct sync_pt_info *pt_info = NULL;
        // status: active(0) signaled(1) error(<0)
        DPLOGE("DpBufferPoolBase: fence(%s) status(%d)\n", info->name, info->status);

        // iterate all sync points
        while ((pt_info = sync_pt_info(info, pt_info)))
        {
            if (NULL != pt_info)
            {
                int ts_sec = pt_info->timestamp_ns / 1000000000LL;
                int ts_usec = (pt_info->timestamp_ns % 1000000000LL) / 1000LL;

                DPLOGE("DpBufferPoolBase: sync point, timeline(%s) drv(%s) status(%d) sync_drv(%u) timestamp(%d.%06d)",
                        pt_info->obj_name,
                        pt_info->driver_name,
                        pt_info->status,
                        *(uint32_t *)pt_info->driver_data,
                        ts_sec, ts_usec);
            }
        }
        sync_fence_info_free(info);
    }
#endif
    return DP_STATUS_RETURN_SUCCESS;
}

// The following APIs are for basic buffer pool
DpBasicBufferPool::DpBasicBufferPool()
    : DpBufferPoolBase(BUFFER_BASIC),
      m_secureMode(DP_SECURE_NONE)
{
}


DpBasicBufferPool::~DpBasicBufferPool()
{
}


DP_STATUS_ENUM DpBasicBufferPool::setSecureMode(DpSecure secure)
{
    AutoMutex lock(m_poolMutex);

    m_secureMode = secure;
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBasicBufferPool::registerBuffer(void     *pVABase[3],
                                                 uint32_t pSizeList[3],
                                                 int32_t  planeNumber,
                                                 int32_t  fenceFd,
                                                 int32_t  *pBufferID)
{
    void **pVAddr;

    if ((0 == pVABase) ||
        (0 == pSizeList) ||
        (planeNumber <= 0) ||
        (planeNumber > 3) ||
        (0 == pBufferID))
    {
        DPLOGE("DpBasicBufferPool: invalid parameter: VA = %p, size = %p, plane = %d or pBuffer = %p\n", pVABase, pSizeList, planeNumber, pBufferID);
        return DP_STATUS_INVALID_PARAX;
    }

    pVAddr = pVABase;
    switch(planeNumber)
    {
        case 3:
            if ((0 == pVAddr[2]) || (0 == pSizeList[2]))
            {
                DPLOGE("Invalid buffer parameter, plane 3: address = %p, size = %d\n", pVAddr[2], pSizeList[2]);
                return DP_STATUS_INVALID_PARAX;
            }
        case 2:
            if ((0 == pVAddr[1]) || (0 == pSizeList[1]))
            {
                DPLOGE("Invalid buffer parameter, plane 2: address = %p, size = %d\n", pVAddr[1], pSizeList[1]);
                return DP_STATUS_INVALID_PARAX;
            }
        case 1:
            if ((0 == pVAddr[0]) || (0 == pSizeList[0]))
            {
                DPLOGE("Invalid buffer parameter, plane 1: address = %p, size = %d\n", pVAddr[0], pSizeList[0]);
                return DP_STATUS_INVALID_PARAX;
            }
            break;
        default:
            DPLOGE("DpBasicBufferPool: invalid plane count: %d\n", planeNumber);
            assert(0);
            return DP_STATUS_INVALID_PARAX;
    }

    *pBufferID = insertBufferNode(DP_MEMORY_VA,
                                  planeNumber,
                                  pVAddr,
                                  NULL,
                                  pSizeList,
                                  NULL,
                                  fenceFd);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBasicBufferPool::registerBuffer(uint32_t pMVABase[3],
                                                 uint32_t pSizeList[3],
                                                 int32_t  planeNumber,
                                                 int32_t  fenceFd,
                                                 int32_t  *pBufferID)
{
    uint32_t *pMVAddr;

    if ((0 == pMVABase) ||
        (0 == pSizeList) ||
        (planeNumber <= 0) ||
        (planeNumber > 3) ||
        (0 == pBufferID))
    {
        DPLOGE("DpBasicBufferPool: invalid parameter: MVA = %p, size = %p, plane = %d or pBuffer = %p\n", pMVABase, pSizeList, planeNumber, pBufferID);
        return DP_STATUS_INVALID_PARAX;
    }

    pMVAddr = (uint32_t*)pMVABase;
    switch(planeNumber)
    {
        case 3:
            if ((0 == pMVAddr[2]) || (0 == pSizeList[2]))
            {
                DPLOGE("Invalid buffer parameter, plane 3: MVA = 0x%08x, size = %d\n", pMVAddr[2], pSizeList[2]);
                return DP_STATUS_INVALID_PARAX;
            }
        case 2:
            if ((0 == pMVAddr[1]) || (0 == pSizeList[1]))
            {
                DPLOGE("Invalid buffer parameter, plane 2: MVA = 0x%08x, size = %d\n", pMVAddr[1], pSizeList[1]);
                return DP_STATUS_INVALID_PARAX;
            }
        case 1:
            if ((0 == pMVAddr[0]) || (0 == pSizeList[0]))
            {
                DPLOGE("Invalid buffer parameter, plane 1: MVA = 0x%08x, size = %d\n", pMVAddr[0], pSizeList[0]);
                return DP_STATUS_INVALID_PARAX;
            }
            break;
        default:
            DPLOGE("DpBasicBufferPool: invalid plane count: %d\n", planeNumber);
            assert(0);
            return DP_STATUS_INVALID_PARAX;
    }

    // Get buffer ID
    *pBufferID = insertBufferNode(DP_MEMORY_MVA,
                                  planeNumber,
                                  NULL,
                                  pMVAddr,
                                  pSizeList,
                                  NULL,
                                  fenceFd);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBasicBufferPool::registerBuffer(void     *pVABase[3],
                                                 uint32_t pMVABase[3],
                                                 uint32_t pSizeList[3],
                                                 int32_t  planeNumber,
                                                 int32_t  fenceFd,
                                                 int32_t  *pBufferID)
{
    void     **pVAddr;
    uint32_t *pMVAddr;

    if ((0 == pVABase) ||
        (0 == pMVABase) ||
        (0 == pSizeList) ||
        (planeNumber <= 0) ||
        (planeNumber > 3) ||
        (0 == pBufferID))
    {
        DPLOGE("DpBasicBufferPool: invalid parameter: VA = %p, MVA = %p, size = %p, plane = %d or pBuffer = %p\n", pVABase, pMVABase, pSizeList, planeNumber, pBufferID);
        return DP_STATUS_INVALID_PARAX;
    }

    pVAddr  = pVABase;
    pMVAddr = (uint32_t*)pMVABase;
    switch(planeNumber)
    {
        case 3:
            if ((0 == pVAddr[2]) || (0 == pMVAddr[2]) || (0 == pSizeList[2]))
            {
                DPLOGE("Invalid buffer parameter, plane 3: VA = %p, MVA = 0x%08x, size = %d\n", pVAddr[2], pMVAddr[2], pSizeList[2]);
                return DP_STATUS_INVALID_PARAX;
            }
        case 2:
            if ((0 == pVAddr[1]) || (0 == pMVAddr[1]) || (0 == pSizeList[1]))
            {
                DPLOGE("Invalid buffer parameter, plane 2: VA = %p, MVA = 0x%08x, size = %d\n", pVAddr[1], pMVAddr[1], pSizeList[1]);
                return DP_STATUS_INVALID_PARAX;
            }
        case 1:
            if ((0 == pVAddr[0]) || (0 == pMVAddr[0]) || (0 == pSizeList[0]))
            {
                DPLOGE("Invalid buffer parameter, plane 1: VA = %p, MVA = 0x%08x, size = %d\n", pVAddr[0], pMVAddr[0], pSizeList[0]);
                return DP_STATUS_INVALID_PARAX;
            }
            break;
        default:
            DPLOGE("DpBasicBufferPool: invalid plane count: %d\n", planeNumber);
            assert(0);
            return DP_STATUS_INVALID_PARAX;
    }

    // Get buffer ID
    *pBufferID = insertBufferNode(DP_MEMORY_MVA,
                                  planeNumber,
                                  pVAddr,
                                  pMVAddr,
                                  pSizeList,
                                  NULL,
                                  fenceFd);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBasicBufferPool::registerBufferFD(int32_t  fileDesc,
                                                   uint32_t *pSizeList,
                                                   int32_t  planeNumber,
                                                   int32_t  fenceFd,
                                                   int32_t  *pBufferID)
{
    unsigned long fileList[3];

    if ((fileDesc == -1) ||
        (0 == pSizeList) ||
        (planeNumber <= 0) ||
        (planeNumber > 3) ||
        (0 == pBufferID))
    {
        DPLOGE("DpBasicBufferPool: invalid parameter: fileDesc = 0x%08x, pSizeList = %p, planeNumber = %d or pBuffer = %p\n", fileDesc, pSizeList, planeNumber, pBufferID);
        return DP_STATUS_INVALID_PARAX;
    }

    switch(planeNumber)
    {
        case 3:
            fileList[2] = fileDesc;
        case 2:
            fileList[1] = fileDesc;
        case 1:
            fileList[0] = fileDesc;
            break;
        default:
            DPLOGE("DpBasicBufferPool: invalid buffer plane count(%d)\n", planeNumber);
            return DP_STATUS_INVALID_PARAX;
    }

    *pBufferID = insertBufferNode(DP_MEMORY_ION,
                                  planeNumber,
                                  (void **)fileList,
                                  NULL,
                                  pSizeList,
                                  NULL,
                                  fenceFd);

    DPLOGI("DpBasicBufferPool: register buffer FD(%d) with ID(%d)", fileDesc, *pBufferID);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBasicBufferPool::dequeueBuffer(int32_t  *pBufID,
                                                void     *pVABase[3],
                                                uint32_t pBufSize[3],
                                                bool     waitBuf)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM       status;
    BufferList::iterator iterator;

    if (NULL == pBufID)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    while (0 == m_availCount)
    {
        if ((true == waitBuf) && (false == m_abortPool))
        {
            m_producer.wait(m_poolMutex);
            waitBuf = false;
        }
        else if (true == m_abortPool)
        {
            *pBufID = -1;
            return DP_STATUS_ABORTED_BY_USER;
        }
        else
        {
            *pBufID = -1;
            return DP_STATUS_BUFFER_FULL;
        }
    }

    for(iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        if (STATE_FREE == iterator->state)
        {
            status = decorateProxy(&(*iterator));
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }

            iterator->state = STATE_DEQUEUED;

            // Default values
            pVABase[1] = 0;
            pVABase[2] = 0;

            switch(iterator->plane)
            {
                case 3:
                    pVABase[2]  = (void*)(iterator->addr[2]);
                    pBufSize[2] = iterator->size[2];
                case 2:
                    pVABase[1]  = (void*)(iterator->addr[1]);
                    pBufSize[1] = iterator->size[1];
                case 1:
                    pVABase[0]  = (void*)(iterator->addr[0]);
                    pBufSize[0] = iterator->size[0];
                    break;
                default:
                    DPLOGE("DpBasicBufferPool: invalid plane count: %d\n", iterator->plane);
                    assert(0);
                    return DP_STATUS_INVALID_STATE;
            }

            *pBufID = iterator->bufID;

            // Decrease free buffer count
            m_availCount--;

            return DP_STATUS_RETURN_SUCCESS;
        }
    }

    return DP_STATUS_BUFFER_FULL;
}


DP_STATUS_ENUM DpBasicBufferPool::asyncReleaseBuffer(int32_t bufferID)
{
    BufferList::iterator iterator;

    if (bufferID < 0)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    for (iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        if (bufferID == iterator->bufID)
        {
            if (STATE_ACQUIRED == iterator->state)
            {
                iterator->state = STATE_USED;
                // Increase free buffer count
                m_availCount++;
                // Wakeup producer
                m_producer.signal();

                return DP_STATUS_RETURN_SUCCESS;
            }
            else
            {
                return DP_STATUS_INVALID_STATE;
            }
        }
    }

    return DP_STATUS_INVALID_BUFFER;
}


DP_STATUS_ENUM DpBasicBufferPool::acquireBuffer(int32_t  *pBufID,
                                                void     *pVABase[3],
                                                uint32_t pBufSize[3],
                                                bool     waitBuf)
{
    DP_TRACE_CALL();
    BufferNode *pCurNode;

    if (NULL == pBufID)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    while (m_bufferQueue.empty())
    {
        if ((true == waitBuf) && (false == m_abortPool))
        {
            m_consumer.wait(m_poolMutex);
            waitBuf = false;
        }
        else if (true == m_abortPool)
        {
            *pBufID = -1;
            return DP_STATUS_ABORTED_BY_USER;
        }
        else
        {
            *pBufID = -1;
            return DP_STATUS_BUFFER_EMPTY;
        }
    }

    pCurNode = m_bufferQueue.remove(m_bufferQueue.begin());
    pCurNode->state = STATE_ACQUIRED;

    *pBufID = pCurNode->bufID;

    switch(pCurNode->plane)
    {
        case 3:
            pVABase[2]  = (void*)(pCurNode->addr[2]);
            pBufSize[2] = pCurNode->size[2];
        case 2:
            pVABase[1]  = (void*)(pCurNode->addr[1]);
            pBufSize[1] = pCurNode->size[1];
        case 1:
            pVABase[0]  = (void*)(pCurNode->addr[0]);
            pBufSize[0] = pCurNode->size[0];
            break;
        default:
            DPLOGE("No match buffer type : %d", pCurNode->plane);
            assert(0);
            return DP_STATUS_INVALID_STATE;
    }

    // Decrease queued buffer count
    m_queueCount--;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBasicBufferPool::getHWBaseWithID(int32_t      bufferID,
                                                  DpEngineType type,
                                                  uint32_t     base[3],
                                                  uint32_t     size[3])
{
    BufferList::iterator iterator;

    if (DP_SECURE_NONE == m_secureMode)
    {
        return DpBufferPoolBase::getHWBaseWithID(bufferID,
                                                 type,
                                                 base,
                                                 size);
    }
    else
    {
        AutoMutex lock(m_poolMutex);

        for(iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
        {
            if(bufferID  == iterator->bufID)
            {
                base[0] = (uint32_t)((unsigned long)iterator->addr[0]);
                size[0] = iterator->size[0];

                if (1 < iterator->plane)
                {
                   base[1] = iterator->size[0];
                   size[1] = iterator->size[1];
                }

                if (2 < iterator->plane)
                {
                    base[2] = iterator->size[0] + iterator->size[1];
                    size[2] = iterator->size[2];
                }
            }
        }
    }
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpBasicBufferPool::getSWBaseWithID(int32_t      bufferID,
                                                  void         *pBase[3],
                                                  uint32_t     size[3])
{
    if (DP_SECURE_NONE == m_secureMode)
    {
        return DpBufferPoolBase::getSWBaseWithID(bufferID,
                                                 pBase,
                                                 size);
    }
    else
    {
        return DP_STATUS_INVALID_BUFFER;
    }
}


DP_STATUS_ENUM DpBasicBufferPool::unregisterBuffer(int32_t bufferID)
{
    BufferList::iterator iterator;
    BufferNode           *pCurNode;

    DPLOGI("DpBasicBufferPool: unregister buffer id(%d)\n", bufferID);

    if (bufferID < 0)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    for(iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        if (bufferID == iterator->bufID)
        {
            DPLOGI("DpBasicBufferPool: found node %d, decorate %d\n", bufferID, iterator->decorate);

            if ( (STATE_FREE == iterator->state) || (STATE_USED == iterator->state) )
            {
                pCurNode = m_bufferList.remove(iterator);

                if (true == pCurNode->decorate)
                {
                    delete pCurNode->pProxy[2];
                    pCurNode->pProxy[2] = NULL;

                    delete pCurNode->pProxy[1];
                    pCurNode->pProxy[1] = NULL;

                    delete pCurNode->pProxy[0];
                    pCurNode->pProxy[0] = NULL;

                    pCurNode->decorate = false;
                }

                delete pCurNode;

                // Decrease free buffer count
                m_bufferCount--;
                m_availCount--;

                return DP_STATUS_RETURN_SUCCESS;
            }
            else
            {
                DPLOGE("DpBasicBufferPool: node is not in free state\n");
                return DP_STATUS_INVALID_STATE;
            }
        }
    }

    return DP_STATUS_INVALID_BUFFER;
}


// The following APIs are for auto buffer pool
DpAutoBufferPool::DpAutoBufferPool()
    : DpBufferPoolBase(BUFFER_AUTO)
{
}


DpAutoBufferPool::~DpAutoBufferPool()
{
}


DP_STATUS_ENUM DpAutoBufferPool::createBuffer(DpColorFormat format,
                                              int32_t       width,
                                              int32_t       height,
                                              int32_t       pitch,
                                              uint32_t      count)
{
    uint32_t index;
    DpMemory *pProxy[3];
    int32_t  bufID;
    void     *base[3];
    uint32_t size[3];

    if (pitch < width)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    pProxy[0] = 0;
    pProxy[1] = 0;
    pProxy[2] = 0;

    for (index = 0; index < count; index++)
    {
        // Allocate buffer here
        switch(format)
        {
            case DP_COLOR_GREY:
            case DP_COLOR_UYVY:
            case DP_COLOR_VYUY:
            case DP_COLOR_YUYV:
            case DP_COLOR_YVYU:
            case DP_COLOR_RGB565:
            case DP_COLOR_BGR565:
            case DP_COLOR_RGB565_RAW:
            case DP_COLOR_RGB888:
            case DP_COLOR_BGR888:
            case DP_COLOR_RGBX8888:
            case DP_COLOR_BGRX8888:
            case DP_COLOR_XRGB8888:
            case DP_COLOR_XBGR8888:
                pProxy[0] = DpMemory::Factory(DP_MEMORY_ION, -1 , pitch * height);
                if (0 == pProxy[0])
                {
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                base[0] = pProxy[0]->mapSWAddress();  // Handled by memory handler
                size[0] = pitch * height;
                break;
            case DP_COLOR_I420:
            case DP_COLOR_YV12:
                // First plane
                pProxy[0] = DpMemory::Factory(DP_MEMORY_ION, -1, pitch * height);
                if (0 == pProxy[0])
                {
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                // Second plane
                pProxy[1] = DpMemory::Factory(DP_MEMORY_ION, -1, (pitch * height) >> 2);
                if (0 == pProxy[1])
                {
                    delete pProxy[0];
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                // Third plane
                pProxy[2] = DpMemory::Factory(DP_MEMORY_ION, -1, (pitch * height) >> 2);
                if (0 == pProxy[2])
                {
                    delete pProxy[0];
                    delete pProxy[1];
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                base[0] = pProxy[0]->mapSWAddress();  // Handled by memory handler
                base[1] = pProxy[1]->mapSWAddress();  // Handled by memory handler
                base[2] = pProxy[2]->mapSWAddress();  // Handled by memory handler

                size[0] = pitch * height;
                size[1] = (pitch * height) >> 2;
                size[2] = (pitch * height) >> 2;
                break;
            case DP_COLOR_NV12:
            case DP_COLOR_NV21:
                // First plane
                pProxy[0] = DpMemory::Factory(DP_MEMORY_ION, -1, pitch * height);
                if (0 == pProxy[0])
                {
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                // Second plane
                pProxy[1] = DpMemory::Factory(DP_MEMORY_ION, -1, (pitch * height) >> 1);
                if (0 == pProxy[1])
                {
                    delete pProxy[0];
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                base[0] = pProxy[0]->mapSWAddress();  // Handled by memory handler
                base[1] = pProxy[1]->mapSWAddress();  // Handled by memory handler

                // Buffer size of each plane
                size[0] = pitch * height;
                size[1] = (pitch * height) >> 1;
                break;
            case DP_COLOR_I422:
            case DP_COLOR_YV16:
                // First plane
                pProxy[0] = DpMemory::Factory(DP_MEMORY_ION, -1, pitch * height);
                if (0 == pProxy[0])
                {
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                // Second plane
                pProxy[1] = DpMemory::Factory(DP_MEMORY_ION, -1, (pitch * height) >> 1);
                if (0 == pProxy[1])
                {
                    delete pProxy[0];
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                // Third plane
                pProxy[2] = DpMemory::Factory(DP_MEMORY_ION, -1, (pitch * height) >> 1);
                if (0 == pProxy[2])
                {
                    delete pProxy[0];
                    delete pProxy[1];
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                base[0] = pProxy[0]->mapSWAddress();  // Handled by memory handler
                base[1] = pProxy[1]->mapSWAddress();  // Handled by memory handler
                base[2] = pProxy[2]->mapSWAddress();  // Handled by memory handler

                size[0] = pitch * height;
                size[1] = (pitch * height) >> 1;
                size[2] = (pitch * height) >> 1;
                break;
            case DP_COLOR_NV16:
            case DP_COLOR_NV61:
                // First plane
                pProxy[0] = DpMemory::Factory(DP_MEMORY_ION, -1, pitch * height);
                if (0 == pProxy[0])
                {
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                // Second plane
                pProxy[1] = DpMemory::Factory(DP_MEMORY_ION, -1, pitch * height);
                if (0 == pProxy[1])
                {
                    delete pProxy[0];
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                base[0] = pProxy[0]->mapSWAddress();  // Handled by memory handler
                base[1] = pProxy[1]->mapSWAddress();  // Handled by memory handler

                // Buffer size of each plane
                size[0] = pitch * height;
                size[1] = pitch * height;
                break;
            case DP_COLOR_I444:
                pProxy[0] = DpMemory::Factory(DP_MEMORY_ION, -1, pitch * height);
                if (0 == pProxy[0])
                {
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                // Second plane
                pProxy[1] = DpMemory::Factory(DP_MEMORY_ION, -1, pitch * height);
                if (0 == pProxy[1])
                {
                    delete pProxy[0];
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                // Third plane
                pProxy[2] = DpMemory::Factory(DP_MEMORY_ION, -1, pitch * height);
                if (0 == pProxy[2])
                {
                    delete pProxy[0];
                    delete pProxy[1];
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                base[0] = pProxy[0]->mapSWAddress();  // Handled by memory handler
                base[1] = pProxy[1]->mapSWAddress();  // Handled by memory handler
                base[2] = pProxy[2]->mapSWAddress();  // Handled by memory handler

                size[0] = pitch * height;
                size[1] = pitch * height;
                size[2] = pitch * height;
                break;
            case DP_COLOR_NV24:
            case DP_COLOR_NV42:
                pProxy[0] = DpMemory::Factory(DP_MEMORY_ION, -1, pitch * height);
                if (0 == pProxy[0])
                {
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                // Second plane
                pProxy[1] = DpMemory::Factory(DP_MEMORY_ION, -1, pitch * height * 2);
                if (0 == pProxy[1])
                {
                    delete pProxy[0];
                    return DP_STATUS_OUT_OF_MEMORY;
                }

                base[0] = pProxy[0]->mapSWAddress();  // Handled by memory handler
                base[1] = pProxy[1]->mapSWAddress();  // Handled by memory handler

                size[0] = pitch * height;
                size[1] = pitch * height * 2;
                break;
            default:
                assert(0);
                return DP_STATUS_INVALID_FORMAT;
        }

        bufID = insertBufferNode(DP_MEMORY_ION,
                                 DP_COLOR_GET_PLANE_COUNT(format),
                                 base,
                                 NULL,
                                 size,
                                 pProxy,
                                 -1);
        if (bufID < 0)
        {
            return DP_STATUS_INVALID_STATE;
        }
    }

    m_bufferCount  = count;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAutoBufferPool::dequeueBuffer(int32_t  *pBufID,
                                               void     *pVABase[3],
                                               uint32_t pBufSize[3],
                                               bool     waitBuf)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM       status;
    BufferList::iterator iterator;

    if ((0 == pBufID) ||
        (0 == pVABase) ||
        (0 == pBufSize))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    if (m_bufferCount <= 0)
    {
        return DP_STATUS_INVALID_BUFFER;
    }

    while (0 == m_availCount)
    {
        if ((true == waitBuf) && (false == m_abortPool))
        {
            m_producer.wait(m_poolMutex);
            waitBuf = false;
        }
        else if (true == m_abortPool)
        {
            *pBufID = -1;
            return DP_STATUS_ABORTED_BY_USER;
        }
        else
        {
            *pBufID = -1;
            return DP_STATUS_BUFFER_FULL;
        }
    }

    for(iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        if (STATE_FREE == iterator->state)
        {
            status = decorateProxy(&(*iterator));
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                return status;
            }

            iterator->state = STATE_DEQUEUED;

            *pBufID  = iterator->bufID;

            switch(iterator->plane)
            {
                case 3:
                    pVABase[2]  = (void*)(iterator->addr[2]);
                    pBufSize[2] = iterator->size[2];
                case 2:
                    pVABase[1]  = (void*)(iterator->addr[1]);
                    pBufSize[1] = iterator->size[1];
                case 1:
                    pVABase[0]  = (void*)(iterator->addr[0]);
                    pBufSize[0] = iterator->size[0];
                    break;
                default:
                    DPLOGE("DpAutoBufferPool: invalid plane count: %d\n", iterator->plane);
                    assert(0);
                    return DP_STATUS_INVALID_STATE;
            }

            // Decrease free buffer count
            m_availCount--;
            break;
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAutoBufferPool::acquireBuffer(int32_t  *pBufID,
                                               void     *pVABase[3],
                                               uint32_t pBufSize[3],
                                               bool     waitBuf)
{
    DP_TRACE_CALL();
    BufferNode *pCurNode;

    if ((0 == pBufID) ||
        (0 == pVABase) ||
        (0 == pBufSize))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    if (m_bufferCount <= 0)
    {
        return DP_STATUS_INVALID_BUFFER;
    }

    while (m_bufferQueue.empty())
    {
        if ((true == waitBuf) && (false == m_abortPool))
        {
            m_consumer.wait(m_poolMutex);
            waitBuf = false;
        }
        else if (true == m_abortPool)
        {
            *pBufID = -1;
            return DP_STATUS_ABORTED_BY_USER;
        }
        else
        {
            *pBufID = -1;
            return DP_STATUS_BUFFER_EMPTY;
        }
    }

    pCurNode = m_bufferQueue.remove(m_bufferQueue.begin());
    pCurNode->state = STATE_ACQUIRED;

    *pBufID  = pCurNode->bufID;

    switch(pCurNode->plane)
    {
        case 3:
            pVABase[2]  = (void*)pCurNode->addr[2];
            pBufSize[2] = pCurNode->size[2];
        case 2:
            pVABase[1]  = (void*)pCurNode->addr[1];
            pBufSize[1] = pCurNode->size[1];
        case 1:
            pVABase[0]  = (void*)pCurNode->addr[0];
            pBufSize[0] = pCurNode->size[0];
            break;
        default:
            DPLOGE("DpAutoBufferPool: invalid plane count: %d\n", pCurNode->plane);
            assert(0);
            return DP_STATUS_INVALID_STATE;
    }

    // Decrease queued buffer count
    m_queueCount--;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAutoBufferPool::destroyBuffer()
{
    BufferList::iterator  iterator;

    AutoMutex lock(m_poolMutex);

    for(iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        switch(iterator->plane)
        {
            case 3:
                delete iterator->pProxy[2];
                iterator->pProxy[2] = NULL;
            case 2:
                delete iterator->pProxy[1];
                iterator->pProxy[1] = NULL;
            case 1:
                delete iterator->pProxy[0];
                iterator->pProxy[0] = NULL;
                break;
            default:
                DPLOGE("DpAutoBufferPool: invalid plane count: %d\n", iterator->plane);
                assert(0);
                return DP_STATUS_INVALID_STATE;
        }

        delete &(*iterator);
    }

    m_bufferList.clear();

    m_bufferCount = 0;
    return DP_STATUS_RETURN_SUCCESS;
}


// The following APIs are for ring buffer pool
DpRingBufferPool::DpRingBufferPool()
    : DpBufferPoolBase(BUFFER_RING),
      m_bufferFormat(DP_COLOR_UNKNOWN),
      m_bufferWidth(0),
      m_bufferHeight(0),
      m_bufferYPitch(0),
      m_bufferUVPitch(0),
      m_verticalShift(0),
      m_ringMCUYSize(0),
      m_ringMCUCount(0),
      m_ringMCUCol(0),
      m_currentLeft(0),
      m_currentRight(0),
      m_currentTop(-1),
      m_currentBottom(0),
      m_frameDone(false),
      m_embeddedJPEG(false),
      m_bufferFD(-1)
{
    m_pProxy[0]  = NULL;
    m_pProxy[1]  = NULL;
    m_pProxy[2]  = NULL;

    m_pRingVA[0] = NULL;
    m_pRingVA[1] = NULL;
    m_pRingVA[2] = NULL;

    m_ringSize[0] = 0;
    m_ringSize[1] = 0;
    m_ringSize[2] = 0;
}


DpRingBufferPool::~DpRingBufferPool()
{
    BufferList::iterator iterator;

    AutoMutex lock(m_poolMutex);

    for(iterator = m_activeList.begin(); iterator != m_activeList.end(); iterator++)
    {
        DPLOGI("DpRingBufferPool: delete active ring buffer node 0x%08x\n", &(*iterator));
        delete &(*iterator);
    }
    m_activeList.clear();

    for(iterator = m_pendingList.begin(); iterator != m_pendingList.end(); iterator++)
    {
        DPLOGI("DpRingBufferPool: delete pending ring buffer node 0x%08x\n", &(*iterator));
        delete &(*iterator);
    }
    m_pendingList.clear();

    delete m_pProxy[2];
    m_pProxy[2] = NULL;

    delete m_pProxy[1];
    m_pProxy[1] = NULL;

    delete m_pProxy[0];
    m_pProxy[0] = NULL;
}


DP_STATUS_ENUM DpRingBufferPool::pollBufferStatus(CLIENT_TYPE_ENUM client __unused,
                                                  bool             waitBuf)
{
    AutoMutex lock(m_poolMutex);

    while (m_bufferCount <= 0)
    {
        if ((true == waitBuf) && (false == m_abortPool))
        {
            m_producer.wait(m_poolMutex);
            waitBuf = false;
        }
        else if (true == m_abortPool)
        {
            return DP_STATUS_ABORTED_BY_USER;
        }
        else
        {
            return DP_STATUS_INVALID_BUFFER;
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpRingBufferPool::pollBufferStatus(CLIENT_TYPE_ENUM client,
                                                  int32_t          bufID  __unused,
                                                  int32_t          left   __unused,
                                                  int32_t          right  __unused,
                                                  int32_t          top    __unused,
                                                  int32_t          bottom __unused,
                                                  bool             waitBuf)
{
    AutoMutex lock(m_poolMutex);

    if (m_bufferCount <= 0)
    {
        return DP_STATUS_INVALID_BUFFER;
    }

    if (CLIENT_PRODUCER == client)
    {
        // Ring biffer can't be used for producer
        assert(0);

        return DP_STATUS_INVALID_BUFFER;
    }
    else
    {
        while (!m_pendingList.empty())
        {
            if (m_bufferCount <= 0)
            {
                return DP_STATUS_INVALID_BUFFER;
            }
            else
            {
                if ((true == waitBuf) && (false == m_abortPool))
                {
                    m_consumer.wait(m_poolMutex);
                    waitBuf = false;
                }
                else if (true == m_abortPool)
                {
                    return DP_STATUS_ABORTED_BY_USER;
                }
                else
                {
                    return DP_STATUS_BUFFER_EMPTY;
                }
            }
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpRingBufferPool::mapBufferLine(int32_t top,
                                               int32_t bottom,
                                               int32_t *pStartLine,
                                               int32_t *pEndLine) const
{
    uint32_t lineCount;

    if ((0 == pStartLine) ||
        (0 == pEndLine))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    lineCount = m_ringMCUYSize * m_ringMCUCount;

    *pStartLine = top    % lineCount;
    *pEndLine   = bottom % lineCount;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpRingBufferPool::createBuffer(int32_t       MCUYSize,
                                              DpColorFormat srcFormat,
                                              int32_t       srcWidth,
                                              int32_t       srcHeight,
                                              int32_t       srcYPitch,
                                              int32_t       srcUVPitch,
                                              int32_t       dstWidth __unused,
                                              int32_t       dstHeight,
                                              int32_t       mcu_col,
                                              DpRect        *pCropInfo)
{
    int32_t  YPlaneSize;
    int32_t  UVPlaneSize;
    int32_t  totalSize;

    const int32_t  limitMCUCount = 256;

    AutoMutex lock(m_poolMutex);

    if (m_bufferCount > 0)
    {
        return DP_STATUS_INVALID_STATE;
    }

    m_ringMCUYSize = MCUYSize;
    m_ringMCUCount = 2 * (srcHeight / dstHeight) * 16 / MCUYSize + 1;
    if (m_ringMCUCount < limitMCUCount)
    {
        m_ringMCUCount = limitMCUCount;
    }
    m_ringMCUCol = mcu_col;

    m_pProxy[2] = NULL;
    m_pProxy[1] = NULL;
    m_pProxy[0] = NULL;

    YPlaneSize  = m_ringMCUCount * MCUYSize * srcYPitch;  // Y plane size
    UVPlaneSize = 0;

    if (1 == DP_COLOR_GET_PLANE_COUNT(srcFormat))
    {
        totalSize = YPlaneSize;
    }
    else if (3 == DP_COLOR_GET_PLANE_COUNT(srcFormat))
    {
        UVPlaneSize = (m_ringMCUCount * MCUYSize * srcUVPitch) >> DP_COLOR_GET_V_SUBSAMPLE(srcFormat);
        totalSize   = YPlaneSize + UVPlaneSize * 2;
    }
    else if (2 == DP_COLOR_GET_PLANE_COUNT(srcFormat))
    {
        UVPlaneSize = (m_ringMCUCount * MCUYSize * srcUVPitch) >> DP_COLOR_GET_V_SUBSAMPLE(srcFormat);
        if (DP_COLOR_IS_UV_COPLANE(srcFormat))
        {
            UVPlaneSize *= 2;
        }

        totalSize = YPlaneSize + UVPlaneSize;
    }
    else
    {
        return DP_STATUS_INVALID_FORMAT;
    }

    m_pProxy[0]   = DpMemory::Factory(DP_MEMORY_ION, -1, totalSize, true, &m_bufferFD);
    m_ringSize[0] = YPlaneSize;
    m_pRingVA[0]  = m_pProxy[0]->mapSWAddress();

    if (3 == DP_COLOR_GET_PLANE_COUNT(srcFormat))
    {
        m_ringSize[1] = UVPlaneSize;
        m_pRingVA[1]  = (uint8_t*)m_pRingVA[0] + YPlaneSize;
        //m_pProxy[1]   = DpMemory::Factory(DP_MEMORY_VA, m_pRingVA[1], UVPlaneSize);

        m_ringSize[2] = UVPlaneSize;
        m_pRingVA[2]  = (uint8_t*)m_pRingVA[1] + UVPlaneSize;
        //m_pProxy[2]   = DpMemory::Factory(DP_MEMORY_VA, m_pRingVA[2], UVPlaneSize);
    }
    else if (2 == DP_COLOR_GET_PLANE_COUNT(srcFormat))
    {
        m_ringSize[1] = UVPlaneSize;
        m_pRingVA[1]  = (uint8_t*)m_pRingVA[0] + YPlaneSize;
        //m_pProxy[1]   = DpMemory::Factory(DP_MEMORY_VA, m_pRingVA[1], UVPlaneSize);
    }

    m_bufferCount   = 1;
    m_bufferFormat  = srcFormat;
    m_bufferWidth   = srcWidth;
    m_bufferHeight  = srcHeight;
    m_bufferYPitch  = srcYPitch;
    m_bufferUVPitch = srcUVPitch;
    m_verticalShift = DP_COLOR_GET_V_SUBSAMPLE(srcFormat);

    if(0 == pCropInfo)
    {
        m_cropInfo.x = 0;
        m_cropInfo.y = 0;
        m_cropInfo.w = srcWidth;
        m_cropInfo.h = srcHeight;
    }
    else
    {
        m_cropInfo = *pCropInfo;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpRingBufferPool::dequeueBuffer(int32_t  *pBufID,
                                               void     *pVABase[3],
                                               uint32_t pBufSize[3],
                                               bool     waitBuf)
{
    DP_TRACE_CALL();
    BufferList::iterator iterator;
    int32_t              plane;

    if ((0 == pBufID) ||
        (0 == pVABase) ||
        (0 == pBufSize))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    while (m_pendingList.empty())
    {
        if ((m_bufferCount > 0) &&
            (true == waitBuf) && (false == m_abortPool))
        {
            m_producer.wait(m_poolMutex);
            waitBuf = false;
        }
        else
        {
            if (true == m_abortPool)
            {
                return DP_STATUS_ABORTED_BY_USER;
            }
            else
            {
                if (((m_currentBottom + 1) >= m_cropInfo.h) &&
                    ((m_currentRight  + 1) >= m_cropInfo.w))
                {
                    return DP_STATUS_BUFFER_DONE;
                }
            }

            return DP_STATUS_BUFFER_FULL;
        }
    }

    for(iterator = m_pendingList.begin(); iterator != m_pendingList.end(); iterator++)
    {
        if (STATE_FREE == iterator->state)
        {
            *pBufID = iterator->bufID;

            plane = DP_COLOR_GET_PLANE_COUNT(iterator->format);
            switch(plane)
            {
                case 3:
                    pVABase[0]  = iterator->pVABase[0];
                    pBufSize[0] = iterator->size[0];

                    pVABase[1]  = iterator->pVABase[1];
                    pBufSize[1] = iterator->size[1];

                    pVABase[2]  = iterator->pVABase[2];
                    pBufSize[2] = iterator->size[2];
                    break;
                case 2:
                    pVABase[0]  = iterator->pVABase[0];
                    pBufSize[0] = iterator->size[0];

                    pVABase[1]  = iterator->pVABase[1];
                    pBufSize[1] = iterator->size[1];

                    pVABase[2]  = 0;
                    pBufSize[2] = 0;
                    break;
                case 1:
                    pVABase[0]  = iterator->pVABase[0];
                    pBufSize[0] = iterator->size[0];

                    pVABase[1]  = 0;
                    pBufSize[1] = 0;

                    pVABase[2]  = 0;
                    pBufSize[2] = 0;
                    break;
                default:
                    DPLOGE("DpRingBufferPool: invalid plane count: %d\n", plane);
                    assert(0);
                    return DP_STATUS_INVALID_FORMAT;
                    break;
            }

            iterator->state = STATE_ACQUIRED;

            return DP_STATUS_RETURN_SUCCESS;
        }
    }

    return DP_STATUS_BUFFER_FULL;
}


DP_STATUS_ENUM DpRingBufferPool::dequeueBuffer(int32_t       *pBufID,
                                               DpColorFormat *pFormat,
                                               void          *pVABase[3],
                                               int32_t       *pXStart,
                                               int32_t       *pYStart,
                                               int32_t       *pWidth,
                                               int32_t       *pHeight,
                                               int32_t       *pPitch,
                                               bool          waitBuf)
{
    DP_TRACE_CALL();
    BufferList::iterator iterator;
    int32_t              plane;

    if ((0 == pBufID) ||
        (0 == pFormat) ||
        (0 == pVABase) ||
        (0 == pXStart) ||
        (0 == pYStart) ||
        (0 == pWidth) ||
        (0 == pHeight) ||
        (0 == pPitch))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    while (m_pendingList.empty())
    {
        if ((m_bufferCount > 0) &&
            (true == waitBuf) && (false == m_abortPool))
        {
            if (ETIMEDOUT == m_producer.waitRelative(m_poolMutex, 1000000000ll))
            {
                DPLOGE("DpRingBufferPool::dequeueBuffer timeout\n");
                return DP_STATUS_OPERATION_FAILED;
            }
            waitBuf = false;
        }
        else
        {
            if (true == m_abortPool)
            {
                return DP_STATUS_ABORTED_BY_USER;
            }
            else
            {
                if (((m_currentBottom + 1) >= m_cropInfo.h) &&
                    ((m_currentRight  + 1) >= m_cropInfo.w))
                {
                    return DP_STATUS_BUFFER_DONE;
                }
            }

            return DP_STATUS_BUFFER_FULL;
        }
    }

    for(iterator = m_pendingList.begin(); iterator != m_pendingList.end(); iterator++)
    {
        if (STATE_FREE == iterator->state)
        {
            *pBufID = iterator->bufID;

            plane = DP_COLOR_GET_PLANE_COUNT(iterator->format);
            switch(plane)
            {
                case 3:
                    pVABase[0] = iterator->pVABase[0];
                    pVABase[1] = iterator->pVABase[1];
                    pVABase[2] = iterator->pVABase[2];
                    break;
                case 2:
                    pVABase[0] = iterator->pVABase[0];
                    pVABase[1] = iterator->pVABase[1];
                    pVABase[2] = 0;
                    break;
                case 1:
                    pVABase[0] = iterator->pVABase[0];
                    pVABase[1] = 0;
                    pVABase[2] = 0;
                    break;
                default:
                    DPLOGE("DpRingBufferPool: invalid plane count: %d\n", plane);
                    assert(0);
                    return DP_STATUS_INVALID_FORMAT;
                    break;
            }

            *pXStart = iterator->left;
            *pFormat = iterator->format;
            *pYStart = iterator->top;
            *pWidth  = iterator->width;
            *pHeight = iterator->height;
            *pPitch  = iterator->pitch;

            iterator->state = STATE_ACQUIRED;

            return DP_STATUS_RETURN_SUCCESS;
        }
    }

    return DP_STATUS_BUFFER_FULL;
}


DP_STATUS_ENUM DpRingBufferPool::queueBuffer(int32_t bufID,
                                             bool)
{
    DP_TRACE_CALL();
    BufferList::iterator iterator;
    RingBufferNode       *pCurNode;

    AutoMutex lock(m_poolMutex);

    for (iterator = m_pendingList.begin(); iterator != m_pendingList.end(); iterator++)
    {
        if (bufID == iterator->bufID)
        {
            m_currentLeft  = MIN(m_currentLeft,  iterator->left);
            m_currentRight = MAX(m_currentRight, iterator->left + iterator->width - 1);
            if (-1 == m_currentTop)
            {
                // The first buffer data
                m_currentTop = MAX(m_currentTop, iterator->top);
            }
            else
            {
                // The following buffer data
                m_currentTop = MIN(m_currentTop, iterator->top);
            }

            m_currentBottom = MAX(m_currentBottom, iterator->top + iterator->height - 1);

            pCurNode = m_pendingList.remove(iterator);

            m_activeList.push_back(pCurNode);

            if (m_pendingList.empty())
            {
                // Wake up the consumer
                m_consumer.signal();
            }

            return DP_STATUS_RETURN_SUCCESS;
        }
    }

    return DP_STATUS_INVALID_STATE;
}


DP_STATUS_ENUM DpRingBufferPool::acquireBuffer(int32_t  *pBufID,
                                               void     *pVABase[3],
                                               uint32_t pBufSize[3],
                                               bool     waitBuf)
{
    DP_TRACE_CALL();
    int32_t plane;

    if ((0 == pBufID) ||
        (0 == pVABase) ||
        (0 == pBufSize))

    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    if (m_bufferCount <= 0)
    {
        if (m_frameDone == false)
        {
            return DP_STATUS_INVALID_BUFFER;
        }
        else
        {
            return DP_STATUS_RETURN_SUCCESS;
        }
    }

    while (0 == m_bufferCount)
    {
        if ((true == waitBuf) && (false == m_abortPool))
        {
            m_consumer.wait(m_poolMutex);
            waitBuf = false;
        }
        else if (true == m_abortPool)
        {
            return DP_STATUS_ABORTED_BY_USER;
        }
        else
        {
            return DP_STATUS_BUFFER_EMPTY;
        }
    }

    *pBufID  = 0;

    plane = DP_COLOR_GET_PLANE_COUNT(m_bufferFormat);
    switch(plane)
    {
        case 3:
            pVABase[0]  = m_pRingVA[0];
            pBufSize[0] = m_ringSize[0];
            pVABase[1]  = m_pRingVA[1];
            pBufSize[1] = m_ringSize[1];
            pVABase[2]  = m_pRingVA[2];
            pBufSize[2] = m_ringSize[2];
            break;
        case 2:
            pVABase[0]  = m_pRingVA[0];
            pBufSize[0] = m_ringSize[0];
            pVABase[1]  = m_pRingVA[1];
            pBufSize[1] = m_ringSize[1];
            pVABase[2]  = 0;
            pBufSize[2] = 0;
            break;
        case 1:
            pVABase[0]  = m_pRingVA[0];
            pBufSize[0] = m_ringSize[0];
            pVABase[1]  = 0;
            pBufSize[1] = 0;
            pVABase[2]  = 0;
            pBufSize[2] = 0;
            break;
        default:
            DPLOGE("DpRingBufferPool: invalid plane count: %d\n", plane);
            assert(0);
            return DP_STATUS_INVALID_FORMAT;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpRingBufferPool::releaseBuffer(int32_t bufferID __unused)
{
    DP_TRACE_CALL();
    AutoMutex lock(m_poolMutex);

    m_bufferCount = 0;
    m_frameDone   = true;

    m_producer.signal();

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpRingBufferPool::destroyBuffer()
{
    DP_TRACE_CALL();
    BufferList::iterator iterator;

    AutoMutex lock(m_poolMutex);

    for(iterator = m_activeList.begin(); iterator != m_activeList.end(); iterator++)
    {
        DPLOGI("DpRingBufferPool: delete active ring buffer node 0x%08x\n", &(*iterator));
        delete &(*iterator);
    }
    m_activeList.clear();

    for(iterator = m_pendingList.begin(); iterator != m_pendingList.end(); iterator++)
    {
        DPLOGI("DpRingBufferPool: delete pending ring buffer node 0x%08x\n", &(*iterator));
        delete &(*iterator);
    }
    m_pendingList.clear();

    m_bufferFD = -1;

    delete m_pProxy[2];
    m_pProxy[2] = NULL;

    delete m_pProxy[1];
    m_pProxy[1] = NULL;

    delete m_pProxy[0];
    m_pProxy[0] = NULL;

    m_bufferCount = 0;
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpRingBufferPool::getSWBaseWithID(int32_t  bufferID __unused,
                                                 void     *pBase[3],
                                                 uint32_t size[3])
{
    DP_TRACE_CALL();
    AutoMutex lock(m_poolMutex);

    if (0 == m_bufferCount)
    {
        return DP_STATUS_INVALID_BUFFER;
    }
    else
    {
        pBase[0] = m_pRingVA[0];
        pBase[1] = m_pRingVA[1];
        pBase[2] = m_pRingVA[2];

        size[0]  = m_ringSize[0];
        size[1]  = m_ringSize[1];
        size[2]  = m_ringSize[2];
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpRingBufferPool::getHWBaseWithID(int32_t      bufferID __unused,
                                                 DpEngineType type,
                                                 uint32_t     base[3],
                                                 uint32_t     size[3])
{
    DP_TRACE_CALL();
    AutoMutex lock(m_poolMutex);

    if (NULL != m_pProxy[0])
    {
        base[0] = m_pProxy[0]->mapHWAddress(type, 0);
        size[0] = m_ringSize[0];

        if (0 == base[0])
        {
            return DP_STATUS_INVALID_BUFFER;
        }
    }
    else
    {
        return DP_STATUS_INVALID_BUFFER;
    }

    if (3 == DP_COLOR_GET_PLANE_COUNT(m_bufferFormat))
    {
        // Configfure port 1
        if (0 == m_pProxy[0]->mapHWAddress(type, 1))
        {
            return DP_STATUS_INVALID_BUFFER;
        }

        // Configfure port 2
        if (0 == m_pProxy[0]->mapHWAddress(type, 2))
        {
            return DP_STATUS_INVALID_BUFFER;
        }

        base[1] = base[0] + size[0];
        size[1] = m_ringSize[1];

        base[2] = base[1] + size[1];
        size[2] = m_ringSize[2];
    }
    else if (2 == DP_COLOR_GET_PLANE_COUNT(m_bufferFormat))
    {
        // Configfure port 1
        if (0 == m_pProxy[0]->mapHWAddress(type, 1))
        {
            return DP_STATUS_INVALID_BUFFER;
        }

        base[1] = base[0] + size[0];
        size[1] = m_ringSize[1];
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpRingBufferPool::flushReadBuffer(int32_t bufferID __unused)
{
    DP_TRACE_CALL();
#if (CONFIG_FOR_OS_WINDOWS || CONFIG_FOR_FLUSH_RANGE)
    BufferList::iterator iterator;

    AutoMutex lock(m_poolMutex);

    for(iterator = m_activeList.begin(); iterator != m_activeList.end(); iterator++)
    {
        if (false == iterator->flush)
        {
            if (NULL != m_pProxy[0])
            {
                m_pProxy[0]->flushMemory(DpMemory::FLUSH_BEFORE_HW_READ_BUFFER_RANGE,
                                         (unsigned long)(iterator->pVABase[0]) - (unsigned long)m_pRingVA[0],
                                         iterator->size[0]);

                if (DP_COLOR_GET_PLANE_COUNT(m_bufferFormat) >= 2)
                {
                    m_pProxy[0]->flushMemory(DpMemory::FLUSH_BEFORE_HW_READ_BUFFER_RANGE,
                                             (unsigned long)(iterator->pVABase[1]) - (unsigned long)m_pRingVA[0],
                                             iterator->size[1]);
                }

                if (DP_COLOR_GET_PLANE_COUNT(m_bufferFormat) >= 3)
                {
                    m_pProxy[0]->flushMemory(DpMemory::FLUSH_BEFORE_HW_READ_BUFFER_RANGE,
                                             (unsigned long)(iterator->pVABase[2]) - (unsigned long)m_pRingVA[0],
                                             iterator->size[2]);
                }
            }
            iterator->flush = true;
        }
    }
#else
    m_pProxy[0]->flushMemory(DpMemory::FLUSH_BEFORE_HW_READ_WHOLE_CACHE);
#endif  // CONFIG_FOR_FLUSH_RANGE
    return DP_STATUS_RETURN_SUCCESS;
}

static ISP_MESSAGE_ENUM dp_status_to_tile_message(DP_STATUS_ENUM status)
{
    switch (status)
    {
    case DP_STATUS_BUFFER_EMPTY:    return MDP_MESSAGE_BUFFER_EMPTY;
    case DP_STATUS_INVALID_STATE:   return MDP_MESSAGE_INVALID_STATE;
    case DP_STATUS_RETURN_SUCCESS:  return ISP_MESSAGE_OK;
    //case DP_STATUS_UNKNOWN_ERROR:
    default:                        return MDP_MESSAGE_UNKNOWN_ERROR;
    }
}

int32_t funcPullBufferData(DpRingBufferPool *pBuf,
                           int32_t bufID,
                           int32_t left,
                           int32_t right,
                           int32_t top,
                           int32_t bottom)
{
    DP_STATUS_ENUM status;
    // Must check if the producer has provided the required data
    status = pBuf->pullBufferData(bufID,
                                  left,
                                  right,
                                  top,
                                  bottom);

    return (int32_t)dp_status_to_tile_message(status);
}

int32_t funcFitConstraint(DpRingBufferPool *pBuf,
                          int32_t orgLeft,
                          int32_t orgRight,
                          int32_t orgTop,
                          int32_t orgBottom,
                          int32_t &newLeft,
                          int32_t &newRight,
                          int32_t &newTop,
                          int32_t &newBottom)
{
    DP_STATUS_ENUM status;
    // Must check if the producer has provided the required data
    status = pBuf->fitConstraint(orgLeft,
                                 orgRight,
                                 orgTop,
                                 orgBottom,
                                 newLeft,
                                 newRight,
                                 newTop,
                                 newBottom);

    return (int32_t)dp_status_to_tile_message(status);
}
