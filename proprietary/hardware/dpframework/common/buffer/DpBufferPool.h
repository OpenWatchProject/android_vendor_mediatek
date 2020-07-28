#ifndef __DP_BUFFER_POOL_H__
#define __DP_BUFFER_POOL_H__

#include "DpDataType.h"
#include "DpMutex.h"
#include "DpCondition.h"
#include "DpList.h"
#include "DpLogger.h"
#include "DpProfiler.h"

class DpMemory;

enum CLIENT_TYPE_ENUM
{
    CLIENT_PRODUCER,
    CLIENT_CONSUMER
};

enum BUFFER_TYPE_ENUM
{
    BUFFER_UNKNOWN,
    BUFFER_BASIC,
    BUFFER_AUTO,
    BUFFER_RING
};

class DpBufferPoolBase
{
public:
    DpBufferPoolBase(BUFFER_TYPE_ENUM type);

    virtual ~DpBufferPoolBase();

    virtual DP_STATUS_ENUM setSecureMode(DpSecure);

    inline DP_STATUS_ENUM getBufferType(BUFFER_TYPE_ENUM *pType) const;

    virtual DP_STATUS_ENUM pollBufferStatus(CLIENT_TYPE_ENUM client,
                                            bool             waitBuf = false);

    virtual DP_STATUS_ENUM pollBufferStatus(CLIENT_TYPE_ENUM client,
                                            int32_t          bufID,
                                            int32_t          left,
                                            int32_t          right,
                                            int32_t          top,
                                            int32_t          bottom,
                                            bool             waitBuf = true);

    virtual DP_STATUS_ENUM dequeueBuffer(int32_t  *pBufID,
                                         void     *pVABase[3],
                                         uint32_t pBufSize[3],
                                         bool     waitBuf = true) = 0;

    virtual DP_STATUS_ENUM queueBuffer(int32_t bufferID,
                                       bool    exclusive = false);

    virtual DP_STATUS_ENUM acquireBuffer(int32_t  *pBufID,
                                         void     *pVABase[3],
                                         uint32_t pBufSize[3],
                                         bool     waitBuf = true) = 0;

    virtual DP_STATUS_ENUM cancelBuffer(int32_t bufferID);

    virtual DP_STATUS_ENUM releaseBuffer(int32_t bufferID);

    virtual DP_STATUS_ENUM waitBufferFence(int32_t bufferID);

    virtual DP_STATUS_ENUM getHWBaseWithID(int32_t      bufID,
                                           DpEngineType type,
                                           uint32_t     base[3],
                                           uint32_t     size[3]);

    virtual DP_STATUS_ENUM getSWBaseWithID(int32_t      bufferID,
                                           void         *pBase[3],
                                           uint32_t     size[3]);

    virtual DP_STATUS_ENUM flushReadBuffer(int32_t bufferID);

    virtual DP_STATUS_ENUM flushWriteBuffer(int32_t bufferID);

    virtual DP_STATUS_ENUM abortPoll(CLIENT_TYPE_ENUM client);

    virtual DP_STATUS_ENUM activateBuffer();

    virtual DP_STATUS_ENUM dumpDebugInfo(CLIENT_TYPE_ENUM client);

    virtual DP_STATUS_ENUM dumpBufferInfo(char* bufferInfoStr, uint32_t strLen);

    virtual DP_STATUS_ENUM dumpBuffer(int32_t       bufferID,
                                      DpColorFormat format,
                                      int32_t       width,
                                      int32_t       height,
                                      int32_t       YPitch,
                                      int32_t       UVPitch,
                                      const char    *keyword);

protected:
    enum BUFFER_STATE_ENUM
    {
        STATE_FREE,
        STATE_DEQUEUED,
        STATE_QUEUED,
        STATE_ACQUIRED,
        STATE_USED
    };

    struct BufferNode
    {
        int32_t           bufID;
        DP_MEMORY_ENUM    memType;
        int32_t           plane;
        void              *addr[3];
        uint32_t          addrMVA[3];
        uint32_t          size[3];
        DpMemory          *pProxy[3];
        bool              decorate;
        BUFFER_STATE_ENUM state;
        int32_t           fenceFd;
    };

    typedef ListPtr<BufferNode> BufferList;

    const BUFFER_TYPE_ENUM m_bufferType;
    int32_t          m_bufferCount;
    int32_t          m_bufferID;
    mutable DpMutex  m_poolMutex;
    DpCondition      m_producer;
    DpCondition      m_consumer;
    BufferList       m_bufferList;
    BufferList       m_bufferQueue;
    int32_t          m_availCount;
    int32_t          m_queueCount;
    uint32_t         m_userToken;
    bool             m_abortPool;

    inline int32_t nextBufferID();

    DP_STATUS_ENUM decorateProxy(BufferNode *pCurNode);

    int32_t insertBufferNode(DP_MEMORY_ENUM memType,
                             int32_t        plane,
                             void           **pAddr,
                             uint32_t       *pMVAddr,
                             uint32_t       *pSize,
                             DpMemory       *pProxy[3],
                             int32_t        fenceFd);

    inline BufferNode* findBufferPool(int32_t bufferID);

    DP_STATUS_ENUM dumpFenceInfo(int fd);
};

inline DP_STATUS_ENUM DpBufferPoolBase::getBufferType(BUFFER_TYPE_ENUM *pType) const
{
    if (NULL == pType)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    *pType = m_bufferType;
    return DP_STATUS_RETURN_SUCCESS;
}

inline int32_t DpBufferPoolBase::nextBufferID()
{
    AutoMutex lock(m_poolMutex);

    return m_bufferID++;
}

inline DpBufferPoolBase::BufferNode* DpBufferPoolBase::findBufferPool(int32_t bufferID)
{
    BufferList::iterator iterator;

    for (iterator = m_bufferList.begin(); iterator != m_bufferList.end(); iterator++)
    {
        if (iterator->bufID == bufferID)
        {
            return &(*iterator);
        }
    }

    return 0;
}


class DpBasicBufferPool: public DpBufferPoolBase
{
public:
    DpBasicBufferPool();

    virtual ~DpBasicBufferPool();

    virtual DP_STATUS_ENUM setSecureMode(DpSecure secure);

    DP_STATUS_ENUM registerBuffer(void     *pVABase[3],
                                  uint32_t pSizeList[3],
                                  int32_t  planeNumber,
                                  int32_t  fenceFd,
                                  int32_t  *pBufferID);

    DP_STATUS_ENUM registerBuffer(uint32_t pMVABase[3],
                                  uint32_t pSizeList[3],
                                  int32_t  planeNumber,
                                  int32_t  fenceFd,
                                  int32_t  *pBufferID);

    DP_STATUS_ENUM registerBuffer(void     *pVABase[3],
                                  uint32_t pMVABase[3],
                                  uint32_t pSizeList[3],
                                  int32_t  planeNumber,
                                  int32_t  fenceFd,
                                  int32_t  *pBufferID);

    DP_STATUS_ENUM registerBufferFD(int32_t  fileDesc,
                                    uint32_t *pSizeList,
                                    int32_t  planeNumber,
                                    int32_t  fenceFd,
                                    int32_t  *pBufferID);

    virtual DP_STATUS_ENUM dequeueBuffer(int32_t  *pBufID,
                                         void     *pVABase[3],
                                         uint32_t pBufSize[3],
                                         bool     waitBuf = true);

    virtual DP_STATUS_ENUM acquireBuffer(int32_t  *pBufID,
                                         void     *pVABase[3],
                                         uint32_t pBufSize[3],
                                         bool     waitBuf = true);

    virtual DP_STATUS_ENUM getHWBaseWithID(int32_t      bufferID,
                                           DpEngineType type,
                                           uint32_t     base[3],
                                           uint32_t     size[3]);

    virtual DP_STATUS_ENUM getSWBaseWithID(int32_t      bufferID,
                                           void         *pBase[3],
                                           uint32_t     size[3]);

    DP_STATUS_ENUM unregisterBuffer(int32_t bufferID);

    DP_STATUS_ENUM asyncReleaseBuffer(int32_t bufferID);

private:
    DpSecure m_secureMode;
};


class DpAutoBufferPool: public DpBufferPoolBase
{
public:
    DpAutoBufferPool();

    virtual ~DpAutoBufferPool();

    DP_STATUS_ENUM createBuffer(DpColorFormat format,
                                int32_t       width,
                                int32_t       height,
                                int32_t       pitch,
                                uint32_t      count);

    virtual DP_STATUS_ENUM dequeueBuffer(int32_t  *pBufID,
                                         void     *pVABase[3],
                                         uint32_t pBufSize[3],
                                         bool     waitBuf = true);

    virtual DP_STATUS_ENUM acquireBuffer(int32_t  *pBufID,
                                         void     *pVABase[3],
                                         uint32_t pBufSize[3],
                                         bool     waitBuf = true);

    DP_STATUS_ENUM destroyBuffer();
};


class DpRingBufferPool: public DpBufferPoolBase
{
public:
    DpRingBufferPool();

    virtual ~DpRingBufferPool();

    virtual DP_STATUS_ENUM pollBufferStatus(CLIENT_TYPE_ENUM client,
                                            bool             waitBuf = false);

    virtual DP_STATUS_ENUM pollBufferStatus(CLIENT_TYPE_ENUM client,
                                            int32_t          bufID,
                                            int32_t          left,
                                            int32_t          right,
                                            int32_t          top,
                                            int32_t          bottom,
                                            bool             waitBuf = true);

    inline DP_STATUS_ENUM fitConstraint(int32_t orgLeft,
                                        int32_t orgRight,
                                        int32_t orgTop,
                                        int32_t orgBottom,
                                        int32_t &newLeft,
                                        int32_t &newRight,
                                        int32_t &newTop,
                                        int32_t &newBottom) const;

    inline DP_STATUS_ENUM pullBufferData(int32_t bufID,
                                         int32_t left,
                                         int32_t right,
                                         int32_t top,
                                         int32_t bottom);

    DP_STATUS_ENUM mapBufferLine(int32_t top,
                                 int32_t bottom,
                                 int32_t *pStartLine,
                                 int32_t *pEndLine) const;

    DP_STATUS_ENUM createBuffer(int32_t       MCUYSize,
                                DpColorFormat srcFormat,
                                int32_t       srcWidth,
                                int32_t       srcHeight,
                                int32_t       srcYPitch,
                                int32_t       srcUVPitch,
                                int32_t       dstWidth,
                                int32_t       dstHeight,
                                int32_t       mcu_col,
                                DpRect        *pCropInfo = 0);

    virtual DP_STATUS_ENUM dequeueBuffer(int32_t  *pBufID,
                                         void     *pVABase[3],
                                         uint32_t pBufSize[3],
                                         bool     waitBuf = true);

    DP_STATUS_ENUM dequeueBuffer(int32_t       *pBufID,
                                 DpColorFormat *pFormat,
                                 void          *pVABase[3],
                                 int32_t       *pXStart,
                                 int32_t       *pYStart,
                                 int32_t       *pWidth,
                                 int32_t       *pHeight,
                                 int32_t       *pPitch,
                                 bool          waitBuf = true);

    virtual DP_STATUS_ENUM queueBuffer(int32_t bufferID,
                                       bool    exclusive = false);

    virtual DP_STATUS_ENUM acquireBuffer(int32_t  *pBufID,
                                         void     *pVABase[3],
                                         uint32_t pBufSize[3],
                                         bool     waitBuf = true);

    virtual DP_STATUS_ENUM releaseBuffer(int32_t bufferID);

    virtual DP_STATUS_ENUM getSWBaseWithID(int32_t  bufferID,
                                           void     *pBase[3],
                                           uint32_t size[3]);

    virtual DP_STATUS_ENUM getHWBaseWithID(int32_t      bufferID,
                                           DpEngineType type,
                                           uint32_t     base[3],
                                           uint32_t     size[3]);

    virtual DP_STATUS_ENUM flushReadBuffer(int32_t bufferID);

    DP_STATUS_ENUM destroyBuffer();

    inline DP_STATUS_ENUM getBufferInfo(DpColorFormat *pFormat,
                                        int32_t       *pWidth,
                                        int32_t       *pHeight,
                                        int32_t       *pYPitch,
                                        int32_t       *pCPitch,
                                        int32_t       *pMCUYSize,
                                        int32_t       *pMCUCol) const;

    inline void setEmbeddedJPEG(bool embeddedJPEG);

    inline bool getEmbeddedJPEG() const;

    inline int32_t getRingBufferMCUCount() const;

    inline int32_t getBufferFD() const;

private:
    struct RingBufferNode
    {
        BUFFER_STATE_ENUM   state;
        int32_t             bufID;
        DpColorFormat       format;
        void                *pVABase[3];
        uint32_t            size[3];
        int32_t             left;
        int32_t             top;
        int32_t             width;
        int32_t             height;
        int32_t             pitch;
        bool                flush;
    };

    typedef ListPtr<RingBufferNode> BufferList;

    inline int32_t insertBufferNode(int32_t left,
                                    int32_t right,
                                    int32_t top,
                                    int32_t bottom);

    DpColorFormat m_bufferFormat;
    int32_t    m_bufferWidth;
    int32_t    m_bufferHeight;
    int32_t    m_bufferYPitch;
    int32_t    m_bufferUVPitch;
    int32_t    m_verticalShift;
    BufferList m_pendingList;
    BufferList m_activeList;
    int32_t    m_ringMCUYSize;
    int32_t    m_ringMCUCount;
    int32_t    m_ringMCUCol;
    int32_t    m_currentLeft;
    int32_t    m_currentRight;
    int32_t    m_currentTop;
    int32_t    m_currentBottom;
    DpMemory   *m_pProxy[3];
    void       *m_pRingVA[3];
    int32_t    m_ringSize[3];
    DpRect     m_cropInfo;
    bool       m_frameDone;
    bool       m_embeddedJPEG;
    int32_t    m_bufferFD;
};

inline DP_STATUS_ENUM DpRingBufferPool::fitConstraint(int32_t orgLeft   __unused,
                                                      int32_t orgRight  __unused,
                                                      int32_t orgTop,
                                                      int32_t orgBottom __unused,
                                                      int32_t &newLeft  __unused,
                                                      int32_t &newRight __unused,
                                                      int32_t &newTop   __unused,
                                                      int32_t &newBottom) const
{
    int32_t MCUStart;
    int32_t MCUEnd;

    MCUStart  = orgTop / m_ringMCUYSize;
    MCUEnd    = MCUStart + m_ringMCUCount / 2 - 1;

    newBottom = (MCUEnd + 1) * m_ringMCUYSize - 1;

    return DP_STATUS_RETURN_SUCCESS;
}

// the reason why this function is inlined is to optimize fpga verification program
inline DP_STATUS_ENUM DpRingBufferPool::pullBufferData(int32_t bufID,
                                                       int32_t left,
                                                       int32_t right,
                                                       int32_t top,
                                                       int32_t bottom)
{
    DP_TRACE_CALL();
    int32_t startY;
    int32_t endY;

    do
    {
        if (-1 == m_currentTop)
        {
            bufID = insertBufferNode(0,
                                     m_bufferWidth - 1,
                                     top,
                                     bottom);
            if (bufID < 0)
                DPLOGE("invalid bufID %d\n", bufID);

            return DP_STATUS_BUFFER_EMPTY;
        }
        else if ((left  >= m_currentLeft) &&
                 (right <= m_currentRight) &&
                 (top   >= m_currentTop))
        {
            if (bottom <= m_currentBottom)
            {
                // Full inside the available window
                return DP_STATUS_RETURN_SUCCESS;
            }
            else
            {
                startY = MAX(m_currentBottom + 1, top);
                endY   = MAX(m_currentBottom + 1, bottom);

                bufID = insertBufferNode(0,
                                         m_bufferWidth - 1,
                                         startY,
                                         endY);
                if (bufID < 0)
                    DPLOGE("invalid bufID %d\n", bufID);

                return DP_STATUS_BUFFER_EMPTY;
            }
        }
        else if (top < m_currentTop)
        {
            // Should not happen
            DPLOGE("Should not happened top(%d) < m_currentTop(%d)\n", top, m_currentTop);
        }
    } while (0);

    return DP_STATUS_INVALID_STATE;
}

inline DP_STATUS_ENUM DpRingBufferPool::getBufferInfo(DpColorFormat *pFormat,
                                                      int32_t       *pWidth,
                                                      int32_t       *pHeight,
                                                      int32_t       *pYPitch,
                                                      int32_t       *pCPitch,
                                                      int32_t       *pMCUYSize,
                                                      int32_t       *pMCUCol) const
{
    if ((0 == pFormat) ||
        (0 == pWidth) ||
        (0 == pHeight))
    {
        return DP_STATUS_INVALID_PARAX;
    }

    AutoMutex lock(m_poolMutex);

    if (m_bufferCount <= 0)
    {
        return DP_STATUS_INVALID_BUFFER;
    }

    *pFormat   = m_bufferFormat;
    *pWidth    = m_bufferWidth;
    *pHeight   = m_bufferHeight;
    *pYPitch   = m_bufferYPitch;
    *pCPitch   = m_bufferUVPitch;
    *pMCUYSize = m_ringMCUYSize ;
    *pMCUCol   = m_ringMCUCol;

    return DP_STATUS_RETURN_SUCCESS;
}

inline void DpRingBufferPool::setEmbeddedJPEG(bool embeddedJPEG)
{
    m_embeddedJPEG = embeddedJPEG;
}

inline bool DpRingBufferPool::getEmbeddedJPEG() const
{
    return m_embeddedJPEG;
}

inline int32_t DpRingBufferPool::getRingBufferMCUCount() const
{
    return m_ringMCUCount;
}

inline int32_t DpRingBufferPool::getBufferFD() const
{
    return m_bufferFD;
}


// the reason why this function is inlined is to optimize fpga verification program
inline int32_t DpRingBufferPool::insertBufferNode(int32_t left,
                                                  int32_t right,
                                                  int32_t top,
                                                  int32_t bottom)
{
    RingBufferNode *pNode;
    int32_t        nodeID;
    int32_t        startLine;
    int32_t        endLine;
    int32_t        middle;

    if (0 != m_ringMCUYSize)
    {
        top = (top / m_ringMCUYSize)*m_ringMCUYSize;
    }

    startLine = top     % (m_ringMCUYSize * m_ringMCUCount);
    endLine   = bottom  % (m_ringMCUYSize * m_ringMCUCount);

    if (endLine <= startLine)
    {
        middle = top + (m_ringMCUYSize * m_ringMCUCount - startLine) - 1;

        // First node
        pNode = new RingBufferNode();
        if (0 == pNode)
        {
            DPLOGE("cannot allocate new buffer node\n");
            return -1;
        }

        nodeID = nextBufferID();

        pNode->bufID      = nodeID;
        pNode->state      = STATE_FREE;
        pNode->format     = m_bufferFormat;
        pNode->bufID      = nodeID;
        pNode->left       = left;
        pNode->top        = top;
        pNode->width      = right - left + 1;
        pNode->height     = middle - top + 1;
        pNode->pitch      = m_bufferYPitch;
        pNode->pVABase[0] = (uint8_t*)m_pRingVA[0] + startLine * m_bufferYPitch;
        pNode->size[0]    = pNode->height * m_bufferYPitch;
        pNode->pVABase[1] = (uint8_t*)m_pRingVA[1] + (startLine >> m_verticalShift) * m_bufferUVPitch;
        pNode->size[1]    = (pNode->height >> m_verticalShift) * m_bufferUVPitch;
        pNode->pVABase[2] = (uint8_t*)m_pRingVA[2] + (startLine >> m_verticalShift) * m_bufferUVPitch;
        pNode->size[2]    = (pNode->height >> m_verticalShift) * m_bufferUVPitch;
        pNode->flush      = false;

        {
            AutoMutex lock(m_poolMutex);
            m_pendingList.push_back(pNode);

            // Wakeup the producer
            m_producer.signal();
        }

        // Second node
        pNode = new RingBufferNode();
        if (0 == pNode)
        {
            DPLOGE("cannot allocate new buffer node\n");
            return -1;
        }

        nodeID = nextBufferID();

        pNode->state      = STATE_FREE;
        pNode->format     = m_bufferFormat;
        pNode->bufID      = nodeID;
        pNode->left       = left;
        pNode->top        = middle + 1;
        pNode->width      = right - left + 1;
        pNode->height     = endLine + 1;
        pNode->pitch      = m_bufferYPitch;
        pNode->pVABase[0] = (uint8_t*)m_pRingVA[0];
        pNode->size[0]    = pNode->height * m_bufferYPitch;
        pNode->pVABase[1] = (uint8_t*)m_pRingVA[1];
        pNode->size[1]    = (pNode->height >> m_verticalShift) * m_bufferUVPitch;
        pNode->pVABase[2] = (uint8_t*)m_pRingVA[2];
        pNode->size[2]    = (pNode->height >> m_verticalShift) * m_bufferUVPitch;
        pNode->flush      = false;

        {
            AutoMutex lock(m_poolMutex);
            m_pendingList.push_back(pNode);

            // Wakeup the producer
            m_producer.signal();
        }
    }
    else
    {
        pNode = new RingBufferNode();
        if (0 == pNode)
        {
            DPLOGE("cannot allocate new buffer node\n");
            return -1;
        }

        nodeID = nextBufferID();

        pNode->state      = STATE_FREE;
        pNode->format     = m_bufferFormat;
        pNode->bufID      = nodeID;
        pNode->left       = left;
        pNode->top        = top;
        pNode->width      = right - left + 1;
        pNode->height     = bottom - top + 1;
        pNode->pitch      = m_bufferYPitch;
        pNode->pVABase[0] = (uint8_t*)m_pRingVA[0] + startLine * m_bufferYPitch;
        pNode->size[0]    = pNode->height * m_bufferYPitch;
        pNode->pVABase[1] = (uint8_t*)m_pRingVA[1] + (startLine >> m_verticalShift) * m_bufferUVPitch;
        pNode->size[1]    = (pNode->height >> m_verticalShift) * m_bufferUVPitch;
        pNode->pVABase[2] = (uint8_t*)m_pRingVA[2] + (startLine >> m_verticalShift) * m_bufferUVPitch;
        pNode->size[2]    = (pNode->height >> m_verticalShift) * m_bufferUVPitch;
        pNode->flush      = false;

        {
            AutoMutex lock(m_poolMutex);
            m_pendingList.push_back(pNode);

            // Wakeup the producer
            m_producer.signal();
        }
    }

    return nodeID;
}

int32_t funcPullBufferData(DpRingBufferPool *pBuf,
                           int32_t bufID,
                           int32_t left,
                           int32_t right,
                           int32_t top,
                           int32_t bottom);

int32_t funcFitConstraint(DpRingBufferPool *pBuf,
                          int32_t orgLeft,
                          int32_t orgRight,
                          int32_t orgTop,
                          int32_t orgBottom,
                          int32_t &newLeft,
                          int32_t &newRight,
                          int32_t &newTop,
                          int32_t &newBottom);

#endif  // __DP_BUFFER_POOL_H__
