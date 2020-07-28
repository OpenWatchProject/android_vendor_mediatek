#ifndef __DP_STREAM_H__
#define __DP_STREAM_H__

#include "DpDataType.h"
#include "DpMutex.h"
#include "DpList.h"
#include "DpEngineBase.h"
#include "DpPathControl.h"
#include <vector>

typedef std::vector<DpJobID> JobIDList;

typedef struct DpReadbackEngines_t
{
    uint32_t engineID;
    uint32_t PABufferStart;
    uint32_t PABufferEnd;
} DpReadbackEngines;

typedef std::vector<DpReadbackEngines> ReadbackEngineList;

class DpStream
{
private:
    enum STREAM_STATE_ENUM
    {
        STATE_RESET,
        STATE_INIT,
        STATE_START,
        STATE_IDLE,
        STATE_STOP
    };

    typedef struct ChannelNode
    {
        int32_t     channelID;
        DpChannel   *pChannel;

        int32_t first()
        {
            return channelID;
        }

        DpChannel* second()
        {
            return pChannel;
        }

    } ChanNode;

    typedef ListPtr<ChanNode>  ChanList;

public:
    enum CONFIG_FLAG_ENUM
    {
        CONFIG_ALL                = 0x1,
        CONFIG_FRAME_ONLY         = 0x2,
        CONFIG_TILE_ONLY          = 0x4,
        CONFIG_ISP_FRAME_MODE     = 0x8,
        CONFIG_BUFFER_IGNORE      = 0x10,
        CONFIG_WPE_ONLY_TILE_MODE = 0x20
    };

    typedef ChanList::iterator ChanIterator;

    DpStream(STREAM_TYPE_ENUM type);

    ~DpStream();

    // Register channel for this stream and return id
    DP_STATUS_ENUM addChannel(DpChannel *pChannel,
                              int32_t   *pChanID);

    // Remove channel from this stream
    DP_STATUS_ENUM delChannel(int32_t chanID);

    inline ChanIterator chanBegin()
    {
        return m_channelList.begin();
    }

    inline ChanIterator chanEnd()
    {
        return m_channelList.end();
    }

    inline STREAM_TYPE_ENUM getScenario() const;

    inline DP_STATUS_ENUM setConfigFlag(CONFIG_FLAG_ENUM configFlag);

    inline uint32_t getConfigFlags() const;

    DP_STATUS_ENUM setHistogram();

    DP_STATUS_ENUM resetStream();

    DP_STATUS_ENUM initStream();

    DP_STATUS_ENUM startStream(bool bInit = true);

    DP_STATUS_ENUM waitStream();

    DP_STATUS_ENUM stopStream();

    DP_STATUS_ENUM dumpDebugStream();

    inline DP_STATUS_ENUM setSyncMode(bool enSync);

    inline bool getSyncMode() const;

    inline DP_STATUS_ENUM setAsyncJob(DpJobID hJob, DpJobID hJobIndex = 1, char* pFrameInfo = NULL); //1 means first job

    inline DpJobID getAsyncJob(DpJobID hJobIndex = 1, char* pFrameInfo = NULL) const;

	inline DP_STATUS_ENUM addAsyncJob(DpJobID hJob, char* pFrameInfo);

	inline JobIDList& getAsyncJobs();

    inline DP_STATUS_ENUM setPQReadback(bool enReadback);

    inline DP_STATUS_ENUM setHDRReadback(bool enReadback);

    inline DP_STATUS_ENUM setDREReadback(int32_t enReadback);

    inline bool getPQReadback() const;

    inline bool getHDRReadback() const;

    inline int32_t getDREReadback() const;

    inline DP_STATUS_ENUM setReadbackValues(const uint32_t* pReadbackValues, uint32_t numValues);

    inline const uint32_t* getReadbackValues(uint32_t& numValues) const;

    inline DP_STATUS_ENUM setScenario(STREAM_TYPE_ENUM streamType);

    DP_STATUS_ENUM setReadbackPABuffer(uint32_t *pReadbackPABuffer, uint32_t readbackPABufferIndex);

    uint32_t* getReadbackPABuffer(uint32_t& readbackPABufferIndex);

    DP_STATUS_ENUM initNumReadbackPABuffer();

    DP_STATUS_ENUM setNumReadbackPABuffer(uint32_t numReadbackPABuffer, uint32_t readbackEngineID);

    uint32_t getNumReadbackPABuffer(uint32_t& numReadbackEngine);

    DP_STATUS_ENUM getReadbackEngineStatus(ReadbackEngineList& readbackEngineList);

    DP_STATUS_ENUM cleanReadbackEngineStatus(uint32_t numReadbackEngine);

private:
    inline int32_t nextChannelID();

    inline int32_t insertChannelNode(DpChannel *pChannel);

    DP_STATUS_ENUM initialize();

    STREAM_TYPE_ENUM    m_streamType;
    DpMutex             m_streamMutex;
    DpPathControl       m_controller;
    int32_t             m_channelID;
    STREAM_STATE_ENUM   m_streamState;
    ChanList            m_channelList;
    bool                m_syncMode;
    DpJobID             m_asynchJob;
    DpJobID             m_asynchExtJob;
    uint32_t            m_asynchNumJob;
    bool                m_pqReadback;
    bool                m_hdrReadback;
    int32_t             m_dreReadback;
    const uint32_t      *m_pReadbackValues;
    uint32_t            m_numReadbackValues;
    uint32_t            m_configFlags;
    char                *m_frameInfo;

    //PQ DC, HDR, DRE readback
    uint32_t            *m_pReadbackPABuffer;
    uint32_t            m_readbackPABufferIndex;
    uint32_t            m_numReadbackPABuffer;
    uint32_t            m_numReadbackEngine;
    ReadbackEngineList  m_readbackEngineList;
    DpMutex             m_readbackMutex;

    JobIDList			m_jobIds;
};


inline STREAM_TYPE_ENUM DpStream::getScenario() const
{
    return m_streamType;
}

inline DP_STATUS_ENUM DpStream::setConfigFlag(CONFIG_FLAG_ENUM configFlag)
{
    AutoMutex lock(m_streamMutex);

    m_configFlags |= configFlag;
    return DP_STATUS_RETURN_SUCCESS;
}

inline uint32_t DpStream::getConfigFlags() const
{
    return m_configFlags;
}

inline DP_STATUS_ENUM DpStream::setSyncMode(bool enSync)
{
    m_syncMode = enSync;
    return DP_STATUS_RETURN_SUCCESS;
}

inline bool DpStream::getSyncMode() const
{
    return m_syncMode;
}

inline DP_STATUS_ENUM DpStream::setAsyncJob(DpJobID hJob, DpJobID hJobIndex, char* pFrameInfo) //1 means first job
{
    if(1 == hJobIndex)
    {
        m_asynchJob = hJob;
        m_asynchNumJob = 1;
    }
    else if(2 == hJobIndex)
    {
        m_asynchExtJob = hJob;
        m_asynchNumJob = 2;
    }
    else
    {
        DPLOGE("setAsyncJob is out of range: %llu\n",hJobIndex);
        return DP_STATUS_INVALID_PARAX;
    }
    m_frameInfo = pFrameInfo;

    return DP_STATUS_RETURN_SUCCESS;
}

inline DpJobID DpStream::getAsyncJob(DpJobID hJobIndex, char* pFrameInfo) const
{
    if ((m_frameInfo != NULL) && (pFrameInfo != NULL))
    {
        memcpy(pFrameInfo, m_frameInfo, strlen(m_frameInfo) + 1);
    }

    if(1 == hJobIndex)
        return m_asynchJob;
    else if(2 == hJobIndex)
        return m_asynchExtJob;
    else if(0 == hJobIndex)
        return m_asynchNumJob;
    else
    {
        DPLOGE("getAsyncJob is out of range: %llu\n",hJobIndex);
        return DP_STATUS_INVALID_PARAX;
    }
}

inline DP_STATUS_ENUM DpStream::addAsyncJob(DpJobID hJob, char* pFrameInfo)
{
	if ((m_frameInfo != NULL) && (pFrameInfo != NULL))
	{
		memcpy(pFrameInfo, m_frameInfo, strlen(m_frameInfo) + 1);
	}
    DPLOGI("DpStream::addAsyncJob: %llx", hJob);
	m_jobIds.push_back(hJob);
	return DP_STATUS_RETURN_SUCCESS;
}

inline JobIDList& DpStream::getAsyncJobs()
{
	return m_jobIds;
}
inline DP_STATUS_ENUM DpStream::setPQReadback(bool enReadback)
{
    m_pqReadback = enReadback;
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpStream::setHDRReadback(bool enReadback)
{
    m_hdrReadback = enReadback;
    return DP_STATUS_RETURN_SUCCESS;
}

inline DP_STATUS_ENUM DpStream::setDREReadback(int32_t dreReadback)
{
    m_dreReadback = dreReadback;
    return DP_STATUS_RETURN_SUCCESS;
}

inline bool DpStream::getPQReadback() const
{
    return m_pqReadback;
}

inline bool DpStream::getHDRReadback() const
{
    return m_hdrReadback;
}

inline int32_t DpStream::getDREReadback() const
{
    return m_dreReadback;
}

inline DP_STATUS_ENUM DpStream::setReadbackValues(const uint32_t* pReadbackValues, uint32_t numValues)
{
    m_pReadbackValues = pReadbackValues;
    m_numReadbackValues = numValues;

    DPLOGI("setReadbackValues in DpStream is done\n");
    return DP_STATUS_RETURN_SUCCESS;
}

inline const uint32_t* DpStream::getReadbackValues(uint32_t& numValues) const
{
    numValues = m_numReadbackValues;
    return m_pReadbackValues;
}

inline DP_STATUS_ENUM DpStream::setScenario(STREAM_TYPE_ENUM streamType)
{
    DPLOGI("m_streamType: %d to %d\n", m_streamType, streamType);

    switch (m_streamType)
    {
    case STREAM_BITBLT:
        if (streamType == STREAM_GPU_BITBLT ||
            streamType == STREAM_DUAL_BITBLT ||
            streamType == STREAM_2ND_BITBLT)
        {
            m_streamType = streamType;
        }
        break;
    case STREAM_GPU_BITBLT:
    case STREAM_DUAL_BITBLT:
    case STREAM_2ND_BITBLT:
#ifdef CONFIG_FOR_SOURCE_PQ
    case STREAM_COLOR_BITBLT:
#endif
        break;
    case STREAM_FRAG:
    case STREAM_FRAG_JPEGDEC:
        if (streamType == STREAM_FRAG || streamType == STREAM_FRAG_JPEGDEC)
        {
            m_streamType = streamType;
        }
        break;
    default:
        DPLOGE("setScenario is NOT allowed among different streams\n");
        return DP_STATUS_INVALID_PARAX;
    }

    return DP_STATUS_RETURN_SUCCESS;
}

#endif  // __DP_STREAM_H__
