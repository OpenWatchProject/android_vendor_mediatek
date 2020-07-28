#include "DpMutex.h"
#include "DpIspStream.h"
#include "DpStream.h"
#include "DpChannel.h"
#include "DpTimer.h"
#include "tpipe_config.h"
#include "DpPlatform.h"

#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
#include "PQSessionManager.h"
#include "PQReadBackFactory.h"
#include "PQConfig.h"
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID

static uint32_t s_PqCount = 0;
static DpMutex  s_PqCountMutex;

#define ISP_MAX_MULTI_PORT_NUM 2

uint32_t DpIspStream::queryMultiPortSupport(DpIspStream::ISPStreamType scenario)
{

    if (scenario == DpIspStream::ISP_IC_STREAM ||
        scenario == DpIspStream::ISP_VR_STREAM ||
        scenario == DpIspStream::ISP_VSS_STREAM)
    {
        return ISP_MAX_MULTI_PORT_NUM;
    }
    else
    {
#ifdef RDMA0_RSZ1_SRAM_SHARING
        return (ISP_MAX_MULTI_PORT_NUM - 1);
#else
        return ISP_MAX_MULTI_PORT_NUM;
#endif

    }
}

void DpIspStream::queryISPFeatureSupport(std::map<DP_ISP_FEATURE_ENUM, bool> &mapISPFeature){

#ifdef SUPPORT_CLEARZOOM
        mapISPFeature[ISP_FEATURE_CLEARZOOM] = true;
#else
        mapISPFeature[ISP_FEATURE_CLEARZOOM] = false;
#endif
        mapISPFeature[ISP_FEATURE_REFOCUS] = true;

#ifdef SUPPORT_WCG
        mapISPFeature[ISP_FEATURE_WCG] = true;
#else
        mapISPFeature[ISP_FEATURE_WCG] = false;
#endif

#ifdef SUPPORT_DRE
        mapISPFeature[ISP_FEATURE_DRE] = true;
#else
        mapISPFeature[ISP_FEATURE_DRE] = false;
#endif

#ifdef SUPPORT_HFG
        mapISPFeature[ISP_FEATURE_HFG] = true;
#else
        mapISPFeature[ISP_FEATURE_HFG] = false;
#endif
}

DpIspStream::BaseConfig::BaseConfig()
    : srcWidth(-1),
      srcHeight(-1),
      srcYPitch(-1),
      srcUVPitch(-1),
      srcFormat(DP_COLOR_RGB888),
      srcProfile(DP_PROFILE_BT601),
      srcSecure(DP_SECURE_NONE),
      srcFlush(true)
{
    int32_t index;

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        dstWidth[index] = -1;
        dstHeight[index] = -1;
        dstYPitch[index] = -1;
        dstUVPitch[index] = -1;
        dstFormat[index] = DP_COLOR_RGB888;
        dstProfile[index] = DP_PROFILE_BT601;
        dstSecure[index] = DP_SECURE_NONE;
        dstFlush[index] = true;
        dstEnabled[index] = false;
        dstPlane[index] = -1;

        multiSrcXStart[index] = 0;
        multiSrcYStart[index] = 0;
        multiCropWidth[index] = 0;
        multiCropWidthSubpixel[index] = 0;
        multiCropHeight[index] = 0;
        multiCropHeightSubpixel[index] = 0;
        multiSrcXSubpixel[index] = 0;
        multiSrcYSubpixel[index] = 0;

        targetXStart[index] = -1;
        targetYStart[index] = -1;

        rotation[index] = 0;
        flipStatus[index] = false;
        ditherStatus[index] = false;

    }

    memset(&PqConfig, 0, sizeof(PqConfig));

}

DpIspStream::IspConfig::IspConfig()
    : frameChange(false),
      tpipeChange(false),
      srcBuffer(-1)
{
    BaseConfig();
    int32_t index;

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        dstBuffer[index] = -1;
        multiCropChange[index] = false;
        dstPortType[index] = 0;
    }

    jpegEnc_cfg.fileDesc     = -1;
    jpegEnc_cfg.fQuality     = 0;
    jpegEnc_cfg.size         = 0;
    jpegEnc_cfg.soi_en       = 0;
    jpegEnc_cfg.memSWAddr[0] = 0;
    jpegEnc_cfg.memSWAddr[1] = 0;
    jpegEnc_cfg.memSWAddr[2] = 0;
    callback = NULL;
    callbackData = NULL;

    memset(&pqParam, 0, sizeof(pqParam));

}

DpIspStream::DpIspStream(ISPStreamType type)
    : m_streamType(type),
      m_pStream(NULL),
      m_pChannel(new DpChannel()),
      m_channelID(-1),
      m_pListMutex(new DpMutex()),
      m_pBufferMutex(new DpMutex()),
      m_pRegMutex(new DpMutex()),
      m_pConfigMutex(new DpMutex()),
      m_pSrcPool(new DpBasicBufferPool()),
      m_jpegEnc_filesize(0),
      m_configFrameCount(0),
      m_dequeueFrameCount(0),
      m_pJobCond(new DpCondition()),
      m_abortJobs(false),
      m_allocatePA(false),
      m_pDRERegMutex(new DpMutex())
{
    int32_t index;

    switch(m_streamType)
    {
        case ISP_CC_STREAM:
        case ISP_IC_STREAM:
            m_pStream = new DpStream(STREAM_ISP_VR);
            m_pStream->setSyncMode(false);
            break;
        case ISP_ZSD_STREAM:
            m_pStream = new DpStream(STREAM_ISP_ZSD);
            m_pStream->setSyncMode(false);
            break;
        case ISP_ZSD_SLOW_STREAM:
            m_pStream = new DpStream(STREAM_ISP_ZSD_SLOW);
            m_pStream->setSyncMode(true);
            break;
        case ISP_VSS_STREAM:
            m_pStream = new DpStream(STREAM_ISP_VSS);
#ifdef MDP_VSS_ASYNC_ENABLE
            m_pStream->setSyncMode(false);
            createThread();
#else
            m_pStream->setSyncMode(true);
#endif
            break;
        case WPE_STREAM:
            m_pStream = new DpStream(STREAM_WPE);
            m_pStream->setSyncMode(false);
            break;
        case WPE_STREAM2:
            m_pStream = new DpStream(STREAM_WPE2);
            m_pStream->setSyncMode(false);
            break;
        default:
            assert(0);
            break;
    }

    assert(NULL != m_pStream);

#ifndef BASIC_PACKAGE
    m_pqSupport = DpDriver::getInstance()->getPQSupport();
#endif

    m_pBufferMutex->lock();
    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        m_pDstPool[index]     = new DpBasicBufferPool();
        m_sharpness[index]    = 0;
        m_dstBufferList[index].clear();

#ifndef BASIC_PACKAGE
        if (0 != m_pqSupport) {
            m_PqID[index] = getPqID();
        }
#endif // BASIC_PACKAGE

        m_pDstMatrix[index] = NULL;
    }
    m_srcBufferList.clear();
    m_srcBufferStartTimeList.clear();
    m_regValueList.clear();
    m_regCountList.clear();
    m_pBufferMutex->unlock();

    m_pListMutex->lock();
    m_frameDoneList.clear();
    m_jobIDsList.clear();
    m_pListMutex->unlock();

    m_pConfigMutex->lock();
    m_ispConfigList.clear();
    m_pConfigMutex->unlock();

    memset(m_PABuffer, 0, sizeof(m_PABuffer));
    memset(m_DREPABuffer, 0, sizeof(m_DREPABuffer));

    m_DREPABufferSizeList.clear();
    m_DREPABufferStartIndexList.clear();
    m_numReadbackEngineList.clear();
#ifdef SUPPORT_DRE
    DpDriver::getInstance()->allocatePABuffer(MAX_NUM_READBACK_PA_BUFFER, m_DREPABuffer);
#endif //SUPPORT_DRE
}


DpIspStream::~DpIspStream()
{
    int32_t index;

    m_pBufferMutex->lock();
    m_srcBufferList.clear();
    m_srcBufferStartTimeList.clear();
    m_regValueList.clear();
    m_regCountList.clear();

    m_DREPABufferSizeList.clear();
    m_DREPABufferStartIndexList.clear();
    m_numReadbackEngineList.clear();

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        delete m_pDstPool[index];
        m_pDstPool[index] = NULL;

        m_dstBufferList[index].clear();

#ifndef BASIC_PACKAGE
        if (0 != m_pqSupport) {
#if CONFIG_FOR_OS_ANDROID
            PQSessionManager::getInstance()->destroyPQSession(m_PqID[index]);
#endif // CONFIG_FOR_OS_ANDROID
        }
#endif // BASIC_PACKAGE

        delete m_pDstMatrix[index];
        m_pDstMatrix[index] = NULL;
    }
    m_pBufferMutex->unlock();

    m_pListMutex->lock();
    m_frameDoneList.clear();
    m_jobIDsList.clear();
    m_pListMutex->unlock();

    m_pConfigMutex->lock();
    m_ispConfigList.clear();
    m_pConfigMutex->unlock();

    if (m_allocatePA == true)
    {
        DpDriver::getInstance()->releasePABuffer(MAX_NUM_READBACK_REGS, m_PABuffer);
    }

#ifdef MDP_VSS_ASYNC_ENABLE
    if(m_pStream->getScenario() == STREAM_ISP_VSS)
    {
        {
            AutoMutex lock(m_pListMutex);
            m_abortJobs = true;
            m_pJobCond->signal();
        }
        pthread_join(m_thread, NULL);
    }
#endif

    delete m_pJobCond;
    m_pJobCond = NULL;

#ifdef SUPPORT_DRE
    DpDriver::getInstance()->releasePABuffer(MAX_NUM_READBACK_PA_BUFFER, m_DREPABuffer);
#endif // SUPPORT_DRE

    delete m_pSrcPool;
    m_pSrcPool = NULL;

    delete m_pStream;
    m_pStream = NULL;

    delete m_pChannel;
    m_pChannel = NULL;

    delete m_pListMutex;
    m_pListMutex = NULL;

    delete m_pBufferMutex;
    m_pBufferMutex = NULL;

    delete m_pRegMutex;
    m_pRegMutex = NULL;

    delete m_pDRERegMutex;
    m_pDRERegMutex = NULL;

    delete m_pConfigMutex;
    m_pConfigMutex = NULL;
}

void DpIspStream::createThread()
{
    pthread_attr_t     attribute;
    int32_t            schedPolicy;
    struct sched_param threadParam;
    DpTimeValue begin;
    DpTimeValue end;
    int32_t     diff;

    DP_TIMER_GET_CURRENT_TIME(begin);

    pthread_attr_init(&attribute);

    pthread_getschedparam(pthread_self(), &schedPolicy, &threadParam);

    if ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy))
    {
        pthread_attr_setschedpolicy(&attribute, schedPolicy);
        pthread_attr_setschedparam(&attribute, &threadParam);
    }

    if (pthread_create(&m_thread, &attribute, waitCompleteLoop, this))
    {
        pthread_create(&m_thread, NULL, waitCompleteLoop, this);
    }

    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff);
    if (diff > 10)
    {
        DPLOGW("DpIspStream: create thread (%s) %d ms\n",
            ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy)) ? "real-time" : "normal", diff);
    }

    pthread_attr_destroy(&attribute);
}

/*
    waitComplete thread function for VSS stream
*/
void* DpIspStream::waitCompleteLoop(void* para)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status = DP_STATUS_RETURN_SUCCESS;
    DP_STATUS_ENUM rtn_status = DP_STATUS_RETURN_SUCCESS;
    DpReadbackRegs readBackRegs;
    DpReadbackRegs DREReadBackRegs;
    ReadbackEngineList readbackEngineList;
    DpJobID        _jobID = 0;
#ifdef SUPPORT_DRE
    uint32_t       index;
#endif
    DpIspStream    *stream;
#if 0  //code for ispstream callback, disabled temporally
    IspConfig      curConfig;
    int32_t        srcBufId;
    bool           bAcquireDstBufferFail = false;
    void           *pBase[3];
    uint32_t       size[3];
#endif

    if (para != NULL)
    {
        stream = (DpIspStream*)para;
    }
    else
    {
        DPLOGE("DpAsyncBlitStream: cannot create thread\n");
        return NULL;
    }

    memset(&readBackRegs, 0, sizeof(readBackRegs));
    memset(&DREReadBackRegs, 0, sizeof(DREReadBackRegs));

    stream->m_pListMutex->lock();
    while (1)
    {
        if(0 < stream->m_jobIDsList.size())
        {
            stream->m_pStream->getReadbackEngineStatus(readbackEngineList);
            JobIDList curJobs = stream->m_jobIDsList.front();
            stream->m_pListMutex->unlock();
            for(int i = 0; i < curJobs.size(); i++)
            {
                _jobID = curJobs[i];

#ifdef SUPPORT_DRE
                stream->m_pDRERegMutex->lock();
                if(stream->m_DREPABufferStartIndexList.empty() || (i != (curJobs.size() - 1)))
                {
                    stream->m_pDRERegMutex->unlock();
#endif // SUPPORT_DRE
                    readBackRegs.m_engineFlag = 0;

                    DPLOGI("VSS wait frame done %llx", _jobID);
                    status = DpDriver::getInstance()->waitFramedone(_jobID, readBackRegs);
                    if (DP_STATUS_RETURN_SUCCESS != status)
                    {
                        DPLOGE("DpIspStream: wait complete stream failed(%d) vss job(%llx) %d/%d\n", status,_jobID, i, curJobs.size());
                        stream->m_pStream->dumpDebugStream();
                        //TODO fix curConfig
                        stream->m_curConfig.frameChange = true;
                        rtn_status = status;
                        continue;
                    }
#ifdef SUPPORT_DRE
                }
                else
                {
                    if (!stream->m_DREPABufferSizeList.empty())
                    {
                        DREReadBackRegs.m_num = stream->m_DREPABufferSizeList.front();

                        if (DREReadBackRegs.m_num > MAX_NUM_READBACK_REGS)
                        {
                            DPLOGE("DpIspStream: wait complete stream readbeck fail\n");
                        }
                        else
                        {
                            DREReadBackRegs.m_engineFlag = (1 << tAAL0);
                            for (index = 0; index < DREReadBackRegs.m_num; index++)
                            {
                                DREReadBackRegs.m_regs[index] = stream->m_DREPABuffer[(stream->m_DREPABufferStartIndexList.front() + index) & (MAX_NUM_READBACK_PA_BUFFER - 1)];
                            }
                        }
                    }
                    else
                    {
                         DPLOGE("DpIspStream::waitCompleteLoop: m_DREPABufferStartIndexList and m_DREPABufferSizeList not match\n");
                    }

                    stream->m_pDRERegMutex->unlock();

                    status = DpDriver::getInstance()->waitFramedone(_jobID, DREReadBackRegs);
                    if (DP_STATUS_RETURN_SUCCESS != status)
                    {
                        DPLOGE("DpIspStream: wait complete stream failed(%d) vss job(%llx) %d/%d\n", status,_jobID, i, curJobs.size());
                        stream->m_pStream->dumpDebugStream();
                        //TODO fix curConfig
                        stream->m_curConfig.frameChange = true;
                        rtn_status = status;
                        continue;
                    }
#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
                    IspConfig ispConfig;

                    stream->m_pConfigMutex->lock();
                    if (!stream->m_ispConfigList.empty())
                    {
                        ispConfig = stream->m_ispConfigList.front();
                        stream->m_pConfigMutex->unlock();

                        PQDREHistogramAdaptor *pPQDREHistogramAdaptor = PQReadBackFactory::getInstance()->getDRE(0);
                        int32_t portID;
                        PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
                        uint64_t PQSessionID;
                        int32_t prevWidth, prevHeight;

                        for (portID = 0; portID < ISP_MAX_OUTPUT_PORT_NUM; portID++)
                        {
                            if (ispConfig.pqParam[portID].scenario == MEDIA_ISP_PREVIEW ||
                                ispConfig.pqParam[portID].scenario == MEDIA_ISP_CAPTURE)
                            {
                                if (pPQDREHistogramAdaptor != NULL &&
                                    ispConfig.PqConfig[portID].enDRE != 0 &&
                                    (ispConfig.pqParam[portID].u.isp.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM00 ||
                                    ispConfig.pqParam[portID].u.isp.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM01))
                                {
#ifndef BASIC_PACKAGE
                                    PQSessionID = (static_cast<uint64_t>(stream->m_PqID[portID]) << 32) | ispConfig.pqParam[portID].scenario;
#else
                                    PQSessionID = (static_cast<uint64_t>(stream->m_PqID[portID]) << 32);
#endif // BASIC_PACKAGE

                                    PQSession* pPQSession = pPQSessionManager->getPQSession(PQSessionID);

                                    if (pPQSession != NULL)
                                    {
                                        pPQSession->getDrePreviousSize(&prevWidth, &prevHeight);

                                        pPQDREHistogramAdaptor->setHistogram(
                                            DREReadBackRegs.m_values,
                                            DREReadBackRegs.m_num,
                                            ispConfig.pqParam[portID].u.isp.dpDREParam.buffer,
                                            ispConfig.pqParam[portID].u.isp.dpDREParam.userId,
                                            prevWidth,
                                            prevHeight);
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        stream->m_pConfigMutex->unlock();
                        DPLOGE("DpIspStream::waitCompleteLoop set histogram fail, stream->m_ispConfigList is empty\n");
                    }
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID
                }
#endif // SUPPORT_DRE
                stream->m_dequeueFrameCount = 0;
            }

#ifdef SUPPORT_DRE
            stream->m_pDRERegMutex->lock();
            if (!stream->m_DREPABufferStartIndexList.empty())
            {
                stream->m_DREPABufferStartIndexList.erase(stream->m_DREPABufferStartIndexList.begin());
            }
            else
            {
                DPLOGE("DpIspStream::waitCompleteLoop m_DREPABufferStartIndexList empty\n");
            }

            if (!stream->m_DREPABufferSizeList.empty())
            {
                stream->m_DREPABufferSizeList.erase(stream->m_DREPABufferSizeList.begin());
            }
            else
            {
                DPLOGE("DpIspStream::waitCompleteLoop m_DREPABufferSizeList empty\n");
            }

            if (!stream->m_numReadbackEngineList.empty())
            {
                stream->m_pStream->cleanReadbackEngineStatus(stream->m_numReadbackEngineList.front());
                stream->m_numReadbackEngineList.erase(stream->m_numReadbackEngineList.begin());
            }
            else
            {
                DPLOGE("DpIspStream::waitCompleteLoop m_numReadbackEngineList empty\n");
            }
            stream->m_pDRERegMutex->unlock();
#endif //SUPPORT_DRE

            stream->m_pListMutex->lock();
#if 0  //code for ispstream callback, disabled temporally
            curConfig = stream->m_ispConfigList[stream->m_frameDoneList.size()];
            srcBufId = stream->m_srcBufferList[stream->m_frameDoneList.size()];
#endif
            DpDoneJobItem doneJob;
            doneJob.jobId = _jobID;
            doneJob.jobStatus = rtn_status;
            stream->m_frameDoneList.push_back(doneJob);
            IspConfig ispConfig;
            stream->m_jobIDsList.erase(stream->m_jobIDsList.begin()); //Remove job set
            stream->m_pListMutex->unlock();

            stream->m_pConfigMutex->lock();
            if (!stream->m_ispConfigList.empty())
            {
                ispConfig = stream->m_ispConfigList.front();
                stream->m_pConfigMutex->unlock();
                if(ispConfig.callback != NULL) {
                    DPLOGD("VSS trigger callback status: %d, %p", rtn_status, stream);
                    ispConfig.callback(ispConfig.callbackData);
                }
            }
            else
            {
                stream->m_pConfigMutex->unlock();
            }

#if 0  //code for ispstream callback, disabled temporally
            //release buffer pool
            status = stream->m_pSrcPool->asyncReleaseBuffer(srcBufId);
            DPLOGI("DpAsyncBlitStream: srcBuffer release %d status: %d\n", srcBufId, status);

            for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
            {
                if (!curConfig.dstEnabled[index])
                    continue;
                if (true == curConfig.dstFlush[index])
                {
                    status = stream->m_pDstPool[index]->flushWriteBuffer(curConfig.dstBuffer[index]);
                    DPLOGI("DpAsyncBlitStream: flushWriteBuffer %d status: %d\n", curConfig.dstBuffer[index], status);
                }

                // acquire and release destination buffer, this is to reset output buffer state
                status = stream->m_pDstPool[index]->queueBuffer(curConfig.dstBuffer[index]);
                DPLOGI("DpAsyncBlitStream: queueBuffer %d status: %d\n", curConfig.dstBuffer[index], status);

                status = stream->m_pDstPool[index]->acquireBuffer(&curConfig.dstBuffer[index], pBase, size);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpAsyncBlitStream: acquire dst buffer failed(%d)\n", status);
                    bAcquireDstBufferFail = true;
                    continue;
                }

                status = stream->m_pDstPool[index]->asyncReleaseBuffer(curConfig.dstBuffer[index]);
                DPLOGI("DpAsyncBlitStream: dstBuffer release %d status: %d \n", curConfig.dstBuffer[index], status);

                stream->m_pDstPool[index]->unregisterBuffer(curConfig.dstBuffer[index]);

            }

            if (bAcquireDstBufferFail)
            {
                break;
            }

            stream->m_pSrcPool->unregisterBuffer(srcBufId);
#endif
            DPLOGI("DpIspStream: VSS Job ID : %llx is done!!!\n", _jobID);
            stream->m_pListMutex->lock();
        }
        else if (stream->m_abortJobs)
        {
            break;
        }
        else
        {
            stream->m_pJobCond->wait(*stream->m_pListMutex);
        }
    }

    stream->m_pListMutex->unlock();
    return NULL;
}

DP_STATUS_ENUM DpIspStream::queueSrcBuffer(void     *pVA,
                                           uint32_t size)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if (NULL == pVA)
    {
        DPLOGE("DpIspStream: invalid buffer base address");
        return DP_STATUS_INVALID_PARAX;
    }

#if 0  //code for ispstream callback, disabled temporally
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBuffer(&pVA,
                                        &size,
                                        1,
                                        -1,
                                        &curSourceBuffer);

    if ((-1 == curSourceBuffer) || (status != DP_STATUS_RETURN_SUCCESS))
    {
        DPLOGE("DpIsp: register src buffer failed\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();
    m_curConfig.srcBuffer = curSourceBuffer;
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpIspStream::queueSrcBuffer(void     *pVA,
                                           uint32_t MVA,
                                           uint32_t size)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if ((NULL == pVA) || (0 == MVA))
    {
        DPLOGE("DpIspStream: invalid buffer base address\n");
        return DP_STATUS_INVALID_PARAX;
    }

#if 0  //code for ispstream callback, disabled temporally
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBuffer(&pVA,
                                        &MVA,
                                        &size,
                                        1,
                                        -1,
                                        &curSourceBuffer);

    if ((-1 == curSourceBuffer) || (status != DP_STATUS_RETURN_SUCCESS))
    {
        DPLOGE("DpIsp: register src buffer failed\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();
    m_curConfig.srcBuffer = curSourceBuffer;

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpIspStream::queueSrcBuffer(void     **pVAList,
                                           uint32_t *pSizeList,
                                           int32_t  planeNum)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpIspStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

#if 0  //code for ispstream callback, disabled temporally
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBuffer(pVAList,
                                        pSizeList,
                                        planeNum,
                                        -1,
                                        &curSourceBuffer);

    if ((-1 == curSourceBuffer) || (status != DP_STATUS_RETURN_SUCCESS))
    {
        DPLOGE("DpIsp: register src buffer failed\n");
        return DP_STATUS_OPERATION_FAILED;
    }

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();
    m_curConfig.srcBuffer = curSourceBuffer;

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpIspStream::queueSrcBuffer(uint32_t *pMVAList,
                                           uint32_t *pSizeList,
                                           int32_t  planeNum)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpIspStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

#if 0  //code for ispstream callback, disabled temporally
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBuffer(pMVAList,
                                        pSizeList,
                                        planeNum,
                                        -1,
                                        &curSourceBuffer);
    assert(-1 != curSourceBuffer);

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();
    m_curConfig.srcBuffer = curSourceBuffer;

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpIspStream::queueSrcBuffer(void     **pVAList,
                                           uint32_t *pMVAList,
                                           uint32_t *pSizeList,
                                           int32_t  planeNum)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpIspStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

#if 0  //code for ispstream callback, disabled temporally
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBuffer(pVAList,
                                        pMVAList,
                                        pSizeList,
                                        planeNum,
                                        -1,
                                        &curSourceBuffer);
    assert(-1 != curSourceBuffer);

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();
    m_curConfig.srcBuffer = curSourceBuffer;

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpIspStream::queueSrcBuffer(int32_t  fileDesc,
                                           uint32_t *pSizeList,
                                           uint32_t planeNum)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    int32_t        curSourceBuffer;
    DpTimeValue    initTime;

    DPLOGI("==============queue src buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpIspStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

#if 0  //code for ispstream callback, disabled temporally
    if(-1 != m_srcBuffer)
    {
        m_pSrcPool->unregisterBuffer(m_srcBuffer);
        m_srcBuffer = -1;
    }
#endif

    status = m_pSrcPool->registerBufferFD(fileDesc,
                                          pSizeList,
                                          planeNum,
                                          -1,
                                          &curSourceBuffer);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    m_pBufferMutex->lock();
    m_srcBufferList.push_back(curSourceBuffer);
    DP_TIMER_GET_CURRENT_TIME(initTime)
    m_srcBufferStartTimeList.push_back(initTime);
    m_pBufferMutex->unlock();
    m_curConfig.srcBuffer = curSourceBuffer;

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue src buffer failed(%d)\n", status);
        return status;
    }

    DPLOGI("==============queue src buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpIspStream::dequeueSrcBuffer()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    DpTimeValue    begin;
    DpTimeValue    begin2;
    DpTimeValue    end;
    int32_t        diff[5];
    int32_t        curSourceBuffer;
    IspConfig      ispConfig;

    DPLOGI("==============dequeue src buffer begin:==============\n");
    DP_TIMER_GET_CURRENT_TIME(begin2);

    m_pBufferMutex->lock();
    curSourceBuffer = m_srcBufferList.front();
    begin = m_srcBufferStartTimeList.front();
    m_srcBufferList.erase(m_srcBufferList.begin());
    m_srcBufferStartTimeList.erase(m_srcBufferStartTimeList.begin());
    m_pBufferMutex->unlock();
    //time for m_pBufferMutex
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin2,
                                end,
                                diff[0]);

    //Could get ISP config here
    m_pConfigMutex->lock();
    if (!m_ispConfigList.empty())
    {
        ispConfig = m_ispConfigList.front();
    }
    m_pConfigMutex->unlock();

    //time for m_pBufferMutex
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin2,
                                end,
                                diff[1]);


    DPLOGI("DpIsp: dequeueSrc: (%d, %d, %d, %d, C%d%s%s%s%s, P%d)\n",
           ispConfig.srcWidth, ispConfig.srcHeight,
           ispConfig.srcYPitch, ispConfig.srcUVPitch,
           DP_COLOR_GET_UNIQUE_ID(ispConfig.srcFormat),
           DP_COLOR_GET_SWAP_ENABLE(ispConfig.srcFormat) ? "s" : "",
           DP_COLOR_GET_BLOCK_MODE(ispConfig.srcFormat) ? "b" : "",
           DP_COLOR_GET_INTERLACED_MODE(ispConfig.srcFormat) ? "i" : "",
           DP_COLOR_GET_UFP_ENABLE(ispConfig.srcFormat) ? "u" : "", ispConfig.srcProfile);

    if (-1 == curSourceBuffer)
    {
        DPLOGE("DpIspStream: empty src buffer of port\n");
        return DP_STATUS_INVALID_BUFFER;
    }

#ifdef MDP_VSS_ASYNC_ENABLE
    if(m_pStream->getScenario() != STREAM_ISP_VSS)
#endif
    {
        status = waitComplete();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpIspStream: wait stream failed(%d)\n", status);
            m_pStream->stopStream();

            return status;
        }
    }

    //time for wait complete
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin2,
                                end,
                                diff[2]);

    if(false == m_pStream->getSyncMode()) //Async mode!
    {
        status = m_pSrcPool->asyncReleaseBuffer(curSourceBuffer);
        DPLOGI("DpIspStream:SrcBuffer releaseBuffer %d status: %d \n",curSourceBuffer,status);
    }

    if(-1 != curSourceBuffer)
    {
        m_pSrcPool->unregisterBuffer(curSourceBuffer);
        curSourceBuffer = -1;
    }

    DP_TIMER_GET_CURRENT_TIME(end);

    //time for all
    DP_TIMER_GET_DURATION_IN_MS(begin2,
                                end,
                                diff[3]);

    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[4]);

    if (diff[4] > 34)
    {
        DPLOGW("DpIsp: 0x%08x, src %d ms, %d, %d, %d, %d\n"
                , this, diff[4], diff[0], diff[1], diff[2], diff[3]);
    }
    else if (m_pStream->getScenario() == STREAM_ISP_ZSD)
    {
        DPLOGD("DpIsp: %p, src %d ms, %d, %d, %d\n"
                , this, diff[3], diff[0], diff[1], diff[2]);
    }
    else
    {
        DPLOGI("DpIsp: 0x%08x, src %d ms, %d, %d, %d, %d\n"
                , this, diff[4], diff[0], diff[1], diff[2], diff[3]);
    }

    DPLOGI("==============dequeue src buffer end:==============\n");

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::setSrcConfig(DpColorFormat srcFormat,
                                         int32_t       srcWidth,
                                         int32_t       srcHeight,
                                         int32_t       srcPitch,
                                         bool          doFlush)
{
    DP_TRACE_CALL();

    if ((srcWidth  <= 0) ||
        (srcHeight <= 0) ||
        (srcPitch  <= 0))
    {
        DPLOGE("DpIspStream: invalid source width(%d), height(%d) or pitch(%d)\n", srcWidth, srcHeight, srcPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (srcPitch < ((srcWidth * DP_COLOR_BITS_PER_PIXEL(srcFormat)) >> 3))
    {
        DPLOGE("DpIspStream: pitch(%d) is less than width(%d)\n", srcPitch, srcWidth);
        return DP_STATUS_INVALID_PARAX;
    }

    m_curConfig.srcFormat  = srcFormat;
    m_curConfig.srcWidth   = srcWidth;
    m_curConfig.srcHeight  = srcHeight;
    m_curConfig.srcYPitch  = srcPitch;
    m_curConfig.srcUVPitch = 0;
    m_curConfig.srcFlush   = doFlush;
    m_curConfig.srcProfile = DP_PROFILE_FULL_BT601;
    m_curConfig.srcSecure  = DP_SECURE_NONE;
    m_pSrcPool->setSecureMode(m_curConfig.srcSecure);
    m_curConfig.callback   = NULL;
    m_curConfig.callbackData = NULL;
    DPLOGI("src flush status %d\n", doFlush);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::setSrcConfig(int32_t           srcWidth,
                                         int32_t           srcHeight,
                                         int32_t           srcYPitch,
                                         int32_t           srcUVPitch,
                                         DpColorFormat     srcFormat,
                                         DP_PROFILE_ENUM   srcProfile,
                                         DpInterlaceFormat,
                                         DpRect*,
                                         bool              doFlush,
                                         DpSecure          secure)
{
    DP_TRACE_CALL();

    if ((srcWidth  <= 0) ||
        (srcHeight <= 0) ||
        (srcYPitch <= 0))
    {
        DPLOGE("DpIspStream: invalid source width(%d), height(%d) or pitch(%d)\n", srcWidth, srcHeight, srcYPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (srcYPitch < ((srcWidth * DP_COLOR_BITS_PER_PIXEL(srcFormat)) >> 3))
    {
        DPLOGE("DpIspStream: pitch(%d) is less than width(%d)\n", srcYPitch, srcWidth);
        return DP_STATUS_INVALID_PARAX;
    }

    m_curConfig.srcFormat  = srcFormat;
    m_curConfig.srcWidth   = srcWidth;
    m_curConfig.srcHeight  = srcHeight;
    m_curConfig.srcYPitch  = srcYPitch;
    m_curConfig.srcUVPitch = srcUVPitch;
    m_curConfig.srcFlush   = doFlush;
    m_curConfig.srcProfile = srcProfile;
    m_curConfig.srcSecure  = secure;
    m_pSrcPool->setSecureMode(m_curConfig.srcSecure);
    m_curConfig.callback   = NULL;
    m_curConfig.callbackData = NULL;

    DPLOGI("src profile (%d) flush status %d\n", srcProfile, doFlush);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::setSrcCrop(int32_t XCropStart,
                                       int32_t XSubpixel,
                                       int32_t YCropStart,
                                       int32_t YSubpixel,
                                       int32_t cropWidth,
                                       int32_t cropHeight)
{
    DP_TRACE_CALL();

    for (int i = 0; i < ISP_MAX_OUTPUT_PORT_NUM; i++)
    {
        m_curConfig.multiSrcXStart[i]    = XCropStart;
        m_curConfig.multiSrcXSubpixel[i] = XSubpixel;
        m_curConfig.multiSrcYStart[i]    = YCropStart;
        m_curConfig.multiSrcYSubpixel[i] = YSubpixel;
        m_curConfig.multiCropWidth[i]    = cropWidth;
        m_curConfig.multiCropWidthSubpixel[i]  = 0;
        m_curConfig.multiCropHeight[i]   = cropHeight;
        m_curConfig.multiCropHeightSubpixel[i] = 0;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::setSrcCrop(int32_t portIndex,
                                       int32_t XCropStart,
                                       int32_t XSubpixel,
                                       int32_t YCropStart,
                                       int32_t YSubpixel,
                                       int32_t cropWidth,
                                       int32_t cropHeight)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
        return DP_STATUS_INVALID_PORT;

    m_curConfig.multiSrcXStart[portIndex]    = XCropStart;
    m_curConfig.multiSrcXSubpixel[portIndex] = XSubpixel;
    m_curConfig.multiSrcYStart[portIndex]    = YCropStart;
    m_curConfig.multiSrcYSubpixel[portIndex] = YSubpixel;
    m_curConfig.multiCropWidth[portIndex]    = cropWidth;
    m_curConfig.multiCropWidthSubpixel[portIndex]  = 0;
    m_curConfig.multiCropHeight[portIndex]   = cropHeight;
    m_curConfig.multiCropHeightSubpixel[portIndex] = 0;

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpIspStream::setSrcCrop(int32_t portIndex,
                                       int32_t XCropStart,
                                       int32_t XSubpixel,
                                       int32_t YCropStart,
                                       int32_t YSubpixel,
                                       int32_t cropWidth,
                                       int32_t cropWidthSubpixel,
                                       int32_t cropHeight,
                                       int32_t cropHeightSubpixel)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
        return DP_STATUS_INVALID_PORT;

    m_curConfig.multiSrcXStart[portIndex]    = XCropStart;
    m_curConfig.multiSrcXSubpixel[portIndex] = XSubpixel;
    m_curConfig.multiSrcYStart[portIndex]    = YCropStart;
    m_curConfig.multiSrcYSubpixel[portIndex] = YSubpixel;
    m_curConfig.multiCropWidth[portIndex]    = cropWidth;
    m_curConfig.multiCropWidthSubpixel[portIndex]  = cropWidthSubpixel;
    m_curConfig.multiCropHeight[portIndex]   = cropHeight;
    m_curConfig.multiCropHeightSubpixel[portIndex] = cropHeightSubpixel;

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpIspStream::queueDstBuffer(int32_t  portIndex,
                                           void     **pVAList,
                                           uint32_t *pSizeList,
                                           int32_t  planeNum)
{
    DP_TRACE_CALL();
    int32_t dstBuffer = -1;
    DP_STATUS_ENUM status;

    DPLOGI("==============queue dst buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpIspStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

#if 0  //code for ispstream callback, disabled temporally
    if(-1 != m_dstBuffer[portIndex])
    {
        m_pDstPool[portIndex]->cancelBuffer(m_dstBuffer[portIndex]);
        m_pDstPool[portIndex]->unregisterBuffer(m_dstBuffer[portIndex]);
        m_dstBuffer[portIndex] = -1;
    }
#endif

    status = m_pDstPool[portIndex]->registerBuffer(pVAList,
                                                   pSizeList,
                                                   planeNum,
                                                   -1,
                                                   &dstBuffer);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue dst%d buffer failed(%d)\n", portIndex, status);
        return status;
    }

    m_pBufferMutex->lock();
    m_dstBufferList[portIndex].push_back(dstBuffer);
    m_pBufferMutex->unlock();

    m_curConfig.dstPlane[portIndex] = planeNum;
    m_curConfig.dstBuffer[portIndex] = dstBuffer;
    DPLOGI("==============queue dst buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpIspStream::queueDstBuffer(int32_t  portIndex,
                                           int32_t  fileDesc,
                                           uint32_t *pSizeList,
                                           int32_t  planeNum)
{
    DP_TRACE_CALL();
    int32_t dstBuffer = -1;
    DP_STATUS_ENUM status;

    DPLOGI("==============queue dst buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpIspStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

#if 0  //code for ispstream callback, disabled temporally
    if(-1 != m_dstBuffer[portIndex])
    {
        m_pDstPool[portIndex]->cancelBuffer(m_dstBuffer[portIndex]);
        m_pDstPool[portIndex]->unregisterBuffer(m_dstBuffer[portIndex]);
        m_dstBuffer[portIndex] = -1;
    }
#endif

    status = m_pDstPool[portIndex]->registerBufferFD(fileDesc,
                                                     pSizeList,
                                                     planeNum,
                                                     -1,
                                                     &dstBuffer);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue dst%d buffer failed(%d)\n", portIndex, status);
        return status;
    }

    m_pBufferMutex->lock();
    m_dstBufferList[portIndex].push_back(dstBuffer);
    m_pBufferMutex->unlock();

    m_curConfig.dstPlane[portIndex] = planeNum;
    m_curConfig.dstBuffer[portIndex] = dstBuffer;

    DPLOGI("==============queue dst buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpIspStream::queueDstBuffer(int32_t  portIndex,
                                           uint32_t *pMVAList,
                                           uint32_t *pSizeList,
                                           int32_t  planeNum)
{
    DP_TRACE_CALL();
    int32_t dstBuffer = -1;
    DP_STATUS_ENUM status;

    DPLOGI("==============queue dst buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpIspStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }
#if 0  //code for ispstream callback, disabled temporally
    if(-1 != m_dstBuffer[portIndex])
    {
        m_pDstPool[portIndex]->cancelBuffer(m_dstBuffer[portIndex]);
        m_pDstPool[portIndex]->unregisterBuffer(m_dstBuffer[portIndex]);
        m_dstBuffer[portIndex] = -1;
    }
#endif

    status = m_pDstPool[portIndex]->registerBuffer(pMVAList,
                                                   pSizeList,
                                                   planeNum,
                                                   -1,
                                                   &dstBuffer);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue dst%d buffer failed(%d)\n", portIndex, status);
        return status;
    }

    m_pBufferMutex->lock();
    m_dstBufferList[portIndex].push_back(dstBuffer);
    m_pBufferMutex->unlock();

    m_curConfig.dstPlane[portIndex] = planeNum;
    m_curConfig.dstBuffer[portIndex] = dstBuffer;

    DPLOGI("==============queue dst buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpIspStream::queueDstBuffer(int32_t  portIndex,
                                           void     **pVAList,
                                           uint32_t *pMVAList,
                                           uint32_t *pSizeList,
                                           int32_t  planeNum)
{
    DP_TRACE_CALL();
    int32_t dstBuffer = -1;
    DP_STATUS_ENUM status;

    DPLOGI("==============queue dst buffer begin:==============\n");

    if((planeNum < 0) || (planeNum > 3))
    {
        DPLOGE("DpIspStream: error argument - planceNumber: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }
#if 0  //code for ispstream callback, disabled temporally
    if(-1 != m_dstBuffer[portIndex])
    {
        m_pDstPool[portIndex]->cancelBuffer(m_dstBuffer[portIndex]);
        m_pDstPool[portIndex]->unregisterBuffer(m_dstBuffer[portIndex]);
        m_dstBuffer[portIndex] = -1;
    }
#endif

    status = m_pDstPool[portIndex]->registerBuffer(pVAList,
                                                   pMVAList,
                                                   pSizeList,
                                                   planeNum,
                                                   -1,
                                                   &dstBuffer);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: queue dst%d buffer failed(%d)\n", portIndex, status);
        return status;
    }

    m_pBufferMutex->lock();
    m_dstBufferList[portIndex].push_back(dstBuffer);
    m_pBufferMutex->unlock();

    m_curConfig.dstPlane[portIndex] = planeNum;
    m_curConfig.dstBuffer[portIndex] = dstBuffer;

    DPLOGI("==============queue dst buffer end:==============\n");

    return status;
}


DP_STATUS_ENUM DpIspStream::setDstConfig(int32_t       portIndex,
                                         DpColorFormat dstFormat,
                                         int32_t       dstWidth,
                                         int32_t       dstHeight,
                                         int32_t       dstPitch,
                                         bool          doFlush)
{
    DP_TRACE_CALL();

    if ((dstWidth  <= 0) ||
        (dstHeight <= 0) ||
        (dstPitch  <= 0))
    {
        DPLOGE("DpIspStream: error argument - width(%d), height(%d) or pitch(%d)\n", dstWidth, dstHeight, dstPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if (dstPitch < ((dstWidth * DP_COLOR_BITS_PER_PIXEL(dstFormat)) >> 3))
    {
        DPLOGE("DpIspStream: pitch(%d) is less than width(%d)\n", dstPitch, dstWidth);
        return DP_STATUS_INVALID_PARAX;
    }

    m_curConfig.dstFormat[portIndex]  = dstFormat;
    m_curConfig.dstWidth[portIndex]   = dstWidth;
    m_curConfig.dstHeight[portIndex]  = dstHeight;
    m_curConfig.dstYPitch[portIndex]  = dstPitch;
    m_curConfig.dstUVPitch[portIndex] = 0;
    m_curConfig.dstEnabled[portIndex] = true;
    m_curConfig.dstFlush[portIndex]   = doFlush;
    m_curConfig.dstProfile[portIndex] = DP_PROFILE_BT601;
    m_curConfig.dstSecure[portIndex]  = DP_SECURE_NONE;
    m_pDstPool[portIndex]->setSecureMode(m_curConfig.dstSecure[portIndex]);

    DPLOGI("dst %d flush status %d\n", portIndex, doFlush);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::setDstConfig(int32_t           portIndex,
                                         int32_t           dstWidth,
                                         int32_t           dstHeight,
                                         int32_t           dstYPitch,
                                         int32_t           dstUVPitch,
                                         DpColorFormat     dstFormat,
                                         DP_PROFILE_ENUM   dstProfile,
                                         DpInterlaceFormat,
                                         DpRect*,
                                         bool              doFlush,
                                         DpSecure          secure)
{
    DP_TRACE_CALL();

    if ((dstWidth  <= 0) ||
        (dstHeight <= 0) ||
        (dstYPitch <= 0))
    {
        DPLOGE("DpIspStream: error argument - width(%d), height(%d) or pitch(%d)\n", dstWidth, dstHeight, dstYPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if (dstYPitch < ((dstWidth * DP_COLOR_BITS_PER_PIXEL(dstFormat)) >> 3))
    {
        DPLOGE("DpIspStream: pitch(%d) is less than width(%d)\n", dstYPitch, dstWidth);
        return DP_STATUS_INVALID_PARAX;
    }

    m_curConfig.dstFormat[portIndex]  = dstFormat;
    m_curConfig.dstWidth[portIndex]   = dstWidth;
    m_curConfig.dstHeight[portIndex]  = dstHeight;
    m_curConfig.dstYPitch[portIndex]  = dstYPitch;
    m_curConfig.dstUVPitch[portIndex] = dstUVPitch;
    m_curConfig.dstEnabled[portIndex] = true;
    m_curConfig.dstFlush[portIndex]   = doFlush;
    m_curConfig.dstProfile[portIndex] = dstProfile;
    m_curConfig.dstSecure[portIndex]  = secure;
    m_pDstPool[portIndex]->setSecureMode(m_curConfig.dstSecure[portIndex]);

    DPLOGI("dst %d flush status %d\n", portIndex, doFlush);
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::setPortType(int32_t           portIndex,
                                        uint32_t          portType,
                                        DpJPEGEnc_Config* JPEGEnc_cfg)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: invalid port index %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    m_curConfig.dstPortType[portIndex] = portType;

    if (portType == PORT_JPEG)
    {
        if (JPEGEnc_cfg)
        {
            memcpy(&(m_curConfig.jpegEnc_cfg), JPEGEnc_cfg, sizeof(m_curConfig.jpegEnc_cfg));
        }
        else
        {
            DPLOGE("DpIspStream: error argument - invalid JPEGEnc parameter index: %d\n", portIndex);
            return DP_STATUS_INVALID_PARAX;
        }
    }
    else if (portType == PORT_ISP || portType == PORT_WPE)
    {
        m_curConfig.dstEnabled[portIndex] = true;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::setSharpness(int32_t portIndex,
                                         int32_t gain)
{
    if (m_sharpness[portIndex] != gain)
    {
        m_sharpness[portIndex] = gain;
        m_curConfig.frameChange = true;
    }

    DpPqParam pqParam;

    if (gain != 0)
        pqParam.enable = true;
    else
        pqParam.enable = false;

    pqParam.scenario = MEDIA_ISP_PREVIEW;

    return setPQParameter(portIndex, pqParam);
}


DP_STATUS_ENUM DpIspStream::setDither(int32_t portIndex,
                                      bool    enDither)
{
    m_curConfig.ditherStatus[portIndex] = enDither;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::setRotation(int32_t portIndex,
                                        int32_t rotation)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    m_curConfig.rotation[portIndex] = rotation;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::setFlipStatus(int32_t portIndex,
                                          bool    flipStatus)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    m_curConfig.flipStatus[portIndex] = flipStatus;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::setParameter(ISP_TPIPE_CONFIG_STRUCT &extraPara, uint32_t hint)
{
    DP_TRACE_CALL();
    uint32_t offset = (m_configFrameCount & 0xF) << 2;

#if ReadRegister == 1
    if (extraPara.drvinfo.regCount > DP_MAX_SINGLE_PABUFFER_COUNT)
    {
        return DP_STATUS_INVALID_PARAX;
    }

    if ((extraPara.drvinfo.regCount != 0) && (m_allocatePA == false))
    {
        m_allocatePA = true;
        DpDriver::getInstance()->allocatePABuffer(MAX_NUM_READBACK_REGS, m_PABuffer);
        m_pRegMutex->lock();
        m_regCountList.push_back(extraPara.drvinfo.regCount);
        m_pRegMutex->unlock();

        m_configFrameCount++;
    }
    else if ((extraPara.drvinfo.regCount == 0) && (m_regCountList.empty()) && (m_allocatePA == true))
    {
        m_allocatePA = false;
        DpDriver::getInstance()->releasePABuffer(MAX_NUM_READBACK_REGS, m_PABuffer);
        m_configFrameCount = 0;
    }
    else if ((extraPara.drvinfo.regCount != 0) && (m_allocatePA == true))
    {
        m_pRegMutex->lock();
        m_regCountList.push_back(extraPara.drvinfo.regCount);
        m_pRegMutex->unlock();

        m_configFrameCount++;
    }
#endif

    m_curConfig.tpipeChange = !(hint & HINT_ISP_TPIPE_NO_CHANGE);
    if (m_curConfig.tpipeChange)
    {
        m_curConfig.frameChange = true;
        DPLOGI("DpIsp: ISP TPIPE changed\n");
    }

    if (hint & HINT_ISP_FRAME_MODE)
    {
        DPLOGI("DpIsp: ISP-only frame mode!\n");
        m_pStream->setConfigFlag(DpStream::CONFIG_ISP_FRAME_MODE);
    }

    return m_pChannel->setParameter(&extraPara,
                                    sizeof(ISP_TPIPE_CONFIG_STRUCT), m_PABuffer + offset);
}

DP_STATUS_ENUM DpIspStream::setParameter(ISP_TPIPE_CONFIG_STRUCT &extraPara, ISP2MDP_STRUCT *isp2mdp, uint32_t hint){
    DP_STATUS_ENUM status;
    status = setParameter(extraPara, hint);
    if (status != DP_STATUS_RETURN_SUCCESS)
        return status;
    return m_pChannel->setIsp2MdpParam(isp2mdp, sizeof(ISP2MDP_STRUCT));
}

DP_STATUS_ENUM DpIspStream::setPortMatrix(int32_t portIndex, DpColorMatrix *matrix)
{
    DP_TRACE_CALL();

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if (NULL == matrix || !matrix->enable)
    {
        if (NULL != m_pDstMatrix[portIndex])
        {
            delete m_pDstMatrix[portIndex];
            m_pDstMatrix[portIndex] = NULL;
            m_curConfig.frameChange = true;
        }
    }
    else if (NULL == m_pDstMatrix[portIndex])
    {
        m_pDstMatrix[portIndex] = new DpColorMatrix(*matrix);
        m_curConfig.frameChange = true;
    }
    else
    {
        *m_pDstMatrix[portIndex] = *matrix;
        //m_curConfig.frameChange = true; // no need
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::startStream(struct timeval *endTime)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff[10];
    int32_t        index;
    void           *pBase[3];
    uint32_t       size[3];
    int32_t        curSourceBuffer;
    int32_t        enableLog = DpDriver::getInstance()->getEnableLog();
    char           bufferInfoStr[256] = "";
    bool           enDRE = false;
    bool           dreReadback = false;
    uint32_t       dreSRAM = DpDREParam::DRESRAM::SRAMDefault;

#if 0  //code for ispstream callback, disabled temporally
    if (-1 == m_srcBuffer)
    {
        DPLOGE("DpIspStream: src buffer not ready\n");
        return DP_STATUS_INVALID_BUFFER;
    }
#endif
    memset(diff, 0, sizeof(diff));
    DPLOGI("DpIsp: start 0x%08x\n", this);
    DP_TIMER_GET_CURRENT_TIME(begin);
    if (!m_srcBufferStartTimeList.empty())
        DP_TIMER_GET_CURRENT_TIME(m_srcBufferStartTimeList.front());

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        if (false == m_curConfig.dstEnabled[index])
        {
            continue;
        }
        status = configPQParameter(index);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpIspStream: config PQ Parameter failed\n");
            return status;
        }
#ifndef BASIC_PACKAGE
        if (m_curConfig.pqParam[index].scenario == MEDIA_ISP_PREVIEW ||
            m_curConfig.pqParam[index].scenario == MEDIA_ISP_CAPTURE)
        {
            if (m_curConfig.PqConfig[index].enDRE != 0)
            {
                enDRE = enDRE || m_curConfig.PqConfig[index].enDRE;
                dreReadback = dreReadback ||
                    (m_curConfig.pqParam[index].u.isp.dpDREParam.cmd & DpDREParam::Cmd::Default) ||
                    (m_curConfig.pqParam[index].u.isp.dpDREParam.cmd & DpDREParam::Cmd::Generate);

                if (m_curConfig.pqParam[index].u.isp.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM00 ||
                    m_curConfig.pqParam[index].u.isp.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM01)
                {
                    dreSRAM = m_curConfig.pqParam[index].u.isp.dpDREParam.SRAMId;
                }
            }
        }
#endif // BASIC_PACKAGE
    }

    //PORt setup
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[0]);

#ifdef SUPPORT_DRE
    // Set DRE Readback
    if (dreSRAM != DpDREParam::DRESRAM::SRAM00 && dreSRAM != DpDREParam::DRESRAM::SRAM01)
    {
        dreReadback = false;
    }

    m_pStream->initNumReadbackPABuffer();

    m_pStream->setDREReadback(dreSRAM);
    //PQ DRE readback
    m_pDRERegMutex->lock();
    if (m_DREPABufferStartIndexList.empty())
    {
        m_DREPABufferStartIndexList.push_back(0);
    }
    else
    {
        m_DREPABufferStartIndexList.push_back((m_DREPABufferStartIndexList.back() + m_DREPABufferSizeList.back()) & (MAX_NUM_READBACK_PA_BUFFER - 1));
    }
    m_pStream->setReadbackPABuffer(m_DREPABuffer, m_DREPABufferStartIndexList.back());
    m_pDRERegMutex->unlock();
#endif // SUPPORT_DRE

    //dre readback m_pDRERegMutex
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[1]);

    DPLOGI("DpIspStream: frameChange = %d before detect\n", m_curConfig.frameChange);
    status = detectFrameChange(m_curConfig);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: detect frameChange failed\n");
        return status;
    }
    DPLOGI("DpIspStream: frameChange = %d after detect\n", m_curConfig.frameChange);

    m_pBufferMutex->lock();
    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        if (m_curConfig.dstEnabled[index] &&
            (((PORT_ISP != m_curConfig.dstPortType[index]) && (PORT_WPE != m_curConfig.dstPortType[index])) || !m_dstBufferList[index].empty()))
        {
            break;
        }
    }
    m_pBufferMutex->unlock();

    if (index == ISP_MAX_OUTPUT_PORT_NUM) // all ports are ISP
    {
        m_pStream->setConfigFlag(DpStream::CONFIG_BUFFER_IGNORE);
    }

#ifdef WPE_TILE
    if ((index == ISP_MAX_OUTPUT_PORT_NUM) &&
        !(DpStream::CONFIG_ISP_FRAME_MODE & m_pStream->getConfigFlags()) &&
        (STREAM_WPE == m_pStream->getScenario() ||
         STREAM_WPE2 == m_pStream->getScenario())) // all ports are ISP, Stream WPE, WPE only
    {
        m_pStream->setConfigFlag(DpStream::CONFIG_WPE_ONLY_TILE_MODE);
    }
#endif // WPE_TILE

    if (m_pStream->getScenario() == STREAM_ISP_VSS ||
        m_pStream->getScenario() == STREAM_ISP_ZSD_SLOW)
    {
        m_curConfig.frameChange = true;
    }
    if (m_curConfig.frameChange && !m_curConfig.tpipeChange && enableLog)
    {
        DPLOGD("DpIspStream: cannot skip tpipe because frame changed\n");
    }

    //before config
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[2]);

    status = m_pChannel->setEndTime(endTime);

    if (m_curConfig.frameChange)
    {
        status = m_pStream->resetStream();
        assert(DP_STATUS_RETURN_SUCCESS == status);

        status = m_pChannel->resetChannel();
        assert(DP_STATUS_RETURN_SUCCESS == status);

        if (DpStream::CONFIG_BUFFER_IGNORE & m_pStream->getConfigFlags())
        {
            // Add dummy source port
            status = m_pChannel->setSourcePort(PORT_MEMORY,
                                               DP_COLOR_UNKNOWN,
                                               0,
                                               0,
                                               0,
                                               0,
                                               false, // HDR
                                               false, // DRE
                                               NULL,
                                               DP_PROFILE_BT601,
                                               DP_SECURE_NONE,
                                               false);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpIspStream: set dummy source port failed\n");
                return status;
            }

            // Add dummy target port
            for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
            {
                if (false == m_curConfig.dstEnabled[index])
                {
                    continue;
                }

                switch ((PORT_TYPE_ENUM)m_curConfig.dstPortType[index])
                {
                case PORT_ISP:
                    status = m_pChannel->addTargetPort(index,
                                                       PORT_ISP,
                                                       DP_COLOR_UNKNOWN,
                                                       0,
                                                       0,
                                                       0,
                                                       0,
                                                       0,
                                                       false,
                                                       0,
                                                       0,
                                                       false,
                                                       NULL,
                                                       DP_PROFILE_BT601,
                                                       DP_SECURE_NONE,
                                                       false,
                                                       NULL,
                                                       NULL);
                    break;
                case PORT_WPE:
                    status = m_pChannel->addTargetPort(index,
                                                       PORT_WPE,
                                                       DP_COLOR_UNKNOWN,
                                                       0,
                                                       0,
                                                       0,
                                                       0,
                                                       0,
                                                       false,
                                                       0,
                                                       0,
                                                       false,
                                                       NULL,
                                                       DP_PROFILE_BT601,
                                                       DP_SECURE_NONE,
                                                       false,
                                                       NULL,
                                                       NULL);
                    break;
                default:
                    DPLOGE("DpIspStream: Port Type is not supported\n");
                    return DP_STATUS_INVALID_PORT;
                }

                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpIspStream: add dummy target port failed\n");
                    return status;
                }
            }
        }
        else
        {
            // Add source port
            status = m_pChannel->setSourcePort(PORT_MEMORY,
                                               m_curConfig.srcFormat,
                                               m_curConfig.srcWidth,
                                               m_curConfig.srcHeight,
                                               m_curConfig.srcYPitch,
                                               m_curConfig.srcUVPitch,
                                               false, // HDR
                                               enDRE, // DRE
                                               m_pSrcPool,
                                               m_curConfig.srcProfile,
                                               m_curConfig.srcSecure,
                                               (DP_SECURE_NONE != m_curConfig.srcSecure) ? false : m_curConfig.srcFlush);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpIspStream: set source port failed\n");
                return status;
            }

            if (enableLog)
            {
                memset(bufferInfoStr, '\0', sizeof(bufferInfoStr));
                m_pSrcPool->dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
                DPLOGD("DpIsp: in: (%d, %d, %d, %d, C%d%s%s%s%s, P%d) sec%d %s\n",
                       m_curConfig.srcWidth, m_curConfig.srcHeight,
                       m_curConfig.srcYPitch, m_curConfig.srcUVPitch,
                       DP_COLOR_GET_UNIQUE_ID(m_curConfig.srcFormat),
                       DP_COLOR_GET_SWAP_ENABLE(m_curConfig.srcFormat) ? "s" : "",
                       DP_COLOR_GET_BLOCK_MODE(m_curConfig.srcFormat) ? "b" : "",
                       DP_COLOR_GET_INTERLACED_MODE(m_curConfig.srcFormat) ? "i" : "",
                       DP_COLOR_GET_UFP_ENABLE(m_curConfig.srcFormat) ? "u" : "",
                       m_curConfig.srcProfile, m_curConfig.srcSecure ,bufferInfoStr);
            }

            m_pSrcPool->activateBuffer();

            for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
            {
                status = detectCropChange(index);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpIspStream: detect crop change failed\n");
                    return status;
                }

                if (false == m_curConfig.dstEnabled[index])
                {
                    continue;
                }

                // Error check for the target buffer
#if 0  //code for ispstream callback, disabled temporally
                if (-1 == m_dstBuffer[index])
                {
                    DPLOGE("DpIspStream: dst buffer not ready\n");
                    return DP_STATUS_INVALID_BUFFER;
                }
#endif
                //if (m_PqConfig[index].enDC || m_PqConfig[index].enSharp)
                //{
                //    m_sharpness[index] = m_PqConfig[index].enSharp;
                //}
                //m_pStream->setPQReadback(m_PqConfig[index].enDC);
                uint32_t tdshp = m_sharpness[index] ?
                    m_sharpness[index] :
                    (m_curConfig.PqConfig[index].enDC || m_curConfig.PqConfig[index].enSharp || m_curConfig.PqConfig[index].enColor);

                if (enableLog)
                {
                    memset(bufferInfoStr, '\0', sizeof(bufferInfoStr));
                    m_pDstPool[index]->dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
                    DPLOGD("DpIsp: out%d: (%d, %d, %d, %d, C%d%s, P%d), misc: (R:%d, F:%d, S:%d, A:%d D:%d, T:%d) sec%d %s\n",
                           index, m_curConfig.dstWidth[index], m_curConfig.dstHeight[index],
                           m_curConfig.dstYPitch[index], m_curConfig.dstUVPitch[index],
                           DP_COLOR_GET_UNIQUE_ID(m_curConfig.dstFormat[index]),
                           DP_COLOR_GET_SWAP_ENABLE(m_curConfig.dstFormat[index]) ? "s" : "",
                           m_curConfig.dstProfile[index], m_curConfig.rotation[index],
                           m_curConfig.flipStatus[index] ? 1 : 0, tdshp, enDRE,
                           m_curConfig.ditherStatus[index] ? 1 : 0,
                           m_curConfig.dstPortType[index], m_curConfig.dstSecure[index], bufferInfoStr);
                }

                if (m_curConfig.dstPlane[index] != DP_COLOR_GET_PLANE_COUNT(m_curConfig.dstFormat[index]))
                {
                    DPLOGE("DpIspStream: invalid color format: %d, %d\n", m_curConfig.dstPlane[index], DP_COLOR_GET_PLANE_COUNT(m_curConfig.dstFormat[index]));
                    assert(0);
                    return DP_STATUS_INVALID_FORMAT;
                }

                switch ((PORT_TYPE_ENUM)m_curConfig.dstPortType[index])
                {
                case PORT_MEMORY:
                case PORT_JPEG:
                case PORT_ISP:
                case PORT_WPE:
                case PORT_MDP2:
                    {
#ifndef BASIC_PACKAGE
                        uint64_t PqSessionID = ((static_cast<uint64_t>(m_PqID[index]) << 32) | m_curConfig.pqParam[index].scenario);
#else
                        uint64_t PqSessionID = (static_cast<uint64_t>(m_PqID[index]) << 32);
#endif // BASIC_PACKAGE
                        status = m_pChannel->addTargetPort(index,
                                            (PORT_TYPE_ENUM)m_curConfig.dstPortType[index],
                                            m_curConfig.dstFormat[index],
                                            m_curConfig.dstWidth[index],
                                            m_curConfig.dstHeight[index],
                                            m_curConfig.dstYPitch[index],
                                            m_curConfig.dstUVPitch[index],
                                            m_curConfig.rotation[index],
                                            m_curConfig.flipStatus[index],
                                            PqSessionID,
                                            tdshp,
                                            m_curConfig.ditherStatus[index],
                                            m_pDstPool[index],
                                            m_curConfig.dstProfile[index],
                                            m_curConfig.dstSecure[index],
                                            (DP_SECURE_NONE != m_curConfig.dstSecure[index]) ? false : m_curConfig.dstFlush[index],
                                            m_pDstMatrix[index],
                                            (PORT_JPEG == (PORT_TYPE_ENUM)m_curConfig.dstPortType[index]) ? &(m_curConfig.jpegEnc_cfg) : NULL);
                    }
                    break;
                default:
                    DPLOGE("DpIspStream: Port Type is not supported\n");
                    return DP_STATUS_INVALID_PORT;
                }

                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpIspStream: add target port failed\n");
                    return status;
                }

                if (true == m_curConfig.multiCropChange[index])
                {
                    if (enableLog)
                    {
                        DPLOGD("DpIsp: crop%d: (%d, %d, %d, %d, %d, %d, %d, %d)\n",
                            index, m_curConfig.multiSrcXStart[index], m_curConfig.multiSrcYStart[index], m_curConfig.multiCropWidth[index], m_curConfig.multiCropHeight[index], m_curConfig.multiSrcXSubpixel[index], m_curConfig.multiSrcYSubpixel[index], m_curConfig.multiCropWidthSubpixel[index], m_curConfig.multiCropHeightSubpixel[index]);
                    }

                    status = m_pChannel->setSourceCrop(index,
                                                       m_curConfig.multiSrcXStart[index],
                                                       m_curConfig.multiSrcXSubpixel[index],
                                                       m_curConfig.multiSrcYStart[index],
                                                       m_curConfig.multiSrcYSubpixel[index],
                                                       m_curConfig.multiCropWidth[index],
                                                       m_curConfig.multiCropWidthSubpixel[index],
                                                       m_curConfig.multiCropHeight[index],
                                                       m_curConfig.multiCropHeightSubpixel[index]);
                    if (DP_STATUS_RETURN_SUCCESS != status)
                    {
                        DPLOGE("DpIspStream: set source crop failed\n");
                        return status;
                    }
                }

                status = m_pChannel->setTargetROI(index,
                                                  0,
                                                  0,
                                                  m_curConfig.dstWidth[index],
                                                  m_curConfig.dstHeight[index]);

                m_pDstPool[index]->activateBuffer();
            }
        }

        status = m_pStream->addChannel(m_pChannel,
                                       &m_channelID);
        assert(DP_STATUS_RETURN_SUCCESS == status);
        //after config
        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin,
                                    end,
                                    diff[3]);
    }

    if (false == m_curConfig.frameChange)
    {
        DPLOGI("DpIsp: config frame only!\n");
        m_pStream->setConfigFlag(DpStream::CONFIG_FRAME_ONLY);
    }
    else
    {
        DPLOGI("DpIsp: config all!\n");
        m_pStream->setConfigFlag(DpStream::CONFIG_ALL);
    }

    //set config
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[4]);
    m_prevConfig = m_curConfig;
    m_pConfigMutex->lock();
    m_ispConfigList.push_back(m_curConfig);
    m_pConfigMutex->unlock();
    //m_pConfigMutex
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[5]);

    status = m_pStream->startStream(m_curConfig.frameChange);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: start ISP stream failed(%d)\n", status);
        m_curConfig.frameChange = true;
        return status;
    }
    //start stream
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[6]);

    m_curConfig.frameChange = false;
    DPLOGI("DpIspStream: startStream setframeChange to false\n");

    if (0 == (DpStream::CONFIG_BUFFER_IGNORE & m_pStream->getConfigFlags()))
    {
        status = m_pSrcPool->dequeueBuffer(&curSourceBuffer, pBase, size);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpIspStream: dequeue source buffer failed(%d)\n", status);
            return status;
        }

        // Queue and trigger the source buffer
        DPLOGI("DpIspStream:: enqueue source buffer: %d!!\n", curSourceBuffer);
        status = m_pSrcPool->queueBuffer(curSourceBuffer);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpIspStream: queue source buffer failed(%d)\n", status);
            return status;
        }
    }

    if(m_pStream->getSyncMode())
    {
        DPLOGI("DpIspStream: in synchronize mode, back to caller directly\n");

#ifdef SUPPORT_DRE
        //PQ DRE readback
        m_pDRERegMutex->lock();
        uint32_t bufferSize;
        uint32_t numReadbackEngine;
        bufferSize = m_pStream->getNumReadbackPABuffer(numReadbackEngine);
        m_DREPABufferSizeList.push_back(bufferSize);
        m_numReadbackEngineList.push_back(numReadbackEngine);
        m_pDRERegMutex->unlock();
#endif //SUPPORT_DRE

        return status;
    }
    //m_pDRERegMutex
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[7]);

    status = waitSubmit();

    //wait submit
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[8]);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: waitSubmit is failed in asynchronize mode (%d)\n", status);
        m_curConfig.frameChange = true;
        return status;
    }
    DPLOGI("DpIspStream: waitSubmit is done in asynchronize mode\n");
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[9]);
    if (diff[9] > 20)
    {
        DPLOGW("DpIsp: %p, scenario %d, startStream %dms, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
                this, m_pStream->getScenario(), diff[9], diff[0], diff[1], diff[2], diff[3], diff[4], diff[5],
                diff[6], diff[7], diff[8]);
    }
    else
    {
        DPLOGI("DpIsp: %p, scenario %d, startStream %dms, %d, %d, %d, %d, %d, %d, %d, %d, %d\n",
                this, m_pStream->getScenario(), diff[9], diff[0], diff[1], diff[2], diff[3], diff[4], diff[5],
                diff[6], diff[7], diff[8]);
    }

#ifdef MDP_VSS_ASYNC_ENABLE
        if(m_pStream->getScenario() == STREAM_ISP_VSS){
            m_pJobCond->signal();
        }
#endif

    return status;
}


DP_STATUS_ENUM DpIspStream::getJPEGFilesize(uint32_t* filesize)
{
    if (NULL == filesize)
    {
        DPLOGE("DpIspStream::getJPEGFilesize *filesize == NULL\n");
        return DP_STATUS_INVALID_PARAX;
    }

    *filesize = m_jpegEnc_filesize;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::waitComplete()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status = DP_STATUS_RETURN_SUCCESS;
    DpReadbackRegs readBackRegs;
    DpReadbackRegs DREReadBackRegs;
    DpJobID        _jobID = 0;
    const uint32_t *readBackValues = NULL;
    uint32_t       readBackNum = 0;
    int32_t        enableLog = DpDriver::getInstance()->getEnableLog();
    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff[5];
    uint32_t       index;
    ReadbackEngineList readbackEngineList;

    memset(diff, 0, sizeof(diff));
    DP_TIMER_GET_CURRENT_TIME(begin);
    if (m_pStream->getSyncMode())
    {
        DPLOGI("DpIspStream: Wait complete in synchronized mode\n");
        status = m_pStream->waitStream();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpIspStream: Can not complete in synchronized mode (%d)\n", status);
            m_curConfig.frameChange = true;
        }
        readBackValues = m_pStream->getReadbackValues(readBackNum);
        if (NULL != readBackValues)
        {
            m_jpegEnc_filesize = readBackValues[0];
        }
        else
        {
            DPLOGW("DpIspStream: getReadbackValues is NULL\n");
        }
        return status;
    }

    DPLOGI("DpIspStream: Wait complete in asynchronized mode: %x\n", this);

    memset(&readBackRegs, 0, sizeof(readBackRegs));
    memset(&DREReadBackRegs, 0, sizeof(DREReadBackRegs));

    m_pListMutex->lock();
    if (!m_frameDoneList.empty())
    {
        DpDoneJobItem doneJob = m_frameDoneList.front();
        uint32_t size = m_frameDoneList.size();
        m_pListMutex->unlock();
        _jobID = doneJob.jobId;

        if (_jobID)
        {
            if (m_pStream->getScenario() == STREAM_ISP_ZSD)
            {
                DPLOGW("DpIspStream %p: ZSD Job ID : %llx is in Framedone List (size:%d)!!!\n", this, _jobID, size);
            }
            else
            {
                DPLOGI("DpIspStream: Job ID : %llx is in Framedone List!!!\n", _jobID);
            }
            return DP_STATUS_RETURN_SUCCESS;
        }
        else
        {
            DPLOGE("DpIspStream: Invalid Job ID : %llx is in Framedone List!!!\n", _jobID);
            assert(0);
            return DP_STATUS_INVALID_PARAX;
        }
    }
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[0]);

    if (0 < m_jobIDsList.size())
    {
        m_pStream->getReadbackEngineStatus(readbackEngineList);

        _jobID = m_jobIDsList[0][0];
        m_pListMutex->unlock();

        m_pRegMutex->lock();
        m_pDRERegMutex->lock();
        if (m_regCountList.empty()
#ifdef SUPPORT_DRE
            && m_DREPABufferStartIndexList.empty()
#endif // SUPPORT_DRE
           )
        {
            m_pRegMutex->unlock();
            m_pDRERegMutex->unlock();

            readBackRegs.m_engineFlag = 0;

            status = DpDriver::getInstance()->waitFramedone(_jobID, readBackRegs);
            DP_TIMER_GET_CURRENT_TIME(end);
            DP_TIMER_GET_DURATION_IN_MS(begin,
                                        end,
                                        diff[2]);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpIspStream: wait complete stream failed(%d)\n", status);
                m_pStream->dumpDebugStream();
                m_curConfig.frameChange = true;
            }
            m_dequeueFrameCount = 0;
        }
        else if(!m_regCountList.empty())
        {
            m_pRegMutex->unlock();
            m_pDRERegMutex->unlock();
            uint32_t offset = (m_dequeueFrameCount & 0xF) << 2;

            readBackRegs.m_num = DP_MAX_SINGLE_PABUFFER_COUNT;
            memcpy(readBackRegs.m_regs, m_PABuffer + offset, sizeof(uint32_t) * DP_MAX_SINGLE_PABUFFER_COUNT);
            readBackRegs.m_engineFlag = (1 << tCAMIN);
            status = DpDriver::getInstance()->waitFramedone(_jobID, readBackRegs);
            DP_TIMER_GET_CURRENT_TIME(end);
            DP_TIMER_GET_DURATION_IN_MS(begin,
                                        end,
                                        diff[2]);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpIspStream: wait complete stream failed(%d)\n", status);
                m_pStream->dumpDebugStream();
                m_curConfig.frameChange = true;
            }

            m_pRegMutex->lock();
            for (index=0; index < m_regCountList.front(); index++)
            {
                m_regValueList.push_back(readBackRegs.m_values[index]);

                if (enableLog)
                {
                    DPLOGD("DpIspStream: regValue : %x \n", readBackRegs.m_values[index]);
                }
            }
            m_pRegMutex->unlock();
            m_dequeueFrameCount++;
        }
        else
        {
#ifdef SUPPORT_DRE
            m_pRegMutex->unlock();

            //PQ DRE readback
            if (!m_DREPABufferSizeList.empty())
            {
                DREReadBackRegs.m_num = m_DREPABufferSizeList.front();

                if (DREReadBackRegs.m_num > MAX_NUM_READBACK_REGS)
                {
                    DPLOGE("DpIspStream: wait complete stream readbeck fail\n");
                }
                else
                {
                    DREReadBackRegs.m_engineFlag = (1 << tAAL0);
                    for (index = 0; index < DREReadBackRegs.m_num; index++)
                    {
                        DREReadBackRegs.m_regs[index] = m_DREPABuffer[(m_DREPABufferStartIndexList.front() + index) & (MAX_NUM_READBACK_PA_BUFFER - 1)];
                    }
                }
            }
            else
            {
                DPLOGE("DpIspStream::waitComplete: m_DREPABufferStartIndexList and m_DREPABufferSizeList not match\n");
            }

            m_pDRERegMutex->unlock();

            DP_TIMER_GET_CURRENT_TIME(end);
            DP_TIMER_GET_DURATION_IN_MS(begin,
                                        end,
                                        diff[1]);
            status = DpDriver::getInstance()->waitFramedone(_jobID, DREReadBackRegs);
            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpIspStream: wait complete stream failed(%d)\n", status);
                m_pStream->dumpDebugStream();
                m_curConfig.frameChange = true;
            }
            DP_TIMER_GET_CURRENT_TIME(end);
            DP_TIMER_GET_DURATION_IN_MS(begin,
                                        end,
                                        diff[2]);

#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
            IspConfig ispConfig;
            m_pConfigMutex->lock();
            if (!m_ispConfigList.empty())
            {
                ispConfig = m_ispConfigList.front();
                m_pConfigMutex->unlock();

                PQDREHistogramAdaptor *pPQDREHistogramAdaptor = PQReadBackFactory::getInstance()->getDRE(0);
                int32_t portID;
                PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
                uint64_t PQSessionID;
                int32_t prevWidth, prevHeight;

                for (portID = 0; portID < ISP_MAX_OUTPUT_PORT_NUM; portID++)
                {
                    if (ispConfig.pqParam[portID].scenario == MEDIA_ISP_PREVIEW ||
                        ispConfig.pqParam[portID].scenario == MEDIA_ISP_CAPTURE)
                    {
                        if (pPQDREHistogramAdaptor != NULL &&
                            ispConfig.PqConfig[portID].enDRE != 0 &&
                            (ispConfig.pqParam[portID].u.isp.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM00 ||
                            ispConfig.pqParam[portID].u.isp.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM01))
                        {
#ifndef BASIC_PACKAGE
                            PQSessionID = (static_cast<uint64_t>(m_PqID[portID]) << 32) | ispConfig.pqParam[portID].scenario;
#else
                            PQSessionID = (static_cast<uint64_t>(m_PqID[portID]) << 32);
#endif // BASIC_PACKAGE

                            PQSession* pPQSession = pPQSessionManager->getPQSession(PQSessionID);

                            if (pPQSession != NULL)
                            {
                                pPQSession->getDrePreviousSize(&prevWidth, &prevHeight);

                                pPQDREHistogramAdaptor->setHistogram(
                                    DREReadBackRegs.m_values,
                                    DREReadBackRegs.m_num,
                                    ispConfig.pqParam[portID].u.isp.dpDREParam.buffer,
                                    ispConfig.pqParam[portID].u.isp.dpDREParam.userId,
                                    prevWidth,
                                    prevHeight);
                            }
                        }
                    }
                }
                DP_TIMER_GET_CURRENT_TIME(end);
                DP_TIMER_GET_DURATION_IN_MS(begin,
                                            end,
                                            diff[3]);

            }
            else
            {
                m_pConfigMutex->unlock();
                DPLOGE("DpIspStream::waitComplete set histogram fail, m_ispConfigList is empty\n");
            }
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID
#else
            m_pRegMutex->unlock();
            m_pDRERegMutex->unlock();
            DPLOGE("DpIspStream: wait complete stream failed; DRE is unsupported, but m_DREPABufferStartIndexList is not empty\n");
            assert(0);
            return DP_STATUS_INVALID_PARAX;
#endif // SUPPORT_DRE
        }
#ifdef SUPPORT_DRE
        m_pDRERegMutex->lock();
        if (!m_DREPABufferStartIndexList.empty())
        {
            m_DREPABufferStartIndexList.erase(m_DREPABufferStartIndexList.begin());
        }
        else
        {
            DPLOGE("DpIspStream::waitComplete m_DREPABufferStartIndexList empty\n");
        }

        if (!m_DREPABufferSizeList.empty())
        {
            m_DREPABufferSizeList.erase(m_DREPABufferSizeList.begin());
        }
        else
        {
            DPLOGE("DpIspStream::waitComplete m_DREPABufferSizeList empty\n");
        }

        if (!m_numReadbackEngineList.empty())
        {
            m_pStream->cleanReadbackEngineStatus(m_numReadbackEngineList.front());
            m_numReadbackEngineList.erase(m_numReadbackEngineList.begin());
        }
        else
        {
            DPLOGE("DpIspStream::waitComplete m_numReadbackEngineList empty\n");
        }
        m_pDRERegMutex->unlock();
#endif //SUPPORT_DRE
        DpDoneJobItem doneJob;
        doneJob.jobId = _jobID;
        doneJob.jobStatus = status;
        m_pListMutex->lock();
        m_frameDoneList.push_back(doneJob);
        m_jobIDsList.erase(m_jobIDsList.begin()); //Remove job set
        if (m_pStream->getScenario() == STREAM_ISP_ZSD)
        {
            if (1 < m_frameDoneList.size())
                DPLOGW("DpIspStream %p: ZSD Job ID : %llx is in JobID List (size:%d)\n", this, _jobID, m_frameDoneList.size());
            else
                DPLOGD("DpIspStream %p: ZSD Job ID : %llx is in JobID List (size:%d)\n", this, _jobID, m_frameDoneList.size());
        }
        else
        {
            DPLOGI("DpIspStream: Job ID : %x is in JobID List!!!\n", _jobID);
        }
        m_pListMutex->unlock();

        m_jpegEnc_filesize = readBackRegs.m_jpegEnc_filesize;

        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin,
                                    end,
                                    diff[4]);
        if (diff[4] > 34)
        {
            DPLOGW("DpIsp: %p, scenario %d, waitComplete %d ms, %d, %d, %d, %d\n"
                    , this, m_pStream->getScenario(), diff[4], diff[0], diff[1], diff[2], diff[3]);
        }
        else
        {
            DPLOGI("DpIsp: %p, scenario %d, waitComplete %d ms, %d, %d, %d, %d\n"
                    , this, m_pStream->getScenario(), diff[4], diff[0], diff[1], diff[2], diff[3]);
        }

        return status;
    }
    m_pListMutex->unlock();

    DPLOGE("DpIspStream: Both JobID and Framedone List are empty!!!\n");
    assert(0);
    return DP_STATUS_INVALID_PARAX;
}


DP_STATUS_ENUM DpIspStream::waitSubmit()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff[3];
    DpJobID        jobID = 0;
    int32_t        index;
    char           inBufferInfoStr[256] = "";
    char           outBufferInfoStr[ISP_MAX_OUTPUT_PORT_NUM][256] = {"", "", "", ""};

    DP_TIMER_GET_CURRENT_TIME(begin);

    status = m_pStream->waitStream();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: wait submit stream failed(%d)\n", status);
        m_pStream->dumpDebugStream();
        m_curConfig.frameChange = true;
        return status;
    }
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[0]);

#ifdef SUPPORT_DRE
    //PQ DRE readback
    m_pDRERegMutex->lock();
    uint32_t bufferSize;
    uint32_t numReadbackEngine;
    bufferSize = m_pStream->getNumReadbackPABuffer(numReadbackEngine);
    m_DREPABufferSizeList.push_back(bufferSize);
    m_numReadbackEngineList.push_back(numReadbackEngine);
    m_pDRERegMutex->unlock();
#endif //SUPPORT_DRE
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[1]);

#ifdef MDP_VSS_ASYNC_ENABLE
    if(m_pStream->getScenario() == STREAM_ISP_VSS)
    {
        m_pListMutex->lock();
        m_jobIDsList.push_back(m_pStream->getAsyncJobs());
    }
    else
#endif
    {
        JobIDList tmp_list;
        jobID = m_pStream->getAsyncJob();
        tmp_list.push_back(jobID);
        m_pListMutex->lock();
        m_jobIDsList.push_back(tmp_list);
        if (m_pStream->getScenario() == STREAM_ISP_ZSD)
        {
            memset(inBufferInfoStr, '\0', sizeof(inBufferInfoStr));
            m_pSrcPool->dumpBufferInfo(inBufferInfoStr, sizeof(inBufferInfoStr));
            for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
            {
                if (false == m_curConfig.dstEnabled[index])
                {
                    continue;
                }
                memset(outBufferInfoStr[index], '\0', sizeof(outBufferInfoStr[index]));
                m_pDstPool[index]->dumpBufferInfo(outBufferInfoStr[index], sizeof(outBufferInfoStr[index]));
            }
            if (1 < m_jobIDsList.size())
            {
                DPLOGW("DpIspStream %p: ZSD Job ID : %llx is submitted (queue:%d), in:%s, out0:%s, out1:%s, out2:%s, out3:%s\n",
                    this, jobID, m_jobIDsList.size(),
                    inBufferInfoStr, outBufferInfoStr[0], outBufferInfoStr[1], outBufferInfoStr[2], outBufferInfoStr[3]);
            }
            else
            {
                DPLOGD("DpIspStream %p: ZSD Job ID : %llx is submitted (queue:%d), in:%s, out0:%s, out1:%s, out2:%s, out3:%s\n",
                    this, jobID, m_jobIDsList.size(),
                    inBufferInfoStr, outBufferInfoStr[0], outBufferInfoStr[1], outBufferInfoStr[2], outBufferInfoStr[3]);
            }
        }
    }
    if (m_jobIDsList.size() > 10)
    {
        DPLOGE("DpIspStream: Job list size is more than 10 (%lu)\n", m_jobIDsList.size());
    }

    m_pListMutex->unlock();

    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[2]);

    if (diff[2] > 34)
    {
        DPLOGW("DpIsp: %p, scenario %d, waitSubmit %d ms, %d, %d\n"
                , this, m_pStream->getScenario(), diff[2], diff[0], diff[1]);
    }
    else
    {
        DPLOGI("DpIsp: %p, scenario %d, waitSubmit %d ms, %d, %d\n"
                , this, m_pStream->getScenario(), diff[2], diff[0], diff[1]);
    }
    DPLOGI("DpIspStream: submit stream done %llx\n", jobID);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::dequeueDstBuffer(int32_t portIndex,
                                             void    **pVABase,
                                             bool    waitBuf)
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status;
    uint32_t       size[3];
    int32_t        bufID;
    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff[10];
    int32_t        dstBuffer = -1;
    IspConfig      ispConfig;

    DPLOGI("==============dequeue dst buffer begin:============== %d, %p\n", waitBuf,  this);
    memset(diff, 0, sizeof(diff));
    DP_TIMER_GET_CURRENT_TIME(begin);

    //Could get ISP config here
    m_pConfigMutex->lock();
    if (!m_ispConfigList.empty())
    {
        ispConfig = m_ispConfigList.front();
    }
    m_pConfigMutex->unlock();

    //m_pConfigMutex
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[0]);
    #if 0
    //TODO: add m_sharpness
    uint32_t tdshp = (m_curConfig.PqConfig[portIndex].enDC
                   || m_curConfig.PqConfig[portIndex].enSharp
                   || m_curConfig.PqConfig[portIndex].enColor);
    DPLOGI("DpIsp: dequeueDst port%d: (%d, %d, %d, %d, C%d%s, P%d), misc: (R:%d, F:%d, S:%d, D:%d, T:%d)\n",
           portIndex, m_curConfig.dstWidth[portIndex], m_curConfig.dstHeight[portIndex],
           m_curConfig.dstYPitch[portIndex], m_curConfig.dstUVPitch[portIndex],
           DP_COLOR_GET_UNIQUE_ID(m_curConfig.dstFormat[portIndex]),
           DP_COLOR_GET_SWAP_ENABLE(m_curConfig.dstFormat[portIndex]) ? "s" : "",
           m_curConfig.dstProfile[portIndex], m_curConfig.rotation[portIndex],
           m_curConfig.flipStatus[portIndex] ? 1 : 0, tdshp,
           m_curConfig.ditherStatus[portIndex] ? 1 : 0,
           m_curConfig.dstPortType[portIndex]);
    #endif
    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    status = DP_STATUS_RETURN_SUCCESS;

    if (true == waitBuf)
    {

#ifdef MDP_VSS_ASYNC_ENABLE
        if(m_pStream->getScenario() == STREAM_ISP_VSS)
        {
            AutoMutex lock(m_pListMutex);
            if (!(m_frameDoneList.empty())) {
                DpDoneJobItem doneJob = m_frameDoneList.front();
                status = doneJob.jobStatus;
            }
            else{
                status = DP_STATUS_INVALID_PARAX;
            }
        }
        else
#endif
        {
            DPLOGI("DpIspStream: goto wait stream\n");
            status = waitComplete();
        }
        //m_pListMutex or waitComplete
        DP_TIMER_GET_CURRENT_TIME(end);
        DP_TIMER_GET_DURATION_IN_MS(begin,
                                    end,
                                    diff[1]);

        m_pBufferMutex->lock();
        if(!m_dstBufferList[portIndex].empty())
        {
            dstBuffer = m_dstBufferList[portIndex].front();
            m_dstBufferList[portIndex].erase(m_dstBufferList[portIndex].begin());
            m_pBufferMutex->unlock();
            //m_pBufferMutex
            DP_TIMER_GET_CURRENT_TIME(end);
            DP_TIMER_GET_DURATION_IN_MS(begin,
                                        end,
                                        diff[2]);
        }
        else
        {
            m_pBufferMutex->unlock();
            DPLOGE("DpIspStream: dequeueDstBuffer empty of port %d\n", portIndex);
            return DP_STATUS_INVALID_BUFFER;
        }

        if (-1 == dstBuffer)
        {
            DPLOGE("DpIspStream: empty dst buffer of port %d\n", portIndex);
            return DP_STATUS_INVALID_BUFFER;
        }
    }else
    {
        if(false == m_pStream->getSyncMode()) //Async mode!
        {
#ifdef MDP_VSS_ASYNC_ENABLE
            if(m_pStream->getScenario() == STREAM_ISP_VSS)
            {
                AutoMutex lock(m_pListMutex);
                if (!(m_frameDoneList.empty())){
                    DpDoneJobItem doneJob = m_frameDoneList.front();
                    //m_pListMutex
                    DP_TIMER_GET_CURRENT_TIME(end);
                    DP_TIMER_GET_DURATION_IN_MS(begin,
                                                end,
                                                diff[3]);
                    if (diff[3] > 10)
                    {
                        DPLOGW("DpIsp: %p, scenario %d, dst%d %d ms, %d, %d, %d\n"
                                , this, m_pStream->getScenario(), portIndex, diff[3], diff[0], diff[1], diff[2]);
                    }
                    else
                    {
                        DPLOGI("DpIsp: %p, scenario %d, dst%d %d ms, %d, %d, %d\n"
                                , this, m_pStream->getScenario(), portIndex, diff[3], diff[0], diff[1], diff[2]);
                    }
                    return doneJob.jobStatus;
                }
                else {
                    //m_pListMutex
                    DP_TIMER_GET_CURRENT_TIME(end);
                    DP_TIMER_GET_DURATION_IN_MS(begin,
                                                end,
                                                diff[3]);
                    if (diff[3] > 10)
                    {
                        DPLOGW("DpIsp: %p, scenario %d, dst%d %d ms, %d, %d, %d\n"
                                , this, m_pStream->getScenario(), portIndex, diff[3], diff[0], diff[1], diff[2]);
                    }
                    else
                    {
                        DPLOGI("DpIsp: %p, scenario %d, dst%d %d ms, %d, %d, %d\n"
                                , this, m_pStream->getScenario(), portIndex, diff[3], diff[0], diff[1], diff[2]);
                    }

                    if(ispConfig.callback != NULL)
                    {
                        DPLOGE("VSS callback mode should not be here!");
                    }
                    return DP_STATUS_BUFFER_EMPTY;
                }
            }
#endif
            DPLOGE("DpIspStream: NOT supporting to nonblocking dequeueDstBuffer in async mode %d\n", portIndex);
            assert(0);
            return DP_STATUS_INVALID_PARAX;
        }
    }

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: wait stream failed(%d)\n", status);
#ifdef MDP_VSS_ASYNC_ENABLE
        if(m_pStream->getScenario() != STREAM_ISP_VSS)
#endif
        m_pStream->stopStream();

        return status;
    }
#if 1
    if(false == m_pStream->getSyncMode()) //Async mode!
    {
        if(true == m_curConfig.dstFlush[portIndex])
        {
            status = m_pDstPool[portIndex]->flushWriteBuffer(dstBuffer);
            DPLOGI("DpIspStream:Port[%d] flushWriteBuffer %d status: %d \n", portIndex, dstBuffer, status);
        }

        status = m_pDstPool[portIndex]->queueBuffer(dstBuffer);
        DPLOGI("DpIspStream:Port[%d] queueBuffer %d status: %d \n", portIndex, dstBuffer, status);
    }
#endif

    status = m_pDstPool[portIndex]->acquireBuffer(&bufID,
                                                  pVABase,
                                                  size,
                                                  waitBuf);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        if (true == waitBuf)
        {
            DPLOGE("DpIspStream: acquire dst buffer failed(%d)\n", status);
        }

        return status;
    }

    //before get read back
    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[3]);
    if( (false == waitBuf) && (true == m_pStream->getSyncMode()))
    {
        //Sync Mode
        const uint32_t *readBackValues = NULL;
        uint32_t       readBackNum = 0;
        readBackValues = m_pStream->getReadbackValues(readBackNum);

        m_jpegEnc_filesize = (NULL != readBackValues) ? readBackValues[0] : 0;
        DPLOGI("DpIspStream: JPEG size: %d in %p\n", m_jpegEnc_filesize, readBackValues);
    }

    if (false == m_pStream->getSyncMode()) //Async mode!
    {
        status = m_pDstPool[portIndex]->asyncReleaseBuffer(bufID);
        DPLOGI("DpIspStream: dequeueDstBuffer releaseBuffer %d status: %d \n", bufID, status);
    }
    else
    {
        m_pDstPool[portIndex]->releaseBuffer(bufID);
    }

    m_pDstPool[portIndex]->unregisterBuffer(bufID);

    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff[4]);

    if (diff[4] > 34)
    {
        DPLOGW("DpIsp: %p, scenario %d, dst%d %d ms, %d, %d, %d, %d\n"
                , this, m_pStream->getScenario(), portIndex, diff[4], diff[0], diff[1], diff[2], diff[3]);
    }
    else
    {
        DPLOGI("DpIsp: %p, scenario %d, dst%d %d ms, %d, %d, %d, %d\n"
                , this, m_pStream->getScenario(), portIndex, diff[4], diff[0], diff[1], diff[2], diff[3]);
    }

    DPLOGI("==============dequeue dst buffer end:==============%p\n", this);
    return status;
}


DP_STATUS_ENUM DpIspStream::dequeueFrameEnd(uint32_t *pValueList)
{
    DP_TRACE_CALL();
    DpDoneJobItem doneJob;
    DP_STATUS_ENUM status;
    uint32_t       index;

    status = waitComplete();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpIspStream: wait stream failed(%d)\n", status);
        m_pStream->stopStream();
    }

    if(true == m_pStream->getSyncMode()) //Sync mode!
    {
        DPLOGI("bypass dequeueFrameEnd in Sync mode\n");
        return DP_STATUS_RETURN_SUCCESS;
    }

    DPLOGI("==============dequeue frame end:============== %p\n", this);
    m_pListMutex->lock();
    if (m_frameDoneList.empty())
    {
        m_pListMutex->unlock();
        DPLOGE("============== dequeue frame end: Failed due to empty framedone List ==============\n");
        return DP_STATUS_INVALID_STATE;
    }
    doneJob = m_frameDoneList.front();
    if (m_pStream->getScenario() == STREAM_ISP_ZSD)
    {
        if (1 < m_frameDoneList.size())
            DPLOGW("DpIspStream %p: ZSD Job ID : %llx dequeue end (size:%d)\n", this, doneJob.jobId, m_frameDoneList.size());
        else
            DPLOGD("DpIspStream %p: ZSD Job ID : %llx dequeue end (size:%d)\n", this, doneJob.jobId, m_frameDoneList.size());
    }
    m_frameDoneList.erase(m_frameDoneList.begin());
    m_pListMutex->unlock();

    if(false == m_pStream->getSyncMode())
    {
        m_pConfigMutex->lock();
        if (!m_ispConfigList.empty())
        {
            m_ispConfigList.erase(m_ispConfigList.begin());
        }
        m_pConfigMutex->unlock();
    }

    m_jpegEnc_filesize = 0;
    DPLOGI("FRAME: %X  - dequeue end\n", doneJob.jobId);

    if (pValueList != NULL)
    {
        m_pRegMutex->lock();
        if (m_regCountList.empty())
        {
            m_pRegMutex->unlock();
            DPLOGE("============== dequeue frame end: Failed due to empty register count List ==============\n");
            return DP_STATUS_INVALID_STATE;
        }

        for (index = 0; index < m_regCountList.front(); index++)
        {
            pValueList[index] = m_regValueList[index];
            DPLOGI("DpIspStream: index = %d regValue : %x \n", index, pValueList[index]);
        }
        m_regValueList.erase(m_regValueList.begin(), m_regValueList.begin() + index);
        m_regCountList.erase(m_regCountList.begin());
        m_pRegMutex->unlock();
    }
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::stopStream()
{
    DP_TRACE_CALL();
    DP_STATUS_ENUM status = DP_STATUS_RETURN_SUCCESS;
    int32_t        index;

    DPLOGI("=================stop stream begin:=================\n");

    DPLOGI("DpIsp: stop 0x%08x\n", this);

    m_pStream->stopStream();

    m_pSrcPool->activateBuffer();

    if(true == m_pStream->getSyncMode())
    {
        m_pConfigMutex->lock();
        if (!m_ispConfigList.empty())
        {
            m_ispConfigList.erase(m_ispConfigList.begin());
        }
        m_pConfigMutex->unlock();
    }

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        if (true == m_curConfig.dstEnabled[index])
        {
            m_pDstPool[index]->activateBuffer();
        }

        m_curConfig.dstEnabled[index] = false;
        m_curConfig.dstBuffer[index] = -1;
    }

    DPLOGI("=================stop stream end:=================\n");

    return status;
}

#ifndef BASIC_PACKAGE
uint32_t DpIspStream::getPqID()
{
    AutoMutex lock(s_PqCountMutex);
    s_PqCount = (s_PqCount+1) & 0xFFFFFFF;

    DPLOGI("DpIspStream::s_PqCount %x\n", s_PqCount);

    return (s_PqCount | DP_ISPSTREAM);
}
#endif // BASIC_PACKAGE

DP_STATUS_ENUM DpIspStream::setPQParameter(int32_t portIndex, const DpPqParam &pqParam)
{
    DP_TRACE_CALL();
    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpIspStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {
        if (pqParam.scenario != MEDIA_ISP_PREVIEW &&
            pqParam.scenario != MEDIA_ISP_CAPTURE)
        {
            DPLOGE("DpIspStream: setPQParameter scenario %d\n", pqParam.scenario);
            return DP_STATUS_INVALID_PARAX;
        }

        m_curConfig.pqParam[portIndex] = pqParam;
#ifdef SUPPORT_DRE
        // Select DRE SRAM by scenario
        if (pqParam.scenario == MEDIA_ISP_PREVIEW)
        {
            m_curConfig.pqParam[portIndex].u.isp.dpDREParam.SRAMId = DpDREParam::DRESRAM::SRAM00;
        }
        else if (pqParam.scenario == MEDIA_ISP_CAPTURE)
        {
            m_curConfig.pqParam[portIndex].u.isp.dpDREParam.SRAMId = DpDREParam::DRESRAM::SRAM01;
        }
#endif // SUPPORT_DRE
    }
#endif // BASIC_PACKAGE
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpIspStream::setPQParameter(int32_t portIndex, const DpPqParam *pqParam)
{
    DpPqParam localDpPqParam;
    localDpPqParam.enable = false;
    localDpPqParam.scenario = MEDIA_ISP_PREVIEW;
    if (pqParam == NULL)
    {
        DPLOGI("DpIspStream: setPQParameter pqParam is NULL, set scenario to MEDIA_ISP_PREVIEW\n");
        pqParam = &localDpPqParam;
    }
    return setPQParameter(portIndex, *pqParam);
}

DP_STATUS_ENUM DpIspStream::detectFrameChange(IspConfig &config)
{
    bool diff;
    int32_t index;

    diff = (memcmp(&m_prevConfig, &config, sizeof(BaseConfig)) != 0);
    config.frameChange = config.frameChange || diff;
    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        if (config.dstEnabled[index] &&
            (m_prevConfig.dstBuffer[index] == -1 && config.dstBuffer[index] != -1))
        {
            config.frameChange = true;
            break;
        }
    }

    if (DpDriver::getInstance()->getDisableFrameChange())
    {
        config.frameChange = true;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::detectCropChange(int32_t portIndex)
{
    if ((m_curConfig.multiSrcXStart[portIndex]    != m_prevConfig.multiSrcXStart[portIndex]) ||
        (m_curConfig.multiSrcXSubpixel[portIndex] != m_prevConfig.multiSrcXSubpixel[portIndex]) ||
        (m_curConfig.multiSrcYStart[portIndex]    != m_prevConfig.multiSrcYStart[portIndex]) ||
        (m_curConfig.multiSrcYSubpixel[portIndex] != m_prevConfig.multiSrcYSubpixel[portIndex]) ||
        (m_curConfig.multiCropWidth[portIndex]    != m_prevConfig.multiCropWidth[portIndex]) ||
        (m_curConfig.multiCropWidthSubpixel[portIndex]    != m_prevConfig.multiCropWidthSubpixel[portIndex]) ||
        (m_curConfig.multiCropHeight[portIndex]   != m_prevConfig.multiCropHeight[portIndex]) ||
        (m_curConfig.multiCropHeightSubpixel[portIndex]   != m_prevConfig.multiCropHeightSubpixel[portIndex]))
    {
        m_curConfig.multiCropChange[portIndex] = true;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpIspStream::configPQParameter(int32_t portIndex)
{
#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {

#if CONFIG_FOR_OS_ANDROID
        uint64_t PQSessionID = ((static_cast<uint64_t>(m_PqID[portIndex]) << 32) | m_curConfig.pqParam[portIndex].scenario);

        PQSession* pPQsession = PQSessionManager::getInstance()->createPQSession(PQSessionID);
        pPQsession->setPQparam(&(m_curConfig.pqParam[portIndex]));

        DpPqConfig* pDpPqConfig;
        pPQsession->getDpPqConfig(&pDpPqConfig);
        m_curConfig.PqConfig[portIndex] = *pDpPqConfig;

        DPLOGI("DpIspStream: pPQsession id %llx created\n", PQSessionID);
        DPLOGI("DpIspStream: setPQParameter id %x enable %d scenario %d\n",
               m_PqID[portIndex], m_curConfig.pqParam[portIndex].enable, m_curConfig.pqParam[portIndex].scenario);
        DPLOGI("DpIspStream: getPQConfig sharp %d DC %d color %d\n",
               m_curConfig.PqConfig[portIndex].enSharp, m_curConfig.PqConfig[portIndex].enDC, m_curConfig.PqConfig[portIndex].enColor);
#endif // CONFIG_FOR_OS_ANDROID
    }
#endif // BASIC_PACKAGE
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpIspStream::setCallback(void (*callback)(void *), void * data)
{
    DPLOGI("DpIspStream::setCallback %p, callback %p, data %p", this, callback, data);
    m_curConfig.callback = callback;
    m_curConfig.callbackData = data;
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpIspStream::initPQService()
{
#ifndef BASIC_PACKAGE
    PQConfig* pPQConfig = PQConfig::getInstance();

    if (pPQConfig->getPQServiceStatus() != PQSERVICE_READY)
    {
        DPLOGI("DpIspStream::initPQService getPQServiceStatus() Bypassed ...\n");
    }
#endif // BASIC_PACKAGE
    return DP_STATUS_RETURN_SUCCESS;
}
