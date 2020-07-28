#include "DpStream.h"
#include "DpChannel.h"
#include "DpProfiler.h"
#ifndef BASIC_PACKAGE
#if CONFIG_FOR_OS_ANDROID
#include "PQSessionManager.h"
#endif // CONFIG_FOR_OS_ANDROID
#include "DpPathBase.h"
#endif // BASIC_PACKAGE
#include <utils/CallStack.h>

static uint32_t g_streamCounter = 0;
static DpMutex  g_counterLock;

DpStream::DpStream(STREAM_TYPE_ENUM type)
    : m_streamType(type),
      m_controller(this),
      m_channelID(0),
      m_streamState(STATE_RESET),
      m_syncMode(true),
      m_asynchJob(0),
      m_asynchExtJob(0),
      m_asynchNumJob(0),
      m_pqReadback(false),
      m_hdrReadback(false),
      m_dreReadback(DpDREParam::DRESRAM::SRAMDefault),
      m_pReadbackValues(NULL),
      m_numReadbackValues(0),
      m_configFlags(0),
      m_frameInfo(NULL),
      m_pReadbackPABuffer(NULL),
      m_readbackPABufferIndex(0),
      m_numReadbackPABuffer(0),
      m_numReadbackEngine(0)
{
    DP_PROFILER_RESET_PROFILE_INFO();

    m_readbackEngineList.clear();

	//clear job id list
	m_jobIds.clear();
    g_counterLock.lock();
    g_streamCounter++;
    if(g_streamCounter > 500){
        //print call stack
        DPLOGW("Over 500 DpStream instance created without release! %d", g_streamCounter);
        android::CallStack cs;
        cs.update();
        cs.log("MDP",  ANDROID_LOG_WARN);
    }
    g_counterLock.unlock();

}


DpStream::~DpStream()
{
    ChanList::iterator iterator;

    AutoMutex lock(m_streamMutex);

    for(iterator = m_channelList.begin(); iterator != m_channelList.end(); iterator++)
    {
        delete &(*iterator);
    }

    m_asynchJob = 0;
    m_asynchExtJob = 0;
    m_asynchNumJob = 0;
    m_numReadbackValues = 0;
    m_pReadbackValues = NULL;
	//clear job id list
	m_jobIds.clear();

    m_pReadbackPABuffer = NULL;
    m_readbackPABufferIndex = 0;
    m_numReadbackPABuffer = 0;
    m_readbackEngineList.clear();

    g_counterLock.lock();
    g_streamCounter--;
    g_counterLock.unlock();

    DP_PROFILER_DUMP_PROFILE_INFO();
}


inline int32_t DpStream::nextChannelID()
{
    AutoMutex lock(m_streamMutex);

    return m_channelID++;
}

inline int32_t DpStream::insertChannelNode(DpChannel *pChannel)
{
    ChannelNode *pNode;
    int32_t     chanID;

    pNode = new ChannelNode();
    if (0 == pNode)
    {
        return -1;
    }

    chanID = nextChannelID();

    pNode->channelID = chanID;
    pNode->pChannel  = pChannel;

    AutoMutex lock(m_streamMutex);

    m_channelList.push_back(pNode);

    return chanID;
}

DP_STATUS_ENUM DpStream::addChannel(DpChannel *pChannel,
                                    int32_t   *pChanID)
{
    int32_t chanID;

    if (NULL == pChannel || NULL == pChanID)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    chanID = insertChannelNode(pChannel);
    if (-1 == chanID)
    {
        return DP_STATUS_UNKNOWN_ERROR;
    }

    *pChanID = chanID;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpStream::delChannel(int32_t chanID)
{
    ChanList::iterator  iterator;
    ChannelNode         *pCurNode;

    AutoMutex lock(m_streamMutex);

    for(iterator = m_channelList.begin(); iterator != m_channelList.end(); iterator++)
    {
        if (iterator->channelID == chanID)
        {
            pCurNode = m_channelList.remove(iterator);
            delete pCurNode;
            break;
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpStream::setHistogram()
{
#ifndef BASIC_PACKAGE
#if CONFIG_FOR_OS_ANDROID
    int32_t        pqSupport;
    pqSupport = DpDriver::getInstance()->getPQSupport();
    if (0 != pqSupport) {
        uint32_t       numLeftValues;
        uint32_t       numRightValues;
        uint32_t       lHist[TOTAL_HISTOGRAM_NUM];
        uint32_t       rHist[TOTAL_HISTOGRAM_NUM];
        uint32_t       totalHist[TOTAL_HISTOGRAM_NUM];
        DpCommand*     pCommand;

        DpPathComposer::iterator iterator = m_controller.getComposer().begin();
        const uint32_t *pLeftRegVal = iterator->getReadbackValues(numLeftValues);
        memcpy(lHist, pLeftRegVal, numLeftValues * sizeof(uint32_t));

        iterator++;
        const uint32_t *pRightRegVal = iterator->getReadbackValues(numRightValues);
        memcpy(rHist, pRightRegVal, numRightValues * sizeof(uint32_t));

        for (uint32_t i = 0; i < numRightValues; i++)
        {
            totalHist[i] = lHist[i] + rHist[i];
        }

        iterator->getCommander(&pCommand);

        uint64_t PQSessionID = pCommand->getPQSessionID();
        PQSession* pPQSession = PQSessionManager::getInstance()->getPQSession(PQSessionID);

        if (pPQSession != NULL)
        {
            pPQSession->setHistogram(totalHist, numLeftValues);
        }
    }
#endif // CONFIG_FOR_OS_ANDROID
#endif // BASIC_PACKAGE

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpStream::resetStream()
{
    DP_STATUS_ENUM      status;
    ChanList::iterator  iterator;

    DP_TRACE_CALL();

    AutoMutex lock(m_streamMutex);

    if (STATE_START == m_streamState)
    {
        status = m_controller.stopStream();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            return status;
        }
    }

    for(iterator = m_channelList.begin(); iterator != m_channelList.end(); iterator++)
    {
        delete &(*iterator);
    }
    m_channelList.clear();

    // Reset to zero
    m_channelID   = 0;

    m_streamState = STATE_RESET;

    m_asynchJob = 0;
    m_asynchExtJob = 0;
    m_asynchNumJob = 0;
    m_numReadbackValues = 0;
    m_pReadbackValues = NULL;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpStream::initStream()
{
    AutoMutex lock(m_streamMutex);

    return initialize();
}


DP_STATUS_ENUM DpStream::startStream(bool bInit)
{
    DP_STATUS_ENUM status;

    DP_TRACE_CALL();
    DP_PROFILER_PERFORMANCE_BEGIN();

    AutoMutex lock(m_streamMutex);

    if (STATE_START == m_streamState)
    {
        DPLOGE("DpStream: stream had been started\n");
        return DP_STATUS_INVALID_STATE;
    }

    if ((STATE_INIT != m_streamState) &&
        (STATE_IDLE != m_streamState) &&
        (STATE_STOP != m_streamState) &&
        bInit)
    {
        status = initialize();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpStream: init the stream failed(%d)\n", status);
            return status;
        }
    }

    status = m_controller.startStream();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpStream: start the stream failed(%d)\n", status);
        return status;
    }

    m_streamState = STATE_START;

    DP_PROFILER_PERFORMANCE_END();

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpStream::waitStream()
{
    DP_STATUS_ENUM status;

    // Default return value
    status = DP_STATUS_RETURN_SUCCESS;

    AutoMutex lock(m_streamMutex);

    if ((STATE_IDLE == m_streamState) ||
        (STATE_STOP == m_streamState))
    {
        return DP_STATUS_RETURN_SUCCESS;
    }

    if (STATE_START != m_streamState)
    {
        DPLOGE("DpStream: incorrect state(%d) for wait\n", m_streamState);
        return DP_STATUS_INVALID_STATE;
    }

    DP_PROFILER_PERFORMANCE_BEGIN();
    status = m_controller.waitStream();
    DP_PROFILER_PERFORMANCE_END();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    m_streamState = STATE_IDLE;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpStream::stopStream()
{
    DP_STATUS_ENUM      status;
    ChanList::iterator  iterator;
    DpChannel           *pChannel;

    AutoMutex lock(m_streamMutex);

    DP_PROFILER_PERFORMANCE_BEGIN();
    status = m_controller.stopStream();
    DP_PROFILER_PERFORMANCE_END();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpStream: stop the stream failed(%d)\n", status);
        return status;
    }

    for(iterator = m_channelList.begin() ; iterator != m_channelList.end() ; iterator++)
    {
        pChannel = iterator->second();

        // Unlock the channel
        pChannel->unlockChannel();
    }

    // Reset config flags
    m_configFlags = 0;

    m_streamState = STATE_STOP;

	//clear job id list
	m_jobIds.clear();

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpStream::dumpDebugStream()
{
    DP_STATUS_ENUM      status;
    AutoMutex lock(m_streamMutex);

    status = m_controller.dumpDebugStream();

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpStream::initialize()
{
    DP_STATUS_ENUM      status;
    ChanList::iterator  iterator;
    DpChannel           *pChannel;

    DP_TRACE_CALL();

    for (iterator = m_channelList.begin(); iterator != m_channelList.end(); iterator++)
    {
        pChannel = iterator->second();

        // Unlock the channel
        pChannel->lockChannel();
    }

    status = m_controller.initStream();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    m_streamState = STATE_INIT;

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpStream::setReadbackPABuffer(uint32_t *pReadbackPABuffer, uint32_t readbackPABufferIndex)
{
    m_pReadbackPABuffer = pReadbackPABuffer;
    m_readbackPABufferIndex = readbackPABufferIndex;
    return DP_STATUS_RETURN_SUCCESS;
}

uint32_t* DpStream::getReadbackPABuffer(uint32_t& readbackPABufferIndex)
{
    readbackPABufferIndex = m_readbackPABufferIndex;
    return m_pReadbackPABuffer;
}

DP_STATUS_ENUM DpStream::initNumReadbackPABuffer()
{
    DPLOGI("DpStream::initNumReadbackPABuffer initialize number of readback\n");
    m_numReadbackPABuffer = 0;
    m_numReadbackEngine = 0;
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpStream::setNumReadbackPABuffer(uint32_t numReadbackPABuffer, uint32_t readbackEngineID)
{
    if (numReadbackPABuffer == 0)
    {
        DPLOGI("DpStream::setNumReadbackPABuffer numReadbackPABuffer = %d, do nothing\n", numReadbackPABuffer);
        return DP_STATUS_RETURN_SUCCESS;
    }
    m_numReadbackPABuffer = m_numReadbackPABuffer + numReadbackPABuffer;
    DPLOGI("DpStream::setNumReadbackPABuffer m_numReadbackPABuffer = %d\n", m_numReadbackPABuffer);

    m_numReadbackEngine++;
    DPLOGI("DpStream::setNumReadbackPABuffer m_numReadbackEngine = %d\n", m_numReadbackEngine);

    DpReadbackEngines readbackEngine;
    readbackEngine.engineID = readbackEngineID;
    readbackEngine.PABufferStart = m_readbackPABufferIndex;
    readbackEngine.PABufferEnd = m_readbackPABufferIndex + numReadbackPABuffer - 1;

    m_readbackMutex.lock();
    m_readbackEngineList.push_back(readbackEngine);
    DPLOGI("DpStream::setNumReadbackPABuffer m_readbackEngineList size = %d, engine ID = %d\n", m_readbackEngineList.size(), readbackEngineID);
    m_readbackMutex.unlock();

    m_readbackPABufferIndex = (m_readbackPABufferIndex + numReadbackPABuffer) & (MAX_NUM_READBACK_PA_BUFFER - 1);
    DPLOGI("DpStream::setNumReadbackPABuffer m_readbackPABufferIndex = %d\n", m_readbackPABufferIndex);

    return DP_STATUS_RETURN_SUCCESS;
}

uint32_t DpStream::getNumReadbackPABuffer(uint32_t& numReadbackEngine)
{
    numReadbackEngine = m_numReadbackEngine;
    return m_numReadbackPABuffer;
}

DP_STATUS_ENUM DpStream::getReadbackEngineStatus(ReadbackEngineList& readbackEngineList)
{
    uint32_t index;
    m_readbackMutex.lock();
    for (index = 0; index < m_readbackEngineList.size(); index++)
    {
        readbackEngineList.push_back(m_readbackEngineList[index]);
    }
    m_readbackMutex.unlock();
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpStream::cleanReadbackEngineStatus(uint32_t numReadbackEngine)
{
    uint32_t index;
    m_readbackMutex.lock();
    uint32_t listSize = m_readbackEngineList.size();
    for (index = 0; index < numReadbackEngine; index++)
    {
        if (index == listSize)
        {
            DPLOGE("DpStream::cleanReadbackEngineStatus: clean index(%d), clean size(%d) > list size(%d)\n", index, numReadbackEngine, m_readbackEngineList.size());
            break;
        }
        m_readbackEngineList.erase(m_readbackEngineList.begin());
    }
    m_readbackMutex.unlock();
    return DP_STATUS_RETURN_SUCCESS;
}
