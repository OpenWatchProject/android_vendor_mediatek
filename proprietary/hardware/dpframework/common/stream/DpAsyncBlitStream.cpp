#include "DpAsyncBlitStream.h"
#include "DpStream.h"
#include "DpChannel.h"
#include "DpBufferPool.h"
#include "DpSync.h"
#include "DpPlatform.h"
#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
#include "PQSessionManager.h"
#include "PQReadBackFactory.h"
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID
#if PMQOS_SETTING
#include "mmdvfs.h"
#endif


#define CONFIG_CURRENT_JOB(pJob, pMutex)                           \
do {                                                               \
    AutoMutex lock(pMutex);                                        \
    if (pJob == NULL || pJob->state != JOB_CONFIGING)              \
    {                                                              \
        DPLOGE("%s called before setConfigBegin\n", __FUNCTION__); \
        return DP_STATUS_INVALID_STATE;                            \
    }                                                              \
} while (0)

static uint32_t s_PqCount = 0;
static DpMutex  s_PqCountMutex;

static uint32_t s_jobCount = 0;
static DpMutex  s_jobCountMutex;

bool DpAsyncBlitStream::queryHWSupport(uint32_t         srcWidth,
                                       uint32_t         srcHeight,
                                       uint32_t         dstWidth,
                                       uint32_t         dstHeight,
                                       int32_t          Orientation,
                                       DpColorFormat    srcFormat,
                                       DpColorFormat    dstFormat,
                                       DpPqParam        *PqParam,
                                       DpRect           *srcCrop)
{
    uint32_t tmpDstWidth  = dstWidth;
    uint32_t tmpDstHeight = dstHeight;

    //contain rotation 90 and 270
    if ((DpOrientation)Orientation & ROT_90)
    {
        dstWidth  = tmpDstHeight;
        dstHeight = tmpDstWidth;
    }

    // Temporarily
    if ((srcWidth / dstWidth) > 20)
    {
        DPLOGE("DpAsyncBlitStream:: exceed HW limitation, srcWidth %d, dstWidth %d\n", srcWidth, dstWidth);
        return false;
    }

    if ((srcHeight / dstHeight) > 255)
    {
        DPLOGE("DpAsyncBlitStream:: exceed HW limitation, srcHeight %d, dstHeight %d\n", srcHeight, dstHeight);
        return false;
    }

    // Temporarily
    if ((dstWidth / srcWidth) > 32)
    {
        DPLOGE("DpAsyncBlitStream:: exceed HW limitation, dstWidth %d, srcWidth %d\n", dstWidth, srcWidth);
        return false;
    }

    if ((dstHeight / srcHeight) > 32)
    {
        DPLOGE("DpAsyncBlitStream:: exceed HW limitation, dstHeight %d, srcHeight %d\n", dstHeight, srcHeight);
        return false;
    }

    if (DP_COLOR_GET_H_SUBSAMPLE(srcFormat) && (srcWidth & 0x1))
    {
        DPLOGE("DpAsyncBlitStream:: invalid src width alignment\n");
        return false;
    }

    if (DP_COLOR_GET_V_SUBSAMPLE(srcFormat) && (srcHeight & 0x1))
    {
        DPLOGE("DpAsyncBlitStream:: invalid src height alignment\n");
        return false;
    }

    if (DP_COLOR_GET_H_SUBSAMPLE(dstFormat) && (dstWidth & 0x1))
    {
        DPLOGE("DpAsyncBlitStream:: invalid dst width alignment\n");
        return false;
    }

    if (DP_COLOR_GET_V_SUBSAMPLE(dstFormat) && (dstHeight & 0x1))
    {
        DPLOGE("DpAsyncBlitStream:: invalid dst height alignment\n");
        return false;
    }

    if ((srcWidth == dstWidth) && (srcHeight == dstHeight) &&
        (DP_COLOR_GET_HW_FORMAT(srcFormat) == 2 || DP_COLOR_GET_HW_FORMAT(srcFormat) == 3) &&
        (DP_COLOR_GET_HW_FORMAT(dstFormat) == 2 || DP_COLOR_GET_HW_FORMAT(dstFormat) == 3))
    {
        if (!DMA_SUPPORT_ALPHA_ROT)
        {
            DPLOGE("DpAsyncBlitStream:: unsupport alpha rotation on this platform\n");
            return false;
        }
        else
        {
            if (srcWidth < 9)
            {
                DPLOGE("DpAsyncBlitStream:: exceed HW limitation, srcWidth %d < 9\n", srcWidth);
                return false;
            }
        }
    }

    if (NULL != srcCrop && NULL != PqParam)
    {
#ifdef SUPPORT_VIDEO_DRE
        if (srcCrop->w < 48 &&
            MEDIA_VIDEO == PqParam->scenario &&
	        1 == PqParam->enable)
        {
            return false;
        }
#else
        DP_UNUSED(PqParam);
        DP_UNUSED(srcCrop);
#endif
    }

    return true;
}

DP_STATUS_ENUM DpAsyncBlitStream::queryTheoreticalExecTime(uint32_t         srcWidth,
                                       uint32_t         srcHeight,
                                       uint32_t         dstWidth,
                                       uint32_t         dstHeight,
                                       std::vector<uint32_t >* pExecTimeList)
{
#if PMQOS_SETTING
    uint32_t       index;
    uint32_t       srcPixelCount;
    uint32_t       dstPixelCount;
    uint32_t       pixelCount;
    int            mdp_freq[] = MMDVFS_MDP_FREQ;

    srcPixelCount = srcWidth * srcHeight;
    dstPixelCount = dstWidth * dstHeight;

    if (srcPixelCount > dstPixelCount)
    {
        pixelCount = srcPixelCount;
    }
    else
    {
        pixelCount = dstPixelCount;
    }

    for (index = 0; index < (sizeof(mdp_freq) / sizeof(*(mdp_freq))); index++)
    {
        pExecTimeList->push_back(pixelCount / (mdp_freq[index] * 1000));
        DPLOGD("DpAsyncBlitStream: index = %d ExecTime : %d ms\n", index, (pixelCount / (mdp_freq[index] * 1000)));
    }
#endif

    return DP_STATUS_RETURN_SUCCESS;
}

DpAsyncBlitStream::BaseConfig::BaseConfig()
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

        cropXStart[index] = -1;
        cropYStart[index] = -1;
        cropWidth[index] = -1;
        cropHeight[index] = -1;
        cropSubPixelX[index] = 0;
        cropSubPixelY[index] = 0;
        cropSubPixelW[index] = 0;
        cropSubPixelH[index] = 0;
        cropEnabled[index] = false;

        targetXStart[index] = -1;
        targetYStart[index] = -1;
        roiWidth[index] = -1;
        roiHeight[index] = -1;

        rotation[index] = 0;
        flipStatus[index] = false;
        ditherStatus[index] = false;
    }

    memset(&PqConfig, 0, sizeof(PqConfig));

}


DpAsyncBlitStream::BlitConfig::BlitConfig()
    : frameChange(false),
      srcBuffer(-1)
{
    BaseConfig();
    int32_t index;

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        dstBuffer[index] = -1;
    }

    memset(&PqParam, 0, sizeof(PqParam));

}


DpAsyncBlitStream::DpAsyncBlitStream()
    : m_pStream(new DpStream(STREAM_BITBLT)),
      m_pChannel(new DpChannel()),
      m_channelID(-1),
      m_pSrcPool(new DpBasicBufferPool()),
#if 0
      m_pPqStream(NULL),
      m_pPqChannel(NULL),
      m_pPqPool(NULL),
#endif
      m_userID(DP_BLIT_GENERAL_USER),
      m_pqSupport(0),
      m_pJobMutex(new DpMutex()),
      m_pJobCond(new DpCondition()),
      m_timeValue(0),
      m_abortJobs(false),
      m_pCurJob(NULL),
#if CONFIG_FOR_OS_ANDROID
      m_thread(0),
#endif // CONFIG_FOR_OS_ANDROID
      m_pSync(new DpSync()),
      m_regMutex(new DpMutex())
{
    int32_t index;

    m_pStream->setSyncMode(false);

#ifndef BASIC_PACKAGE
    m_pqSupport = DpDriver::getInstance()->getPQSupport();
#endif

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        m_pDstPool[index]     = new DpBasicBufferPool();

#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
        if (0 != m_pqSupport) {
            m_PqID[index] = getPqID();
        }
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID

    }

    createThread();

    memset(m_PABuffer, 0, sizeof(m_PABuffer));

    m_PABufferSizeList.clear();
    m_PABufferStartIndexList.clear();
    m_numReadbackEngineList.clear();
    DpDriver::getInstance()->allocatePABuffer(MAX_NUM_READBACK_PA_BUFFER, m_PABuffer);
}


DpAsyncBlitStream::~DpAsyncBlitStream()
{
    DPLOGI("DpAsyncBlitStream::destruct DpAsyncBlitStream object begin\n");
    int32_t index;

    {
        AutoMutex lock(m_pJobMutex);
        m_abortJobs = true;
        m_pJobCond->signal();
    }
    pthread_join(m_thread, NULL);

    m_jobList.clear();
    m_newJobList.clear();

    m_PABufferSizeList.clear();
    m_PABufferStartIndexList.clear();
    m_numReadbackEngineList.clear();

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        delete m_pDstPool[index];
        m_pDstPool[index] = NULL;

#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
        if (0 != m_pqSupport) {
            PQSessionManager::getInstance()->destroyPQSession(m_PqID[index]);
        }
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID
    }

    DpDriver::getInstance()->releasePABuffer(MAX_NUM_READBACK_PA_BUFFER, m_PABuffer);

    delete m_pSrcPool;
    m_pSrcPool = NULL;

    delete m_pStream;
    m_pStream = NULL;

    delete m_pChannel;
    m_pChannel = NULL;

    delete m_pJobMutex;
    m_pJobMutex = NULL;

    delete m_pJobCond;
    m_pJobCond = NULL;

    delete m_pSync;
    m_pSync = NULL;

    delete m_regMutex;
    m_regMutex = NULL;

    DPLOGI("DpAsyncBlitStream::destruct DpAsyncBlitStream object end\n");
}


void DpAsyncBlitStream::createThread()
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

    if (pthread_create(&m_thread, &attribute, waitComplete, this))
    {
        pthread_create(&m_thread, NULL, waitComplete, this);
    }

    DP_TIMER_GET_CURRENT_TIME(end);
    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff);
    if (diff > 10)
    {
        DPLOGW("DpAsyncBlitStream: create thread (%s) %d ms\n",
            ((SCHED_RR == schedPolicy) || (SCHED_FIFO == schedPolicy)) ? "real-time" : "normal", diff);
    }

    pthread_attr_destroy(&attribute);
}


uint32_t DpAsyncBlitStream::getJobID()
{
    AutoMutex lock(s_jobCountMutex);

    s_jobCount++;
    if (s_jobCount == 0)
    {
        s_jobCount = 1;
    }

    return s_jobCount;
}


DP_STATUS_ENUM DpAsyncBlitStream::createJob(uint32_t &jobID, int32_t &fence)
{
    AsyncBlitJob *pJob;

    pJob = new AsyncBlitJob;
    if (pJob == NULL)
    {
        DPLOGE("DpAsyncBlitStream: cannot allocate job\n");
        return DP_STATUS_OUT_OF_MEMORY;
    }

    pJob->jobID = getJobID();
    pJob->timeValue = pJob->jobID;

    //DpSync::createInstance()->createFence(fence, m_pCurJob->timeValue);
    m_pSync->createFence(pJob->fenceFD, pJob->timeValue);
    if (pJob->fenceFD < 0)
    {
        delete pJob;
        return DP_STATUS_OPERATION_FAILED;
    }

    pJob->cmdTaskID = 0;
    pJob->state = JOB_CREATE;

    jobID = pJob->jobID;
    fence = pJob->fenceFD;

    memset(pJob->dumpFrameInfo, 0x0, sizeof(pJob->dumpFrameInfo));

    AutoMutex lock(m_pJobMutex);
    m_newJobList.push_back(pJob);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::cancelJob(uint32_t jobID)
{
    AutoMutex lock(m_pJobMutex);

    JobList::iterator iterator;
    uint32_t          timeValue;

    if (jobID)
    {
        for (iterator = m_newJobList.begin(); iterator != m_newJobList.end(); iterator++)
        {
            if (jobID == (*iterator)->jobID)
            {
                break;
            }
        }

        if (iterator == m_newJobList.end())
        {
            DPLOGE("DpAsyncBlitStream: cannot find job with ID %d\n", jobID);
            return DP_STATUS_INVALID_PARAX;
        }

        timeValue = (*iterator)->timeValue;
        iterator++;
    }
    else
    {
        timeValue = m_newJobList.back()->timeValue;
        iterator = m_newJobList.end();
    }

    for (JobList::iterator it = m_newJobList.begin(); it != iterator; it++)
    {
        //close((*it)->fenceFd); // closed by caller
        delete *it;
    }
    m_newJobList.erase(m_newJobList.begin(), iterator);

    if (m_pCurJob)
    {
        m_pCurJob->timeValue = timeValue;
    }
    else if (!m_jobList.empty())
    {
        m_jobList.back()->timeValue = timeValue;
    }
    else if (!m_waitJobList.empty())
    {
        m_waitJobList.back()->timeValue = timeValue;
    }
    else
    {
        // wake up directly
        m_pSync->wakeup((uint32_t)((int32_t)timeValue - (int32_t)m_timeValue));
        m_timeValue = timeValue;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::setConfigBegin(uint32_t jobID)
{
    int32_t index;

    {
        AutoMutex lock(m_pJobMutex);

        AsyncBlitJob *pJob = NULL;
        JobList::iterator iterator;

        for (iterator = m_newJobList.begin(); iterator != m_newJobList.end(); iterator++)
        {
            if (jobID == (*iterator)->jobID)
            {
                pJob = *iterator;
                break;
            }
        }

        if (pJob == NULL)
        {
            DPLOGE("DpAsyncBlitStream: cannot find job with ID %d\n", jobID);
            return DP_STATUS_INVALID_PARAX;
        }

        for (JobList::iterator it = m_newJobList.begin(); it != iterator; it++)
        {
            //close((*it)->fenceFd); // closed by caller
            delete *it;
        }
        m_newJobList.erase(m_newJobList.begin(), ++iterator);

        if (m_pCurJob)
        {
            //close(m_pCurJob->fenceFd); // closed by caller
            delete m_pCurJob;
        }

        m_pCurJob = pJob;
        m_pCurJob->state = JOB_CONFIGING;
    }

    // clone previous config as current job config
    //m_pCurJob->config = m_prevConfig;

    // reset job flag
    m_pCurJob->config.frameChange = false;
    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        m_pCurJob->config.dstEnabled[index] = 0;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::setSrcBuffer(void     *pVABase,
                                               uint32_t size,
                                               int32_t  fenceFd)
{
    DP_STATUS_ENUM status;
    int32_t        bufferID;

    DPLOGI("DpAsyncBlitStream: register source buffer with VA begin\n");

    status = m_pSrcPool->registerBuffer(&pVABase,
                                        &size,
                                        1,
                                        fenceFd,
                                        &bufferID);

    DPLOGI("DpAsyncBlitStream: register source buffer with VA end(%d)", status);

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.srcBuffer = bufferID;

    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream::setSrcBuffer(void     **pVABaseList,
                                               uint32_t *pSizeList,
                                               uint32_t planeNumber,
                                               int32_t  fenceFd)
{
    DP_STATUS_ENUM status;
    int32_t        bufferID;

    DPLOGI("DpAsyncBlitStream: register source buffer with VA begin\n");

    status = m_pSrcPool->registerBuffer(pVABaseList,
                                        pSizeList,
                                        planeNumber,
                                        fenceFd,
                                        &bufferID);

    DPLOGI("DpAsyncBlitStream: register source buffer with VA end(%d)", status);

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.srcBuffer = bufferID;

    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream::setSrcBuffer(void**   pVABaseList,
                                               void**   pMVABaseList,
                                               uint32_t *pSizeList,
                                               uint32_t planeNumber,
                                               int32_t  fenceFd)
{
    DP_STATUS_ENUM status;
    int32_t        bufferID;

    DPLOGI("DpAsyncBlitStream: register source buffer with MVA begin\n");

    status = m_pSrcPool->registerBuffer(pVABaseList,
                                        (uint32_t*)pMVABaseList,
                                        pSizeList,
                                        planeNumber,
                                        fenceFd,
                                        &bufferID);

    DPLOGI("DpAsyncBlitStream: register source buffer with MVA end(%d)", status);

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.srcBuffer = bufferID;

    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream::setSrcBuffer(int32_t  fileDesc,
                                               uint32_t *pSizeList,
                                               uint32_t planeNumber,
                                               int32_t  fenceFd)
{
    DP_STATUS_ENUM status;
    int32_t        bufferID;

    DPLOGI("DpAsyncBlitStream: register source buffer with FD begin\n");

    status = m_pSrcPool->registerBufferFD(fileDesc,
                                          pSizeList,
                                          planeNumber,
                                          fenceFd,
                                          &bufferID);

    DPLOGI("DpAsyncBlitStream: register source buffer with FD end(%d)", status);

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.srcBuffer = bufferID;

    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream::setSrcConfig(int32_t           width,
                                               int32_t           height,
                                               DpColorFormat     format,
                                               DpInterlaceFormat)
{
    if ((width <= 0) || (height <= 0))
    {
        DPLOGE("DpAsyncBlitStream: invalid source width(%d), height(%d)\n", width, height);
        return DP_STATUS_INVALID_PARAX;
    }

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.srcWidth   = width;
    m_pCurJob->config.srcHeight  = height;
    m_pCurJob->config.srcFormat  = format;
    m_pCurJob->config.srcYPitch  = DP_COLOR_GET_MIN_Y_PITCH(format, width);
    m_pCurJob->config.srcUVPitch = DP_COLOR_GET_MIN_UV_PITCH(format, width);
    m_pCurJob->config.srcProfile = DP_PROFILE_BT601;
    m_pCurJob->config.srcFlush   = true;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::setSrcConfig(int32_t           width,
                                               int32_t           height,
                                               int32_t           YPitch,
                                               int32_t           UVPitch,
                                               DpColorFormat     format,
                                               DP_PROFILE_ENUM   profile,
                                               DpInterlaceFormat,
                                               DpSecure          secure,
                                               bool              doFlush)
{
    if ((width <= 0) || (height <= 0) || (YPitch <= 0))
    {
        DPLOGE("DpAsyncBlitStream: invalid source width(%d), height(%d), Ypitch(%d)\n", width, height, YPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (YPitch < DP_COLOR_GET_MIN_Y_PITCH(format, width))
    {
        DPLOGE("DpAsyncBlitStream: source Y pitch(%d) is less than min Y pitch(%d) for width(%d)\n", YPitch, DP_COLOR_GET_MIN_Y_PITCH(format, width), width);
        return DP_STATUS_INVALID_PARAX;
    }

    if (DP_COLOR_GET_PLANE_COUNT(format) > 1)
    {
        if (UVPitch < DP_COLOR_GET_MIN_UV_PITCH(format, width))
        {
            DPLOGE("DpAsyncBlitStream: source UV pitch(%d) is less than min UV pitch(%d) for width(%d)\n", UVPitch, DP_COLOR_GET_MIN_UV_PITCH(format, width), width);
            return DP_STATUS_INVALID_PARAX;
        }
    }

    if (DP_STATUS_RETURN_SUCCESS != m_pSrcPool->setSecureMode(secure))
    {
        return DP_STATUS_UNKNOWN_ERROR;
    }

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.srcWidth   = width;
    m_pCurJob->config.srcHeight  = height;
    m_pCurJob->config.srcFormat  = format;
    m_pCurJob->config.srcYPitch  = YPitch;
    m_pCurJob->config.srcUVPitch = UVPitch;
    m_pCurJob->config.srcProfile = profile;
    m_pCurJob->config.srcSecure  = secure;
    m_pCurJob->config.srcFlush   = doFlush;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::setSrcCrop(int32_t portIndex, DpRect  roi)
{
    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.cropXStart[portIndex]    = roi.x;
    m_pCurJob->config.cropYStart[portIndex]    = roi.y;
    m_pCurJob->config.cropWidth[portIndex]     = roi.w;
    m_pCurJob->config.cropHeight[portIndex]    = roi.h;
    m_pCurJob->config.cropSubPixelX[portIndex] = roi.sub_x;
    m_pCurJob->config.cropSubPixelY[portIndex] = roi.sub_y;
    m_pCurJob->config.cropSubPixelW[portIndex] = roi.sub_w;
    m_pCurJob->config.cropSubPixelH[portIndex] = roi.sub_h;
    m_pCurJob->config.cropEnabled[portIndex]   = true;

    return DP_STATUS_RETURN_SUCCESS;
}



DP_STATUS_ENUM DpAsyncBlitStream::setDstBuffer(int32_t  portIndex,
                                               void     *pVABase,
                                               uint32_t size,
                                               int32_t  fenceFd)
{
    DP_STATUS_ENUM status;
    int32_t        bufferID;

    DPLOGI("DpAsyncBlitStream: register target buffer with VA begin\n");

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    status = m_pDstPool[portIndex]->registerBuffer(&pVABase,
                                                   &size,
                                                   1,
                                                   fenceFd,
                                                   &bufferID);

    DPLOGI("DpAsyncBlitStream: register target buffer with VA end(%d)\n", status);

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.dstBuffer[portIndex] = bufferID;

    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream::setDstBuffer(int32_t  portIndex,
                                               void     **pVABaseList,
                                               uint32_t *pSizeList,
                                               uint32_t planeNumber,
                                               int32_t  fenceFd)
{
    DP_STATUS_ENUM status;
    int32_t        bufferID;

    DPLOGI("DpAsyncBlitStream: register target buffer with VA begin\n");

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    status = m_pDstPool[portIndex]->registerBuffer(pVABaseList,
                                                   pSizeList,
                                                   planeNumber,
                                                   fenceFd,
                                                   &bufferID);

    DPLOGI("DpAsyncBlitStream: register target buffer with VA end(%d)\n", status);

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.dstBuffer[portIndex] = bufferID;

    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream::setDstBuffer(int32_t  portIndex,
                                               void**   pVABaseList,
                                               void**   pMVABaseList,
                                               uint32_t *pSizeList,
                                               uint32_t planeNumber,
                                               int32_t  fenceFd)
{
    DP_STATUS_ENUM status;
    int32_t        bufferID;

    DPLOGI("DpAsyncBlitStream: register target buffer with MVA begin\n");

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    status = m_pDstPool[portIndex]->registerBuffer(pVABaseList,
                                                   (uint32_t*)pMVABaseList,
                                                   pSizeList,
                                                   planeNumber,
                                                   fenceFd,
                                                   &bufferID);

    DPLOGI("DpAsyncBlitStream: register target buffer with MVA end(%d)\n", status);

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.dstBuffer[portIndex] = bufferID;

    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream::setDstBuffer(int32_t  portIndex,
                                               int32_t  fileDesc,
                                               uint32_t *pSizeList,
                                               uint32_t planeNumber,
                                               int32_t  fenceFd)
{
    DP_STATUS_ENUM status;
    int32_t        bufferID;

    DPLOGI("DpAsyncBlitStream: register target buffer with FD(%d) begin", fileDesc);

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    status = m_pDstPool[portIndex]->registerBufferFD(fileDesc,
                                                     pSizeList,
                                                     planeNumber,
                                                     fenceFd,
                                                     &bufferID);

    DPLOGI("DpAsyncBlitStream: register target buffer with FD end(%d)", status);

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.dstBuffer[portIndex] = bufferID;

    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream::setDstConfig(int32_t           portIndex,
                                               int32_t           width,
                                               int32_t           height,
                                               DpColorFormat     format,
                                               DpInterlaceFormat,
                                               DpRect            *pROI)
{
    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if ((width <= 0) || (height <= 0))
    {
        DPLOGE("DpAsyncBlitStream: invalid target width(%d), height(%d)\n", width, height);
        return DP_STATUS_INVALID_PARAX;
    }

    if (NULL != pROI)
    {
        // roi_width must equal to width or width -1
        if (!(pROI->w == width) && !(pROI->w == width - 1))
        {
            DPLOGE("invalid width ROI setting. width = %d, roi_width = %d\n", width, pROI->w);
            return DP_STATUS_INVALID_PARAX;
        }
        // roi_height must equal to height or height -1
        if (!(pROI->h == height) && !(pROI->h == height - 1))
        {
            DPLOGE("invalid height ROI setting. height = %d, roi_height = %d\n", height, pROI->h);
            return DP_STATUS_INVALID_PARAX;
        }

        // ROI offset must be aligned
        if (DP_COLOR_GET_H_SUBSAMPLE(format) && (pROI->x & 0x1))
        {
            DPLOGE("invalid ROI x offset alignment\n");
            return DP_STATUS_INVALID_X_ALIGN;
        }
        if (DP_COLOR_GET_V_SUBSAMPLE(format) && (pROI->y & 0x1))
        {
            DPLOGE("invalid ROI y offset alignment\n");
            return DP_STATUS_INVALID_Y_ALIGN;
        }
    }

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    if (NULL != pROI)
    {
        m_pCurJob->config.targetXStart[portIndex] = pROI->x;
        m_pCurJob->config.targetYStart[portIndex] = pROI->y;
        m_pCurJob->config.roiWidth[portIndex]     = pROI->w;
        m_pCurJob->config.roiHeight[portIndex]    = pROI->h;
    }
    else
    {
        m_pCurJob->config.targetXStart[portIndex] = 0;
        m_pCurJob->config.targetYStart[portIndex] = 0;
        m_pCurJob->config.roiWidth[portIndex]     = width;
        m_pCurJob->config.roiHeight[portIndex]    = height;
    }

    m_pCurJob->config.dstWidth[portIndex]     = width;
    m_pCurJob->config.dstHeight[portIndex]    = height;

    m_pCurJob->config.dstYPitch[portIndex]  = DP_COLOR_GET_MIN_Y_PITCH(format,  width);
    m_pCurJob->config.dstUVPitch[portIndex] = DP_COLOR_GET_MIN_UV_PITCH(format, width);
    m_pCurJob->config.dstFormat[portIndex]  = format;
    m_pCurJob->config.dstProfile[portIndex] = DP_PROFILE_BT601;
    m_pCurJob->config.dstFlush[portIndex]   = true;
    m_pCurJob->config.dstEnabled[portIndex] = true;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::setDstConfig(int32_t           portIndex,
                                               int32_t           width,
                                               int32_t           height,
                                               int32_t           YPitch,
                                               int32_t           UVPitch,
                                               DpColorFormat     format,
                                               DP_PROFILE_ENUM   profile,
                                               DpInterlaceFormat,
                                               DpRect            *pROI,
                                               DpSecure          secure,
                                               bool              doFlush)
{
    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if ((width <= 0) || (height <= 0) || (YPitch <= 0))
    {
        DPLOGE("DpAsyncBlitStream: invalid target width(%d), height(%d), YPitch(%d)\n", width, height, YPitch);
        return DP_STATUS_INVALID_PARAX;
    }

    if (YPitch < DP_COLOR_GET_MIN_Y_PITCH(format, width))
    {
        DPLOGE("DpAsyncBlitStream: target Y pitch(%d) is less than min Y pitch(%d) for width(%d)\n", YPitch, DP_COLOR_GET_MIN_Y_PITCH(format, width), width);
        return DP_STATUS_INVALID_PARAX;
    }

    if (DP_COLOR_GET_PLANE_COUNT(format) > 1)
    {
        if (UVPitch < DP_COLOR_GET_MIN_UV_PITCH(format, width))
        {
            DPLOGE("DpAsyncBlitStream: target UV pitch(%d) is less than min UV pitch(%d) for width(%d)\n", UVPitch, DP_COLOR_GET_MIN_UV_PITCH(format, width), width);
            return DP_STATUS_INVALID_PARAX;
        }
    }

    if (NULL != pROI)
    {
        // roi_width must equal to width or width -1
        if (!(pROI->w == width) && !(pROI->w == width - 1))
        {
            DPLOGE("invalid width ROI setting. width = %d, roi_width = %d\n", width, pROI->w);
            return DP_STATUS_INVALID_PARAX;
        }
        // roi_height must equal to height or height -1
        if (!(pROI->h == height) && !(pROI->h == height - 1))
        {
            DPLOGE("invalid height ROI setting. height = %d, roi_height = %d\n", height, pROI->h);
            return DP_STATUS_INVALID_PARAX;
        }

        // ROI offset must be aligned
        if (DP_COLOR_GET_H_SUBSAMPLE(format) && (pROI->x & 0x1))
        {
            DPLOGE("invalid ROI x offset alignment\n");
            return DP_STATUS_INVALID_X_ALIGN;
        }
        if (DP_COLOR_GET_V_SUBSAMPLE(format) && (pROI->y & 0x1))
        {
            DPLOGE("invalid ROI y offset alignment\n");
            return DP_STATUS_INVALID_Y_ALIGN;
        }
    }

    if (DP_STATUS_RETURN_SUCCESS != m_pDstPool[portIndex]->setSecureMode(secure))
    {
        return DP_STATUS_UNKNOWN_ERROR;
    }

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    if (NULL != pROI)
    {
        m_pCurJob->config.targetXStart[portIndex] = pROI->x;
        m_pCurJob->config.targetYStart[portIndex] = pROI->y;
        m_pCurJob->config.roiWidth[portIndex]     = pROI->w;
        m_pCurJob->config.roiHeight[portIndex]    = pROI->h;
    }
    else
    {
        m_pCurJob->config.targetXStart[portIndex] = 0;
        m_pCurJob->config.targetYStart[portIndex] = 0;
        m_pCurJob->config.roiWidth[portIndex]     = width;
        m_pCurJob->config.roiHeight[portIndex]    = height;
    }

    m_pCurJob->config.dstWidth[portIndex]     = width;
    m_pCurJob->config.dstHeight[portIndex]    = height;

    m_pCurJob->config.dstYPitch[portIndex]  = YPitch;
    m_pCurJob->config.dstUVPitch[portIndex] = UVPitch;
    m_pCurJob->config.dstFormat[portIndex]  = format;
    m_pCurJob->config.dstProfile[portIndex] = profile;
    m_pCurJob->config.dstFlush[portIndex]   = doFlush;
    m_pCurJob->config.dstSecure[portIndex]  = secure;
    m_pCurJob->config.dstEnabled[portIndex] = true;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::setDstROI(int32_t      portIndex,
                                            int32_t      width,
                                            int32_t      height,
                                            const DpRect &roi)
{
    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    if ((width <= 0) || (height <= 0))
    {
        DPLOGE("DpAsyncBlitStream: invalid target width(%d), height(%d)\n", width, height);
        return DP_STATUS_INVALID_PARAX;
    }

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    if (!m_pCurJob->config.dstEnabled[portIndex])
    {
        DPLOGE("setDstROI called before setDstConfig\n");
        return DP_STATUS_INVALID_STATE;
    }

    // roi_width must equal to width or width -1
    if (!(roi.w == width) && !(roi.w == width - 1))
    {
        DPLOGE("invalid width ROI setting. width = %d, roi_width = %d\n", width, roi.w);
        return DP_STATUS_INVALID_PARAX;
    }
    // roi_height must equal to height or height -1
    if (!(roi.h == height) && !(roi.h == height - 1))
    {
        DPLOGE("invalid height ROI setting. height = %d, roi_height = %d\n", height, roi.h);
        return DP_STATUS_INVALID_PARAX;
    }

    // ROI offset must be aligned
    if (DP_COLOR_GET_H_SUBSAMPLE(m_pCurJob->config.dstFormat[portIndex]) && (roi.x & 0x1))
    {
        DPLOGE("invalid ROI x offset alignment\n");
        return DP_STATUS_INVALID_X_ALIGN;
    }
    if (DP_COLOR_GET_V_SUBSAMPLE(m_pCurJob->config.dstFormat[portIndex]) && (roi.y & 0x1))
    {
        DPLOGE("invalid ROI y offset alignment\n");
        return DP_STATUS_INVALID_Y_ALIGN;
    }

    m_pCurJob->config.targetXStart[portIndex] = roi.x;
    m_pCurJob->config.targetYStart[portIndex] = roi.y;
    m_pCurJob->config.roiWidth[portIndex]     = roi.w;
    m_pCurJob->config.roiHeight[portIndex]    = roi.h;

    m_pCurJob->config.dstWidth[portIndex]     = width;
    m_pCurJob->config.dstHeight[portIndex]    = height;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::setConfigEnd()
{

    AutoMutex lock(m_pJobMutex);

    if (m_pCurJob == NULL)
    {
        DPLOGE("setConfigEnd called before setConfigBegin\n");
        return DP_STATUS_INVALID_STATE;
    }

    m_pCurJob->state = JOB_CONFIG_DONE;


    // keep previous job config same as last job pusked back into joblist
    //m_prevConfig = m_pCurJob->config;

    // current job done
    m_jobList.push_back(m_pCurJob);
    m_pCurJob = NULL;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::setRotate(int32_t portIndex, int32_t rotation)
{
    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.rotation[portIndex] = rotation;

    return DP_STATUS_RETURN_SUCCESS;
}

//Compatible to 89
DP_STATUS_ENUM DpAsyncBlitStream::setFlip(int32_t portIndex, int flip)
{
    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.flipStatus[portIndex] = flip ? true : false;

    return DP_STATUS_RETURN_SUCCESS;
}

// Compatible to 6589
DP_STATUS_ENUM DpAsyncBlitStream::setOrientation(int32_t portIndex, uint32_t transform)
{
    uint32_t flip = 0;
    uint32_t rot  = 0;

    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    // operate on FLIP_H, FLIP_V and ROT_90 respectively
    // to achieve the final orientation
    if (FLIP_H & transform)
    {
        flip ^= 1;
    }

    if (FLIP_V & transform)
    {
        // FLIP_V is equivalent to a 180-degree rotation with a horizontal flip
        rot += 180;
        flip ^= 1;
    }

    if (ROT_90 & transform)
    {
        rot += 90;
    }

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.flipStatus[portIndex] = (0 != flip) ? true: false;
    m_pCurJob->config.rotation[portIndex]   = rot %= 360;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::setDither(int32_t portIndex, bool enDither)
{
    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

    CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);

    m_pCurJob->config.ditherStatus[portIndex] = enDither;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::invalidate(struct timeval *endTime)
{
    DP_STATUS_ENUM status;
    void           *pBase[3];
    uint32_t       size[3];
    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff;
    AsyncBlitJob   *pJob;
    int32_t        enableLog = DpDriver::getInstance()->getEnableLog();
    int32_t        enableDumpBuffer = DpDriver::getInstance()->getEnableDumpBuffer();
    int32_t        index;
    uint32_t       tdshp[ISP_MAX_OUTPUT_PORT_NUM];
    bool           enHDR = false;
    char           bufferInfoStr[256] = "";
    bool           enDRE = false;
    bool           dreReadback = false;
    uint32_t       dreSRAM = DpDREParam::DRESRAM::SRAMDefault;

    DP_TRACE_CALL();
    DP_TIMER_GET_CURRENT_TIME(begin);

    {
        AutoMutex lock(m_pJobMutex);

        if (m_jobList.empty())
        {
            DPLOGE("DpAsyncBlitStream: no job to invalidate!\n");
            return DP_STATUS_INVALID_STATE;
        }

        pJob = m_jobList.front();
        if (pJob->state != JOB_CONFIG_DONE)
        {
            DPLOGE("DpAsyncBlitStream: cannot invalidate job in state %d!\n", pJob->state);
            return DP_STATUS_INVALID_STATE;
        }

        pJob->state = JOB_INVALIDATE;
    }

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        // TODO: readback PQ in asynchronous mode
        //if (pJob->config.PqConfig.enDC || pJob->config.PqConfig.enSharp)
        //{
        //    pJob->config.Tdshp = pJob->config.PqConfig.enSharp;
        //}
        if (!pJob->config.dstEnabled[index])
            continue;
        status = configPQParameter(index, pJob->config);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpAsyncBlitStream: config PQ Parameter failed %d\n", status);
            return status;
        }
        m_pStream->setPQReadback(pJob->config.PqConfig[index].enDC);
        m_pStream->setHDRReadback(pJob->config.PqConfig[index].enHDR);
        tdshp[index] = (pJob->config.PqConfig[index].enDC || pJob->config.PqConfig[index].enSharp || \
                        pJob->config.PqConfig[index].enColor);
        //enHDR is decided by source port, and has no difference between different output ports.
        enHDR = enHDR || pJob->config.PqConfig[index].enHDR;
#if defined(HDR_MT6779)
        enHDR = enHDR || pJob->config.PqConfig[index].enCcorr;
#endif

        //TODO: video.id should not affect frameChange
        if (m_prevConfig.PqParam[index].u.video.id != pJob->config.PqParam[index].u.video.id)
        {
            pJob->config.frameChange = true;
        }

#ifndef BASIC_PACKAGE
        if (pJob->config.PqParam[index].scenario == MEDIA_VIDEO)
        {
            if (pJob->config.PqConfig[index].enDRE != 0)
            {
                enDRE = enDRE || pJob->config.PqConfig[index].enDRE;
                dreReadback = dreReadback ||
                    (pJob->config.PqParam[index].u.video.dpDREParam.cmd & DpDREParam::Cmd::Default) ||
                    (pJob->config.PqParam[index].u.video.dpDREParam.cmd & DpDREParam::Cmd::Generate);

                if (pJob->config.PqParam[index].u.video.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM00 ||
                    pJob->config.PqParam[index].u.video.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM01)
                {
                    dreSRAM = pJob->config.PqParam[index].u.video.dpDREParam.SRAMId;
                }
            }
        }
#endif // BASIC_PACKAGE
    }

    m_pStream->initNumReadbackPABuffer();

    //PQ DC or HDR readback
    m_regMutex->lock();
    if (m_PABufferStartIndexList.empty())
    {
        m_PABufferStartIndexList.push_back(0);
    }
    else
    {
        m_PABufferStartIndexList.push_back((m_PABufferStartIndexList.back() + m_PABufferSizeList.back()) & (MAX_NUM_READBACK_PA_BUFFER - 1));
    }
    m_pStream->setReadbackPABuffer(m_PABuffer, m_PABufferStartIndexList.back());
    m_regMutex->unlock();

#ifdef SUPPORT_VIDEO_DRE
    // Set DRE Readback
    if (dreSRAM != DpDREParam::DRESRAM::SRAM00 && dreSRAM != DpDREParam::DRESRAM::SRAM01)
    {
        dreReadback = false;
    }

    m_pStream->setDREReadback(dreSRAM);
#endif // SUPPORT_VIDEO_DRE

    DPLOGI("DpAsyncBlitStream: frameChange = %d before detect\n", pJob->config.frameChange);
    status = detectFrameChange(pJob->config);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpAsyncBlitStream: detect FrameChange failed %d\n", status);
        return status;
    }
    DPLOGI("DpAsyncBlitStream: frameChange = %d after detect\n", pJob->config.frameChange);

    status = m_pChannel->setEndTime(endTime);

    if (pJob->config.frameChange)
    {
        status = m_pStream->resetStream();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpAsyncBlitStream: reset stream object failed %d\n", status);
            return status;
        }

        status = m_pChannel->resetChannel();
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpAsyncBlitStream: reset stream channel failed %d\n", status);
            return status;
        }

        status = m_pChannel->setSourcePort(PORT_MEMORY,
                                           pJob->config.srcFormat,
                                           pJob->config.srcWidth,
                                           pJob->config.srcHeight,
                                           pJob->config.srcYPitch,
                                           pJob->config.srcUVPitch,
                                           enHDR,
                                           enDRE,
                                           m_pSrcPool,
                                           pJob->config.srcProfile,
                                           pJob->config.srcSecure,
                                           (DP_SECURE_NONE != pJob->config.srcSecure)? false : pJob->config.srcFlush);
        if (DP_STATUS_RETURN_SUCCESS != status)
        {
            DPLOGE("DpAsyncBlitStream: set source port failed %d\n", status);
            return status;
        }

        m_pSrcPool->activateBuffer();

        if (enableLog)
        {
            memset(bufferInfoStr, '\0', sizeof(bufferInfoStr));
            m_pSrcPool->dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
            DPLOGD("DpAsyncBlit: in: (%d, %d, %d, %d, C%d%s%s%s%s%s%s, P%d), sec%d %s\n",
                pJob->config.srcWidth, pJob->config.srcHeight,
                pJob->config.srcYPitch, pJob->config.srcUVPitch,
                DP_COLOR_GET_UNIQUE_ID(pJob->config.srcFormat),
                DP_COLOR_GET_SWAP_ENABLE(pJob->config.srcFormat) ? "s" : "",
                DP_COLOR_GET_BLOCK_MODE(pJob->config.srcFormat) ? "b" : "",
                DP_COLOR_GET_INTERLACED_MODE(pJob->config.srcFormat) ? "i" : "",
                DP_COLOR_GET_UFP_ENABLE(pJob->config.srcFormat) ? "u" : "",
                DP_COLOR_GET_10BIT_TILE_MODE(pJob->config.srcFormat) ? "t" :
                DP_COLOR_GET_10BIT_PACKED(pJob->config.srcFormat) ? "p" :
                DP_COLOR_GET_10BIT_LOOSE(pJob->config.srcFormat) ? "l" : "",
                DP_COLOR_GET_10BIT_JUMP_MODE(pJob->config.srcFormat) ? "j" : "",
                pJob->config.srcProfile, pJob->config.srcSecure,
                bufferInfoStr);
        }

        for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
        {
            if (!pJob->config.dstEnabled[index])
                continue;

#ifdef BASIC_PACKAGE
            uint32_t videoID = pJob->config.PqParam[index].u.video.id;
#else
            PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
            uint32_t videoID = pPQSessionManager->findVideoID(pJob->config.PqParam[index].u.video.id);
#endif // BASIC_PACKAGE
            uint64_t PQSessionID = (static_cast<uint64_t>(m_PqID[index]) << 32) | videoID;

            status = m_pChannel->addTargetPort(index,
                                   PORT_MEMORY,
                                   pJob->config.dstFormat[index],
                                   pJob->config.dstWidth[index],
                                   pJob->config.dstHeight[index],
                                   pJob->config.dstYPitch[index],
                                   pJob->config.dstUVPitch[index],
                                   pJob->config.rotation[index],
                                   pJob->config.flipStatus[index]? true: false,
                                   PQSessionID,
                                   tdshp[index],
                                   pJob->config.ditherStatus[index]? true: false,
                                   m_pDstPool[index],
                                   pJob->config.dstProfile[index],
                                   pJob->config.dstSecure[index],
                                   (DP_SECURE_NONE != pJob->config.dstSecure[index])? false: \
                                   pJob->config.dstFlush[index]);


            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpAsyncBlitStream: index %d add target port failed %d\n", index, status);
                return status;
            }

#if 0
            if (0 != (tdshp & 0xFFFF0000))
            {
                m_pStream->setPQReadback(true);
            }
#endif

            if (!pJob->config.cropEnabled[index])
            {
                pJob->config.cropXStart[index] = 0;
                pJob->config.cropSubPixelX[index] = 0;
                pJob->config.cropYStart[index] = 0;
                pJob->config.cropSubPixelY[index] = 0;
                pJob->config.cropWidth[index] = pJob->config.srcWidth;
                pJob->config.cropSubPixelW[index] = 0;
                pJob->config.cropHeight[index] = pJob->config.srcHeight;
                pJob->config.cropSubPixelH[index] = 0;
            }

            status = m_pChannel->setSourceCrop(index,
                                               pJob->config.cropXStart[index],
                                               pJob->config.cropSubPixelX[index],
                                               pJob->config.cropYStart[index],
                                               pJob->config.cropSubPixelY[index],
                                               pJob->config.cropWidth[index],
                                               pJob->config.cropSubPixelW[index],
                                               pJob->config.cropHeight[index],
                                               pJob->config.cropSubPixelH[index],
                                               true);

            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpAsyncBlitStream: index %d setSourceCrop failed %d\n", index, status);
                return status;
            }

            status = m_pChannel->setTargetROI(index,
                                              pJob->config.targetXStart[index],
                                              pJob->config.targetYStart[index],
                                              pJob->config.roiWidth[index],
                                              pJob->config.roiHeight[index]);

            if (DP_STATUS_RETURN_SUCCESS != status)
            {
                DPLOGE("DpAsyncBlitStream: index %d setTargetROI failed %d\n", index, status);
                return status;
            }

            m_pDstPool[index]->activateBuffer();

            if (enableLog)
            {
                //if (pJob->config.cropEnabled[index])
                {
                    DPLOGD("DpAsyncBlit: crop%d: (%d, %d, %d, %d, %d, %d, %d, %d)\n",
                        index, pJob->config.cropXStart[index], pJob->config.cropYStart[index],
                        pJob->config.cropWidth[index], pJob->config.cropHeight[index],
                        pJob->config.cropSubPixelX[index], pJob->config.cropSubPixelY[index],
                        pJob->config.cropSubPixelW[index], pJob->config.cropSubPixelH[index]);
                }

                memset(bufferInfoStr, '\0', sizeof(bufferInfoStr));
                m_pDstPool[index]->dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
                DPLOGD("DpAsyncBlit: out%d: (%d, %d, %d, %d, C%d%s%s%s%s, P%d), misc: (X:%d, Y:%d, R:%d, F:%d, S:%d, A:%d, D:%d), sec%d %s\n",
                    index, pJob->config.dstWidth[index], pJob->config.dstHeight[index],
                    pJob->config.dstYPitch[index], pJob->config.dstUVPitch[index],
                    DP_COLOR_GET_UNIQUE_ID(pJob->config.dstFormat[index]),
                    DP_COLOR_GET_SWAP_ENABLE(pJob->config.dstFormat[index]) ? "s" : "",
                    DP_COLOR_GET_BLOCK_MODE(pJob->config.dstFormat[index]) ? "b" : "",
                    DP_COLOR_GET_INTERLACED_MODE(pJob->config.dstFormat[index]) ? "i" : "",
                    DP_COLOR_GET_UFP_ENABLE(pJob->config.dstFormat[index]) ? "u" : "", pJob->config.dstProfile[index],
                    pJob->config.targetXStart[index], pJob->config.targetYStart[index],
                    pJob->config.rotation[index], pJob->config.flipStatus[index] ? 1 : 0,
                    tdshp[index], enDRE, pJob->config.ditherStatus[index] ? 1 : 0, pJob->config.dstSecure[index],
                    bufferInfoStr);
            }
        }

        m_pStream->addChannel(m_pChannel, &m_channelID);
    }

    // dequeue source buffer
    m_pSrcPool->dequeueBuffer(&pJob->config.srcBuffer, pBase, size);

    if (enableDumpBuffer)
    {
        char name[256] = "";

        sprintf(name, "Async_in_job%05u", pJob->jobID);
        m_pSrcPool->dumpBuffer(pJob->config.srcBuffer,
                               pJob->config.srcFormat,
                               pJob->config.srcWidth,
                               pJob->config.srcHeight,
                               pJob->config.srcYPitch,
                               pJob->config.srcUVPitch,
                               name);
    }

    // queue and trigger the source buffer
    m_pSrcPool->queueBuffer(pJob->config.srcBuffer);

    DPLOGI("DpAsyncBlitStream::start stream\n");

    if (false == pJob->config.frameChange)
    {
        DPLOGI("DpAsyncBlit: config frame only!\n");
        m_pStream->setConfigFlag(DpStream::CONFIG_FRAME_ONLY);
    }
    else
    {
        DPLOGI("DpAsyncBlit: config all!\n");
        m_pStream->setConfigFlag(DpStream::CONFIG_ALL);
    }

    m_prevConfig = pJob->config;

    status = m_pStream->startStream(pJob->config.frameChange);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpAsyncBlitStream::start stream failed: %d\n", status);
        m_pStream->stopStream();
        pJob->config.frameChange = true;
        return status;
    }

    //PQ DC or HDR readback
    m_regMutex->lock();
    uint32_t bufferSize;
    uint32_t numReadbackEngine;
    bufferSize = m_pStream->getNumReadbackPABuffer(numReadbackEngine);
    m_PABufferSizeList.push_back(bufferSize);
    m_numReadbackEngineList.push_back(numReadbackEngine);
    m_regMutex->unlock();

    status = waitSubmit();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpAsyncBlitStream: waitSubmit is failed in asynchronize mode (%d)\n", status);
        pJob->config.frameChange = true;
        return status;
    }

    status = m_pStream->stopStream();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpAsyncBlitStream: stopStream is failed in asynchronize mode (%d)\n", status);
        pJob->config.frameChange = true;
        return status;
    }

    m_pSrcPool->activateBuffer();

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        if (!pJob->config.dstEnabled[index])
            continue;

        m_pDstPool[index]->activateBuffer();
    }

    DPLOGI("DpAsyncBlitStream::return with success status\n");

    DP_TIMER_GET_CURRENT_TIME(end);

    DP_TIMER_GET_DURATION_IN_MS(begin,
                                end,
                                diff);

    if (pJob->config.frameChange || enableLog)
    {
        if (diff >= 33)
        {
            DPLOGW("DpAsyncBlit: time %d ms, type %d, pq %d:%d:%d:%d\n", diff,
                   pJob->config.frameChange, pJob->config.PqConfig[0].enColor,
                   pJob->config.PqConfig[1].enColor, pJob->config.PqConfig[2].enColor,
                   pJob->config.PqConfig[3].enColor);
        }
        else
        {
            DPLOGI("DpAsyncBlit: time %d ms, type %d, pq %d:%d:%d:%d\n", diff,
                   pJob->config.frameChange, pJob->config.PqConfig[0].enColor,
                   pJob->config.PqConfig[1].enColor, pJob->config.PqConfig[2].enColor,
                   pJob->config.PqConfig[3].enColor);
        }
    }
    else
    {
        DPLOGI("DpAsyncBlit: time %d ms, type %d, pq %d:%d:%d:%d\n", diff,
               pJob->config.frameChange, pJob->config.PqConfig[0].enColor,
               pJob->config.PqConfig[1].enColor, pJob->config.PqConfig[2].enColor,
               pJob->config.PqConfig[3].enColor);
    }

    m_pJobCond->signal();

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::waitSubmit()
{
    DP_STATUS_ENUM status;
    DpJobID        jobID;
    AsyncBlitJob   *pJob;

    status = m_pStream->waitStream();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpAsyncBlitStream: wait submit stream failed(%d)\n", status);
        m_pStream->dumpDebugStream();
        return status;
    }
    AutoMutex lock(m_pJobMutex);

    pJob = m_jobList.front();
    jobID = m_pStream->getAsyncJob(1, pJob->dumpFrameInfo);
    pJob->cmdTaskID = jobID;
    m_waitJobList.push_back(pJob);
    m_jobList.erase(m_jobList.begin());

    if (m_waitJobList.size() > 5)
    {
        DPLOGE("DpAsyncBlitStream: job list size is more than 5 (%u)\n", m_waitJobList.size());
    }

    DPLOGI("DpAsyncBlitStream: submit stream done %llx\n", jobID);

    return DP_STATUS_RETURN_SUCCESS;
}


void* DpAsyncBlitStream::waitComplete(void* para)
{
    DP_STATUS_ENUM status;
    DpReadbackRegs readBackRegs;
    AsyncBlitJob   *pJob;
    void           *pBase[3];
    uint32_t       size[3];
    DpTimeValue    begin;
    DpTimeValue    end;
    int32_t        diff;
    DpAsyncBlitStream *stream;
    int32_t        enableLog = DpDriver::getInstance()->getEnableLog();
    int32_t        enableDumpBuffer = DpDriver::getInstance()->getEnableDumpBuffer();
    uint32_t       index;
    uint32_t       engineIndex;
    bool           bAcquireDstBufferFail = false;
    uint32_t       tdshp[ISP_MAX_OUTPUT_PORT_NUM] = {0};
    ReadbackEngineList readbackEngineList;

    DPLOGI("DpAsyncBlitStream: Wait complete in asynchronized mode\n");

    if (para != NULL)
    {
        stream = (DpAsyncBlitStream*)para;
    }
    else
    {
        DPLOGE("DpAsyncBlitStream: cannot create thread\n");
        return NULL;
    }

    memset(&readBackRegs, 0, sizeof(readBackRegs));

    stream->m_pJobMutex->lock();

    while (1)
    {
        if (!stream->m_waitJobList.empty())
        {
            readbackEngineList.clear();
            stream->m_pStream->getReadbackEngineStatus(readbackEngineList);

            pJob = stream->m_waitJobList.front();
            stream->m_pJobMutex->unlock();

            do
            {
                DP_TIMER_GET_CURRENT_TIME(begin);

                //PQ DC or HDR readback
                stream->m_regMutex->lock();
                if(stream->m_PABufferStartIndexList.empty())
                {
                    stream->m_regMutex->unlock();
                    readBackRegs.m_engineFlag = 0;
                }
                else
                {
                    if (!stream->m_PABufferSizeList.empty())
                    {
                        readBackRegs.m_num = stream->m_PABufferSizeList.front();

                        if (readBackRegs.m_num > MAX_NUM_READBACK_REGS)
                        {
                            DPLOGE("DpAsyncBlitStream: wait complete stream readbeck fail\n");
                        }
                        else
                        {
                            if (!stream->m_numReadbackEngineList.empty())
                            {
                                for (engineIndex = 0; engineIndex < stream->m_numReadbackEngineList.front(); engineIndex++)
                                {
                                    switch (readbackEngineList[engineIndex].engineID)
                                    {
                                        case tTDSHP0:
                                            readBackRegs.m_engineFlag = (1 << tTDSHP0);
                                            break;
#ifdef SUPPORT_HDR
#ifdef HDR_MT6799
                                        case tRDMA0:
                                            readBackRegs.m_engineFlag = (1 << tRDMA0);
                                            break;
#else
                                        case tHDR0:
                                            readBackRegs.m_engineFlag = (1 << tHDR0);
                                            break;
#endif // HDR_MT6799
#endif // SUPPORT_HDR
#ifdef SUPPORT_VIDEO_DRE
                                        case tAAL0:
                                            readBackRegs.m_engineFlag = (1 << tAAL0);
                                            break;
#endif // SUPPORT_VIDEO_DRE
                                        default:
                                            DPLOGE("DpAsyncBlitStream::waitComplete: cannot support this engine to readback %d\n", readbackEngineList[engineIndex].engineID);
                                    }
                                }
                            }

                            for (index = 0; index < readBackRegs.m_num; index++)
                            {
                                readBackRegs.m_regs[index] = stream->m_PABuffer[(stream->m_PABufferStartIndexList.front() + index) & (MAX_NUM_READBACK_PA_BUFFER - 1)];
                            }
                        }
                    }
                    else
                    {
                        DPLOGE("DpAsyncBlitStream::waitComplete: m_PABufferStartIndexList and m_PABufferSizeList not match\n");
                    }
                    stream->m_regMutex->unlock();
                }

                status = DpDriver::getInstance()->waitFramedone(pJob->cmdTaskID, readBackRegs);
                if (DP_STATUS_RETURN_SUCCESS != status)
                {
                    DPLOGE("DpAsyncBlitStream: wait complete stream failed(%d)\n", status);
                    DPLOGE("DpAsyncBlitStream: %s", pJob->dumpFrameInfo);
                    stream->m_pStream->dumpDebugStream();
                    pJob->config.frameChange = true;
                }

                // TODO: readback PQ in asynchronous mode
                if (STREAM_DUAL_BITBLT == stream->m_pStream->getScenario())
                {
                    DPLOGD("DpAsyncBlitStream::dual pipe setHistogram\n");
                    status = stream->m_pStream->setHistogram();
                }

                // release source buffer to reset input buffer state
                status = stream->m_pSrcPool->asyncReleaseBuffer(pJob->config.srcBuffer);
                DPLOGI("DpAsyncBlitStream: srcBuffer release %d status: %d\n", pJob->config.srcBuffer, status);

                for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
                {
                    if (!pJob->config.dstEnabled[index])
                        continue;

                    if (true == pJob->config.dstFlush[index])
                    {
                        status = stream->m_pDstPool[index]->flushWriteBuffer(pJob->config.dstBuffer[index]);
                        DPLOGI("DpAsyncBlitStream: flushWriteBuffer %d status: %d\n", pJob->config.dstBuffer[index], status);
                    }

                    // acquire and release destination buffer, this is to reset output buffer state
                    status = stream->m_pDstPool[index]->queueBuffer(pJob->config.dstBuffer[index]);
                    DPLOGI("DpAsyncBlitStream: queueBuffer %d status: %d\n", pJob->config.dstBuffer[index], status);

                    status = stream->m_pDstPool[index]->acquireBuffer(&pJob->config.dstBuffer[index], pBase, size);
                    if (DP_STATUS_RETURN_SUCCESS != status)
                    {
                        DPLOGE("DpAsyncBlitStream: acquire dst buffer failed(%d)\n", status);
                        bAcquireDstBufferFail = true;
                        continue;
                    }

                    tdshp[index] = (pJob->config.PqConfig[index].enDC || pJob->config.PqConfig[index].enSharp || \
                        pJob->config.PqConfig[index].enColor);
                    DPLOGI("DpAsyncBlit: waitComplete port%d: (%d, %d, %d, %d, C%d%s%s%s%s, P%d), misc: (X:%d, Y:%d, R:%d, F:%d, S:%d, D:%d), sec%d\n",
                           index, pJob->config.dstWidth[index], pJob->config.dstHeight[index],
                           pJob->config.dstYPitch[index], pJob->config.dstUVPitch[index],
                           DP_COLOR_GET_UNIQUE_ID(pJob->config.dstFormat[index]),
                           DP_COLOR_GET_SWAP_ENABLE(pJob->config.dstFormat[index]) ? "s" : "",
                           DP_COLOR_GET_BLOCK_MODE(pJob->config.dstFormat[index]) ? "b" : "",
                           DP_COLOR_GET_INTERLACED_MODE(pJob->config.dstFormat[index]) ? "i" : "",
                           DP_COLOR_GET_UFP_ENABLE(pJob->config.dstFormat[index]) ? "u" : "", pJob->config.dstProfile[index],
                           pJob->config.targetXStart[index], pJob->config.targetYStart[index],
                           pJob->config.rotation[index], pJob->config.flipStatus[index] ? 1 : 0,
                           tdshp[index], pJob->config.ditherStatus[index] ? 1 : 0, pJob->config.dstSecure[index]);

                    if (enableDumpBuffer)
                    {
                        char name[256] = "";

                        sprintf(name, "Async_out_job%05u", pJob->jobID);
                        status = stream->m_pDstPool[index]->dumpBuffer(pJob->config.dstBuffer[index],
                                                                       pJob->config.dstFormat[index],
                                                                       pJob->config.dstWidth[index],
                                                                       pJob->config.dstHeight[index],
                                                                       pJob->config.dstYPitch[index],
                                                                       pJob->config.dstUVPitch[index],
                                                                       name);
                    }

                    status = stream->m_pDstPool[index]->asyncReleaseBuffer(pJob->config.dstBuffer[index]);
                    DPLOGI("DpAsyncBlitStream: dstBuffer release %d status: %d \n", pJob->config.dstBuffer[index], status);

                    stream->m_pDstPool[index]->unregisterBuffer(pJob->config.dstBuffer[index]);

                    uint32_t DCValues[MAX_NUM_READBACK_REGS] = {0};
                    uint32_t DCNum = 0;
                    uint32_t HDRValues[MAX_NUM_READBACK_REGS] = {0};
                    uint32_t HDRNum = 0;
                    uint32_t DREValues[MAX_NUM_READBACK_REGS] = {0};
                    uint32_t DRENum = 0;
                    uint32_t eachReadbackIndex = 0;
                    uint32_t readbackRegsIndex = 0;
#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
                    stream->m_regMutex->lock();
                    if (!stream->m_numReadbackEngineList.empty())
                    {
                        for (engineIndex = 0; engineIndex < stream->m_numReadbackEngineList.front(); engineIndex++)
                        {
                            switch (readbackEngineList[engineIndex].engineID)
                            {
                                case tTDSHP0:
                                    DCNum = readbackEngineList[engineIndex].PABufferEnd - readbackEngineList[engineIndex].PABufferStart + 1;
                                    for (eachReadbackIndex = 0; eachReadbackIndex < DCNum; eachReadbackIndex++)
                                    {
                                        if (readbackRegsIndex >= MAX_NUM_READBACK_REGS)
                                        {
                                            DPLOGE("DpAsyncBlitStream: tTDSHP0 readbackRegsIndex >= MAX_NUM_READBACK_REGS\n");
                                        }
                                        else
                                        {
                                            DCValues[eachReadbackIndex] = readBackRegs.m_values[readbackRegsIndex];
                                            readbackRegsIndex++;
                                        }
                                    }
                                    break;
#ifdef SUPPORT_HDR
#ifdef HDR_MT6799
                                case tRDMA0:
#else
                                case tHDR0:
#endif // HDR_MT6799
                                    HDRNum = readbackEngineList[engineIndex].PABufferEnd - readbackEngineList[engineIndex].PABufferStart + 1;
                                    for (eachReadbackIndex = 0; eachReadbackIndex < HDRNum; eachReadbackIndex++)
                                    {
                                        if (readbackRegsIndex >= MAX_NUM_READBACK_REGS)
                                        {
                                            DPLOGE("DpAsyncBlitStream: tRDMA0/tHDR0 readbackRegsIndex >= MAX_NUM_READBACK_REGS\n");
                                        }
                                        else
                                        {
                                            HDRValues[eachReadbackIndex] = readBackRegs.m_values[readbackRegsIndex];
                                            readbackRegsIndex++;
                                        }
                                    }
                                    break;
#endif // SUPPORT_HDR
#ifdef SUPPORT_VIDEO_DRE
                                case tAAL0:
                                    DRENum = readbackEngineList[engineIndex].PABufferEnd - readbackEngineList[engineIndex].PABufferStart + 1;
                                    for (eachReadbackIndex = 0; eachReadbackIndex < DRENum; eachReadbackIndex++)
                                    {
                                        if (readbackRegsIndex >= MAX_NUM_READBACK_REGS)
                                        {
                                            DPLOGE("DpAsyncBlitStream: tAAL0 readbackRegsIndex >= MAX_NUM_READBACK_REGS\n");
                                        }
                                        else
                                        {
                                            DREValues[eachReadbackIndex] = readBackRegs.m_values[readbackRegsIndex];
                                            readbackRegsIndex++;
                                        }
                                    }
                                    break;
#endif // SUPPORT_VIDEO_DRE
                                default:
                                    DPLOGE("DpAsyncBlitStream::waitComplete: cannot support this engine to readback %d\n", readbackEngineList[engineIndex].engineID);
                            }
                        }
                    }
                    stream->m_regMutex->unlock();

                    if(pJob->config.PqConfig[index].enDC)
                    {
                        PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
                        uint32_t videoID = pPQSessionManager->findVideoID(pJob->config.PqParam[index].u.video.id);
                        uint64_t PQSessionID = (static_cast<uint64_t>(stream->m_PqID[index]) << 32) | videoID;

                        DPLOGI("DpAsyncBlitStream:waitComplete (pJob->config.PqID[index]) %llx \n", (stream->m_PqID[index]));
                        DPLOGI("DpAsyncBlitStream:waitComplete (pJob->config.PqParam[index].u.video.id) %llx \n", (pJob->config.PqParam[index].u.video.id));
                        DPLOGI("DpAsyncBlitStream:waitComplete PQSessionID %llx \n", PQSessionID);

                        PQSession* pPQSession = pPQSessionManager->getPQSession(PQSessionID);

                        if (pPQSession != NULL)
                        {
                            pPQSession->setHistogram(DCValues, DCNum);
                        }
                    }
#ifdef SUPPORT_HDR
                    if(pJob->config.PqConfig[index].enHDR)
                    {
                        PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
                        uint32_t videoID = pPQSessionManager->findVideoID(pJob->config.PqParam[index].u.video.id);
                        uint64_t PQSessionID = (static_cast<uint64_t>(stream->m_PqID[index]) << 32) | videoID;

                        DPLOGI("DpAsyncBlitStream:waitComplete (pJob->config.PqID[index]) %llx \n", (stream->m_PqID[index]));
                        DPLOGI("DpAsyncBlitStream:waitComplete (pJob->config.PqParam[index].u.video.id) %llx \n", (pJob->config.PqParam[index].u.video.id));
                        DPLOGI("DpAsyncBlitStream:waitComplete PQSessionID %llx \n", PQSessionID);

                        PQSession* pPQSession = pPQSessionManager->getPQSession(PQSessionID);

                        if (pPQSession != NULL)
                        {
                            pPQSession->setHDRRegInfo(HDRValues, HDRNum);
                        }
                    }
#endif

#ifdef SUPPORT_VIDEO_DRE
                    PQDREHistogramAdaptor *pPQDREHistogramAdaptor = PQReadBackFactory::getInstance()->getDRE(0);
                    PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
                    uint64_t PQSessionID;
                    uint32_t videoID;
                    int32_t prevWidth, prevHeight;

                    if (pJob->config.PqParam[index].scenario == MEDIA_VIDEO)
                    {
                        if (pPQDREHistogramAdaptor != NULL &&
                            pJob->config.PqConfig[index].enDRE != 0 &&
                            (pJob->config.PqParam[index].u.video.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM00 ||
                            pJob->config.PqParam[index].u.video.dpDREParam.SRAMId == DpDREParam::DRESRAM::SRAM01))
                        {
                            videoID = pPQSessionManager->findVideoID(pJob->config.PqParam[index].u.video.id);
                            PQSessionID = (static_cast<uint64_t>(stream->m_PqID[index]) << 32) | videoID;

                            PQSession* pPQSession = pPQSessionManager->getPQSession(PQSessionID);

                            if (pPQSession != NULL)
                            {
                                pPQSession->getDrePreviousSize(&prevWidth, &prevHeight);

                                pPQDREHistogramAdaptor->setHistogram(
                                    DREValues,
                                    DRENum,
                                    pJob->config.PqParam[index].u.video.dpDREParam.buffer,
                                    pJob->config.PqParam[index].u.video.dpDREParam.userId,
                                    prevWidth,
                                    prevHeight);
                            }
                        }
                    }
#endif // SUPPORT_VIDEO_DRE
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID
                }

                stream->m_regMutex->lock();
                if (!stream->m_PABufferStartIndexList.empty())
                {
                    stream->m_PABufferStartIndexList.erase(stream->m_PABufferStartIndexList.begin());
                }
                else
                {
                    DPLOGE("DpAsyncBlitStream::waitComplete m_PABufferStartIndexList empty\n");
                }

                if (!stream->m_PABufferSizeList.empty())
                {
                    stream->m_PABufferSizeList.erase(stream->m_PABufferSizeList.begin());
                }
                else
                {
                    DPLOGE("DpAsyncBlitStream::waitComplete m_PABufferSizeList empty\n");
                }

                if (!stream->m_numReadbackEngineList.empty())
                {
                    stream->m_pStream->cleanReadbackEngineStatus(stream->m_numReadbackEngineList.front());
                    stream->m_numReadbackEngineList.erase(stream->m_numReadbackEngineList.begin());
                }
                else
                {
                    DPLOGE("DpAsyncBlitStream::waitComplete m_numReadbackEngineList empty\n");
                }
                stream->m_regMutex->unlock();

                if (bAcquireDstBufferFail)
                {
                    break;
                }

                stream->m_pSrcPool->unregisterBuffer(pJob->config.srcBuffer);

                DPLOGI("DpAsyncBlitStream: Job ID : %llx is done!!!\n", pJob->cmdTaskID);

                DP_TIMER_GET_CURRENT_TIME(end);

                DP_TIMER_GET_DURATION_IN_MS(begin,
                                            end,
                                            diff);
                if (diff > 30)
                {
                    DPLOGW("DpAsyncBlitStream: %p, %d ms\n", stream, diff);
                }
                else if (enableLog)
                {
                    DPLOGI("DpAsyncBlitStream: %p, %d ms\n", stream, diff);
                }
            } while (0);

            stream->m_pJobMutex->lock();

            // wake up
            stream->m_pSync->wakeup((uint32_t)((int32_t)pJob->timeValue - (int32_t)stream->m_timeValue));
            stream->m_timeValue = pJob->timeValue;

            stream->m_waitJobList.erase(stream->m_waitJobList.begin());
            delete pJob;
        }
        else if (stream->m_abortJobs)
        {
            break;
        }
        else
        {
            stream->m_pJobCond->wait(*stream->m_pJobMutex);
        }
    }

    stream->m_waitJobList.clear();
    stream->m_pJobMutex->unlock();
    DPLOGI("DpAsyncBlitStream: wait thread end\n");

    return NULL;
}


DP_STATUS_ENUM DpAsyncBlitStream::setUser(uint32_t eID)
{
    DP_STATUS_ENUM status;

    switch (eID)
    {
    case DP_BLIT_GPU:
    case DP_BLIT_HWC2:
        status = m_pStream->setScenario(STREAM_GPU_BITBLT);
        break;
    case DP_BLIT_HWC_120FPS:
        status = m_pStream->setScenario(STREAM_DUAL_BITBLT);
        break;
    case DP_BLIT_ADDITIONAL_DISPLAY:
        status = m_pStream->setScenario(STREAM_2ND_BITBLT);
        break;
    default:
        DPLOGE("DpAsyncBlitStream: unrecognizable user %d\n", eID);
        status = DP_STATUS_INVALID_PARAX;
    }

    if (DP_STATUS_RETURN_SUCCESS == status)
    {
        m_userID = (DpBlitUser)eID;
    }
    return status;
}

#ifndef BASIC_PACKAGE
uint32_t DpAsyncBlitStream::getPqID()
{
    AutoMutex lock(s_PqCountMutex);
    s_PqCount = (s_PqCount+1) & 0xFFFFFFF;

    DPLOGI("DpAsyncBlitStream::s_PqCount %x\n", s_PqCount);

    return (s_PqCount | DP_ASYNCBLITSTREAM);
}
#endif // BASIC_PACKAGE

DP_STATUS_ENUM DpAsyncBlitStream::setPQParameter(int32_t portIndex, const DpPqParam &PqParam)
{
    if (portIndex >= ISP_MAX_OUTPUT_PORT_NUM)
    {
        DPLOGE("DpAsyncBlitStream: error argument - invalid output port index: %d\n", portIndex);
        return DP_STATUS_INVALID_PORT;
    }

#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {
        if (PqParam.scenario != MEDIA_VIDEO &&
            PqParam.scenario != MEDIA_PICTURE)
        {
            DPLOGE("DpAsyncBlitStream: setPQParameter scenario %d\n", PqParam.scenario);
            return DP_STATUS_INVALID_PARAX;
        }

        CONFIG_CURRENT_JOB(m_pCurJob, m_pJobMutex);
        m_pCurJob->config.PqParam[portIndex] = PqParam;
    }
#endif // BASIC_PACKAGE
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM DpAsyncBlitStream::detectFrameChange(BlitConfig &config)
{
    bool diff;
    diff = (memcmp(&m_prevConfig, &config, sizeof(BaseConfig)) != 0);
    config.frameChange = config.frameChange || diff;

    if (DpDriver::getInstance()->getDisableFrameChange())
    {
        config.frameChange = true;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream::configPQParameter(int32_t portIndex, BlitConfig &config)
{
#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {

#if CONFIG_FOR_OS_ANDROID
        PQSessionManager* pPQSessionManager = PQSessionManager::getInstance();
        uint32_t videoID = pPQSessionManager->findVideoID(config.PqParam[portIndex].u.video.id);
        uint64_t PQSessionID = (static_cast<uint64_t>(m_PqID[portIndex]) << 32) | videoID;


        PQSession* pPQsession = pPQSessionManager->createPQSession(PQSessionID);
#ifdef SUPPORT_VIDEO_DRE
        /*set dre off if secure video playback*/
        if(config.srcSecure == DP_SECURE && config.PqParam[portIndex].scenario == MEDIA_VIDEO)
        {
            pPQsession->setSVP(true);
            DPLOGD("DpAsyncBlitStream::set dre off when playing svp video\n");
        }
#endif
        pPQsession->setPQparam(&(config.PqParam[portIndex]));

        DpPqConfig* pDpPqConfig;
        pPQsession->getDpPqConfig(&pDpPqConfig);
        config.PqConfig[portIndex] = *pDpPqConfig;
#ifdef SUPPORT_VIDEO_DRE
        /* set dre setting for enDRE == 1 Video scenario*/
        if (config.PqParam[portIndex].scenario == MEDIA_VIDEO && config.PqConfig[portIndex].enDRE == 1)
        {
            DPLOGI("DpAsyncBlitStream: configPQParameter set VIDEO DRE setting\n");
            config.PqParam[portIndex].u.video.dpDREParam.SRAMId = DpDREParam::DRESRAM::SRAM00;
            config.PqParam[portIndex].u.video.dpDREParam.cmd = DpDREParam::Cmd::Default | DpDREParam::Cmd::Initialize;
            config.PqParam[portIndex].u.video.dpDREParam.userId = (((unsigned long long)MEDIA_VIDEO) << 32);
        }
#endif

        DPLOGI("DpAsyncBlitStream: pPQsession id %llx created\n", PQSessionID);
        DPLOGI("DpAsyncBlitStream: setPQParameter id %x enable %d scenario %d\n",
               m_PqID[portIndex], config.PqParam[portIndex].enable, config.PqParam[portIndex].scenario);
        DPLOGI("DpAsyncBlitStream: getPQConfig sharp %d DC %d color %d\n",
               config.PqConfig[portIndex].enSharp, config.PqConfig[portIndex].enDC, config.PqConfig[portIndex].enColor);
#endif // CONFIG_FOR_OS_ANDROID
    }
#endif // BASIC_PACKAGE
    return DP_STATUS_RETURN_SUCCESS;
}

int32_t DpAsyncBlitStream::queryPaddingSide(uint32_t transform)
{
/* padding side bit rule
*
* bit0: left
* bit1: top
* bit2: right
* bit3: bottom
*
*/
    int32_t padding = 0;
    uint32_t flip = 0;
    uint32_t rot  = 0;

    // operate on FLIP_H, FLIP_V and ROT_90 respectively
    // to achieve the final orientation
    if (FLIP_H & transform)
    {
        flip ^= 1;
    }

    if (FLIP_V & transform)
    {
        // FLIP_V is equivalent to a 180-degree rotation with a horizontal flip
        rot += 180;
        flip ^= 1;
    }

    if (ROT_90 & transform)
    {
        rot += 90;
    }

    rot %= 360;

    if (flip)
    {
        if (rot == 0)
            padding = 0b1001;
        else if (rot == 90)
            padding = 0b1100;
        else if (rot == 180)
            padding = 0b0110;
        else
            padding = 0b0011;
    }
    else
    {
        if (rot == 0)
            padding = 0b1100;
        else if (rot == 90)
            padding = 0b1001;
        else if (rot == 180)
            padding = 0b0011;
        else
            padding = 0b0110;
    }

    DPLOGI("DpAsyncBlitStream: rot(%d) flip(%d) padding(%x)\n", rot, flip, padding);

    return padding;
}

