#include "DpFragStream.h"
#include "DpStream.h"
#include "DpChannel.h"
#include "DpBufferPool.h"

#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
#include "PQSessionManager.h"
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID


static uint32_t s_PqCount = 0;
static DpMutex  s_PqCountMutex;

DpFragStream::DpFragStream()
    : m_pStream(new DpStream(STREAM_FRAG)),
      m_pChannel(new DpChannel()),
      m_channelID(-1),
      m_pSrcPool(new DpRingBufferPool()),
      m_pDstPool(new DpBasicBufferPool()),
      m_srcFormat(DP_COLOR_UNKNOWN),
      m_MCUYSize(0),
      m_MCUXSize(0),
      m_srcWidth(-1),
      m_srcHeight(-1),
      m_srcYPitch(-1),
      m_srcUVPitch(-1),
      m_dstBufID(-1),
      m_dstFormat(DP_COLOR_UNKNOWN),
      m_dstWidth(-1),
      m_dstHeight(-1),
      m_dstYPitch(-1),
      m_dstUVPitch(-1),
      m_MCU_col(-1),
      m_ditherStatus(false),
      m_embeddedJPEG(false),
      m_PqID(0),
      m_pqSupport(0)
{
    memset(&m_PqConfig, 0, sizeof(DpPqConfig));

#ifndef BASIC_PACKAGE
    m_pqSupport = DpDriver::getInstance()->getPQSupport();
#endif

#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {
        m_PqID = getPqID();
    }
#endif //BASIC_PACKAGE
}


DpFragStream::~DpFragStream()
{
    delete m_pDstPool;
    m_pDstPool = NULL;

    delete m_pSrcPool;
    m_pSrcPool = NULL;

    delete m_pStream;
    m_pStream = NULL;

    delete m_pChannel;
    m_pChannel = NULL;

#if CONFIG_FOR_OS_ANDROID
#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {
        PQSessionManager::getInstance()->destroyPQSession(m_PqID);
    }
#endif // BASIC_PACKAGE
#endif // CONFIG_FOR_OS_ANDROID
}


DP_STATUS_ENUM DpFragStream::setSrcConfig(DpColorFormat format,
                                          int32_t       width,
                                          int32_t       height,
                                          int32_t       MCUXSize,
                                          int32_t       MCUYSize,
                                          int32_t       pitch,
                                          DpRect        *pROI)
{
#if 0
    switch(format)
    {
        case eRGB888:
        case eYV12:
        case eNV12:
        case eNV21:
            m_MCUYSize = 16;
            m_MCUXSize = 16;
            break;
        case eYV16:
            m_MCUYSize = 8;
            m_MCUXSize = 16;
            break;
        case eYV24:
            m_MCUYSize = 8;
            m_MCUXSize = 8;
            break;
        default:
            return DP_STATUS_INVALID_FORMAT;
    }
#endif
    m_MCUXSize = MCUXSize;
    m_MCUYSize = MCUYSize;

    m_srcFormat  = format;
    m_srcWidth   = width;
    m_srcHeight  = height;
    m_srcYPitch  = pitch;

    if (1 < DP_COLOR_GET_PLANE_COUNT(format))
    {
        m_srcUVPitch = m_srcYPitch >> DP_COLOR_GET_H_SUBSAMPLE(format);
        if (DP_COLOR_IS_UV_COPLANE(format))
        {
            m_srcUVPitch *= 2;
        }
    }
    else
    {
        m_srcUVPitch = 0;
    }

    if (NULL != pROI)
    {
        m_cropInfo = *pROI;
    }
    else
    {
        m_cropInfo.x = 0;
        m_cropInfo.y = 0;
        m_cropInfo.w  = width;
        m_cropInfo.h = height;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpFragStream::setSrcConfig(DpColorFormat format,
                                          int32_t       width,
                                          int32_t       height,
                                          int32_t       MCUXSize,
                                          int32_t       MCUYSize,
                                          int32_t       YPitch,
                                          int32_t       UVPitch,
                                          DpRect        *pROI)
{
    m_MCUXSize = MCUXSize;
    m_MCUYSize = MCUYSize;

    m_srcFormat  = format;
    m_srcWidth   = width;
    m_srcHeight  = height;
    m_srcYPitch  = YPitch;
    m_srcUVPitch = UVPitch;

    if (NULL != pROI)
    {
        m_cropInfo = *pROI;
    }
    else
    {
        m_cropInfo.x = 0;
        m_cropInfo.y = 0;
        m_cropInfo.w  = width;
        m_cropInfo.h = height;
    }

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpFragStream::setDstBuffer(void     **pVAList,
                                          uint32_t *pSizeList,
                                          uint32_t planeNum)
{
    if (planeNum > 3)
    {
        DPLOGE("DpBlitStream: invalid target plance number: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

    if (-1 != m_dstBufID)
    {
        m_pDstPool->unregisterBuffer(m_dstBufID);
        m_dstBufID = -1;
    }

    return m_pDstPool->registerBuffer(pVAList,
                                      pSizeList,
                                      planeNum,
                                      -1,
                                      &m_dstBufID);
}

DP_STATUS_ENUM DpFragStream::setDstBuffer(int32_t  fileDesc,
                                          uint32_t *pSizeList,
                                          uint32_t planeNum)
{
    if (planeNum > 3)
    {
        DPLOGE("DpBlitStream: invalid target plance number: %d (should <= 3)\n", planeNum);
        return DP_STATUS_INVALID_PARAX;
    }

    if (-1 != m_dstBufID)
    {
        m_pDstPool->unregisterBuffer(m_dstBufID);
        m_dstBufID = -1;
    }

    return m_pDstPool->registerBufferFD(fileDesc,
                                        pSizeList,
                                        planeNum,
                                        -1,
                                        &m_dstBufID);
}

DP_STATUS_ENUM DpFragStream::setDstConfig(DpColorFormat format,
                                          int32_t       width,
                                          int32_t       height,
                                          int32_t       pitch)
{
    m_dstFormat  = format;
    m_dstWidth   = width;
    m_dstHeight  = height;
    m_dstYPitch  = pitch;

    if (1 < DP_COLOR_GET_PLANE_COUNT(format))
    {
        m_dstUVPitch = m_dstYPitch >> DP_COLOR_GET_H_SUBSAMPLE(format);
        if (DP_COLOR_IS_UV_COPLANE(format))
        {
            m_dstUVPitch *= 2;
        }
    }
    else
    {
        m_dstUVPitch = 0;
    }
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpFragStream::setDstConfig(DpColorFormat format,
                                          int32_t       width,
                                          int32_t       height,
                                          int32_t       YPitch,
                                          int32_t       UVPitch)
{
    m_dstFormat  = format;
    m_dstWidth   = width;
    m_dstHeight  = height;
    m_dstYPitch  = YPitch;
    m_dstUVPitch = UVPitch;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpFragStream::startFrag(uint32_t *pMCUYCount, bool bShrpEnabled, struct timeval *endTime)
{
    DP_STATUS_ENUM status;
    int32_t        enableLog = DpDriver::getInstance()->getEnableLog();
    char           bufferInfoStr[256] = "";

    if (m_PqConfig.enSharp)
    {
        bShrpEnabled = 1;
    }

    status = m_pChannel->setEndTime(endTime);

    if (enableLog)
    {
        memset(bufferInfoStr, '\0', sizeof(bufferInfoStr));
        m_pSrcPool->dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
        DPLOGD("DpFrag: in: (%d, %d, %d, %d, C%d%s%s%s%s), crop: (%d, %d, %d, %d) %s\n",
            m_srcWidth, m_srcHeight, m_srcYPitch, m_srcUVPitch,
            DP_COLOR_GET_UNIQUE_ID(m_srcFormat),
            DP_COLOR_GET_SWAP_ENABLE(m_srcFormat) ? "s" : "",
            DP_COLOR_GET_BLOCK_MODE(m_srcFormat) ? "b" : "",
            DP_COLOR_GET_INTERLACED_MODE(m_srcFormat) ? "i" : "",
            DP_COLOR_GET_UFP_ENABLE(m_srcFormat) ? "u" : "",
            m_cropInfo.x, m_cropInfo.y, m_cropInfo.w, m_cropInfo.h,
            bufferInfoStr);

        memset(bufferInfoStr, '\0', sizeof(bufferInfoStr));
        m_pDstPool->dumpBufferInfo(bufferInfoStr, sizeof(bufferInfoStr));
        DPLOGD("DpFrag: out: (%d, %d, %d, %d, C%d%s), misc: (MX:%d, MY:%d, MC:%d, S:%d, D:%d) %s\n",
            m_dstWidth, m_dstHeight, m_dstYPitch, m_dstUVPitch,
            DP_COLOR_GET_UNIQUE_ID(m_dstFormat),
            DP_COLOR_GET_SWAP_ENABLE(m_dstFormat) ? "s" : "",
            m_MCUXSize, m_MCUYSize, m_MCU_col, bShrpEnabled ? 1 : 0, m_ditherStatus ? 1 : 0,
            bufferInfoStr);
    }
    status = m_pSrcPool->createBuffer(m_MCUYSize,
                                      m_srcFormat,
                                      m_srcWidth,
                                      m_srcHeight,
                                      m_srcYPitch,
                                      m_srcUVPitch,
                                      m_dstWidth,
                                      m_dstHeight,
                                      m_MCU_col,
                                      &m_cropInfo);
    *pMCUYCount = m_pSrcPool->getRingBufferMCUCount();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    //Add for embedded JPEG decoder
    m_pSrcPool->setEmbeddedJPEG(m_embeddedJPEG);
    if (m_embeddedJPEG)
    {
        DPLOGD("DpFrag: Embedded JPEG mode is enabled\n");
        m_pStream->setScenario(STREAM_FRAG_JPEGDEC);
    }

    status = m_pChannel->setSourcePort(PORT_MEMORY,
                                       m_srcFormat,
                                       m_srcWidth,
                                       m_srcHeight,
                                       m_srcYPitch,
                                       m_srcUVPitch,
                                       false, // No HDR in Frag
                                       false, // No DRE in Frag
                                       m_pSrcPool,
                                       DP_PROFILE_JPEG);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    m_pSrcPool->activateBuffer();
#ifndef BASIC_PACKAGE
                        uint64_t PQSessionID = ((static_cast<uint64_t>(m_PqID) << 32) | FRAG_VIDEO_ID);
#else
                        uint64_t PQSessionID = (static_cast<uint64_t>(m_PqID) << 32);
#endif // BASIC_PACKAGE
    status = m_pChannel->addTargetPort(0,
                                       PORT_MEMORY,
                                       m_dstFormat,
                                       m_dstWidth,
                                       m_dstHeight,
                                       m_dstYPitch,
                                       m_dstUVPitch,
                                       0,      // No rotation
                                       false,  // No flip
                                       PQSessionID,
                                       (bShrpEnabled)?1:0,
                                       m_ditherStatus,
                                       m_pDstPool,
                                       DP_PROFILE_JPEG);

    m_pDstPool->activateBuffer();

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    status = m_pChannel->setSourceCrop(0,
                                       m_cropInfo.x,
                                       m_cropInfo.y,
                                       m_cropInfo.w,
                                       m_cropInfo.h);

    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    status = m_pStream->addChannel(m_pChannel, &m_channelID);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

#if 0
    if( (0 != pMCUXSize) && ( 0 != pMCUYSize))
    {
        *pMCUXSize = m_MCUXSize;
        *pMCUYSize = m_MCUYSize;
    }
    else
    {
        return DP_STATUS_INVALID_PARAX;
    }
#endif

    status = m_pStream->startStream();

    return status;
}


DP_STATUS_ENUM DpFragStream::dequeueFrag(int32_t       *pBufID,
                                         DpColorFormat *pFormat,
                                         void          **pBase,
                                         int32_t       *pMCUXStart,
                                         int32_t       *pMCUYStart,
                                         int32_t       *pWidth,
                                         int32_t       *pHeight,
                                         int32_t       *pPitch,
                                         bool          waitBuf)
{
    DP_STATUS_ENUM status;
    int32_t XStart,YStart;

    if ( (0 == pMCUXStart) || (0 == pMCUYStart) )
    {
        return DP_STATUS_INVALID_PARAX;
    }

    status = m_pSrcPool->dequeueBuffer(pBufID,
                                       pFormat,
                                       pBase,
                                       &XStart,
                                       &YStart,
                                       pWidth,
                                       pHeight,
                                       pPitch,
                                       waitBuf);

    if(DP_STATUS_RETURN_SUCCESS == status)
    {
        *pMCUXStart = XStart / m_MCUXSize;
        *pMCUYStart = YStart / m_MCUYSize;
    }

    return status;
}

DP_STATUS_ENUM DpFragStream::dequeueFrag(int32_t       *pBufID,
                                         DpColorFormat *pFormat,
                                         void          **pBase,
                                         int32_t       *pFileDesc,
                                         int32_t       *pMCUXStart,
                                         int32_t       *pMCUYStart,
                                         int32_t       *pWidth,
                                         int32_t       *pHeight,
                                         int32_t       *pPitch,
                                         bool          waitBuf)
{
    DP_STATUS_ENUM status;
    int32_t XStart,YStart;

    if ( (0 == pMCUXStart) || (0 == pMCUYStart) )
    {
        return DP_STATUS_INVALID_PARAX;
    }

    status = m_pSrcPool->dequeueBuffer(pBufID,
                                       pFormat,
                                       pBase,
                                       &XStart,
                                       &YStart,
                                       pWidth,
                                       pHeight,
                                       pPitch,
                                       waitBuf);

    if(DP_STATUS_RETURN_SUCCESS == status)
    {
        *pMCUXStart = XStart / m_MCUXSize;
        *pMCUYStart = YStart / m_MCUYSize;
    }

    *pFileDesc = m_pSrcPool->getBufferFD();

    return status;
}



DP_STATUS_ENUM DpFragStream::queueFrag(int32_t bufID)
{
    return m_pSrcPool->queueBuffer(bufID);
}


DP_STATUS_ENUM DpFragStream::stopFrag()
{
    DP_STATUS_ENUM status;

    DPLOGI("DpFragStream: stop fragment begin\n");
    status = m_pStream->stopStream();
    DPLOGI("DpFragStream: stop fragment end\n");
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        DPLOGE("DpFragStream::stopStream() : %d",status);
    }

    return status;
}

#ifndef BASIC_PACKAGE
uint32_t DpFragStream::getPqID()
{
    AutoMutex lock(s_PqCountMutex);
    s_PqCount = (s_PqCount+1) & 0xFFFFFFF;

    DPLOGI("DpFragStream::s_PqCount %x\n", s_PqCount);

    return (s_PqCount | DP_FRAGSTREAM);
}
#endif // BASIC_PACKAGE

DP_STATUS_ENUM DpFragStream::setPQParameter(const DpPqParam &pqParam)
{
#ifndef BASIC_PACKAGE
    if (0 != m_pqSupport) {
        if ((pqParam.scenario != MEDIA_ISP_PREVIEW)
            && (pqParam.scenario != MEDIA_PICTURE))
        {
            DPLOGE("DpFragStream: setPQParameter scenario %d\n", pqParam.scenario);
            return DP_STATUS_INVALID_PARAX;
        }

#if CONFIG_FOR_OS_ANDROID
        uint64_t PQSessionID = ((static_cast<uint64_t>(m_PqID) << 32) | FRAG_VIDEO_ID);

        PQSession* pPQsession = PQSessionManager::getInstance()->createPQSession(PQSessionID);
        pPQsession->setPQparam(&pqParam);

        DpPqConfig* pDpPqConfig;
        pPQsession->getDpPqConfig(&pDpPqConfig);
        m_PqConfig = *pDpPqConfig;

        DPLOGI("DpFragStream: pPQsession id %llx created\n", PQSessionID);
        DPLOGI("DpFragStream: setPQParameter id %x enable %d scenario %d\n", m_PqID, pqParam.enable, pqParam.scenario);
        DPLOGI("DpFragStream: getPQConfig sharp %d DC %d color %d\n", m_PqConfig.enSharp, m_PqConfig.enDC, m_PqConfig.enColor);
#endif // CONFIG_FOR_OS_ANDROID
    }
#endif // BASIC_PACKAGE
    return DP_STATUS_RETURN_SUCCESS;
}
